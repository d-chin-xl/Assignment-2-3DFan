// Author: Davina Chin XiangLing
// File Name: Fan.h
// Date: 5.8.2026
// Description: The spinning fan model carried over from Quiz 2 extended for Quiz 3 with 
// per-fragment Phong lighting, an adjustable camera distance and 3D picking via ray-vs-sphere 
// and many more stared in pdf

#pragma once

#include "Model.h"
#include "Transform.h"
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

    void SetSpeed(float s)
    {
        speed = s;
    }

    void SetCameraDistance(float d)
    { 
        cameraDistance = d;
    }

    bool PickAt(float screenX, float screenY, int viewportW, int viewportH);

private:
    void drawPart(const glm::vec3& color);

    enum PartId
    {
        kPartBase = 0,
        kPartPole = 1,
        kPartHub = 2,
        kPartBlade0 = 3 // blades
    };

    ///GL objects
    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ibo = 0;
    GLint  uMVPLoc = -1;
    GLint  uColorLoc = -1;
    GLint uMVLoc = -1;
    GLint uNormalMatLoc = -1;
    GLint uLightPosLoc = -1;

    static constexpr int kIndexCount = 36;
    static constexpr float kSpeedScale = 0.15f;
#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif

    ///Scenes
    Transform transform;

    ///Animation state
    float spinAngle = 0.0f; //current blade angle
    float speed = 1.5f;
    float cameraDistance = 8.0f;

    glm::vec3 lightPosition{ 3.0f, 5.0f, 4.0f };

    int selectedPart = -1;
};