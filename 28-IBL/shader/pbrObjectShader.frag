#version 330 core
out vec4 FragColor;
in vec3 WorldPos;
in vec3 Normal;
in vec2 TexCoords;
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
    float a = value * value;
    float a2 = a * a;
    float nDotH = max(dot(N, H), 0.0);
    float denominator = nDotH * nDotH * (a2 - 1.0) + 1.0;
    return a2 / max(PI * denominator * denominator, 0.0001);
}
float geometrySchlickGGX(float nDotV, float value)
{
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
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;
    vec3 Q1 = dFdx(WorldPos);
    vec3 Q2 = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);
    vec3 N = normalize(Normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    return normalize(mat3(T, B, N) * tangentNormal);
}
void main()
{
    vec3 albedo = texture(albedoMap, TexCoords).rgb;
    float metallic = texture(metallicMap, TexCoords).r;
    float roughness = texture(roughnessMap, TexCoords).r;
    float ao = texture(aoMap, TexCoords).r;
    vec3 N = getNormalFromMap();
    vec3 V = normalize(camPos - WorldPos);
    vec3 R = reflect(-V, N);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
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
    vec3 diffuse = texture(irradianceMap, N).rgb * albedo;
    vec3 prefiltered = textureLod(prefilterMap, R, roughness * 4.0).rgb;
    vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 ambient = (kD * diffuse + prefiltered * (F * brdf.x + brdf.y)) * ao;
    vec3 color = ambient + Lo;
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));
    FragColor = vec4(color, 1.0);
}
