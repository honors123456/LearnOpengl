#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox; // 特殊采样器：samplerCube

void main() {
    // 直接用 3D 方向向量去采样 Cubemap
    FragColor = texture(skybox, TexCoords);
}