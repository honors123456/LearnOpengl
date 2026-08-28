#version 330 core

out vec4 FragColor;

void main()
{
    // HDR 发光值大于 1.0，经过亮部提取后会产生 Bloom。
    FragColor = vec4(8.0, 6.0, 3.0, 1.0);
}
