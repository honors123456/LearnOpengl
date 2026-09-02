#version 330 core

out float FragColor;
in vec2 TexCoords;

uniform sampler2D ssaoInput;

void main()
{
    // 对原始 SSAO 做 4x4 盒式模糊，消除随机采样产生的颗粒噪声。
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) {
        for (int y = -2; y < 2; ++y) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssaoInput, TexCoords + offset).r;
        }
    }
    FragColor = result / 16.0;
}
