#version 330 core

out vec4 FragColor;

in vec3 fragPos;
in vec3 normal;
in vec2 TexCoords;

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


//材质颜色
uniform Material material;
//光源颜色
uniform Light light;
//摄像机位置
uniform vec3 cameraPos;

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
    float dist = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

    //环境光
    vec3 L = normalize(light.position - fragPos);  // 指向光源
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
    float dist = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

    //环境光
    vec3 L = normalize(light.position - fragPos);  // 指向光源
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

void main()
{
    vec3 N = normalize(normal);
    vec3 V = normalize(cameraPos - fragPos);

    // 手动做 Gamma 校正的逆运算（转回线性空间）
    vec3 diffuseColor = pow(texture(material.diffuse, TexCoords).rgb, vec3(2.2));
    vec3 specularColor = texture(material.specular, TexCoords).rgb;

    vec3 color;
    if (light.type == 0)
        color = CalcDirectional(N, V, diffuseColor, specularColor);
    else if (light.type == 1)
        color = CalcPoint(N, V, diffuseColor, specularColor);
    else
        color = CalcSpot(N, V, diffuseColor, specularColor);

    // 片段着色器的最后一步：Gamma 校正
    vec3 finalColor = pow(color, vec3(1.0 / 2.2));
    FragColor = vec4(finalColor, 1.0);
}
