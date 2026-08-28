#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D image;
uniform bool horizontal;

// 一维高斯卷积核。中心权重最大，距离中心越远权重越小。
const float weight[5] = float[](
    0.227027,
    0.1945946,
    0.1216216,
    0.054054,
    0.016216
);

void main()
{
    // 将一个像素转换成 UV 距离，例如 800 像素宽时 x 为 1.0 / 800.0。
    vec2 texelSize = 1.0 / textureSize(image, 0);

    // 先采样卷积核中心，也就是当前像素。
    vec3 result = texture(image, TexCoords).rgb * weight[0];

    // 采样中心两侧各四个像素，共组成 9 个采样点。
    for (int i = 1; i < 5; ++i)
    {
        // 水平模糊只改变 UV.x，垂直模糊只改变 UV.y。
        vec2 offset = horizontal
            ? vec2(texelSize.x * float(i), 0.0)
            : vec2(0.0, texelSize.y * float(i));

        // 正负方向对称采样，并乘以相同的高斯权重。
        result += texture(image, TexCoords + offset).rgb * weight[i];
        result += texture(image, TexCoords - offset).rgb * weight[i];
    }

    // 输出当前方向的模糊结果，供下一轮或最终 Bloom 合成使用。
    FragColor = vec4(result, 1.0);
}
