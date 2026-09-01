#version 330 core
layout(location=0) in vec3 aPos; 
layout(location=1) in vec3 aNormal; 
layout(location=2) in vec2 aTexCoords;

uniform mat4 model, view, projection;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

void main(){
    // SSAO 在观察空间计算：相机固定在原点，深度比较更直接。
    vec4 viewPosition = view * model * vec4(aPos, 1.0);
    vs_out.FragPos = viewPosition.xyz;
    vs_out.Normal = mat3(transpose(inverse(view * model))) * aNormal;
    //纹理坐标
    vs_out.TexCoords = aTexCoords;
    
    gl_Position = projection * viewPosition;
}
