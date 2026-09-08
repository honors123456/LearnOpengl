// 直接光源标记球的片元着色器，将 HDR 光源强度映射到显示范围。
#version 330 core

out vec4 FragColor;

uniform vec3 lightColor;

void main()
{
    vec3 color = lightColor / (lightColor + vec3(1.0));
    FragColor = vec4(pow(color, vec3(1.0 / 2.2)), 1.0);
}
