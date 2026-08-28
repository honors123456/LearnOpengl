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
    //世界空间坐标
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    //世界空间法线
    vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;
    //纹理坐标
    vs_out.TexCoords = aTexCoords;
    
    gl_Position = projection * view * vec4(vs_out.FragPos, 1.0);
}
