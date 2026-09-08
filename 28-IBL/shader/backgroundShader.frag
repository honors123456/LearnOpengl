#version 330 core
out vec4 FragColor;
in vec3 localPos;
uniform samplerCube environmentMap;
void main(){ vec3 c=texture(environmentMap,normalize(localPos)).rgb; c=c/(c+vec3(1.0)); c=pow(c,vec3(1.0/2.2)); FragColor=vec4(c,1.0); }
