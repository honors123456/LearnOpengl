#version 330 core
layout (location = 0) in vec3 aPos;  // 顶点位置

uniform mat4 model;  // 模型矩阵（把顶点从局部空间变换到世界空间）

void main()
{
    // 只变换到世界空间，不乘 view/projection：
    // 1) 几何着色器需要"世界空间位置"来计算片元到光源的真实距离；
    // 2) 6 个面的 view-projection（shadowMatrices）由几何着色器统一应用。
    gl_Position = model * vec4(aPos, 1.0);
}
