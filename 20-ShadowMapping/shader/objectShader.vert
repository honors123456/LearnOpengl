#version 330 core
layout (location = 0) in vec3 aCoord;   //顶点
layout (location = 1) in vec3 aNormal;  //法线
layout (location = 2) in vec2 aTexCoords; //纹理坐标


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;  //逆转置法线矩阵
uniform mat4 lightSpaceMatrix; // 光源的 Projection * View 矩阵（阴影映射用）

out VS_OUT {
    vec3 FragPos;           //世界空间下的片段坐标
    vec3 Normal;            //世界空间下的法线向量
    vec2 TexCoords;
    vec4 FragPosLightSpace; //当前片段在光源视角下的裁剪空间坐标
} vs_out;

void main()
{
    //局部空间转为世界空间
    vs_out.FragPos = vec3(model * vec4(aCoord,1.0));

    //法线
    vs_out.Normal = normalMatrix * aNormal;
    vs_out.TexCoords = aTexCoords;

    //将世界坐标变换到光源裁剪空间（供片元阶段做阴影比较）
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);

    //转换为裁剪空间坐标
    gl_Position = projection * view * model * vec4(aCoord,1.0);
}
