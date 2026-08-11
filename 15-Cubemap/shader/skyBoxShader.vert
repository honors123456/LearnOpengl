#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords; // 输出 3D 向量作为纹理采样坐标

uniform mat4 projection;
uniform mat4 view;

void main() {
    TexCoords = aPos; // 顶点的 3D 位置向量直接作为 Cubemap 的采样方向！
    
    // 技巧 1：移除 View 矩阵中的位移成分（只保留旋转），让天空盒永远跟着相机走！
    mat4 viewNoTranslation = mat4(mat3(view));
    
    vec4 pos = projection * viewNoTranslation * vec4(aPos, 1.0);
    
    // 技巧 2：将 gl_Position 的 z 分量设为 w，透视除法后 z/w = 1.0，强制让天空盒深度为最大值 (1.0)，确保它永远被场景中其他物体遮挡
    gl_Position = pos.xyww;
}