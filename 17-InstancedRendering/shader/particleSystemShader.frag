#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec4 ParticleColor;

void main() {
    // 用纹理坐标画一个软边圆：中心不透明，边缘平滑淡出，模拟雪花的柔边。
    float dist = length(TexCoords - vec2(0.5));
    float alpha = 1-smoothstep(0.0, 0.5, dist);
    alpha *= ParticleColor.a;

    // 几乎透明的片段直接丢弃，避免无效写入。
    if (alpha < 0.01)
        discard;

    FragColor = vec4(ParticleColor.rgb, alpha);
}