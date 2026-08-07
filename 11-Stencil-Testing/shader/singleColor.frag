#version 330 core

out vec4 FragColor;

uniform vec3 outlineColor; // 描边颜色

void main()
{
    FragColor = vec4(outlineColor, 1.0);
}
