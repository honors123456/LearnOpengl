#version 330 core

out vec4 FragColor;

in GS_OUT {
    vec3 normal;
    vec3 fragPos;
    vec2 TexCoords;
} fs_in;

struct Light {
    vec3 position;  // 光源位置
    vec3 ambient;   // 光源的环境光强度（通常设低一点，如 vec3(0.2)）
    vec3 diffuse;   // 光源的漫反射强度（通常为光源的主色调，如 vec3(0.5)）
    vec3 specular;  // 光源的高光强度（通常设为全强，如 vec3(1.0)）
};
    
struct Material{
    sampler2D diffuse; //漫反射贴图
    sampler2D specular; //高光贴图
    float     shininess;//高光散射半径/粗糙度
};

uniform samplerCube skybox;


//材质颜色
uniform Material material;
//光源颜色
uniform Light light;
//摄像机位置
uniform vec3 cameraPos;

void main()
{
    //物体最终显示颜色 = 光源影响因子 * 物体表面颜色;
    //光源影响因子 = 环境光 + 漫反射 + 高光

    //方向向量
    vec3 N = normalize(fs_in.normal);
    vec3 L = normalize(light.position - fs_in.fragPos); //入射光向量的反向量
    vec3 R = reflect(-L,N);                       //光源入射方向对应的反射光向量
    vec3 V = normalize(cameraPos - fs_in.fragPos);      //从片段指向相机的观察方向

    vec3 diffuseColor = texture(material.diffuse, fs_in.TexCoords).rgb;
    vec3 specularColor = texture(material.specular, fs_in.TexCoords).rgb;

    //环境光
    vec3 ambient = light.ambient * diffuseColor;

    //漫反射
    float diff  = max(dot(N,L),0.0);  //入射光和法线夹角
    vec3 diffuse = light.diffuse * diff * diffuseColor;

    //高光
    float spec = pow(max(dot(V,R),0.0),material.shininess);  //反射光和相机夹角,使用pow进行光线集中收束
    vec3 specular = light.specular * spec * specularColor;

    //物体最终显示颜色
    vec3 objColor = ambient + diffuse + specular;

    FragColor = vec4(objColor.rgb, 1.0);
}
