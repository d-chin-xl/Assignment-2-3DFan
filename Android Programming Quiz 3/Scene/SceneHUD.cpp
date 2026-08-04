// Author: Davina Chin XiangLing
// File Name: SceneHUD.cpp
// Date: 5.8.2026
// Description: Implementation of SceneHUD

#define LOG_TAG "SceneHUD"
#include "SceneHUD.h"
#include "ShaderHelper.h"
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>


SceneHUD::~SceneHUD()
{
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (program) glDeleteProgram(program);
}

void SceneHUD::InitModel()
{
    transform.TransformInit();

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(
            assetMgr, "shader/HUDVertex.glsl", "shader/HUDFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile(
        "HUDVertex.glsl", "HUDFragment.glsl");
#endif
    if (!program)
    {
        LOGE("SceneHUD::InitModel - shader program failed to build");
        return;
    }

    uMVPLoc = glGetUniformLocation(program, "MODELVIEWPROJECTIONMATRIX");

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(HUDVertex),
                          reinterpret_cast<void*>(offsetof(HUDVertex, x)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(HUDVertex),
                          reinterpret_cast<void*>(offsetof(HUDVertex, r)));

    glBindVertexArray(0);

    LOGI("SceneHUD::InitModel - program=%u vao=%u", program, vao);
}

void SceneHUD::appendQuad(std::vector<HUDVertex>& out,
                          float x, float y, float w, float h,
                          float r, float g, float b)
{
    out.push_back({ x,     y,     r, g, b });
    out.push_back({ x + w, y,     r, g, b });
    out.push_back({ x + w, y + h, r, g, b });

    out.push_back({ x,     y,     r, g, b });
    out.push_back({ x + w, y + h, r, g, b });
    out.push_back({ x,     y + h, r, g, b });
}

void SceneHUD::buildGeometry(int w, int h)
{
    vertices.clear();

    const float padding = 40.0f;
    const float btnW = 160.0f;
    const float btnH = 60.0f;
    const float btnGapY = 20.0f;

    plusSpdRect = { padding, padding, btnW, btnH };
    minusSpdRect = { padding, padding + btnH + btnGapY,   btnW, btnH };

    const float rightX = static_cast<float>(w) - padding - btnW;
    plusZoomRect = { rightX, padding, btnW, btnH };
    minusZoomRect = { rightX, padding + btnH + btnGapY, btnW, btnH };

    appendQuad(vertices, plusSpdRect.x, plusSpdRect.y, plusSpdRect.w, plusSpdRect.h, 0.20f, 0.70f, 0.25f);
    appendQuad(vertices, minusSpdRect.x, minusSpdRect.y, minusSpdRect.w, minusSpdRect.h, 0.75f, 0.20f, 0.20f);
    appendQuad(vertices, plusZoomRect.x, plusZoomRect.y, plusZoomRect.w, plusZoomRect.h, 0.20f, 0.40f, 0.80f);
    appendQuad(vertices, minusZoomRect.x, minusZoomRect.y, minusZoomRect.w, minusZoomRect.h, 0.85f, 0.55f, 0.10f);

    const float blockW = 10.0f;
    const float blockH = 20.0f;
    const float barTotalW = blockW * kBarBlockCount;
    const float barX = (static_cast<float>(w) - barTotalW) * 0.5f;
    const float barY = static_cast<float>(h) - padding - blockH;
    barOrigin = { barX, barY, blockW, blockH };

    barVertexStart = vertices.size();

    const int litBlocks = static_cast<int>(speed);
    for (int i = 0; i < kBarBlockCount; ++i)
    {
        const bool lit = (i < litBlocks);
        const float r = lit ? 0.20f : 0.35f;
        const float g = lit ? 0.80f : 0.35f;
        const float b = lit ? 0.25f : 0.35f;
        appendQuad(vertices, barX + i * blockW, barY, blockW, blockH, r, g, b);
    }

    vertexCount = static_cast<GLsizei>(vertices.size());
    uploadGeometry();
}

void SceneHUD::uploadGeometry()
{
    if (!vbo) return;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(HUDVertex)),
                 vertices.empty() ? nullptr : vertices.data(),
                 GL_DYNAMIC_DRAW);
}

void SceneHUD::Resize(int w, int h)
{
    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformLoadIdentity();
    transform.TransformOrtho(0.0f, static_cast<float>(w), static_cast<float>(h), 0.0f, -1.0f, 1.0f);

    buildGeometry(w, h);
}

void SceneHUD::Render()
{
    if (!program || vertexCount == 0) return;

    glUseProgram(program);

    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();

    glm::mat4* mvp = transform.TransformGetModelViewProjectionMatrix();
    glUniformMatrix4fv(uMVPLoc, 1, GL_FALSE, glm::value_ptr(*mvp));

    glDisable(GL_CULL_FACE);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);

    glEnable(GL_CULL_FACE);
}

int SceneHUD::hitTestButtons(float x, float y) const
{
    if (plusSpdRect.contains(x, y))   return 0;
    if (minusSpdRect.contains(x, y))  return 1;
    if (plusZoomRect.contains(x, y))  return 2;
    if (minusZoomRect.contains(x, y)) return 3;
    return -1;
}

void SceneHUD::setSpeed(float newSpeed)
{
    speed = std::clamp(newSpeed, 0.0f, 20.0f);
    rebuildBar();
}

float SceneHUD::ConsumeZoomDelta()
{
    const float d = pendingZoomDelta;
    pendingZoomDelta = 0.0f;
    return d;
}

void SceneHUD::rebuildBar()
{
    const int litBlocks = static_cast<int>(speed);
    const size_t vertsPerQuad = 6;

    for (int i = 0; i < kBarBlockCount; ++i)
    {
        const bool lit = (i < litBlocks);
        const float r = lit ? 0.20f : 0.35f;
        const float g = lit ? 0.80f : 0.35f;
        const float b = lit ? 0.25f : 0.35f;

        const size_t base = barVertexStart + i * vertsPerQuad;
        for (size_t v = 0; v < vertsPerQuad; ++v)
        {
            vertices[base + v].r = r;
            vertices[base + v].g = g;
            vertices[base + v].b = b;
        }
    }

    uploadGeometry();
}

void SceneHUD::applyButtonAction(int button)
{
    switch (button)
    {
    case 0: setSpeed(speed + 1.0f); LOGI("+SPD pressed (speed=%.0f)", speed);  break;
    case 1: setSpeed(speed - 1.0f); LOGI("-SPD pressed (speed=%.0f)", speed);  break;
    case 2: pendingZoomDelta -= 1.0f; LOGI("+ZOOM pressed"); break;
    case 3: pendingZoomDelta += 1.0f; LOGI("-ZOOM pressed"); break;
    default: break;
    }
}

bool SceneHUD::TouchDown(float x, float y)
{
    const int hit = hitTestButtons(x, y);
    if (hit < 0)
    {
        activeButton = -1;
        return false;
    }

    activeButton = hit;
    holdElapsed = 0.0f;
    applyButtonAction(hit);

    return true;
}

void SceneHUD::TouchRelease()
{
    activeButton = -1;
    holdElapsed = 0.0f;
}

void SceneHUD::Update(float dt)
{
    if (activeButton < 0) return;

    holdElapsed += dt;
    if (holdElapsed >= kRepeatInterval)
    {
        holdElapsed -= kRepeatInterval;
        applyButtonAction(activeButton);
    }
}