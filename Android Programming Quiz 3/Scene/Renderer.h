// Author: Davina Chin XiangLing
// File Name: Renderer.h
// Date: 5.8.2026
// Description: Singleton renderer that owns Scene3D and SceneHUD and drives the render loop

#pragma once

#include "Model.h"
#include "Platform.h"
#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Scene3D;
class SceneHUD;

class Renderer {
public:
    static Renderer& Instance() {
        static Renderer instance;
        return instance;
    }

    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

    // -----------------------------------------------------------------------
    // Platform-specific initialisation helpers
    // -----------------------------------------------------------------------
#ifdef PLATFORM_ANDROID
    void setAssetManager(AAssetManager* mgr);
#endif

    bool initializeRenderer();
    void resize(int w, int h);
    void render();

    // Touch / mouse delegation
    void TouchEventDown(float x, float y);
    void TouchEventMove(float x, float y);
    void TouchEventRelease(float x, float y);

private:
    Renderer()  = default;
    ~Renderer();

    void createModels();
    void initializeModels();
    void clearModels();

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif

    Scene3D* scene3D = nullptr;
    SceneHUD* sceneHUD = nullptr;
    int screenWidth = 0;
    int screenHeight = 0;
};
