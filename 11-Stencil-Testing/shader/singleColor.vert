#version 330 core
layout (location = 0) in vec3 aPos;    //顶点
layout (location = 1) in vec3 aNormal; //法线（用于外扩）

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float outlineWidth = 0.03; // 沿法线挤出的外扩距离（描边粗细）

void main()
{
    // 将顶点沿法线方向外扩，形成紧贴物体的外扩轮廓
    vec3 extrudedPos = aPos + aNormal * outlineWidth;
    gl_Position = projection * view * model * vec4(extrudedPos, 1.0);
}
