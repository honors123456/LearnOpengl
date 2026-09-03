#version 330 core
out vec4 FragColor;

in vec3 WorldPos;    // 世界空间位置
in vec3 Normal;      // 世界空间法线
in vec3 LocalPos;    // 局部空间位置（用于程序化纹理）

uniform vec3 camPos;         // 相机位置
uniform vec3 albedo;         // 基础颜色（反照率）
uniform float metallic;      // 金属度 [0,1]，0=非金属，1=金属
uniform float roughness;     // 粗糙度 [0,1]，0=光滑，1=粗糙
uniform float ao;            // 环境光遮蔽 [0,1]
uniform int materialType;    // 材质类型：0=砖块，2=草地，4=大理石，其他=纯色
uniform vec3 lightPositions[4];  // 4个点光源位置
uniform vec3 lightColors[4];     // 4个点光源颜色

const float PI = 3.14159265359;

// 伪随机数生成器，用于程序化纹理
float randomValue(vec3 p)
{
    return fract(sin(dot(p, vec3(12.9898, 78.233, 45.164))) * 43758.5453);
}

// 根据材质类型生成程序化纹理颜色
vec3 materialAlbedo()
{
    if (materialType == 0) {
        // 暗褐色砖块：球面方向构造经纬砖缝，展示粗糙非金属
        vec3 n = normalize(LocalPos);
        // 球面UV映射：经度u [0,1]，纬度v [0,1]
        float u = atan(n.z, n.x) / (2.0 * PI) + 0.5;
        float v = asin(clamp(n.y, -1.0, 1.0)) / PI + 0.5;
        float row = floor(v * 8.0);
        // 交错砖缝：偶数行偏移0.065，模拟真实砖墙排列
        float shiftedU = u + mod(row, 2.0) * 0.065;
        // 砖缝遮罩：v方向8行，u方向13列，step函数创建硬边界
        float mortar = step(0.075, fract(v * 8.0)) *
                       step(0.055, fract(shiftedU * 13.0));
        // 砖块颜色随机变化，增加真实感
        vec3 brick = albedo * (0.72 + 0.28 * randomValue(floor(LocalPos * 9.0)));
        // 混合砖缝（深灰色）和砖块
        return mix(vec3(0.035, 0.045, 0.052), brick, mortar);
    }
    if (materialType == 2) {
        // 绿色粗糙表面：高频颜色变化模拟草地或苔藓
        float grain = randomValue(floor(LocalPos * 55.0));
        return albedo * mix(0.55, 1.35, grain);
    }
    if (materialType == 4) {
        // 红色大理石：低频波纹叠加亮色矿脉
        // 复合正弦波创建自然流动的纹理
        float veins = sin((LocalPos.x + LocalPos.y * 0.55 +
                           sin(LocalPos.z * 7.0) * 0.18) * 13.0);
        // smoothstep创建柔和的矿脉边缘
        float veinMask = smoothstep(0.72, 0.98, abs(veins));
        return mix(albedo, vec3(0.72, 0.61, 0.57), veinMask * 0.75);
    }
    // 默认：直接返回基础颜色
    return albedo;
}

// GGX/Trowbridge-Reitz 法线分布函数
// 描述微平面法线朝向半程向量H的概率分布
// 粗糙度越高，分布越分散，高光越模糊
float distributionGGX(vec3 N, vec3 H, float value)
{
    float a = value * value;      // α = roughness²，Disney提出的重映射
    float a2 = a * a;
    float nDotH = max(dot(N, H), 0.0);
    float denominator = nDotH * nDotH * (a2 - 1.0) + 1.0;
    // 防止除零，返回GGX分布值
    return a2 / max(PI * denominator * denominator, 0.0001);
}

// Schlick-GGX 几何遮蔽函数（单方向）
// 模拟微平面自遮蔽效应，粗糙表面更明显
float geometrySchlickGGX(float nDotV, float value)
{
    float r = value + 1.0;
    float k = (r * r) / 8.0;      // k = (roughness + 1)² / 8，直接光照的推荐值
    return nDotV / (nDotV * (1.0 - k) + k);
}

// Smith 几何遮蔽函数（组合视线和光线方向）
// 分别计算视线方向和光线方向的遮蔽，然后相乘
float geometrySmith(vec3 N, vec3 V, vec3 L, float value)
{
    return geometrySchlickGGX(max(dot(N, V), 0.0), value) *
           geometrySchlickGGX(max(dot(N, L), 0.0), value);
}

// Fresnel-Schlick 菲涅尔方程近似
// 描述视角相关的反射率：掠射角反射更强
// f0：垂直入射时的基础反射率
vec3 fresnelSchlick(float cosTheta, vec3 f0)
{
    // pow(1 - cosTheta, 5) 是Schlick近似的核心
    return f0 + (1.0 - f0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    // 获取程序化材质颜色
    vec3 baseColor = materialAlbedo();
    
    // 标准化向量
    vec3 N = normalize(Normal);           // 法线
    vec3 V = normalize(camPos - WorldPos); // 视线方向（指向相机）
    
    // 计算基础反射率f0
    // 非金属：f0 = 0.04（4%，电介质典型值）
    // 金属：f0 = albedo（金属颜色即反射色）
    vec3 f0 = mix(vec3(0.04), baseColor, metallic);
    
    vec3 directLighting = vec3(0.0);

    // 遍历4个点光源，累加直接光照
    for (int i = 0; i < 4; ++i) {
        // 计算光照方向L和距离衰减
        vec3 lightVector = lightPositions[i] - WorldPos;
        float distanceToLight = length(lightVector);
        vec3 L = lightVector / distanceToLight;  // 标准化光照方向
        
        // 半程向量H：视线和光线的中间方向，用于微平面BRDF
        vec3 H = normalize(V + L);
        
        // 距离平方衰减的辐射度
        vec3 radiance = lightColors[i] /
                        max(distanceToLight * distanceToLight, 0.01);

        // Cook-Torrance BRDF 三大项
        float ndf = distributionGGX(N, H, roughness);        // 法线分布
        float geometry = geometrySmith(N, V, L, roughness);  // 几何遮蔽
        vec3 fresnel = fresnelSchlick(max(dot(H, V), 0.0), f0); // 菲涅尔
        
        // 镜面反射项：NDF * G * F / (4 * NdotV * NdotL)
        vec3 numerator = ndf * geometry * fresnel;
        float denominator = 4.0 * max(dot(N, V), 0.0) *
                            max(dot(N, L), 0.0) + 0.0001;  // 防止除零
        vec3 specular = numerator / denominator;

        // 能量守恒：入射能量 = 反射能量 + 折射能量
        vec3 kS = fresnel;                    // 镜面反射比例（反射）
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);  // 漫反射比例（折射）
        // 金属没有漫反射，因为折射能量被立即吸收
        
        float nDotL = max(dot(N, L), 0.0);    //  Lambert余弦项
        
        // 累加：漫反射 + 镜面反射
        // kD * albedo / PI：Lambertian漫反射BRDF
        directLighting += (kD * baseColor / PI + specular) * radiance * nDotL;
    }

    // 环境光项：本章暂未引入IBL（基于图像的光照）
    // 用很小的环境项避免未受光区域完全变黑
    vec3 color = vec3(0.035) * baseColor * ao + directLighting;
    
    // Reinhard色调映射：压缩高动态范围到[0,1]
    color = color / (color + vec3(1.0));
    
    // Gamma校正：从线性空间转换到sRGB空间
    color = pow(color, vec3(1.0 / 2.2));
    
    FragColor = vec4(color, 1.0);
}
