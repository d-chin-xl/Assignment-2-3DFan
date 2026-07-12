#pragma once

#include "Model.h"
#include "Transform.h"
#include <chrono>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Fan : public Model
{
public:
#ifdef PLATFORM_ANDROID
    explicit Fan(AAssetManager* mgr) : assetMgr(mgr) {}
#else
    Fan() = default;
#endif
    ~Fan() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;

    void TouchEventDown(float x, float y) override;
    void TouchEventMove(float x, float y) override;
    void TouchEventRelease(float x, float y) override;

private:
    void drawPart(const glm::vec3& color);

    ///GL objects
    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ibo = 0;
    GLint  uMVPLoc = -1;
    GLint  uColorLoc = -1;

    static constexpr int kIndexCount = 36;

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif

    ///Scenes
    Transform transform;

    ///Animation state
    float spinAngle = 0.0f; //current blade angle
    bool  fanOn = true; //toggling
    float dragBoost = 0.0f;

    static constexpr float kBaseSpeed = 1.5f;
    static constexpr float kMaxBoost = 20.0f;
    static constexpr float kBoostScale = 8.0f;
    static constexpr float kTapThreshold = 12.0f;

    ///Tracking
    float lastX = 0.0f, lastY = 0.0f;
    float movedDistance = 0.0f;
    std::chrono::steady_clock::time_point lastMoveTime;
};