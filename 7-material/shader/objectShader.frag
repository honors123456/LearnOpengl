#version 330 core

out vec4 FragColor;

in vec3 fragPos;
in vec3 normal;


struct Light {
    vec3 position;  // 光源位置
    vec3 ambient;   // 光源的环境光强度（通常设低一点，如 vec3(0.2)）
    vec3 diffuse;   // 光源的漫反射强度（通常为光源的主色调，如 vec3(0.5)）
    vec3 specular;  // 光源的高光强度（通常设为全强，如 vec3(1.0)）
};
    
struct Material {
    vec3 ambient;   // 物体在环境光下反射什么颜色（通常与 diffuse 一致）
    vec3 diffuse;   // 物体在漫反射光下的固有色（例如珊瑚红）
    vec3 specular;  // 物体高光斑点的颜色（金属通常带有自身颜色，塑料高光多为白色）
    float shininess;// 高光反光度/散射半径（数值越大，高光斑点越小、越锐利）
};

//材质颜色
uniform Material material;
//光源颜色
uniform Light light;

//光源位置
uniform vec3 lightPos;

//摄像机位置
uniform vec3 cameraPos;

void main()
{
    //物体最终显示颜色 = 光源影响因子 * 物体表面颜色;
    //光源影响因子 = 环境光 + 漫反射 + 高光

    //方向向量
    vec3 N = normalize(normal);
    vec3 L = normalize(light.position - fragPos); //入射光向量的反向量
    vec3 R = reflect(-L,N);                 //反射光向量
    vec3 V = normalize(cameraPos - fragPos);//相机方向向量的反向量
    vec3 H = normalize(L+V);          //半角向量

    //环境光
    vec3 ambient = light.ambient * material.ambient;

    //漫反射
    float diff  = max(dot(N,L),0.0);  //入射光和法线夹角
    vec3 diffuse = light.diffuse * (diff * material.diffuse);

    //高光
    float spec = pow(max(dot(V,R),0.0),material.shininess);  //反射光和相机夹角,使用pow进行光线集中收束
    vec3 specular = light.specular * (spec * material.specular);

    //物体最终显示颜色
    vec3 objColor = ambient + diffuse + specular;

    FragColor = vec4(objColor,1.0);
}