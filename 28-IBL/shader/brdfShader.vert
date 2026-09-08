// BRDF LUT 顶点着色器：将全屏 quad 的位置映射为 [0,1] UV。
#version 330 core
layout(location=0) in vec2 aPos;
out vec2 TexCoords;
void main(){ TexCoords=aPos*0.5+0.5; gl_Position=vec4(aPos,0,1); }
