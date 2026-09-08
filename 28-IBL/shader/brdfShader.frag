// BRDF 积分 LUT。
// 横坐标是 NdotV，纵坐标是 roughness，输出 Fresnel 缩放项和偏移项。
#version 330 core
out vec2 FragColor;
in vec2 TexCoords;
const float PI = 3.14159265359;

float radicalInverseVdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}
vec2 hammersley(uint i, uint count){ return vec2(float(i)/float(count), radicalInverseVdC(i)); }
vec3 importanceSampleGGX(vec2 xi, vec3 N, float roughness)
{
    float a=roughness*roughness;
    float phi=2.0*PI*xi.x;
    float cosTheta=sqrt((1.0-xi.y)/(1.0+(a*a-1.0)*xi.y));
    float sinTheta=sqrt(max(1.0-cosTheta*cosTheta,0.0));
    vec3 H=vec3(cos(phi)*sinTheta,sin(phi)*sinTheta,cosTheta);
    vec3 up=abs(N.z)<0.999?vec3(0,0,1):vec3(1,0,0);
    vec3 tangent=normalize(cross(up,N));
    return normalize(tangent*H.x+cross(N,tangent)*H.y+N*H.z);
}
float geometrySchlickGGX(float nDotV,float roughness){ float a=roughness; float k=(a*a)/2.0; return nDotV/(nDotV*(1.0-k)+k); }
float geometrySmith(vec3 N,vec3 V,vec3 L,float roughness){ return geometrySchlickGGX(max(dot(N,V),0.0),roughness)*geometrySchlickGGX(max(dot(N,L),0.0),roughness); }
void main()
{
    float nDotV=TexCoords.x, roughness=TexCoords.y;
    vec3 V=vec3(sqrt(max(1.0-nDotV*nDotV,0.0)),0.0,nDotV), N=vec3(0,0,1);
    float A=0.0,B=0.0;
    const uint sampleCount=1024u;
    for(uint i=0u;i<sampleCount;++i){
        vec3 H=importanceSampleGGX(hammersley(i,sampleCount),N,roughness);
        vec3 L=normalize(2.0*dot(V,H)*H-V);
        float nDotL=max(L.z,0.0), nDotH=max(H.z,0.0), vDotH=max(dot(V,H),0.0);
        if(nDotL>0.0){ float G=geometrySmith(N,V,L,roughness); float gVis=(G*vDotH)/max(nDotH*nDotV,0.0001); float Fc=pow(1.0-vDotH,5.0); A+=(1.0-Fc)*gVis; B+=Fc*gVis; }
    }
    FragColor=vec2(A,B)/float(sampleCount);
}
