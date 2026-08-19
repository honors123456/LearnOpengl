#version 330 core

out vec4 FragColor;

// 光照参数结构体：本 demo 只用了一个点光源。
struct Light {
    vec3 ambient;   // 环境光强度（不随距离衰减）
    vec3 diffuse;   // 漫反射强度
    vec3 specular;  // 镜面高光强度
};

// 材质参数结构体。
struct Material {
    sampler2D normalMap; // 法线贴图：用颜色编码法线方向，制造表面凹凸
    vec3 diffuseColor;   // 漫反射基础颜色（本 demo 为纯色砖红）
    float shininess;     // 高光锐度（越大，高光越集中）
};

// 顶点着色器传来的数据（都是已经变换到切线空间的值）。
// 在切线空间计算光照，才能让法线贴图中的法线与光线、视线处于同一坐标系。
in VS_OUT {
    vec3 FragPos;           // 片元世界坐标（本 demo 未直接使用）
    vec2 TexCoords;         // 纹理坐标，用于采样法线贴图
    vec3 TangentLightPos;   // 光源位置（切线空间）
    vec3 TangentCameraPos;  // 相机位置（切线空间）
    vec3 TangentFragPos;    // 片元位置（切线空间）
} fs_in;

uniform Material material;
uniform Light light;

void main()
{
    // 1. 从法线贴图中采样颜色 (RGB 范围是 0.0 ~ 1.0)
    vec3 N = texture(material.normalMap, fs_in.TexCoords).rgb;

    // 2. 解包 (Unpack)：把 [0, 1] 的颜色映射回 [-1, 1] 的真实法线向量空间
    N = normalize(N * 2.0 - 1.0);

    // 3. 获取切线空间下的光线方向与视线方向
    vec3 L = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    vec3 V = normalize(fs_in.TangentCameraPos - fs_in.TangentFragPos);
    vec3 H = normalize(L + V);

    // 4. 计算光照分量：
    //    先对漫反射颜色做 gamma 逆校正（pow 2.2），因为最终输出还会做一次 gamma 校正，
    //    中间的光照计算需要在线性空间进行，避免颜色失真。
    vec3 diffuseColor = pow(material.diffuseColor, vec3(2.2));

    //    diffuse：法线与光线的夹角越接近 0°，漫反射越强（Lambert 定律）。
    float diff = max(dot(N, L), 0.0);

    //    spec：Blinn-Phong 模型，用半程向量 H 与法线 N 的点积计算镜面高光。
    float spec = pow(max(dot(N, H), 0.0), material.shininess);

    // 5. 点光源距离衰减：光强随距离增大按二次曲线快速减弱。
    float lightDistance = length(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float attenuation = 1.0 / (1.0 + 0.22 * lightDistance + 0.20 * lightDistance * lightDistance);

    // 6. 合成光照：环境光 + (漫反射 + 高光) × 距离衰减。
    vec3 ambient = light.ambient * diffuseColor;
    vec3 diffuse = light.diffuse * diff * diffuseColor;
    vec3 specular = light.specular * spec;
    vec3 lighting = ambient + (diffuse + specular) * attenuation;

    // 7. gamma 校正：把线性光照结果映射回 sRGB 显示空间（pow 1/2.2）。
    vec3 finalColor = pow(lighting, vec3(1.0 / 2.2));

    // 8. 输出最终颜色，alpha 恒为 1.0（不透明）。
    FragColor = vec4(finalColor, 1.0);
}
