// 离屏环境捕获共用顶点着色器。
// 将单位立方体顶点送入 capture view/projection，localPos 保留采样方向。
#version 330 core
layout(location=0) in vec3 aPos;
out vec3 localPos;
uniform mat4 projection;
uniform mat4 view;
void main(){ localPos=aPos; gl_Position=projection*view*vec4(aPos,1.0); }
