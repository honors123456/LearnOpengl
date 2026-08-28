#version 330 core
layout(location=0) out vec4 FragColor; 
layout(location=1) out vec4 BrightColor;

in vec3 FragPos; 
in vec3 Normal; 
in vec2 TexCoords; 
uniform sampler2D diffuseMap;
uniform vec3 lightPos; 
uniform vec3 viewPos;

void main(){
    
    //法线向量和入射向量
    vec3 N=normalize(Normal);
    vec3 L=normalize(lightPos-FragPos);

    //距离衰减因子
    float dist=length(lightPos-FragPos);
    float att=1.0 / (1.0 + 0.22*dist + 0.20*dist*dist);

    //漫反射贴图颜色
    vec3 diffuseColor=pow(texture(diffuseMap,TexCoords).rgb,vec3(2.2));

    //漫反射因子
    float d=max(dot(N,L),0.0);

    //最终颜色
    vec3 color=0.08*diffuseColor+diffuseColor*d*att;

    FragColor=vec4(color,1.0);

    //亮度计算公式
    float brightness=dot(color,vec3(0.2126,0.7152,0.0722));

    //亮度提取
    BrightColor=brightness>1.0?vec4(color,1.0):vec4(0.0);
}
