#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec3 normal;
    vec3 fragPos;
    vec2 TexCoords;
} gs_in[];

out GS_OUT {
    vec3 normal;
    vec3 fragPos;
    vec2 TexCoords;
} gs_out;

uniform mat4 view;
uniform mat4 projection;
uniform float time;

// 根据三角形三个顶点计算世界空间面法线。
vec3 GetFaceNormal()
{
    vec3 edge1 = gs_in[1].fragPos - gs_in[0].fragPos;
    vec3 edge2 = gs_in[2].fragPos - gs_in[0].fragPos;
    return normalize(cross(edge1, edge2));
}

void main()
{
    vec3 faceNormal = GetFaceNormal();

    // 0～0.6 之间周期变化，使模型循环爆炸并复原。
    float phase = sin(time) * 0.5 + 0.5;
    float distance = phase * phase * 0.6;
    vec3 offset = faceNormal * distance;

    for(int i = 0; i < 3; ++i)
    {
        vec3 explodedPos = gs_in[i].fragPos + offset;

        gs_out.fragPos = explodedPos;
        gs_out.normal = gs_in[i].normal;
        gs_out.TexCoords = gs_in[i].TexCoords;

        gl_Position = projection * view * vec4(explodedPos, 1.0);
        EmitVertex();
    }

    EndPrimitive();
}
