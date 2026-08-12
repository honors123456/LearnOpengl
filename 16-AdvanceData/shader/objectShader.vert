#version 330 core
layout (location = 0) in vec3 aCoord;   //顶点
layout (location = 1) in vec3 aNormal;  //法线
layout (location = 2) in vec2 aTexCoords; //纹理坐标


uniform mat4 model;
uniform mat3 normalMatrix;  //逆转置法线矩阵

out VS_OUT {
    vec3 normal;    //世界空间下的法线向量
    vec3 fragPos;   //世界空间下的片段坐标
    vec2 TexCoords;
} vs_out;

void main()
{
    //局部空间转为世界空间
    vec4 worldPos = model * vec4(aCoord,1.0);
    vs_out.fragPos = worldPos.xyz;

    //法线
    vs_out.normal = normalize(normalMatrix * aNormal);
    vs_out.TexCoords = aTexCoords;

    // 几何着色器需要在世界空间移动三角形，投影变换由下一阶段完成。
    gl_Position = worldPos;
}
