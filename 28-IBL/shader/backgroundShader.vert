// Skybox 顶点着色器：去掉相机平移，只保留旋转，让环境无限远。
#version 330 core
layout(location=0) in vec3 aPos;
out vec3 localPos;
uniform mat4 projection;
uniform mat4 view;
void main(){ localPos=aPos; vec4 position=projection*view*vec4(aPos,1.0); gl_Position=position.xyww; }
