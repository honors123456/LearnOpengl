#version 330 core

out vec4 FragColor;

// 光照参数结构体：本 demo 只用了一个点光源。
// 注意：光源方向不用这里传，顶点着色器已把 lightPos 变换到切线空间（fs_in.TangentLightPos）。
struct Light {
    vec3 ambient;   // 环境光强度
    vec3 diffuse;   // 漫反射强度
    vec3 specular;  // 镜面高光强度
};

// 材质参数结构体：三张贴图 + 视差高度缩放。
struct Material {
    sampler2D diffuse;   // 漫反射贴图：物体固有颜色
    sampler2D normalMap; // 法线贴图：颜色编码的切线空间法线
    sampler2D depthMap;  // 高度图（黑白）：R 通道存表面高度
    float shininess;     // 高光锐度（越大越集中）
};

// 顶点着色器传来的数据（都在切线空间）。
in VS_OUT {
    vec3 FragPos;          // 片元世界坐标
    vec2 TexCoords;        // 纹理坐标（视差偏移会修改它）
    vec3 TangentLightPos;  // 光源位置（切线空间）
    vec3 TangentViewPos;   // 相机位置（切线空间）
    vec3 TangentFragPos;   // 片元位置（切线空间）
} fs_in;

uniform Material material;
uniform Light light;
uniform float heightScale; // 视差强度（越大凹陷越深，用 Q/E 键调节）

void main()
{
    // 0. 计算切线空间视线方向（片元指向相机）。
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);

    // 1. 视差贴图核心：根据高度图偏移 UV（全部内联，不再封装函数）。
    //    原理：视线越倾斜、高度越低（凹陷越深），UV 偏移越大，
    //    让平面看起来像有真实的深度遮挡（砖块互相错位）。
    // 1.1 采样当前 UV 处的高度（0 = 最深凹陷，1 = 最高凸起）。
    float height = texture(material.depthMap, fs_in.TexCoords).r;

    // 1.2 视线方向在切线空间下的偏移量：viewDir.xy / viewDir.z 是视线倾斜程度。
    //     视线越斜（viewDir.xy 大 / viewDir.z 小），或高度越低，偏移越大。
    vec2 p = viewDir.xy / viewDir.z * (height * heightScale);

    // 1.3 用偏移后的 UV 采样其他贴图，实现“假深度遮挡”。
    vec2 texCoords = fs_in.TexCoords - p;

    // 2. 采样法线贴图并解包成 [-1, 1] 的切线空间法线。
    vec3 N = texture(material.normalMap, texCoords).rgb;
    N = normalize(N * 2.0 - 1.0);

    // 3. 计算切线空间光线/视线/半程向量。
    vec3 L = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    vec3 V = normalize(fs_in.TangentViewPos  - fs_in.TangentFragPos);
    vec3 H = normalize(L + V);

    // 4. 采样漫反射贴图，做 gamma 逆校正（转回线性空间）。
    vec3 diffuseColor = pow(texture(material.diffuse, texCoords).rgb, vec3(2.2));

    // 5. 漫反射（Lambert 定律）与高光（Blinn-Phong）。
    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(N, H), 0.0), material.shininess);

    // 6. 点光源距离衰减。
    float lightDistance = length(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float attenuation = 1.0 / (1.0 + 0.22 * lightDistance + 0.20 * lightDistance * lightDistance);

    // 7. 合成光照：环境光 + (漫反射 + 高光) × 衰减。
    vec3 ambient = light.ambient * diffuseColor;
    vec3 diffuse = light.diffuse * diff * diffuseColor;
    vec3 specular = light.specular * spec;
    vec3 lighting = ambient + (diffuse + specular) * attenuation;

    // 8. gamma 校正：线性光照结果映射回 sRGB 显示空间。
    vec3 finalColor = pow(lighting, vec3(1.0 / 2.2));

    FragColor = vec4(finalColor, 1.0);
}

