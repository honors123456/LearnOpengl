#version 330 core
out vec4 FragColor;
in vec3 localPos;
uniform sampler2D equirectangularMap;
const vec2 invAtan=vec2(0.1591,0.3183);
vec2 sampleSphericalMap(vec3 v){ vec2 uv=vec2(atan(v.z,v.x),asin(v.y)); uv*=invAtan; uv+=0.5; return uv; }
void main(){ vec3 c=texture(equirectangularMap,sampleSphericalMap(normalize(localPos))).rgb; FragColor=vec4(c,1.0); }
