// Author: Davina Chin XiangLing
// File Name: Ground.h
// Date: 5.8.2026
// Description: A large quad positioned under the fan. The fragment
// shader computes checkerboard pattern from object-space XZ position 
// then feeds that pattern through same Phong lighting model used by Fan, so  ground 
// responds to the light exactly like fan

#pragma once

#include "Model.h"
#include "Transform.h"
#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Ground : public Model
{
public:
#ifdef PLATFORM_ANDROID
    explicit Ground(AAssetManager* mgr) : assetMgr(mgr) {}
#else
    Ground() = default;
#endif
    ~Ground() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;

    void SetCameraDistance(float d) 
    {
        cameraDistance = d;
    }

private:
    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ibo = 0;

    GLint uMVPLoc = -1;
    GLint uMVLoc = -1;
    GLint uNormalMatLoc = -1;
    GLint uLightPosLoc = -1;
    GLint uTileSizeLoc = -1;
    GLint uColorALoc = -1;
    GLint uColorBLoc = -1;

    static constexpr int kIndexCount = 6; // one quad, two triangles

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif

    Transform transform;

    glm::vec3 lightPosition{ 3.0f, 5.0f, 4.0f }; // same as Fan's light
    float tileSize = 1.0f;
    glm::vec3 colorA{ 0.85f, 0.85f, 0.85f }; // light tile
    glm::vec3 colorB{ 0.10f, 0.10f, 0.10f }; // dark tile
    float cameraDistance = 8.0f;
};