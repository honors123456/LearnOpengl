#version 330 core
//四个颜色附件纹理
layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedo;
layout(location = 3) out vec4 gSpecular;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D diffuseMap;
uniform bool useTexture;
uniform vec3 baseColor;
uniform float specularStrength;

void main()
{
    // 几何 Pass 不计算光照，只把后续光照需要的数据写进 G-Buffer。

    // 1. 输出世界空间坐标到 Attachment 0
    gPosition = vec4(fs_in.FragPos, 1.0);

    // 2. 输出世界空间法线到 Attachment 1
    gNormal = vec4(normalize(fs_in.Normal), 1.0);

    // 3. 输出漫反射颜色到 Attachment 2，立方体有贴图材质，地面没有使用baseColor
    vec3 albedo = useTexture
        ? texture(diffuseMap, fs_in.TexCoords).rgb
        : baseColor;
    gAlbedo = vec4(albedo, 1.0);

    //4. 输出高光强度到 Attachment 3
    gSpecular = vec4(specularStrength, 0.0, 0.0, 1.0);
}
