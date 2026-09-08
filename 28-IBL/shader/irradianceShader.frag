#version 330 core
// 对环境 cubemap 做漫反射 irradiance convolution。
// 每个输出 texel 表示一个法线方向接收到的半球环境光总量。
out vec4 FragColor;
in vec3 localPos;
uniform samplerCube environmentMap;
const float PI=3.14159265359;
void main(){
    vec3 N=normalize(localPos); vec3 up=vec3(0,1,0);
    vec3 right=normalize(cross(up,N)); up=normalize(cross(N,right));
    vec3 irradiance=vec3(0); float sampleDelta=0.025; float nr=0;
    for(float p=0;p<2.0*PI;p+=sampleDelta)
        for(float t=0;t<0.5*PI;t+=sampleDelta){
            vec3 tangent=vec3(sin(t)*cos(p),sin(t)*sin(p),cos(t));
            vec3 sampleDir=tangent.x*right+tangent.y*up+tangent.z*N;
            irradiance+=texture(environmentMap,sampleDir).rgb*cos(t)*sin(t);
            nr++;
        }
    FragColor=vec4(PI*irradiance*(1.0/nr),1);
}
