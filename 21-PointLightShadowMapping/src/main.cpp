#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../shader/shader.h"
#include "../Camera/Camera.h"

// 全局相机实例
Camera camera(
    glm::vec3(3.5f, 0.5f, 3.5f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    -135.0f,
    -17.0f
);
float lastX = 800.0f / 2.0f;
float lastY = 600.0f / 2.0f;
bool firstMouse = true;
bool draggingCube = false;
float cubeYaw = 20.0f;
float cubePitch = 20.0f;
int framebufferWidth = 800;
int framebufferHeight = 600;

// 时间补偿变量
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;
    framebufferWidth = width;
    framebufferHeight = height;
    glViewport(0, 0, width, height);
}

// 1. 鼠标移动回调函数：只有按住鼠标左键时，才用拖动距离旋转正方体。
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    (void)window;

    if (!draggingCube) {
        firstMouse = true;
        return;
    }

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // 注意：屏幕 Y 坐标轴自顶向下，故这里用 lastY - ypos

    lastX = xpos;
    lastY = ypos;

    cubeYaw -= xoffset * 0.2f;
    cubePitch -= yoffset * 0.2f;
}

// 2. 鼠标按键回调函数：左键按下开始拖动，松开停止拖动。
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    (void)window;
    (void)mods;

    if (button != GLFW_MOUSE_BUTTON_LEFT) {
        return;
    }

    if (action == GLFW_PRESS) {
        draggingCube = true;
        firstMouse = true;
    } else if (action == GLFW_RELEASE) {
        draggingCube = false;
        firstMouse = true;
    }
}

// 3. 滚轮回调函数
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window;
    (void)xoffset;

    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// 4. 键盘输入检测
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

int main(){
    if(glfwInit() == GLFW_FALSE){
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // 指定 OpenGL 版本和 Core Profile。
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口，后续所有 OpenGL 绘制都会输出到这个窗口。
    GLFWwindow* window=glfwCreateWindow(800,600,"Shadow Mapping - 点光源",nullptr,nullptr);
    if(window == nullptr)
    {
        std::cout << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // 把窗口绑定为当前线程的 OpenGL 上下文。
    glfwMakeContextCurrent(window);

    // 通过 GLFW 获取 OpenGL 函数地址，并交给 GLAD 加载。
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    //设置纹理数据
    //漫反射贴图
    cv::Mat diffuseImage = cv::imread(TEXTURE_FILE1, cv::IMREAD_UNCHANGED);
    if (diffuseImage.empty()) {
        std::cerr << "Failed to load texture: " << TEXTURE_FILE1 << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // OpenGL 的纹理原点位于左下角，因此先将 OpenCV 图像垂直翻转。
    cv::flip(diffuseImage, diffuseImage, 0);
    if (diffuseImage.channels() == 4) {
        cv::cvtColor(diffuseImage, diffuseImage, cv::COLOR_BGRA2RGBA);
    } else if (diffuseImage.channels() == 3) {
        cv::cvtColor(diffuseImage, diffuseImage, cv::COLOR_BGR2RGB);
    } else if (diffuseImage.channels() == 1) {
        cv::cvtColor(diffuseImage, diffuseImage, cv::COLOR_GRAY2RGB);
    } else {
        std::cerr << "Unsupported texture channel count: " << diffuseImage.channels() << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    //漫反射纹理数据上传gpu
    GLuint diffuseMap = 0;
    glGenTextures(1, &diffuseMap);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GLenum diffuseFormat = diffuseImage.channels() == 4 ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(diffuseFormat),
                 diffuseImage.cols, diffuseImage.rows, 0, diffuseFormat,
                 GL_UNSIGNED_BYTE, diffuseImage.data);
    glGenerateMipmap(GL_TEXTURE_2D);


    cv::Mat specularImage = cv::imread(TEXTURE_FILE2, cv::IMREAD_UNCHANGED);
    if (specularImage.empty()) {
        std::cerr << "Failed to load texture: " << TEXTURE_FILE2 << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // OpenGL 的纹理原点位于左下角，因此先将 OpenCV 图像垂直翻转。
    cv::flip(specularImage, specularImage, 0);
    if (specularImage.channels() == 4) {
        cv::cvtColor(specularImage, specularImage, cv::COLOR_BGRA2RGBA);
    } else if (specularImage.channels() == 3) {
        cv::cvtColor(specularImage, specularImage, cv::COLOR_BGR2RGB);
    } else if (specularImage.channels() == 1) {
        cv::cvtColor(specularImage, specularImage, cv::COLOR_GRAY2RGB);
    } else {
        std::cerr << "Unsupported texture channel count: " << specularImage.channels() << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    //高光纹理数据上传gpu
    GLuint specularMap = 0;
    glGenTextures(1, &specularMap);
    glBindTexture(GL_TEXTURE_2D, specularMap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 specularImage.cols, specularImage.rows, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, specularImage.data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    {

    //物体着色器
    Shader objShader(SHADER_DIR "/objectShader.vert", SHADER_DIR "/objectShader.frag");

    //光源着色器（画一个白色小方块标记光源位置，便于观察点光源/聚光灯）
    Shader lightShader(SHADER_DIR "/lightShader.vert", SHADER_DIR "/lightShader.frag");

    //深度着色器（第一遍：从光源视角渲染，只生成 Shadow Map）
    Shader shadowShader(SHADER_DIR "/shadowMap.vert", SHADER_DIR "/shadowMap.geom", SHADER_DIR "/shadowMap.frag");

    // 物体顶点数据：每个顶点依次包含位置、法线和纹理坐标。
    float vertices[] = {
        // 后平面
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,

        // 前平面
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,

        // 左平面
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,

        // 右平面
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,

        // 下平面
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        // 上平面
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    // 物体索引
    unsigned int objIndices[] = {
         0,  1,  2,   2,  3,  0,
         4,  5,  6,   6,  7,  4,
         8,  9, 10,  10, 11,  8,
        12, 13, 14,  14, 15, 12,
        16, 17, 18,  18, 19, 16,
        20, 21, 22,  22, 23, 20
    };
    const GLsizei indexCount = static_cast<GLsizei>(sizeof(objIndices) / sizeof(objIndices[0]));


    GLuint VAO,VBO,EBO;

    // VAO 记录顶点属性格式，以及当前绑定的 EBO。
    glGenVertexArrays(1,&VAO);
    glBindVertexArray(VAO);

    // VBO 保存顶点数组。
    glGenBuffers(1,&VBO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);

    // EBO 保存索引数组，供 glDrawElements 使用。
    glGenBuffers(1,&EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(objIndices),objIndices,GL_STATIC_DRAW);

    // 告诉 OpenGL 如何从每 8 个 float 中解析位置、法线和纹理坐标。
    glVertexAttribPointer(0,3,GL_FLOAT,false,8*sizeof(float),(void*)0);
    glVertexAttribPointer(1,3,GL_FLOAT,false,8*sizeof(float),(void*)(3*sizeof(float)));
    glVertexAttribPointer(2,2,GL_FLOAT,false,8*sizeof(float),(void*)(6*sizeof(float)));

    // 启用位置、法线和纹理坐标三个顶点属性。
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    // 解绑 VAO，避免后续误改它记录的状态。
    glBindVertexArray(0);

    // 光源位置小方块的 VAO（复用同一个 VBO，只需位置属性）
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // 地面顶点数据（与立方体相同格式：位置 + 法线 + UV）
    float groundVertices[] = {
        // 位置                法线              UV
        -5.0f, -0.5f, -5.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
         5.0f, -0.5f, -5.0f,  0.0f, 1.0f, 0.0f,  5.0f, 0.0f,
         5.0f, -0.5f,  5.0f,  0.0f, 1.0f, 0.0f,  5.0f, 5.0f,
         5.0f, -0.5f,  5.0f,  0.0f, 1.0f, 0.0f,  5.0f, 5.0f,
        -5.0f, -0.5f,  5.0f,  0.0f, 1.0f, 0.0f,  0.0f, 5.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f
    };

    GLuint groundVAO, groundVBO;
    glGenVertexArrays(1, &groundVAO);
    glGenBuffers(1, &groundVBO);
    glBindVertexArray(groundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    //3D 深度立方体帧缓冲
    // 1. 创建深度 Cubemap FBO
    unsigned int pointShadowFBO;
    glGenFramebuffers(1, &pointShadowFBO);

    // 2. 创建 3D 立方体深度纹理（阴影贴图分辨率 1024x1024，供 Omni Shadow Map 使用）
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthCubemap;
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

    // 为 Cubemap 的 6 个面分别开辟 1024x1024 的单通道浮点数显存空间
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, 
                    SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }

    // 设置采样滤波与环绕模式
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); // 👈 3D 纹理的 R 轴

    // 3. 将整个 Cubemap 挂载到 FBO 的深度附件上 (使用 glFramebufferTexture 而非 2D 版本的 API)
    glBindFramebuffer(GL_FRAMEBUFFER, pointShadowFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE); // 禁用颜色输出
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Point shadow framebuffer is not complete" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glEnable(GL_DEPTH_TEST);

    // 点光源位置（在立方体上方，阴影投到地面）
    glm::vec3 lightPos(0.0f, 2.5f, 0.0f);

    lastFrame = static_cast<float>(glfwGetTime());
    // ============ 主渲染循环：每帧执行一遍 ============
    while(!glfwWindowShouldClose(window))
    {
        // ---- 时间计算：记录本帧与上一帧的时间差 deltaTime ----
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;  // 每帧耗时（秒），用于保证移动速度与帧率无关
        lastFrame = currentFrame;

        // ---- 处理键盘输入（WASD 移动相机、ESC 退出）----
        processInput(window);

        // ---- 计算相机矩阵（两个物体共用同一相机）----
        glm::mat4 view = camera.GetViewMatrix();
        float aspect = framebufferHeight > 0
            ? static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight)
            : 1.0f;
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspect, 0.1f, 100.0f);

        // ---- 模型矩阵：旋转立方体（地面用单位矩阵即可）----
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(cubePitch), glm::vec3(1.0f, 0.0f, 0.0f));  // 鼠标拖动旋转（俯仰）
        model = glm::rotate(model, glm::radians(cubeYaw),   glm::vec3(0.0f, 1.0f, 0.0f));  // 鼠标拖动旋转（偏航）

        // ---- 光源空间矩阵（点光源：透视投影，覆盖 90 度视野的 6 个方向）----
        float shadowAspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;  // 阴影贴图宽高比
        float nearPlane = 0.1f;   // 近裁剪面
        float farPlane = 15.0f;   // 远裁剪面（决定阴影能投射的最远距离）
        // 透视投影矩阵（90 度 FOV，点光源每个面覆盖 90 度视角，6 面正好覆盖 360 度）
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), shadowAspect, nearPlane, farPlane);
        
        // 6 个方向的 Light View 矩阵（光源为眼睛，分别看向 +X -X +Y -Y +Z -Z）
        std::vector<glm::mat4> shadowTransforms;
        // +X (右)
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
        // -X (左)
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
        // +Y (上)
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)));
        // -Y (下)
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)));
        // +Z (后)
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
        // -Z (前)
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));

        // Pass 1：从光源视角渲染深度贴图（Omni Shadow Map）============
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT); // viewport 对齐阴影贴图分辨率
        glBindFramebuffer(GL_FRAMEBUFFER, pointShadowFBO); // 绑定深度 Cubemap FBO（不写颜色，只写深度）
        glClear(GL_DEPTH_BUFFER_BIT);                      // 清空上一帧的深度

        shadowShader.use();
        // 把 6 个面的 view-projection 传给几何着色器
        for (unsigned int i = 0; i < shadowTransforms.size(); ++i) {
            shadowShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        }
        shadowShader.setVec3("lightPos", lightPos);      // 片元阶段算到光源的距离
        shadowShader.setFloat("far_plane", farPlane);    // 归一化深度用
        // 立方体和地面都要写入深度 Cubemap（否则会漏掉阴影）
        glBindVertexArray(VAO);
        shadowShader.setMat4("model", model);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

        glBindVertexArray(groundVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Pass 2：从相机视角正常渲染
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // 解绑 FBO，切回默认帧缓冲
        glViewport(0, 0, framebufferWidth, framebufferHeight); // 恢复窗口尺寸 viewport
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //绘制光源位置小方块（白色，标记点光源在哪）
        lightShader.use();
        glm::mat4 lightModel = glm::mat4(1.0f);
        lightModel = glm::translate(lightModel, lightPos);
        lightModel = glm::scale(lightModel, glm::vec3(0.12f));
        lightShader.setMat4("model", lightModel);
        lightShader.setMat4("view", view);
        lightShader.setMat4("projection", projection);
        glBindVertexArray(lightCubeVAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // ---- 正式渲染：物体 + 房间（objShader 做完整光照 + 阴影比较）----
        objShader.use();
        objShader.setMat4("view",view);
        objShader.setMat4("projection",projection);
        objShader.setVec3("cameraPos",camera.Position);
        objShader.setInt("depthMap", 2);        // 深度 Cubemap 所在纹理单元
        objShader.setFloat("far_plane", farPlane); // 片元阶段还原真实深度
        objShader.setVec3("light.position", lightPos);
        objShader.setVec3("light.ambient",  glm::vec3(0.06f, 0.06f, 0.06f));
        objShader.setVec3("light.diffuse",  glm::vec3(1.2f, 1.2f, 1.2f));
        objShader.setVec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f));
        objShader.setInt("material.diffuse", 0);
        objShader.setInt("material.specular", 1);
        objShader.setFloat("material.shininess", 64.0f);

        // 绑定贴图：0=漫反射 1=高光 2=深度 Cubemap
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap); // cubemap 深度贴图绑定到纹理单元 2

        // 绘制立方体
        glBindVertexArray(VAO);
        objShader.setMat4("model", model);
        objShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        // 绘制地面
        glBindVertexArray(groundVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glfwSwapBuffers(window); // 交换前后缓冲，把本帧画面显示到窗口
        glfwPollEvents();        // 处理窗口事件（鼠标、键盘、关闭等）
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteVertexArrays(1, &groundVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &groundVBO);
    glDeleteTextures(1, &diffuseMap);
    glDeleteTextures(1, &specularMap);
    glDeleteTextures(1, &depthCubemap);
    glDeleteFramebuffers(1, &pointShadowFBO);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
