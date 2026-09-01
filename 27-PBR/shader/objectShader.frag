#version 330 core
out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;
in vec3 LocalPos;

uniform vec3 camPos;
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;
uniform int materialType;
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

const float PI = 3.14159265359;

float randomValue(vec3 p)
{
    return fract(sin(dot(p, vec3(12.9898, 78.233, 45.164))) * 43758.5453);
}

vec3 materialAlbedo()
{
    if (materialType == 0) {
        // 暗褐色砖块：球面方向构造经纬砖缝，展示粗糙非金属。
        vec3 n = normalize(LocalPos);
        float u = atan(n.z, n.x) / (2.0 * PI) + 0.5;
        float v = asin(clamp(n.y, -1.0, 1.0)) / PI + 0.5;
        float row = floor(v * 8.0);
        float shiftedU = u + mod(row, 2.0) * 0.065;
        float mortar = step(0.075, fract(v * 8.0)) *
                       step(0.055, fract(shiftedU * 13.0));
        vec3 brick = albedo * (0.72 + 0.28 * randomValue(floor(LocalPos * 9.0)));
        return mix(vec3(0.035, 0.045, 0.052), brick, mortar);
    }
    if (materialType == 2) {
        // 绿色粗糙表面：高频颜色变化模拟草地或苔藓。
        float grain = randomValue(floor(LocalPos * 55.0));
        return albedo * mix(0.55, 1.35, grain);
    }
    if (materialType == 4) {
        // 红色大理石：低频波纹叠加亮色矿脉。
        float veins = sin((LocalPos.x + LocalPos.y * 0.55 +
                           sin(LocalPos.z * 7.0) * 0.18) * 13.0);
        float veinMask = smoothstep(0.72, 0.98, abs(veins));
        return mix(albedo, vec3(0.72, 0.61, 0.57), veinMask * 0.75);
    }
    return albedo;
}

float distributionGGX(vec3 N, vec3 H, float value)
{
    float a = value * value;
    float a2 = a * a;
    float nDotH = max(dot(N, H), 0.0);
    float denominator = nDotH * nDotH * (a2 - 1.0) + 1.0;
    return a2 / max(PI * denominator * denominator, 0.0001);
}

float geometrySchlickGGX(float nDotV, float value)
{
    float r = value + 1.0;
    float k = (r * r) / 8.0;
    return nDotV / (nDotV * (1.0 - k) + k);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float value)
{
    return geometrySchlickGGX(max(dot(N, V), 0.0), value) *
           geometrySchlickGGX(max(dot(N, L), 0.0), value);
}

vec3 fresnelSchlick(float cosTheta, vec3 f0)
{
    return f0 + (1.0 - f0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    vec3 baseColor = materialAlbedo();
    vec3 N = normalize(Normal);
    vec3 V = normalize(camPos - WorldPos);
    vec3 f0 = mix(vec3(0.04), baseColor, metallic);
    vec3 directLighting = vec3(0.0);

    for (int i = 0; i < 4; ++i) {
        vec3 lightVector = lightPositions[i] - WorldPos;
        float distanceToLight = length(lightVector);
        vec3 L = lightVector / distanceToLight;
        vec3 H = normalize(V + L);
        vec3 radiance = lightColors[i] /
                        max(distanceToLight * distanceToLight, 0.01);

        float ndf = distributionGGX(N, H, roughness);
        float geometry = geometrySmith(N, V, L, roughness);
        vec3 fresnel = fresnelSchlick(max(dot(H, V), 0.0), f0);
        vec3 numerator = ndf * geometry * fresnel;
        float denominator = 4.0 * max(dot(N, V), 0.0) *
                            max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kS = fresnel;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
        float nDotL = max(dot(N, L), 0.0);
        directLighting += (kD * baseColor / PI + specular) * radiance * nDotL;
    }

    // 本章暂未引入 IBL，因此用很小的环境项避免未受光区域完全变黑。
    vec3 color = vec3(0.035) * baseColor * ao + directLighting;
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));
    FragColor = vec4(color, 1.0);
}
