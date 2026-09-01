#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gSpecular;
uniform vec3 viewPos;

struct Light {
    vec3 position;
    vec3 color;
};

const int LIGHT_COUNT = 100;
uniform Light lights[LIGHT_COUNT];

void main()
{
    //纹理坐标
    vec3 fragPos = texture(gPosition, TexCoords).rgb;
    //法线
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    //漫反射
    vec3 albedo = pow(texture(gAlbedo, TexCoords).rgb, vec3(2.2));
    float specularStrength = texture(gSpecular, TexCoords).r;

    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 lighting = 0.05 * albedo;

    for (int i = 0; i < LIGHT_COUNT; ++i) {
        vec3 lightDir = normalize(lights[i].position - fragPos);
        float diffuse = max(dot(normal, lightDir), 0.0);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float specular = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

        //光线衰减因子
        float distanceToLight = length(lights[i].position - fragPos);
        float attenuation = 1.0 / (1.0 + 0.22 * distanceToLight
                                  + 0.20 * distanceToLight * distanceToLight);

        lighting += (albedo * diffuse + vec3(specular * specularStrength))
                    * lights[i].color * attenuation;
    }

    // G-Buffer 中的颜色按 sRGB 保存，光照在线性空间计算，最后转回显示空间。
    FragColor = vec4(pow(lighting, vec3(1.0 / 2.2)), 1.0);
}
