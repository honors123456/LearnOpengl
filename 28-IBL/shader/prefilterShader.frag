#version 330 core
out vec4 FragColor;
in vec3 localPos;
uniform samplerCube environmentMap;
uniform float roughness;
const float PI = 3.14159265359;

float radicalInverseVdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}

vec2 hammersley(uint i, uint count)
{
    return vec2(float(i) / float(count), radicalInverseVdC(i));
}

vec3 importanceSampleGGX(vec2 xi, vec3 N, float value)
{
    float a = value * value;
    float phi = 2.0 * PI * xi.x;
    float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a * a - 1.0) * xi.y));
    float sinTheta = sqrt(max(1.0 - cosTheta * cosTheta, 0.0));
    vec3 H = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
    return normalize(tangent * H.x + bitangent * H.y + N * H.z);
}

void main()
{
    vec3 N = normalize(localPos);
    vec3 R = N;
    vec3 V = R;
    const uint sampleCount = 1024u;
    vec3 color = vec3(0.0);
    float weight = 0.0;
    for (uint i = 0u; i < sampleCount; ++i) {
        vec3 H = importanceSampleGGX(hammersley(i, sampleCount), N, roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);
        float nDotL = max(dot(N, L), 0.0);
        if (nDotL > 0.0) {
            color += texture(environmentMap, L).rgb * nDotL;
            weight += nDotL;
        }
    }
    FragColor = vec4(color / max(weight, 0.0001), 1.0);
}
