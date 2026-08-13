#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>
#include "../shader/shader.h"
#include "../Camera/Camera.h"

// 全局相机实例
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = 800.0f / 2.0f;
float lastY = 600.0f / 2.0f;
bool firstMouse = true;
bool draggingCamera = false;
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

// 1. 鼠标移动回调函数：只有按住鼠标左键时，才用拖动距离旋转相机视角。
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    (void)window;

    if (!draggingCamera) {
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

// 2. 鼠标按键回调函数：左键按下开始拖动，松开停止拖动。
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    (void)window;
    (void)mods;

    if (button != GLFW_MOUSE_BUTTON_LEFT) {
        return;
    }

    if (action == GLFW_PRESS) {
        draggingCamera = true;
        firstMouse = true;
    } else if (action == GLFW_RELEASE) {
        draggingCamera = false;
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口，后续所有 OpenGL 绘制都会输出到这个窗口。
    GLFWwindow* window = glfwCreateWindow(800, 600, "Snow Particles", nullptr, nullptr);
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

    // 雪花着色器：顶点着色器负责广告牌，片元着色器画出软边圆。
    Shader particleShader(SHADER_DIR "/particleSystemShader.vert", SHADER_DIR "/particleSystemShader.frag");

    // 单个广告牌四边形：4 个顶点，用三角形带拼成两个三角形。
    // 每顶点依次：局部坐标(-0.5~0.5) + 纹理坐标(0~1)
    float quadVertices[] = {
        // 位置              纹理坐标
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f
    };

    GLuint quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    // location 0：四边形局部坐标
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // location 1：纹理坐标
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 实例数据缓冲：每帧用 CPU 更新，上传所有雪花的位置/颜色/大小。
    const unsigned int PARTICLE_COUNT = 1000;
    GLuint instanceVBO;
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, PARTICLE_COUNT * 8 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    // location 2：实例位置
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);
    // location 3：实例颜色
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);
    // location 4：实例大小
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1);
    glBindVertexArray(0);

    // 初始化雪花粒子。
    struct Particle {
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec4 color;
        float scale;
    };
    std::vector<Particle> particles(PARTICLE_COUNT);

    std::mt19937 rng(12345);
    std::uniform_real_distribution<float> distX(-15.0f, 15.0f);
    std::uniform_real_distribution<float> distY(-10.0f, 10.0f);
    std::uniform_real_distribution<float> distZ(-15.0f, 15.0f);
    std::uniform_real_distribution<float> distFall(0.5f, 1.5f);    // 下落速度
    std::uniform_real_distribution<float> distDrift(-0.3f, 0.3f);  // 水平漂移速度
    std::uniform_real_distribution<float> distScale(0.08f, 0.35f); // 雪花大小
    std::uniform_real_distribution<float> distAlpha(0.2f, 0.7f);   // 透明度

    for (auto& p : particles) {
        p.position = glm::vec3(distX(rng), distY(rng), distZ(rng));
        p.velocity = glm::vec3(distDrift(rng), -distFall(rng), distDrift(rng));
        p.color = glm::vec4(1.0f, 1.0f, 1.0f, distAlpha(rng));
        p.scale = distScale(rng);
    }

    // 交互回调。
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 雪花半透明：加法混合叠加增亮，且与绘制顺序无关，不需要排序。
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // 每帧要上传的实例数据：位置(3) + 颜色(4) + 大小(1) = 8 个 float。
    std::vector<float> instanceData(PARTICLE_COUNT * 8);

    lastFrame = static_cast<float>(glfwGetTime());
    while(!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 更新雪花：下落 + 漂移 + 水平摆动；掉出底部后从顶部重新出现。
        for (unsigned int i = 0; i < PARTICLE_COUNT; ++i) {
            Particle& p = particles[i];
            p.position += p.velocity * deltaTime;
            p.position.x += std::sin(currentFrame * 0.8f + i * 0.7f) * 0.01f;
            p.position.z += std::cos(currentFrame * 0.6f + i * 0.9f) * 0.01f;

            if (p.position.y < -10.0f) {
                p.position.y = 10.0f;
                p.position.x = distX(rng);
                p.position.z = distZ(rng);
            }

            float* dst = &instanceData[i * 8];
            dst[0] = p.position.x;
            dst[1] = p.position.y;
            dst[2] = p.position.z;
            dst[3] = p.color.r;
            dst[4] = p.color.g;
            dst[5] = p.color.b;
            dst[6] = p.color.a;
            dst[7] = p.scale;
        }

        // 上传实例数据。
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(instanceData.size() * sizeof(float)), instanceData.data());

        glm::mat4 view = camera.GetViewMatrix();
        float aspect = framebufferHeight > 0
            ? static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight)
            : 1.0f;
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspect, 0.1f, 100.0f);

        particleShader.use();
        particleShader.setMat4("view", view);
        particleShader.setMat4("projection", projection);

        glBindVertexArray(quadVAO);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, PARTICLE_COUNT);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteBuffers(1, &instanceVBO);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
