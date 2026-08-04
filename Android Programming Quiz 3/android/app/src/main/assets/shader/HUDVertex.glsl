#version 300 es
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec3 aColor;

uniform mat4 MODELVIEWPROJECTIONMATRIX;

out vec3 vColor;

void main()
{
    vColor = aColor;
    gl_Position = MODELVIEWPROJECTIONMATRIX * vec4(aPosition, 0.0, 1.0);
}