// Author: Davina Chin XiangLing
// File Name: Ground.cpp
// Date: 5.8.2026
// Description: Implementation of Ground

#define LOG_TAG "Ground"
#include "Ground.h"
#include "ShaderHelper.h"

#include <glm/gtc/type_ptr.hpp>

namespace
{
    constexpr float kHalfSize = 20.0f;

    constexpr float kPositions[4 * 3] = {
        -kHalfSize, 0.0f, -kHalfSize, // back left
         kHalfSize, 0.0f, -kHalfSize, // back right
         kHalfSize, 0.0f,  kHalfSize, // front right
        -kHalfSize, 0.0f,  kHalfSize  // front left
    };

    constexpr float kNormals[4 * 3] = {
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };

    constexpr unsigned short kIndices[6] = {
        0, 2, 1,
        0, 3, 2
    };
}

Ground::~Ground()
{
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (ibo) glDeleteBuffers(1, &ibo);
    if (program) glDeleteProgram(program);
}

void Ground::InitModel()
{
    transform.TransformInit();

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(
        assetMgr, "shader/GroundVertex.glsl", "shader/GroundFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile(
        "GroundVertex.glsl", "GroundFragment.glsl");
#endif
    if (!program)
    {
        LOGE("Ground::InitModel - shader program failed to build");
        return;
    }

    uMVPLoc = glGetUniformLocation(program, "MODELVIEWPROJECTIONMATRIX");
    uMVLoc = glGetUniformLocation(program, "MODELVIEWMATRIX");
    uNormalMatLoc = glGetUniformLocation(program, "NORMALMATRIX");
    uLightPosLoc = glGetUniformLocation(program, "LIGHTPOS");
    uTileSizeLoc = glGetUniformLocation(program, "TILESIZE");
    uColorALoc = glGetUniformLocation(program, "COLORA");
    uColorBLoc = glGetUniformLocation(program, "COLORB");

    const GLsizeiptr posSize = sizeof(kPositions);
    const GLsizeiptr normalSize = sizeof(kNormals);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, posSize + normalSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, posSize, kPositions);
    glBufferSubData(GL_ARRAY_BUFFER, posSize, normalSize, kNormals);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0,
        reinterpret_cast<void*>(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0,
        reinterpret_cast<void*>(static_cast<intptr_t>(posSize)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices, GL_STATIC_DRAW);

    glBindVertexArray(0);

    LOGI("Ground::InitModel - program=%u vao=%u", program, vao);
}

void Ground::Resize(int w, int h)
{
    if (h <= 0) h = 1;
    const float aspect = static_cast<float>(w) / static_cast<float>(h);

    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformLoadIdentity();
    transform.TransformSetPerspective(glm::radians(60.0f), aspect, 0.01f, 1000.0f, 0.0f);
}

void Ground::Render()
{
    if (!program) return;

    glUseProgram(program);
    glBindVertexArray(vao);

    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();

    // same baked-in camera transform Fan applies
    transform.TransformTranslate(0.0f, 0.8f, -cameraDistance);
    transform.TransformRotate(glm::radians(20.0f), 0.0f, 1.0f, 0.0f);

    // position floor below the fan's base
    transform.TransformTranslate(0.0f, -3.0f, 0.0f);

    glm::mat4* mvp = transform.TransformGetModelViewProjectionMatrix();
    glm::mat4* mv = transform.TransformGetModelViewMatrix();

    glm::mat3 normalMatrix;
    transform.TransformGetNormalMatrix(&normalMatrix);

    glUniformMatrix4fv(uMVPLoc, 1, GL_FALSE, glm::value_ptr(*mvp));
    glUniformMatrix4fv(uMVLoc, 1, GL_FALSE, glm::value_ptr(*mv));
    glUniformMatrix3fv(uNormalMatLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    glUniform3fv(uLightPosLoc, 1, glm::value_ptr(lightPosition));
    glUniform1f(uTileSizeLoc, tileSize);
    glUniform3fv(uColorALoc, 1, glm::value_ptr(colorA));
    glUniform3fv(uColorBLoc, 1, glm::value_ptr(colorB));

    glDrawElements(GL_TRIANGLES, kIndexCount, GL_UNSIGNED_SHORT, nullptr);

    glBindVertexArray(0);
}