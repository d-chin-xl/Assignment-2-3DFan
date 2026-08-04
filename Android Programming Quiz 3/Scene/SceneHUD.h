// Author: Davina Chin XiangLing
// File Name: SceneHUD.h
// Date: 5.8.2026
// Description: Owns the 2D screen-space HUD: four buttons (+SPD, -SPD,
// +ZOOM, -ZOOM) and 20-block speed bar.
// uses its own orthographic projection and renders with depth testing disabled, on top of Scene3D.
// BONUS: implemented press-and-hold

#pragma once

#include "Model.h"
#include "Transform.h"
#include <vector>
#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

struct Rect
{
    float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;

    bool contains(float px, float py) const
    {
        return px >= x && px <= x + w && py >= y && py <= y + h;
    }
};

struct HUDVertex
{
    float x, y;
    float r, g, b;
};

class SceneHUD : public Model
{
public:
#ifdef PLATFORM_ANDROID
    explicit SceneHUD(AAssetManager* mgr) : assetMgr(mgr) {}
#else
    SceneHUD() = default;
#endif
    ~SceneHUD() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;
    void Update(float dt) override; // press-and-hold repeat

    bool TouchDown(float x, float y);
    void TouchRelease();

    float CurrentSpeed() const { return speed; }
    float ConsumeZoomDelta();

private:
    void buildGeometry(int w, int h);
    void appendQuad(std::vector<HUDVertex>& out,
        float x, float y, float w, float h,
        float r, float g, float b);
    void uploadGeometry();
    int hitTestButtons(float x, float y) const;

    void rebuildBar();
    void setSpeed(float newSpeed);
    void applyButtonAction(int button);

    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLint  uMVPLoc = -1;

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif

    Transform transform;

    std::vector<HUDVertex> vertices;
    GLsizei vertexCount = 0;

    Rect plusSpdRect;
    Rect minusSpdRect;
    Rect plusZoomRect;
    Rect minusZoomRect;

    Rect barOrigin;
    static constexpr int kBarBlockCount = 20;
    size_t barVertexStart = 0;

    float speed = 8.0f;
    float pendingZoomDelta = 0.0f;
    int   activeButton = -1;

    static constexpr float kRepeatInterval = 0.15f;
    float holdElapsed = 0.0f;
};