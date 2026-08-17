#version 330 core

out vec4 FragColor;

struct Light {
    int type;            // 0=平行光 1=点光源 2=聚光灯

    vec3 position;       // 点光源/聚光灯的位置

    vec3 direction;      // 平行光/聚光灯的照射方向（从光源射向场景）

    float cutOff;        // 聚光灯内锥角余弦
    float outerCutOff;   // 聚光灯外锥角余弦

    float constant;      // 衰减常数项
    float linear;        // 衰减一次项
    float quadratic;     // 衰减二次项

    vec3 ambient;        // 环境光
    vec3 diffuse;        // 漫反射
    vec3 specular;       // 高光
};

struct Material {
    sampler2D diffuse;   // 漫反射贴图
    sampler2D specular;  // 高光贴图
    float shininess;     // 高光锐利度
};

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace; //当前像素在光源视角下的裁剪空间坐标
} fs_in;


//材质颜色
uniform Material material;
//光源颜色
uniform Light light;
//摄像机位置
uniform vec3 cameraPos;

uniform sampler2D shadowMap;

// 平行光：方向全局一致，无位置、无衰减
vec3 CalcDirectional(vec3 N, vec3 V, vec3 diffuseColor, vec3 specularColor)
{
    //环境光
    vec3 L = normalize(-light.direction);          // 指向光源
    vec3 ambient  = light.ambient  * diffuseColor;

    //漫反射
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse  = light.diffuse  * diff * diffuseColor;

    //高光反射
    vec3 R = reflect(-L, N);
    float spec = pow(max(dot(V, R), 0.0), material.shininess);
    vec3 specular = light.specular * spec * specularColor;
    
    return ambient + diffuse + specular;
}

// 点光源：有位置、有衰减，无方向
vec3 CalcPoint(vec3 N, vec3 V, vec3 diffuseColor, vec3 specularColor)
{
    //计算距离衰减系数
    float dist = length(light.position - fs_in.FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

    //环境光
    vec3 L = normalize(light.position - fs_in.FragPos);  // 指向光源
    vec3 ambient  = light.ambient  * diffuseColor  * attenuation;

    //漫反射
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse  = light.diffuse  * diff * diffuseColor  * attenuation;

    //高光反射
    vec3 R = reflect(-L, N);
    float spec = pow(max(dot(V, R), 0.0), material.shininess);
    vec3 specular = light.specular * spec * specularColor * attenuation;

    return ambient + diffuse + specular;
}

// 聚光灯：有位置、有衰减，还受光锥限制
vec3 CalcSpot(vec3 N, vec3 V, vec3 diffuseColor, vec3 specularColor)
{
    //计算距离衰减系数
    float dist = length(light.position - fs_in.FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

    //环境光
    vec3 L = normalize(light.position - fs_in.FragPos);  // 指向光源
    vec3 ambient  = light.ambient  * diffuseColor  * attenuation;

     // 光锥判定：片元指向光源的方向 与 光源照射方向 的夹角
    float theta = dot(L, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    //漫反射
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse  = light.diffuse  * diff * diffuseColor  * attenuation * intensity;

    //高光反射
    vec3 R = reflect(-L, N);
    float spec = pow(max(dot(V, R), 0.0), material.shininess);
    vec3 specular = light.specular * spec * specularColor * attenuation * intensity;

    return ambient + diffuse + specular;
}

// 阴影计算：把片元变换到光源视角，与深度贴图比较，判断是否被遮挡
float ShadowCalculation(vec4 fragPosLightSpace)
{
    // 1. 透视除法：裁剪空间坐标 -> NDC [-1, 1]
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 2. NDC -> 纹理采样坐标 [0, 1]
    projCoords = projCoords * 0.5 + 0.5;

    // 3. 当前片元在光源视角下的深度
    float currentDepth = projCoords.z;

    // 4. 与深度贴图比较（加 bias 消除自阴影 Shadow Acne，用 3x3 PCF 柔化边缘）
    float bias = 0.005;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float closestDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    // 超出阴影贴图范围的部分不判定为阴影（避免物体外被误判成黑）
    if (projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

void main()
{
    vec3 N = normalize(fs_in.Normal);
    vec3 V = normalize(cameraPos - fs_in.FragPos);

    // 手动做 Gamma 校正的逆运算（转回线性空间）
    vec3 diffuseColor = pow(texture(material.diffuse, fs_in.TexCoords).rgb, vec3(2.2));
    vec3 specularColor = texture(material.specular, fs_in.TexCoords).rgb;

    vec3 color;
    if (light.type == 0)
        color = CalcDirectional(N, V, diffuseColor, specularColor);
    else if (light.type == 1)
        color = CalcPoint(N, V, diffuseColor, specularColor);
    else
        color = CalcSpot(N, V, diffuseColor, specularColor);

    // 计算阴影系数（点光源 type=1 全向发光，2D 阴影贴图不适用，直接跳过）
    float shadow = (light.type == 1) ? 0.0 : ShadowCalculation(fs_in.FragPosLightSpace);

    // 阴影区域只保留环境光，亮部保留完整光照
    vec3 finalColor = pow(shadow * light.ambient * diffuseColor + (1.0 - shadow) * color, vec3(1.0 / 2.2));

    FragColor = vec4(finalColor, 1.0);
}
