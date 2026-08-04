#version 300 es
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

uniform mat4 MODELVIEWPROJECTIONMATRIX;
uniform mat4 MODELVIEWMATRIX;
uniform mat3 NORMALMATRIX;

out vec3 vNormal;
out vec3 vPosition;   // eye-space, for lighting
out vec2 vObjectXZ;   // object-space XZ, for the checkerboard pattern

void main()
{
    vNormal = normalize(NORMALMATRIX * aNormal);

    vec4 posEye = MODELVIEWMATRIX * vec4(aPosition, 1.0);
    vPosition = posEye.xyz;

    vObjectXZ = aPosition.xz;

    gl_Position = MODELVIEWPROJECTIONMATRIX * vec4(aPosition, 1.0);
}