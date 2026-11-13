#version 330 core

layout (location = 0) in vec3 aPos; //局部空间的顶点
layout (location = 1) in vec3 aNorm;    //局部空间的法线坐标
layout (location = 2) in vec2 aTexture; //局部空间的纹理坐标

uniform mat4 model,view,projection;

out vec3 WorldPos;  //世界空间坐标
out vec3 Normal;    //世界空间的法线
out vec3 LocalPos;  //局部空间坐标

void main()
{
    LocalPos = aPos;
    WorldPos = vec3(model * vec4(aPos,1.0f));
    Normal = mat3(transpose(inverse(model))) * aNorm;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
