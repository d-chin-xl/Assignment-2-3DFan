// Author: Davina Chin XiangLing
// File Name: Renderer.cpp
// Date: 5.8.2026
// Description: Owns Scene3D + SceneHUD

#define LOG_TAG "Renderer"
#include "Renderer.h"
#include "Scene3D.h"
#include "SceneHUD.h"
#include <chrono>

Renderer::~Renderer() { clearModels(); }

#ifdef PLATFORM_ANDROID
void Renderer::setAssetManager(AAssetManager* mgr) { assetMgr = mgr; }
#endif

bool Renderer::initializeRenderer()
{
    LOGI("Renderer::initializeRenderer");
    createModels();
    initializeModels();
    return true;
}

void Renderer::createModels()
{
    clearModels();
#ifdef PLATFORM_ANDROID
    scene3D = new Scene3D(assetMgr);
    sceneHUD = new SceneHUD(assetMgr);
#else
    scene3D = new Scene3D();
    sceneHUD = new SceneHUD();
#endif
    LOGI("Renderer: Scene3D + SceneHUD created");
}

void Renderer::initializeModels()
{
    if (scene3D)  scene3D->InitModel();
    if (sceneHUD) sceneHUD->InitModel();
}

void Renderer::clearModels()
{
    delete scene3D;  scene3D = nullptr;
    delete sceneHUD; sceneHUD = nullptr;
}

void Renderer::resize(int w, int h)
{
    screenWidth = w;
    screenHeight = h;
    glViewport(0, 0, w, h);
    if (scene3D)  scene3D->Resize(w, h);
    if (sceneHUD) sceneHUD->Resize(w, h);
    LOGI("Renderer::resize %d x %d", w, h);
}

void Renderer::render()
{
    // Real per-frame delta time, needed for SceneHUD's press-and-hold repeat.
    static auto lastTime = std::chrono::steady_clock::now();
    const auto now = std::chrono::steady_clock::now();
    const float dt = std::chrono::duration<float>(now - lastTime).count();
    lastTime = now;

    if (sceneHUD)
    {
        sceneHUD->Update(dt);

        const float zoomDelta = sceneHUD->ConsumeZoomDelta();
        if (zoomDelta != 0.0f && scene3D)
        {
            scene3D->SetCameraDistance(scene3D->GetCameraDistance() + zoomDelta);
        }
    }

    if (scene3D && sceneHUD)
        scene3D->SetSpeed(sceneHUD->CurrentSpeed());

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    if (scene3D) scene3D->Render();

    glDisable(GL_DEPTH_TEST);
    if (sceneHUD) sceneHUD->Render();

    glEnable(GL_DEPTH_TEST);
}

void Renderer::TouchEventDown(float x, float y)
{
    if (sceneHUD && sceneHUD->TouchDown(x, y)) return;
    if (scene3D) scene3D->PickAt(x, y);
}

void Renderer::TouchEventMove(float x, float y)
{
    (void)x;
    (void)y;
}

void Renderer::TouchEventRelease(float x, float y)
{
    (void)x; (void)y;
    if (sceneHUD) sceneHUD->TouchRelease();
}