#define LOG_TAG "Fan3D"
#include "Fan.h"
#include "ShaderHelper.h"

#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <cstdint>
#include <algorithm>

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

    constexpr float kShades[8] = {
        1.0f, 1.0f, 1.0f, 1.0f,
        0.6f, 0.6f, 0.6f, 0.6f
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
    uColorLoc = glGetUniformLocation(program, "PARTCOLOR");

    const GLsizeiptr posSize = sizeof(kPositions);
    const GLsizeiptr shadeSize = sizeof(kShades);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, posSize + shadeSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, posSize, kPositions);
    glBufferSubData(GL_ARRAY_BUFFER, posSize, shadeSize, kShades);

    // layout vec3 position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0,
        reinterpret_cast<void*>(0));

    // layout float shade
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0,
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
    glUniformMatrix4fv(uMVPLoc, 1, GL_FALSE, glm::value_ptr(*mvp));
    glUniform3fv(uColorLoc, 1, glm::value_ptr(color));
    glDrawElements(GL_TRIANGLES, kIndexCount, GL_UNSIGNED_SHORT, nullptr);
}

void Fan::Render()
{
    if (!program) return;

    if (fanOn) spinAngle += kBaseSpeed + dragBoost;
    if (spinAngle >= 360.0f) spinAngle -= 360.0f;

    dragBoost *= 0.98f; //turning speed will slowly go down (bonus?
    if (dragBoost < 0.001f) dragBoost = 0.0f;

    glUseProgram(program);
    glBindVertexArray(vao);

    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();

    transform.TransformTranslate(0.0f, 0.8f, -8.0f);
    transform.TransformRotate(glm::radians(20.0f), 0.0f, 1.0f, 0.0f);

    ///Base
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, -2.6f, 0.0f);
    transform.TransformScale(1.6f, 0.25f, 0.8f);
    drawPart(kBaseColor);
    transform.TransformPopMatrix();

    ///Pole
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, -1.21f, 0.0f);
    transform.TransformScale(0.15f, 2.53f, 0.15f);
    drawPart(kPoleColor);
    transform.TransformPopMatrix();

    ///Hub
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, 0.2f, 0.0f);
    transform.TransformScale(0.3f, 0.3f, 0.3f);
    drawPart(kHubColor);
    transform.TransformPopMatrix();

    ///Blades
    for (int i = 0; i < 4; ++i)
    {
        transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, 0.2f, 0.15f);
        transform.TransformRotate(glm::radians(spinAngle + i * 90.0f), 0.0f, 0.0f, 1.0f);
        transform.TransformTranslate(0.0f, 0.55f, 0.0f);
        transform.TransformScale(0.22f, 0.8f, 0.05f);
        drawPart(kBladeColors[i]);
        transform.TransformPopMatrix();
    }

    glBindVertexArray(0);
}

void Fan::TouchEventDown(float x, float y)
{
    lastX = x;
    lastY = y;
    movedDistance = 0.0f;
    lastMoveTime = std::chrono::steady_clock::now();
}

void Fan::TouchEventMove(float x, float y)
{
    const float dx = x - lastX;
    const float dy = y - lastY;
    const float dist = std::sqrt(dx * dx + dy * dy);
    movedDistance += dist;

    const auto  now = std::chrono::steady_clock::now();
    const float dtMs = std::chrono::duration<float, std::milli>(now - lastMoveTime).count();
    if (dtMs > 0.0f)
    {
        const float velocity = dist / dtMs;
        dragBoost = std::min(velocity * kBoostScale, kMaxBoost);
    }

    lastX = x;
    lastY = y;
    lastMoveTime = now;
}

void Fan::TouchEventRelease(float x, float y)
{
    const float dx = x - lastX;
    const float dy = y - lastY;
    movedDistance += std::sqrt(dx * dx + dy * dy);

    if (movedDistance < kTapThreshold)
    {
        fanOn = !fanOn;
        LOGI("Fan %s", fanOn ? "ON" : "OFF");
    }
}