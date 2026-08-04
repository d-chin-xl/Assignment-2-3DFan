// Author: Davina Chin XiangLing
// File Name: Scene3D.h
// Date: 5.8.2026
// Description: Owns everything in the 3D world: the Fan,  and procedural checkerboard ground
// and owns the adjustable camera distance and 3D picking

#pragma once

#include "Model.h"
#include "Fan.h"
#include "Ground.h"
#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Scene3D : public Model
{
public:
#ifdef PLATFORM_ANDROID
    explicit Scene3D(AAssetManager* mgr) : assetMgr(mgr) {}
#else
    Scene3D() = default;
#endif
    ~Scene3D() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;

    void SetCameraDistance(float d);
    float GetCameraDistance() const { return cameraDistance; }
    bool PickAt(float screenX, float screenY);
    void SetSpeed(float s);

private:
    Fan* fan = nullptr;
    Ground* ground = nullptr;
    float cameraDistance = 8.0f;

    int screenWidth = 0;
    int screenHeight = 0;

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif
};