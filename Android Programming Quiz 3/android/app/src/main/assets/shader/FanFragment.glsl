#version 300 es
precision mediump float;

in vec3 vNormal;
in vec3 vPosition;

uniform vec3 PARTCOLOR;
uniform vec3 LIGHTPOS; // eye-space light position

out vec4 fragColor;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 L = normalize(LIGHTPOS - vPosition);
    vec3 V = normalize(-vPosition); // eye is at the origin in eye space
    vec3 R = reflect(-L, N);

    const float ambientStrength  = 0.15;
    const float specularStrength = 0.6;
    const float shininess        = 32.0;

    vec3 ambient  = ambientStrength * PARTCOLOR;
    vec3 diffuse  = max(dot(N, L), 0.0) * PARTCOLOR;
    vec3 specular = specularStrength * pow(max(dot(R, V), 0.0), shininess) * vec3(1.0);

    vec3 result = ambient + diffuse + specular;
    fragColor = vec4(result, 1.0);
}