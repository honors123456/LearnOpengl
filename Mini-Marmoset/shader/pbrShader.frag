#version 330 core

out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;
in vec3 LocalPos;

const float PI = 3.14159265359;

uniform vec3 cameraPos;         // 相机位置
uniform vec3 albedo;            // 基础颜色（反照率）

uniform vec3 lightPositions[4];  // 4个点光源位置
uniform vec3 lightColors[4];     // 4个点光源颜色

uniform float metallic;      // 金属度 [0,1]，0=非金属，1=金属
uniform float roughness;     // 粗糙度 [0,1]，0=光滑，1=粗糙

uniform float ao;            // 环境光遮蔽因子[0,1]

uniform int materialType;

//材质颜色
//1.暗褐色砖块
vec3 brickColor()
{
    vec3 n = normalize(LocalPos);

    // 球面UV映射：经度u [0,1]，纬度v [0,1]
    float u = atan(n.z, n.x) / (2.0 * PI) + 0.5;
    float v = asin(clamp(n.y, -1.0, 1.0)) / PI + 0.5;
    float row = floor(v * 8.0);

    // 交错砖缝：偶数行偏移0.065，模拟真实砖墙排列
    float shiftedU = u + mod(row, 2.0) * 0.065;

    // 砖缝遮罩：v方向8行，u方向13列，step函数创建硬边界
    float mortar = step(0.075, fract(v * 8.0)) * step(0.055, fract(shiftedU * 13.0));

    // 砖块颜色随机变化，增加真实感
    vec3 brick = albedo * (0.72 + 0.28 * fract(sin(dot(floor(LocalPos * 9.0), vec3(12.9898, 78.233, 45.164))) * 43758.5453));

    // 混合砖缝（深灰色）和砖块
    return mix(vec3(0.035, 0.045, 0.052), brick, mortar);
}

//2.草地颜色
vec3 grassColor()
{
    // 绿色粗糙表面：高频颜色变化模拟草地或苔藓
    float grain = (0.72 + 0.28 * fract(sin(dot(floor(LocalPos * 55.0), vec3(12.9898, 78.233, 45.164))) * 43758.5453));
    return albedo * mix(0.55, 1.35, grain);
}

//3.大理石颜色
vec3 marbleColor()
{
    // 红色大理石：低频波纹叠加亮色矿脉
    // 复合正弦波创建自然流动的纹理
    float veins = sin((LocalPos.x + LocalPos.y * 0.55 + sin(LocalPos.z * 7.0) * 0.18) * 13.0);
    // smoothstep创建柔和的矿脉边缘
    float veinMask = smoothstep(0.72, 0.98, abs(veins));
    return mix(albedo, vec3(0.72, 0.61, 0.57), veinMask * 0.75);
}

//4.纯色
vec3 solidColor()
{
    // 默认：直接返回基础颜色
    return albedo;
}

// GGX/Trowbridge-Reitz 法线分布函数
// 描述微平面法线朝向半程向量H的概率分布
// 粗糙度越高，分布越分散，高光越模糊
float distributionGGX(vec3 N, vec3 H, float value)
{
    float a = value * value;      // α = roughness2，Disney提出的重映射
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
    float k = (r * r) / 8.0;      // k = (roughness + 1)2 / 8，直接光照的推荐值
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
    //之前的光照模型分为：环境光 + 漫反射 + 高光反射
    //现在引入实际的物理光学原理就是：直接光照(PBR) + 间接光照(IBL)
    vec3 directLightingColor = vec3(0.0);
    vec3 indirectLightingColor = vec3(0.0);

    //工具
    vec3 N = normalize(Normal);     //法线
    vec3 V = normalize(cameraPos - WorldPos);   //视线方向

    vec3 baseColor = vec3(0.0);

    if(materialType == 0)
    {
        baseColor = brickColor();
    }
    else if(materialType == 2)
    {
        baseColor = grassColor();
    }
    else if(materialType == 4)
    {
        baseColor = marbleColor();
    }
    else
        baseColor = solidColor();

    for (int i = 0; i < 4; ++i) {
        //---------------------------- 直接光照 = 漫反射（折射） + 高光反射 (反射) -----------------------------
        //1.一束光从远处照过来，有距离衰减，有入射角度带来的衰减
        vec3 lightVec = lightPositions[i] - WorldPos;
        float distanceToLight = length(lightVec);
        vec3 L = lightVec / distanceToLight;

        vec3 radiance = lightColors[i] / max(distanceToLight * distanceToLight, 0.01);  //光线到达物体表面时的能量，光线能量与距离平方成反比
        float nDotL = max(dot(N, L), 0.0);                                          //

        //2.光线撞击到微平面，产生反射和折射
        vec3 H = normalize(V + L);      //半程向量H：视线和光线的中间方向，用于微平面BRDF

        vec3 f0 = mix(vec3(0.04), baseColor, metallic); //当前材质的反射率

        // DFG 三大函数
        float ndf = distributionGGX(N, H, roughness);        // 法线分布
        float geometry = geometrySmith(N, V, L, roughness);  // 几何遮蔽
        vec3 fresnel = fresnelSchlick(max(dot(H, V), 0.0), f0); // 菲涅尔

        //高光反射(Cook-Torrance 渲染方程)-----反射
        vec3 numerator = ndf * geometry * fresnel;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;  // 防止除零
        vec3 specular = numerator / denominator;

        //漫反射（能量守恒：入射能量 = 反射能量 + 折射能量）-----折射
        vec3 kS = fresnel;                              // 镜面反射比例（反射）
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);  // 漫反射比例（折射）

        vec3 diffuse = kD * baseColor / PI;

        //直接光照
        directLightingColor += (specular + diffuse)* radiance * nDotL;
    }

    //---------------------------- 间接光照 = 漫反射（折射） + 高光反射 (反射) -----------------------------
    // 环境光项：本章暂未引入IBL（基于图像的光照）
    // 用很小的环境项避免未受光区域完全变黑
    indirectLightingColor = vec3(0.035) * baseColor * ao;

    //最终光照
    vec3 color = indirectLightingColor + directLightingColor;

    // Reinhard色调映射：把物理计算中数值无上限的高动态范围（HDR，[0, +infty)）能量，平滑压缩到显示器能够显示的低动态范围（LDR，[0.0, 1.0]）压缩高动态范围到[0,1]
    color = color / (color + vec3(1.0));

    //Gamma校正：从线性空间转换到sRGB空间
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
