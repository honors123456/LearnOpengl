#version 330 core
layout (location = 0) in vec3 aCoord;   //顶点
layout (location = 1) in vec3 aNormal;  //法线


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;  //逆转置法线矩阵

out vec3 normal;    //世界空间下的法线向量
out vec3 fragPos;   //世界空间下的片段坐标

void main()
{
    //局部空间转为世界空间
    fragPos = vec3(model * vec4(aCoord,1.0));

    //法线
    normal = normalMatrix * aNormal;

    //转换为裁剪空间坐标
    gl_Position = projection * view * model * vec4(aCoord,1.0);
}
