#version 330 core
// ---- 静态属性：所有粒子共享同一个四边形 (Quad) 网格 ----
layout (location = 0) in vec3 aQuadVertex; // 面板 4 个顶点的局部坐标 (-0.5 ~ 0.5)
layout (location = 1) in vec2 aTexCoords;  // 纹理坐标

// ---- 动态实例化属性：每个粒子独一无二的数据 (每 1 个实例更新一次) ----
layout (location = 2) in vec3 aInstancePos;   // 粒子当前世界坐标
layout (location = 3) in vec4 aInstanceColor; // 粒子当前颜色与 Alpha
layout (location = 4) in float aInstanceScale; // 粒子当前大小

out vec2 TexCoords;
out vec4 ParticleColor;

uniform mat4 projection;
uniform mat4 view;

void main() {
    TexCoords = aTexCoords;
    ParticleColor = aInstanceColor;

    // ---- 广告牌 (Billboarding) 技巧：让粒子永远面向摄像机 ----
    // 从 View 矩阵中提取摄像机的右向量 (Right) 和上向量 (Up)
    vec3 CameraRight_worldspace = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 CameraUp_worldspace    = vec3(view[0][1], view[1][1], view[2][1]);

    // 根据粒子位置、尺寸以及摄像机方向，动态计算顶点位置
    vec3 vertexWorldPos = aInstancePos
        + CameraRight_worldspace * aQuadVertex.x * aInstanceScale
        + CameraUp_worldspace    * aQuadVertex.y * aInstanceScale;

    gl_Position = projection * view * vec4(vertexWorldPos, 1.0);
}