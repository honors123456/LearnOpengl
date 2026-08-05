#version 330 core

out vec4 FragColor;

in vec3 fragPos;
in vec3 normal;

//对象物体表面颜色
uniform vec3 objectColor;

//光源颜色
uniform vec3 lightColor;

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
    vec3 L = normalize(lightPos - fragPos); //入射光向量的反向量
    vec3 R = reflect(-L,N);                 //反射光向量
    vec3 V = normalize(cameraPos - fragPos);//相机方向向量的反向量
    vec3 H = normalize(L+V);          //半角向量

    //环境光
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    //漫反射
    float diff  = max(dot(N,L),0.0);  //入射光和法线夹角
    vec3 diffuse = diff * lightColor;

    //高光
    float specularStrength = 0.5;
    //float spec = pow(max(dot(V,R),0.0),32);  //反射光和相机夹角,使用pow进行光线集中收束
    //Phong模型,反射光和相机夹角,在高光夹角大于90时，有断层切线，所以引入Blinn-Phong模型，使用半角向量和法线的夹角
    float spec = pow(max(dot(N,H),0.0),64);
    vec3 specular = specularStrength * spec * lightColor;

    //物体最终显示颜色
    vec3 objColor = (ambient + diffuse + specular) * objectColor;

    FragColor = vec4(objColor,1.0);
}