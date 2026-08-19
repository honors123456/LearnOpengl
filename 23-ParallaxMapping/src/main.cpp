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
    glm::vec3(0.0f, 0.0f, 3.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    -90.0f,
    0.0f
);
float lastX = 800.0f / 2.0f;
float lastY = 600.0f / 2.0f;
bool firstMouse = true;
bool rotatingCamera = false;
int framebufferWidth = 800;
int framebufferHeight = 600;

// 时间补偿变量
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// 视差贴图强度（heightScale），用 Q/E 键在 0.0 ~ 1.0 之间调节。
float heightScale = 0.1f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;
    framebufferWidth = width;
    framebufferHeight = height;
    glViewport(0, 0, width, height);
}

// 1. 鼠标移动回调函数：只有按住鼠标左键时，才用拖动距离旋转相机。
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    (void)window;

    if (!rotatingCamera) {
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

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// 2. 鼠标按键回调函数：左键按下开始旋转相机，松开停止旋转。
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    (void)window;
    (void)mods;

    if (button != GLFW_MOUSE_BUTTON_LEFT) {
        return;
    }

    if (action == GLFW_PRESS) {
        rotatingCamera = true;
        firstMouse = true;
    } else if (action == GLFW_RELEASE) {
        rotatingCamera = false;
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

    // Q/E 键调节视差强度 heightScale（0.0 ~ 1.0），实时观察视差深度变化。
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        if (heightScale > 0.0f)
            heightScale -= 0.0005f;
        else
            heightScale = 0.0f;
    } else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        if (heightScale < 1.0f)
            heightScale += 0.0005f;
        else
            heightScale = 1.0f;
    }
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
    GLFWwindow* window=glfwCreateWindow(800,600,"Parallax Mapping - Q/E调节视差强度",nullptr,nullptr);
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

    // ============ 加载三张纹理：漫反射 / 法线 / 高度 ============
    // 用 lambda 封装通用加载流程：OpenCV 读图 -> 垂直翻转 -> BGR->RGB -> 上传 GPU。
    auto loadTexture = [](const char* path) -> GLuint {
        cv::Mat img = cv::imread(path, cv::IMREAD_UNCHANGED);
        if (img.empty()) {
            std::cerr << "Failed to load texture: " << path << std::endl;
            return 0u;
        }
        // OpenGL 的纹理原点位于左下角，因此先将 OpenCV 图像垂直翻转。
        cv::flip(img, img, 0);
        if (img.channels() == 4) {
            cv::cvtColor(img, img, cv::COLOR_BGRA2RGBA);
        } else if (img.channels() == 3) {
            cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
        } else if (img.channels() == 1) {
            cv::cvtColor(img, img, cv::COLOR_GRAY2RGB);
        } else {
            std::cerr << "Unsupported texture channel count: " << img.channels() << std::endl;
            return 0u;
        }

        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        GLenum format = img.channels() == 4 ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(format),
                     img.cols, img.rows, 0, format, GL_UNSIGNED_BYTE, img.data);
        glGenerateMipmap(GL_TEXTURE_2D);
        return tex;
    };

    // 0 = 漫反射贴图（砖墙颜色）；1 = 法线贴图；2 = 高度图（黑白，R 通道存高度）。
    GLuint diffuseMap = loadTexture(DIFFUSE_TEXTURE_FILE);
    GLuint normalMap  = loadTexture(NORMAL_TEXTURE_FILE);
    GLuint heightMap  = loadTexture(HEIGHT_TEXTURE_FILE);
    if (diffuseMap == 0u || normalMap == 0u || heightMap == 0u) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    {

    //物体着色器
    Shader objShader(SHADER_DIR "/objectShader.vert", SHADER_DIR "/objectShader.frag");

    //光源着色器（画一个白色小墙片标记光源位置）
    Shader lightShader(SHADER_DIR "/lightShader.vert", SHADER_DIR "/lightShader.frag");


    // 墙面顶点数据：每个顶点依次包含位置、法线、纹理坐标、切线和副切线。
    float vertices[] = {
        // position           normal          texcoords  tangent         bitangent
        -2.0f, -1.5f, 0.0f,   0.0f, 0.0f, 1.0f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
         2.0f, -1.5f, 0.0f,   0.0f, 0.0f, 1.0f,  2.0f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
         2.0f,  1.5f, 0.0f,   0.0f, 0.0f, 1.0f,  2.0f, 2.0f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        -2.0f,  1.5f, 0.0f,   0.0f, 0.0f, 1.0f,  0.0f, 2.0f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f
    };

    // 墙面索引
    unsigned int objIndices[] = {
         0, 1, 2,   2, 3, 0
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

    // 告诉 OpenGL 如何从每 14 个 float 中解析位置、法线、纹理坐标、切线和副切线。
    glVertexAttribPointer(0,3,GL_FLOAT,false,14*sizeof(float),(void*)0);
    glVertexAttribPointer(1,3,GL_FLOAT,false,14*sizeof(float),(void*)(3*sizeof(float)));
    glVertexAttribPointer(2,2,GL_FLOAT,false,14*sizeof(float),(void*)(6*sizeof(float)));
    glVertexAttribPointer(3,3,GL_FLOAT,false,14*sizeof(float),(void*)(8*sizeof(float)));
    glVertexAttribPointer(4,3,GL_FLOAT,false,14*sizeof(float),(void*)(11*sizeof(float)));

    // 启用位置、法线、纹理坐标、切线和副切线五个顶点属性。
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    // 解绑 VAO，避免后续误改它记录的状态。
    glBindVertexArray(0);

    // 光源位置小白点的 VAO：只放一个原点顶点，通过 model 平移到 lightPos。
    float lightVertex[] = {
        0.0f, 0.0f, 0.0f
    };
    unsigned int lightVAO, lightVBO;
    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lightVertex), lightVertex, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glEnable(GL_DEPTH_TEST);

    // 点光源位于墙面前方，偏右上角，能明显照出砖缝凹凸。
    glm::vec3 lightPos(0.5f, 1.0f, 0.3f);

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

        // ---- 计算相机矩阵 ----
        glm::mat4 view = camera.GetViewMatrix();
        float aspect = framebufferHeight > 0
            ? static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight)
            : 1.0f;
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspect, 0.1f, 100.0f);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //绘制光源位置小白点（只作为位置标记，不代表真实光源大小）
        lightShader.use();
        glm::mat4 lightModel = glm::mat4(1.0f);
        lightModel = glm::translate(lightModel, lightPos);
        lightShader.setMat4("model", lightModel);
        lightShader.setMat4("view", view);
        lightShader.setMat4("projection", projection);
        glPointSize(6.0f);
        glBindVertexArray(lightVAO);
        glDrawArrays(GL_POINTS, 0, 1);
        glBindVertexArray(0);

        // ---- 正式渲染：砖墙（objShader 做切线空间视差贴图光照）----
        objShader.use();
        objShader.setMat4("view",view);
        objShader.setMat4("projection",projection);
        objShader.setVec3("viewPos",camera.Position);
        objShader.setVec3("lightPos", lightPos);
        objShader.setVec3("light.ambient",  glm::vec3(0.08f, 0.08f, 0.08f));
        objShader.setVec3("light.diffuse",  glm::vec3(1.2f, 1.15f, 1.05f));
        objShader.setVec3("light.specular", glm::vec3(0.7f, 0.7f, 0.7f));

        objShader.setInt("material.diffuse", 0);
        objShader.setInt("material.normalMap", 1);
        objShader.setInt("material.depthMap", 2);
        objShader.setFloat("material.shininess", 64.0f);
        objShader.setFloat("heightScale", heightScale);

        // 绑定三张贴图：0=漫反射，1=法线，2=高度图
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, heightMap);

        // 绘制墙面：固定不旋转（模型矩阵保持单位矩阵），方便从正面观察视差贴图效果。
        glBindVertexArray(VAO);
        glm::mat4 model = glm::mat4(1.0f);
        objShader.setMat4("model", model);
        objShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window); // 交换前后缓冲，把本帧画面显示到窗口
        glfwPollEvents();        // 处理窗口事件（鼠标、键盘、关闭等）
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &lightVBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &diffuseMap);
    glDeleteTextures(1, &normalMap);
    glDeleteTextures(1, &heightMap);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
