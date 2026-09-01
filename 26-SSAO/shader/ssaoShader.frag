#version 330 core

out float FragColor;
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;
uniform vec3 samples[64];
uniform mat4 projection;

const int KERNEL_SIZE = 64;
const float RADIUS = 0.6;
const float BIAS = 0.025;

void main()
{
    vec3 fragPos = texture(gPosition, TexCoords).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoords).xyz);

    // 4x4 噪声纹理平铺到屏幕，为每个片段随机旋转采样半球。
    vec2 noiseScale = vec2(textureSize(gPosition, 0)) /
                      vec2(textureSize(texNoise, 0));
    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);

    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 tbn = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for (int i = 0; i < KERNEL_SIZE; ++i) {
        vec3 samplePos = fragPos + tbn * samples[i] * RADIUS;

        // 把观察空间采样点投影回屏幕纹理坐标。
        vec4 offset = projection * vec4(samplePos, 1.0);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        float sampleDepth = texture(gPosition, offset.xy).z;
        float rangeCheck = smoothstep(
            0.0, 1.0, RADIUS / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + BIAS ? 1.0 : 0.0) *
                     rangeCheck;
    }

    FragColor = 1.0 - occlusion / float(KERNEL_SIZE);
}
