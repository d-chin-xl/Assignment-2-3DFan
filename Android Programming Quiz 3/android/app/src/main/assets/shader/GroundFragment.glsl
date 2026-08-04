#version 300 es
precision mediump float;

in vec3 vNormal;
in vec3 vPosition;
in vec2 vObjectXZ;

uniform vec3 LIGHTPOS;
uniform float TILESIZE;
uniform vec3 COLORA;
uniform vec3 COLORB;

out vec4 fragColor;

float checker(vec2 objectXZ, float tileSize)
{
    vec2 cell = floor(objectXZ / tileSize);
    float parity = mod(cell.x + cell.y, 2.0);
    return parity; // 0.0 or 1.0
}

void main()
{
    float parity = checker(vObjectXZ, TILESIZE);
    vec3 baseColor = mix(COLORA, COLORB, parity);

    vec3 N = normalize(vNormal);
    vec3 L = normalize(LIGHTPOS - vPosition);
    vec3 V = normalize(-vPosition);
    vec3 R = reflect(-L, N);

    const float ambientStrength  = 0.15;
    const float specularStrength = 0.3;
    const float shininess        = 16.0;

    vec3 ambient  = ambientStrength * baseColor;
    vec3 diffuse  = max(dot(N, L), 0.0) * baseColor;
    vec3 specular = specularStrength * pow(max(dot(R, V), 0.0), shininess) * vec3(1.0);

    vec3 result = ambient + diffuse + specular;
    fragColor = vec4(result, 1.0);
}