#version 330 core
out vec4 FragColor; 
in vec2 TexCoords; 
uniform sampler2D hdrScene; 
uniform sampler2D bloomBlur; 
uniform float exposure;

vec3 ACESFilm(vec3 x)
{
    float a=2.51,b=0.03,c=2.43,d=0.59,e=0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e),0,1);
}

void main()
{
    vec3 hdr=texture(hdrScene,TexCoords).rgb+texture(bloomBlur,TexCoords).rgb;
    vec3 color=ACESFilm(hdr*exposure);

    //gamma校正
    FragColor=vec4(pow(color,vec3(1.0/2.2)),1);
}
