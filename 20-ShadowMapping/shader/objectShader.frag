#version 330 core

out vec4 FragColor;

struct Light {
    vec3 direction;      // 平行光照射方向（从光源射向场景）
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

void main()
{
    // ========== 1. 基础向量 ==========
    // N：片元法线（世界空间，插值后）
    vec3 N = normalize(fs_in.Normal);
    // V：片元指向相机的方向
    vec3 V = normalize(cameraPos - fs_in.FragPos);

    // ========== 2. 纹理颜色（Gamma 逆校正：sRGB -> 线性空间）==========
    vec3 diffuseColor  = pow(texture(material.diffuse,  fs_in.TexCoords).rgb, vec3(2.2));
    vec3 specularColor = texture(material.specular, fs_in.TexCoords).rgb;

    // ========== 3. 环境光：恒定，不受阴影遮挡 ==========
    vec3 ambient = light.ambient * diffuseColor;

    // ========== 4. 漫反射（直接光）==========
    vec3 L = normalize(-light.direction);          // 片元指向光源的方向
    float diff = max(dot(N, L), 0.0);              // 法线与光线的夹角余弦
    vec3 diffuse = light.diffuse * diff * diffuseColor;

    // ========== 5. 高光（直接光）==========
    vec3 R = reflect(-L, N);                       // 光线关于法线的反射方向
    float spec = pow(max(dot(V, R), 0.0), material.shininess);
    vec3 specular = light.specular * spec * specularColor;

    // 直接光 = 漫反射 + 高光（这部分会被阴影遮挡）
    vec3 direct = diffuse + specular;

    // ========== 6. 阴影判断：与深度贴图比较 ==========
    // 6.1 透视除法：光源裁剪空间 -> NDC [-1, 1]
    vec3 projCoords = fs_in.FragPosLightSpace.xyz / fs_in.FragPosLightSpace.w;
    // 6.2 NDC -> 纹理采样坐标 [0, 1]
    projCoords = projCoords * 0.5 + 0.5;
    // 6.3 当前片元在光源视角下的深度，就是正方体在光源视角下的深度信息
    float currentDepth = projCoords.z;
    // 6.4 与深度贴图比较：加 bias 消除自阴影（Shadow Acne），3x3 PCF 柔化边缘
    float bias = 0.005;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0); // 单个纹素的大小
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            // 采样周围 3x3 邻域的最近深度
            //深度纹理在当前像素纹理坐标下的深度信息，因为pass 1中开启了深度测试，所以只保留了离光源最近的深度信息
            float closestDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            // 当前深度比记录值更远 -> 被遮挡 -> 记 1，否则记 0
            shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0; // 取 9 个采样点的平均值，得到柔化的阴影系数
    // 超出阴影贴图范围的部分不判定为阴影（避免物体外被误判成黑）
    if (projCoords.z > 1.0)
        shadow = 0.0;

    // ========== 7. 合成颜色 ==========
    // 环境光恒定 + 直接光按阴影系数衰减
    vec3 lighting = ambient + (1.0 - shadow) * direct;
    // Gamma 校正：线性空间 -> sRGB
    vec3 finalColor = pow(lighting, vec3(1.0 / 2.2));

    FragColor = vec4(finalColor, 1.0);
}
