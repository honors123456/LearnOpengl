#version 330 core
layout (location = 0) in vec3 aCoord;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;
out vec3 ourColor;

void main()
{
    gl_Position = projection * view * model * vec4(aCoord,1.0);
    ourColor = aColor;
    TexCoord = aTexCoord;
}