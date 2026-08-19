#version 330 core
layout (location = 0) in vec3 aPos;       // 顶点位置
layout (location = 1) in vec3 aNormal;    // 法线
layout (location = 2) in vec2 aTexCoords; // 纹理坐标
layout (location = 3) in vec3 aTangent;   // 切线
layout (location = 4) in vec3 aBitangent; // 副切线

uniform mat4 projection;  // 投影矩阵（相机 -> 裁剪）
uniform mat4 view;        // 视图矩阵（世界 -> 相机）
uniform mat4 model;       // 模型矩阵（局部 -> 世界）
uniform vec3 viewPos;     // 相机位置（世界空间）
uniform vec3 lightPos;    // 光源位置（世界空间）

// 传给片段着色器的接口块：所有向量都变换到【切线空间】。
out VS_OUT {
    vec3 FragPos;          // 片元世界坐标
    vec2 TexCoords;        // 纹理坐标
    vec3 TangentLightPos;  // 光源位置（切线空间）
    vec3 TangentViewPos;   // 相机位置（切线空间）
    vec3 TangentFragPos;   // 片元位置（切线空间）
} vs_out;

void main()
{
    // 把顶点变换到世界空间，供后面 TBN 变换和光照使用。
    vec3 fragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.FragPos = fragPos;
    vs_out.TexCoords = aTexCoords;

    // 法线、切线、副切线变换到世界空间（normalMatrix 防止非等比缩放扭曲方向）。
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 N = normalize(normalMatrix * aNormal);
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 B = normalize(normalMatrix * aBitangent);

    // 构造世界 -> 切线的 TBN 矩阵：把光线、视线、片元位置翻译进切线空间，
    // 以便片段着色器里能用法线贴图（存于切线空间）直接做点积。
    mat3 TBN = transpose(mat3(T, B, N));

    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos  = TBN * viewPos;
    vs_out.TangentFragPos  = TBN * fragPos;

    gl_Position = projection * view * vec4(fragPos, 1.0);
}

