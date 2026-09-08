#version 330 core

// 深度着色器的片元阶段：
// 深度值由 GPU 光栅化阶段根据 gl_Position 自动写入深度缓冲区，
// 这里不需要做任何计算，也不需要输出颜色（FBO 已 glDrawBuffer(GL_NONE)）。
void main()
{
    
}