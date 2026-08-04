// Author: Davina Chin XiangLing
// File Name: Fan.cpp
// Date: 5.8.2026
// Description: Implementation of Fan

#define LOG_TAG "Fan3D"
#include "Fan.h"
#include "ShaderHelper.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <limits>

namespace
{
    constexpr float kPositions[8 * 3] = {
        -0.5f, -0.5f, 0.5f, //front bottom left
        0.5f, -0.5f, 0.5f, //same but right
        0.5f, 0.5f, 0.5f, //front top right
        -0.5f, 0.5f, 0.5f, //same but left
        -0.5f, -0.5f, -0.5f, //back bottom left
        0.5f, -0.5f, -0.5f, //same but right
        0.5f, 0.5f, -0.5f, //back top right
        -0.5f, 0.5f, -0.5f //same but left
    };

    constexpr unsigned short kIndices[36] = {
         0, 1, 2, 0, 2, 3,
         1, 5, 6, 1, 6, 2,
         5, 4, 7, 5, 7, 6,
         4, 0, 3, 4, 3, 7,
         3, 2, 6, 3, 6, 7,
         4, 5, 1, 4, 1, 0
    };

    const glm::vec3 kBaseColor(0.45f, 0.28f, 0.12f);
    const glm::vec3 kPoleColor(0.55f, 0.55f, 0.58f);
    const glm::vec3 kHubColor(0.20f, 0.20f, 0.22f);
    const glm::vec3 kBladeColors[4] = {
        glm::vec3(0.85f, 0.10f, 0.10f), // red
        glm::vec3(0.10f, 0.35f, 0.85f), // blue
        glm::vec3(0.90f, 0.50f, 0.05f), // orange
        glm::vec3(0.15f, 0.70f, 0.20f) // green
    };
    const glm::vec3 kHighlightColor(1.0f, 0.75f, 0.0f); // amber
}

bool raySphereIntersect(const glm::vec3& O, const glm::vec3& D,
    const glm::vec3& C, float r, float& sOut)
{
    glm::vec3 oc = O - C;
    float b = glm::dot(oc, D);
    float c = glm::dot(oc, oc) - r * r;
    float disc = b * b - c;
    if (disc < 0.0f) return false;
    float sq = std::sqrt(disc);
    float t0 = -b - sq;
    float t1 = -b + sq;
    float t = (t0 >= 0.0f) ? t0 : t1;
    if (t < 0.0f) return false;
    sOut = t;
    return true;
}

void closestPtRaySegment(const glm::vec3& O, const glm::vec3& D,
    const glm::vec3& A, const glm::vec3& B,
    float& sOut, glm::vec3& closestOnRay, glm::vec3& closestOnSeg)
{
    glm::vec3 d2 = B - A;
    glm::vec3 r = O - A;
    float e = glm::dot(d2, d2);

    if (e <= 1e-8f)
    {
        float s = std::max(0.0f, glm::dot(D, A - O));
        sOut = s;
        closestOnRay = O + s * D;
        closestOnSeg = A;
        return;
    }

    float c = glm::dot(D, r);
    float b = glm::dot(D, d2);
    float f = glm::dot(d2, r);
    float denom = e - b * b;

    float s = (denom != 0.0f) ? std::max(0.0f, (b * f - c * e) / denom) : 0.0f;
    float t = (b * s + f) / e;

    if (t < 0.0f)
    {
        t = 0.0f;
        s = std::max(0.0f, -c);
    }
    else if (t > 1.0f)
    {
        t = 1.0f;
        s = std::max(0.0f, b - c);
    }

    sOut = s;
    closestOnRay = O + s * D;
    closestOnSeg = A + t * d2;
}

Fan::~Fan()
{
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (ibo) glDeleteBuffers(1, &ibo);
    if (program) glDeleteProgram(program);
}

void Fan::InitModel()
{
    transform.TransformInit();

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(
        assetMgr, "shader/FanVertex.glsl", "shader/FanFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile(
        "FanVertex.glsl", "FanFragment.glsl");
#endif
    if (!program)
    {
        LOGE("Fan::InitModel - shader program failed to build");
        return;
    }

    uMVPLoc = glGetUniformLocation(program, "MODELVIEWPROJECTIONMATRIX");
    uMVLoc = glGetUniformLocation(program, "MODELVIEWMATRIX");
    uNormalMatLoc = glGetUniformLocation(program, "NORMALMATRIX");
    uColorLoc = glGetUniformLocation(program, "PARTCOLOR");
    uLightPosLoc = glGetUniformLocation(program, "LIGHTPOS");

    float normals[8 * 3];
    for (int i = 0; i < 8; ++i)
    {
        glm::vec3 p(kPositions[i * 3 + 0], kPositions[i * 3 + 1], kPositions[i * 3 + 2]);
        glm::vec3 n = glm::normalize(p);
        normals[i * 3 + 0] = n.x;
        normals[i * 3 + 1] = n.y;
        normals[i * 3 + 2] = n.z;
    }

    const GLsizeiptr posSize = sizeof(kPositions);
    const GLsizeiptr normalSize = sizeof(normals);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, posSize + normalSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, posSize, kPositions);
    glBufferSubData(GL_ARRAY_BUFFER, posSize, normalSize, normals);

    // layout vec3 position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0,
        reinterpret_cast<void*>(0));

    // layout float shade
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0,
        reinterpret_cast<void*>(static_cast<intptr_t>(posSize)));

    // IBO binding captured by VAO cos its bound while VAO is bound
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices, GL_STATIC_DRAW);

    glBindVertexArray(0);

    LOGI("Fan::InitModel - program=%u vao=%u", program, vao);
}

void Fan::Resize(int w, int h)
{
    if (h <= 0) h = 1;
    const float aspect = static_cast<float>(w) / static_cast<float>(h);

    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformLoadIdentity();
    transform.TransformSetPerspective(glm::radians(60.0f), aspect, 0.01f, 1000.0f, 0.0f);
}

void Fan::drawPart(const glm::vec3& color)
{
    glm::mat4* mvp = transform.TransformGetModelViewProjectionMatrix();
    glm::mat4* mv = transform.TransformGetModelViewMatrix();

    glm::mat3 normalMatrix;
    transform.TransformGetNormalMatrix(&normalMatrix);

    glUniformMatrix4fv(uMVPLoc, 1, GL_FALSE, glm::value_ptr(*mvp));
    glUniformMatrix4fv(uMVLoc, 1, GL_FALSE, glm::value_ptr(*mv));
    glUniformMatrix3fv(uNormalMatLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    glUniform3fv(uColorLoc, 1, glm::value_ptr(color));
    glUniform3fv(uLightPosLoc, 1, glm::value_ptr(lightPosition));

    glDrawElements(GL_TRIANGLES, kIndexCount, GL_UNSIGNED_SHORT, nullptr);
}

void Fan::Render()
{
    if (!program) return;

    spinAngle += speed * kSpeedScale;
    if (spinAngle >= 360.0f)
        spinAngle -= 360.0f;

    glUseProgram(program);
    glBindVertexArray(vao);

    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();

    transform.TransformTranslate(0.0f, 0.8f, -cameraDistance);
    transform.TransformRotate(glm::radians(20.0f), 0.0f, 1.0f, 0.0f);

    ///Base
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, -2.6f, 0.0f);
    transform.TransformScale(1.6f, 0.25f, 0.8f);
    drawPart(selectedPart == kPartBase ? kHighlightColor : kBaseColor);
    transform.TransformPopMatrix();

    ///Pole
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, -1.21f, 0.0f);
    transform.TransformScale(0.15f, 2.53f, 0.15f);
    drawPart(selectedPart == kPartPole ? kHighlightColor : kPoleColor);
    transform.TransformPopMatrix();

    ///Hub
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, 0.2f, 0.0f);
    transform.TransformScale(0.3f, 0.3f, 0.3f);
    drawPart(selectedPart == kPartHub ? kHighlightColor : kHubColor);
    transform.TransformPopMatrix();

    ///Blades
    for (int i = 0; i < 4; ++i)
    {
        transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, 0.2f, 0.15f);
        transform.TransformRotate(glm::radians(spinAngle + i * 90.0f), 0.0f, 0.0f, 1.0f);
        transform.TransformTranslate(0.0f, 0.55f, 0.0f);
        transform.TransformScale(0.22f, 0.8f, 0.05f);
        drawPart(selectedPart == kPartBlade0 + i ? kHighlightColor : kBladeColors[i]);
        transform.TransformPopMatrix();
    }

    glBindVertexArray(0);
}

bool Fan::PickAt(float screenX, float screenY, int viewportW, int viewportH)
{
    if (viewportW <= 0 || viewportH <= 0) return false;

    // rebuild  same camera transform Render() applies before the per-part pushes
    glm::mat4 camMV(1.0f);
    camMV = glm::translate(camMV, glm::vec3(0.0f, 0.8f, -cameraDistance));
    camMV = glm::rotate(camMV, glm::radians(20.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 proj = glm::perspective(glm::radians(60.0f),
        static_cast<float>(viewportW) / static_cast<float>(viewportH), 0.01f, 1000.0f);

    int viewport[4] = { 0, 0, viewportW, viewportH };

    Transform unprojector;

    glm::vec3 nearP, farP;
    const float winY = static_cast<float>(viewportH) - screenY; // flip screen coords are top-left origin; Unproject expects bottom-left.

    glm::mat4 identity(1.0f);

    if (!unprojector.TransformUnproject(screenX, winY, 0.0f,
        &identity, &proj, viewport, &nearP.x, &nearP.y, &nearP.z))
        return false;
    if (!unprojector.TransformUnproject(screenX, winY, 1.0f,
        &identity, &proj, viewport, &farP.x, &farP.y, &farP.z))
        return false;

    const glm::vec3 rayOrigin = nearP;
    const glm::vec3 rayDir = glm::normalize(farP - nearP);

    LOGI("PickAt: click(%.1f,%.1f) vp(%d,%d) rayOrigin(%.2f,%.2f,%.2f) rayDir(%.2f,%.2f,%.2f)",
        screenX, screenY, viewportW, viewportH,
        rayOrigin.x, rayOrigin.y, rayOrigin.z,
        rayDir.x, rayDir.y, rayDir.z);

    int bestPart = -1;
    float bestS = std::numeric_limits<float>::max();

    // Base — compact box, sphere test is fine.
    {
        glm::vec3 center(camMV * glm::vec4(0.0f, -2.6f, 0.0f, 1.0f));
        float s;
        if (raySphereIntersect(rayOrigin, rayDir, center, 0.9f, s) && s < bestS)
        {
            bestS = s; bestPart = kPartBase;
        }
    }

    // compact box
    {
        glm::vec3 center(camMV * glm::vec4(0.0f, 0.2f, 0.0f, 1.0f));
        float s;
        if (raySphereIntersect(rayOrigin, rayDir, center, 0.3f, s) && s < bestS)
        {
            bestS = s; bestPart = kPartHub;
        }
    }

    // pole
    {
        glm::mat4 M = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.21f, 0.0f));
        M = glm::scale(M, glm::vec3(0.15f, 2.53f, 0.15f));
        glm::mat4 full = camMV * M;
        glm::vec3 a(full * glm::vec4(0.0f, -0.5f, 0.0f, 1.0f));
        glm::vec3 b(full * glm::vec4(0.0f, 0.5f, 0.0f, 1.0f));

        float s;
        glm::vec3 closestOnRay, closestOnSeg;
        closestPtRaySegment(rayOrigin, rayDir, a, b, s, closestOnRay, closestOnSeg);

        if (glm::length(closestOnRay - closestOnSeg) <= 0.15f && s < bestS)
        {
            bestS = s; bestPart = kPartPole;
        }
    }

    // blades
    for (int i = 0; i < 4; ++i)
    {
        glm::mat4 M(1.0f);
        M = glm::translate(M, glm::vec3(0.0f, 0.2f, 0.15f));
        M = glm::rotate(M, glm::radians(spinAngle + i * 90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        M = glm::translate(M, glm::vec3(0.0f, 0.55f, 0.0f));
        M = glm::scale(M, glm::vec3(0.22f, 0.8f, 0.05f));

        glm::mat4 full = camMV * M;
        glm::vec3 a(full * glm::vec4(0.0f, -0.5f, 0.0f, 1.0f));
        glm::vec3 b(full * glm::vec4(0.0f, 0.5f, 0.0f, 1.0f));

        float s;
        glm::vec3 closestOnRay, closestOnSeg;
        closestPtRaySegment(rayOrigin, rayDir, a, b, s, closestOnRay, closestOnSeg);

        if (glm::length(closestOnRay - closestOnSeg) <= 0.12f && s < bestS)
        {
            bestS = s; bestPart = kPartBlade0 + i;
        }
    }

    LOGI("PickAt: bestPart=%d bestS=%.3f", bestPart, bestS);

    if (bestPart < 0)
    {
        selectedPart = -1;
        return false;
    }

    // clicking  same part again clears the highlight
    selectedPart = (selectedPart == bestPart) ? -1 : bestPart;
    return true;
}