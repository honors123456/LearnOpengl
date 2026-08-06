#version 330 core

out vec4 FragColor;

in vec3 fragPos;
in vec3 normal;
in vec2 TexCoords;

struct Material {
    sampler2D diffuse;   // 漫反射贴图
    sampler2D specular;  // 高光贴图
    float shininess;     // 高光锐利度
};

struct DirLight{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    
    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define NR_POINT_LIGHTS 4 // 定义点光源数量

//材质颜色
uniform Material material;
//摄像机位置
uniform vec3 cameraPos;
//平行光源
uniform DirLight dirLight;
//点光源
uniform PointLight pointLights[NR_POINT_LIGHTS];
//聚光灯
uniform SpotLight spotLight;

//计算平行光
vec3 CalcDirLight(DirLight light, vec3 N, vec3 V)
{
    vec3 L = normalize(-light.direction);
    vec3 R = reflect(-L,N);

    // 环境光复用漫反射贴图（LearnOpenGL 惯例）
    vec3 diffuseColor = texture(material.diffuse,TexCoords).rgb;
    vec3 specularColor = texture(material.specular,TexCoords).rgb;

    //环境光
    vec3 ambient = light.ambient * diffuseColor;

    //漫反射
    float diff = max(dot(L,N),0.0);
    vec3 diffuse = light.diffuse * diff * diffuseColor;

    //高光反射
    float spec = pow(max(dot(V,R),0.0),material.shininess);
    vec3 specular = light.specular * spec * specularColor;

    return ambient+diffuse+specular;
}

//计算点光源
vec3 CalcPointLight(PointLight light, vec3 N, vec3 fragPos, vec3 V)
{
    vec3 L = normalize(light.position - fragPos);
    vec3 R = reflect(-L,N);

    vec3 diffuseColor = texture(material.diffuse,TexCoords).rgb;
    vec3 specularColor = texture(material.specular,TexCoords).rgb;

    //距离衰减因子
    float distance = length(light.position - fragPos);
    float attenuation = 1.0/(light.constant + light.linear * distance + light.quadratic * distance * distance);

    //环境光
    vec3 ambient = light.ambient * diffuseColor * attenuation;

    //漫反射
    float diff = max(dot(L,N),0.0);
    vec3 diffuse = light.diffuse * diff * diffuseColor * attenuation;

    //高光反射
    float spec = pow(max(dot(V,R),0.0),material.shininess);
    vec3 specular = light.specular * spec * specularColor * attenuation;

    return ambient + diffuse + specular;
}

//计算聚光灯
vec3 CalcSpotLight(SpotLight light, vec3 N, vec3 fragPos, vec3 V)
{
    vec3 L = normalize(light.position - fragPos);
    vec3 R = reflect(-L,N);

    vec3 diffuseColor = texture(material.diffuse,TexCoords).rgb;
    vec3 specularColor = texture(material.specular,TexCoords).rgb;

    //距离衰减因子
    float distance = length(light.position - fragPos);
    float attenuation = 1.0/(light.constant + light.linear * distance + light.quadratic * distance * distance);

    //聚光灯边缘光滑
    float theta = dot(L,normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon,0.0,1.0);

    //环境光
    vec3 ambient = light.ambient * diffuseColor * attenuation;

    //漫反射
    float diff = max(dot(N,L),0.0);
    vec3 diffuse = light.diffuse * diff * diffuseColor * attenuation * intensity;

    //高光反射
    float spec = pow(max(dot(V,R),0.0),material.shininess);
    vec3 specular = light.specular * spec * specularColor * attenuation * intensity;

    return ambient + diffuse + specular;
}

void main()
{
    vec3 N = normalize(normal);
    vec3 V = normalize(cameraPos - fragPos);

    //平行光源
    vec3 dirL = CalcDirLight(dirLight,N,V);

    //累加点光源（从 0 开始累加）
    vec3 pointL = vec3(0.0);
    for(int i=0;i<NR_POINT_LIGHTS;i++)
    {
        pointL += CalcPointLight(pointLights[i],N,fragPos,V);
    }

    //聚光灯
    vec3 spotL = CalcSpotLight(spotLight,N,fragPos,V);

    //三种光源叠加
    FragColor = vec4(dirL + pointL + spotL, 1.0);
}
