#version 330 core
layout (location = 0) in vec3 aCoord;    // 顶点位置
layout (location = 1) in vec3 aNormal;   // 法线
layout (location = 2) in vec2 aTexCoords;// 纹理坐标

uniform mat4 model;       // 模型矩阵（局部 -> 世界）
uniform mat4 view;        // 视图矩阵（世界 -> 相机）
uniform mat4 projection;  // 投影矩阵（相机 -> 裁剪）
uniform mat3 normalMatrix;// 法线矩阵（逆转置，防止非等比缩放扭曲法线）

out VS_OUT {
    vec3 FragPos;   // 世界空间片段位置
    vec3 Normal;    // 世界空间法线
    vec2 TexCoords; // 纹理坐标
} vs_out;

void main()
{
    // 局部空间 -> 世界空间
    vs_out.FragPos = vec3(model * vec4(aCoord, 1.0));

    // 法线用逆转置矩阵变换（保持垂直于表面）
    vs_out.Normal = normalMatrix * aNormal;

    vs_out.TexCoords = aTexCoords;

    // 世界空间 -> 裁剪空间（相机视角）
    gl_Position = projection * view * model * vec4(aCoord, 1.0);
}
