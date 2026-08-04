#version 300 es
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

uniform mat4 MODELVIEWPROJECTIONMATRIX;
uniform mat4 MODELVIEWMATRIX;
uniform mat3 NORMALMATRIX;

out vec3 vNormal;
out vec3 vPosition; // eye-space fragment position, for lighting

void main()
{
    // Transform the normal by the normal matrix (inverse-transpose of the
    // upper 3x3 of the modelview matrix), not the raw modelview matrix —
    // required once any part has non-uniform scale (every blade does).
    vNormal = normalize(NORMALMATRIX * aNormal);

    vec4 posEye = MODELVIEWMATRIX * vec4(aPosition, 1.0);
    vPosition = posEye.xyz;

    gl_Position = MODELVIEWPROJECTIONMATRIX * vec4(aPosition, 1.0);
}