#version 330 core

// 几何着色器：把一个三角形复制 6 份，分别写入立方体贴图的 6 个面
// 这样只需渲染一次，就能生成完整的 Omni Shadow Map
layout (triangles) in;                                    // 输入：三角形（3 个顶点）
layout (triangle_strip, max_vertices = 18) out;           // 输出：最多 18 个顶点（1 个三角形 x 6 个面）

uniform mat4 shadowMatrices[6];  // 光源在 6 个方向上的 view-projection 矩阵

out vec4 FragPos;  // 世界空间位置（片元阶段用来算到光源的距离）

void main()
{
    // 遍历立方体贴图的 6 个面（+X -X +Y -Y +Z -Z）
    for (int face = 0; face < 6; ++face)
    {
        gl_Layer = face;  // 把当前面输出的片元写到立方体贴图的第 face 个面

        // 复制输入三角形的 3 个顶点
        for (int i = 0; i < 3; ++i)
        {
            FragPos = gl_in[i].gl_Position;                  // 保留世界空间位置（不随面变化）
            gl_Position = shadowMatrices[face] * FragPos;    // 变换到当前面的裁剪空间
            EmitVertex();                                    // 发射当前顶点
        }

        EndPrimitive();  // 结束当前面的三角形
    }
}
