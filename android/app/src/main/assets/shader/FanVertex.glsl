#version 300 es

layout(location = 0) in vec3 aPosition;
layout(location = 1) in float aShade;

uniform mat4 MODELVIEWPROJECTIONMATRIX;

out float vShade;

void main()
{
    vShade = aShade;
    gl_Position = MODELVIEWPROJECTIONMATRIX * vec4(aPosition, 1.0);
}
