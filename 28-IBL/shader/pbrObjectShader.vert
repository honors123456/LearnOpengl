// PBR 物体顶点着色器。
// 输出世界空间位置、世界空间法线和 UV，供直接光照、IBL 和材质贴图使用。
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoords;

uniform mat4 model, view, projection;

out vec3 WorldPos;
out vec3 Normal;
out vec3 LocalPos;
out vec2 TexCoords;

void main()
{
    LocalPos = aPos;
    TexCoords = aTexCoords;
    WorldPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(WorldPos, 1.0);
}
