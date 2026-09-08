// Skybox 片元着色器：从最终 environment cubemap 读取 HDR 环境颜色。
// 输出前使用 Reinhard tone mapping 和 Gamma 校正。
#version 330 core
out vec4 FragColor;
in vec3 localPos;
uniform samplerCube environmentMap;
void main(){ vec3 c=texture(environmentMap,normalize(localPos)).rgb; c=c/(c+vec3(1.0)); c=pow(c,vec3(1.0/2.2)); FragColor=vec4(c,1.0); }
