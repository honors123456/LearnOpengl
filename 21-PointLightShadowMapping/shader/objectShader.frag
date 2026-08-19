#version 330 core

out vec4 FragColor;

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform Material material;
uniform Light light;
uniform vec3 cameraPos;
uniform samplerCube depthMap;
uniform float far_plane;


void main()
{
    // ========== 1. 基础向量 ==========
    // N：片元法线（世界空间，插值后）
    vec3 N = normalize(fs_in.Normal);
    // V：片元指向相机的方向
    vec3 V = normalize(cameraPos - fs_in.FragPos);

    // ========== 2. 纹理颜色（Gamma 逆校正：sRGB -> 线性空间）==========
    vec3 diffuseColor  = pow(texture(material.diffuse,  fs_in.TexCoords).rgb, vec3(2.2));
    vec3 specularColor = texture(material.specular, fs_in.TexCoords).rgb;

    // ========== 3. 漫反射（直接光）==========
    vec3 L = normalize(light.position - fs_in.FragPos);  // 片元指向光源的方向
    float diff = max(dot(N, L), 0.0);                    // 法线与光线的夹角余弦
    vec3 diffuse = light.diffuse * diff * diffuseColor;

    // ========== 4. 高光（直接光）==========
    vec3 R = reflect(-L, N);                             // 光线关于法线的反射方向
    float spec = pow(max(dot(V, R), 0.0), material.shininess);
    vec3 specular = light.specular * spec * specularColor;

    // ========== 5. 环境光：恒定，不受阴影遮挡 ==========
    vec3 ambient = light.ambient * diffuseColor;

    // ========== 6. 阴影判断（Omni Shadow Map，立方体贴图）==========
    // 6.1 片元到光源的向量，其长度 = 当前深度（真实距离）
    vec3 fragToLight = fs_in.FragPos - light.position;
    float currentDepth = length(fragToLight);

    // 6.2 用方向向量直接采样深度立方体贴图，得到该方向最近的深度（归一化值），乘 far_plane 还原真实距离
    float closestDepth = texture(depthMap, fragToLight).r * far_plane;

    // 6.3 固定 bias：稍微放松比较，避免平面自阴影（Shadow Acne）
    float bias = 0.05;

    // 6.4 比较：当前深度比记录值更远 -> 被遮挡 -> 在阴影里
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    // ========== 7. 点光源距离衰减 ==========
    float lightDistance = length(light.position - fs_in.FragPos);
    float attenuation = 1.0 / (1.0 + 0.09 * lightDistance + 0.032 * lightDistance * lightDistance);

    // ========== 8. 合成颜色 ==========
    // 环境光恒定 + (直接光受阴影遮挡后) * 距离衰减
    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular) * attenuation;
    // Gamma 校正：线性空间 -> sRGB
    vec3 finalColor = pow(lighting, vec3(1.0 / 2.2));

    FragColor = vec4(finalColor, 1.0);
}
