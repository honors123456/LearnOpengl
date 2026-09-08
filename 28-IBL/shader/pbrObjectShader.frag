#version 330 core
// IBL 最终材质着色器：同时计算直接光照和环境光照。
// 直接光照来自四个点光源；间接光照来自 irradiance、prefilter 和 BRDF LUT。
out vec4 FragColor;
in vec3 WorldPos;
in vec3 Normal;
in vec2 TexCoords;

// WorldPos 和 Normal 在顶点着色器中已经转换到世界空间。
// TexCoords 用于读取五套 PBR 材质贴图。
uniform vec3 camPos;
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];
uniform bool enableDirectLights;
const float PI = 3.14159265359;

// Fresnel-Schlick：描述反射率随观察角变化的现象。
// cosTheta 越小，表示越接近掠射角，反射率越高。
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - clamp(cosTheta, 0.0, 1.0), 5.0);
}
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float value)
{
    return F0 + (max(vec3(1.0 - value), F0) - F0) *
           pow(1.0 - clamp(cosTheta, 0.0, 1.0), 5.0);
}
float distributionGGX(vec3 N, vec3 H, float value)
{
    // GGX NDF 决定微表面法线 H 的分布，roughness 越大，高光越宽。
    float a = value * value;
    float a2 = a * a;
    float nDotH = max(dot(N, H), 0.0);
    float denominator = nDotH * nDotH * (a2 - 1.0) + 1.0;
    return a2 / max(PI * denominator * denominator, 0.0001);
}
float geometrySchlickGGX(float nDotV, float value)
{
    // 几何遮蔽项模拟微表面之间互相遮挡和自阴影的现象。
    float r = value + 1.0;
    float k = r * r / 8.0;
    return nDotV / (nDotV * (1.0 - k) + k);
}
float geometrySmith(vec3 N, vec3 V, vec3 L, float value)
{
    return geometrySchlickGGX(max(dot(N, V), 0.0), value) *
           geometrySchlickGGX(max(dot(N, L), 0.0), value);
}
vec3 getNormalFromMap()
{
    // 法线贴图存储的是切线空间法线，范围 [0,1] 需要还原为 [-1,1]。
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;
    vec3 Q1 = dFdx(WorldPos);
    vec3 Q2 = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);
    vec3 N = normalize(Normal);
    // 使用屏幕空间导数从位置和 UV 推导切线，避免顶点数据额外存储 tangent。
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    return normalize(mat3(T, B, N) * tangentNormal);
}
void main()
{
    // 颜色贴图通过 GL_SRGB8 创建，OpenGL 采样时会自动转换到线性空间。
    vec3 albedo = texture(albedoMap, TexCoords).rgb;
    float metallic = texture(metallicMap, TexCoords).r;
    float roughness = texture(roughnessMap, TexCoords).r;
    float ao = texture(aoMap, TexCoords).r;
    vec3 N = getNormalFromMap();
    vec3 V = normalize(camPos - WorldPos);
    vec3 R = reflect(-V, N);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    // Lo 表示解析光源贡献；按键 1 可以关闭它，只观察纯 IBL 效果。
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < 4 && enableDirectLights; ++i) {
        vec3 lightVector = lightPositions[i] - WorldPos;
        float distance = length(lightVector);
        vec3 L = lightVector / distance;
        vec3 H = normalize(V + L);
        vec3 radiance = lightColors[i] / (distance * distance);
        float NDF = distributionGGX(N, H, roughness);
        float G = geometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
        vec3 specular = NDF * G * F /
                        (4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001);
        vec3 kDDirect = (vec3(1.0) - F) * (1.0 - metallic);
        Lo += (kDDirect * albedo / PI + specular) * radiance * max(dot(N, L), 0.0);
    }
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    // irradiance cubemap 已经对环境光做了半球余弦卷积，提供漫反射环境光。
    vec3 diffuse = texture(irradianceMap, N).rgb * albedo;
    // prefilter cubemap 的 mip 级别对应 roughness，粗糙表面采样更模糊的 mip。
    vec3 prefiltered = textureLod(prefilterMap, R, roughness * 4.0).rgb;
    // BRDF LUT 的横坐标是 NdotV，纵坐标是 roughness，返回 Fresnel 的缩放和偏移。
    vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 ambient = (kD * diffuse + prefiltered * (F * brdf.x + brdf.y)) * ao;
    vec3 color = ambient + Lo;
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));
    FragColor = vec4(color, 1.0);
}
