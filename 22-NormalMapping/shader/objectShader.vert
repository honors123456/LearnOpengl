#version 330 core
layout (location = 0) in vec3 aPos;    // 顶点位置
layout (location = 1) in vec3 aNormal;   // 法线
layout (location = 2) in vec2 aTexCoords;// 纹理坐标
layout (location = 3) in vec3 aTangent;  // 切线
layout (location = 4) in vec3 aBitangent;// 副切线

uniform mat4 model;       // 模型矩阵（局部 -> 世界）
uniform mat4 view;        // 视图矩阵（世界 -> 相机）
uniform mat4 projection;  // 投影矩阵（相机 -> 裁剪）
uniform vec3 lightPos;
uniform vec3 cameraPos;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentCameraPos;
    vec3 TangentFragPos;
} vs_out;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    vs_out.FragPos = vec3(worldPos);
    vs_out.TexCoords = aTexCoords;
    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 N = normalize(normalMatrix * aNormal);
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 B = normalize(normalMatrix * aBitangent);

    mat3 TBN = transpose(mat3(T, B, N));
    
    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentCameraPos = TBN * cameraPos;
    vs_out.TangentFragPos = TBN * fragPos;

    gl_Position = projection * view * worldPos;
}
