#version 330 core
// 将 HDR 全景图的经纬度投影采样到 cubemap 的当前面。
out vec4 FragColor;
in vec3 localPos;
uniform sampler2D equirectangularMap;
const vec2 invAtan=vec2(0.1591,0.3183);
vec2 sampleSphericalMap(vec3 v){ vec2 uv=vec2(atan(v.z,v.x),asin(v.y)); uv*=invAtan; uv+=0.5; return uv; }
void main(){
    // localPos 是当前 cubemap 面上片元对应的三维方向。
    vec2 uv = sampleSphericalMap(normalize(localPos));
    FragColor = vec4(texture(equirectangularMap, uv).rgb, 1.0);
}
