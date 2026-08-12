#version 330 core

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT{
    vec3 normal;
}gs_in[];

uniform mat4 view;
uniform mat4 projection;

const float MAGNITUDE = 0.4; // 线段延伸的长度

//辅助函数：在指定顶点处发射一条沿法线方向的线段
void GenerateLine(int index)
{
    vec4 pos = gl_in[index].gl_Position;
    vec3 normal = normalize(gs_in[index].normal);

    //发射起点
    gl_Position = projection * view * pos;
    EmitVertex();

    //发射终点
    gl_Position = projection * view * (pos + vec4(normal * MAGNITUDE ,0.0));
    EmitVertex();

    //结束这条线段图元
    EndPrimitive();
}

void main()
{
    GenerateLine(0); // 为三角形的第 0 个顶点画法线
    GenerateLine(1); // 为三角形的第 1 个顶点画法线
    GenerateLine(2); // 为三角形的第 2 个顶点画法线
}