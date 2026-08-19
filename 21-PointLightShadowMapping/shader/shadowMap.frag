#version 330 core

in vec4 FragPos;          // 世界空间位置（来自几何着色器）
uniform vec3 lightPos;    // 点光源位置
uniform float far_plane;  // 远裁剪面距离（用于把真实距离归一化到 [0,1]）

void main()
{
    // 片元到光源的真实距离
    float lightDistance = length(FragPos.xyz - lightPos);
    // 归一化到 [0,1] 并直接写入深度（覆盖默认深度）
    // 这样深度贴图存的是"归一化的真实距离"，
    // 片元阶段采样后只需乘 far_plane 即可还原，避免默认深度与实际距离不一致
    gl_FragDepth = lightDistance / far_plane;
}
