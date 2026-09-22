#version 330 core

out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;
in vec3 LocalPos;

const float PI = 3.14159265359;

uniform vec3 cameraPos;         // 相机位置
uniform vec3 albedo;            // 基础颜色（反照率）

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
    vec3 brick = albedo * (0.72 + 0.28 * fract(sin(dot(floor(LocalPos * 9.0)), vec3(12.9898, 78.233, 45.164))) * 43758.5453);

    // 混合砖缝（深灰色）和砖块
    return mix(vec3(0.035, 0.045, 0.052), brick, mortar);
}

//2.草地颜色
vec3 grassColor()
{
    // 绿色粗糙表面：高频颜色变化模拟草地或苔藓
    float grain = (0.72 + 0.28 * fract(sin(dot(floor(LocalPos * 55.0)), vec3(12.9898, 78.233, 45.164))) * 43758.5453);
    return albedo * mix(0.55, 1.35, grain);
}

//3.大理石颜色
vec3 marbleColor()
{
    // 红色大理石：低频波纹叠加亮色矿脉
    // 复合正弦波创建自然流动的纹理
    float veins = sin((LocalPos.x + LocalPos.y * 0.55 +
                       sin(LocalPos.z * 7.0) * 0.18) * 13.0);
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

void main()
{
    //之前的光照模型分为：环境光 + 漫反射 + 高管反射
    //现在引入实际的物理光学原理就是：直接光照(PBR) + 间接光照(IBL)

    //工具
    vec3 N = normalize(Normal);     //法线
    vec3 V = normalize(cameraPos - WorldPos);   //视线方向


    //直接光照

}
