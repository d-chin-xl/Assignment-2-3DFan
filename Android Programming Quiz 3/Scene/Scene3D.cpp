// Author: Davina Chin XiangLing
// File Name: Scene3D.cpp
// Date: 5.8.2026
// Description: Implementation of Scene3Ds.

#define LOG_TAG "Scene3D"
#include "Scene3D.h"

Scene3D::~Scene3D()
{
    delete fan;
    delete ground;
}

void Scene3D::InitModel()
{
#ifdef PLATFORM_ANDROID
    fan = new Fan(assetMgr);
    ground = new Ground(assetMgr);
#else
    fan = new Fan();
    ground = new Ground();
#endif
    fan->InitModel();
    ground->InitModel();
}

void Scene3D::Render()
{
    if (fan)    fan->SetCameraDistance(cameraDistance);
    if (ground) ground->SetCameraDistance(cameraDistance);

    if (fan) fan->Render();
    if (ground) ground->Render();
}

void Scene3D::Resize(int w, int h)
{
    screenWidth = w;
    screenHeight = h;
    if (fan) fan->Resize(w, h);
    if (ground) ground->Resize(w, h);
}

void Scene3D::SetCameraDistance(float d)
{
    if (d < 3.0f)  d = 3.0f;
    if (d > 20.0f) d = 20.0f;
    cameraDistance = d;
}

void Scene3D::SetSpeed(float s)
{
    if (fan) fan->SetSpeed(s);
}

bool Scene3D::PickAt(float screenX, float screenY)
{
    if (!fan) return false;
    return fan->PickAt(screenX, screenY, screenWidth, screenHeight);
}