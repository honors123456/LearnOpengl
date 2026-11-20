#version 330 core
out vec4 FragColor;

in vec3 WorldPos;   //世界空间坐标
in vec3 Normal;     //世界空间法线
in vec3 LocalPos;   //局部空间坐标

uniform vec3 cameraPos; //相机的世界坐标
uniform vec3 lightPos;  //光源的世界坐标
uniform vec3 lightColor;    //光源的颜色

//物体材质属性
uniform vec3 albedo;    //基础颜色
uniform float metallic; //金属度[0,1]
uniform float roughness; //粗糙度
uniform float ao;   //环境光遮蔽
uniform float exposure;   // 曝光倍数

const float PI = 3.14159265359;


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
    //PBR就是引入实际的光学原理：直接光照（PBR）+ 间接光照（IBL）
    vec3 directLightingColor = vec3(0.0);
    vec3 indirectLightingColor = vec3(0.0);

    vec3 N = normalize(Normal);     //法线方向
    vec3 V = normalize(cameraPos - WorldPos);   //视线方向

    //直接光照 = 漫反射 + 高光反射
    float distance = length(lightPos - WorldPos);   //光线距离
    vec3 L = normalize(lightPos - WorldPos);        //光线方向

    vec3 radiance = lightColor / max(distance * distance,0.0f);     //光线衰减因子

    float nDotL = max(dot(N,L),0.0f);    //光线的入射角度

    vec3 f0 = mix(vec3(0.04), albedo, metallic); //当前材质的反射率

    vec3 H = normalize(V + L);  //半程向量：视线和光线的中间方向，用于微平面BRDF

    //DGF三大函数
    float ndf = distributionGGX(N, H, roughness);        // 法线分布
    float geometry = geometrySmith(N, V, L, roughness);  // 几何遮蔽
    vec3 fresnel = fresnelSchlick(max(dot(H, V), 0.0), f0); // 菲涅尔

    //高光反射,Cook-Torrance 镜面反射方程
    vec3 numerator = ndf * geometry * fresnel;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;  // 防止除零
    vec3 specular = numerator / denominator;

    //漫反射
    vec3 kS = fresnel;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI;

    directLightingColor = (specular + diffuse) * radiance * nDotL;

    //间接光照 = 漫反射 + 高光反射,暂时硬编码代替
    indirectLightingColor = vec3(0.035) * albedo * ao;

    //最终光照
    vec3 color = directLightingColor + indirectLightingColor;

    //Gamma校正
    // 曝光缩放，默认 1.0 时保持原画面
    color *= exposure;

    color = pow(color,vec3(1.0 / 2.2));

    FragColor = vec4(color,1.0);
}
