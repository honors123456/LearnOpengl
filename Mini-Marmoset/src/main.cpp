#include "../camera/Camera.h"
#include "../shader/shader.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <cmath>
#include <iostream>
#include <vector>

static constexpr int W = 1000;
static constexpr int H = 650;

Camera camera(glm::vec3(0.0f, 0.0f, 11.0f));
float lastX = W * 0.5f;
float lastY = H * 0.5f;
bool firstMouse = true;
bool rotating = false;
float orbitYaw = 90.0f;
float orbitPitch = 2.0f;
float orbitDistance = 11.0f;

//材质
struct Material {
    glm::vec3 albedo; // 基色（漫反射/底色）
    float metallic;   // 金属度 0~1
    float roughness;  // 粗糙度 0~1
    int type;         // 材质种类编号（程序化纹理分支用）
};

void resize(GLFWwindow *, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse(GLFWwindow *, double x, double y) {
    if (!rotating) {
        firstMouse = true;
        return;
    }

    float currentX = static_cast<float>(x);
    float currentY = static_cast<float>(y);
    if (firstMouse) {
        lastX = currentX;
        lastY = currentY;
        firstMouse = false;
    }

    orbitYaw += (currentX - lastX) * 0.2f;
    orbitPitch += (lastY - currentY) * 0.2f;
    orbitPitch = glm::clamp(orbitPitch, -65.0f, 65.0f);
    lastX = currentX;
    lastY = currentY;
}

void mouseButton(GLFWwindow *, int button, int action, int) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        rotating = action == GLFW_PRESS;
        firstMouse = true;
    }
}

void scroll(GLFWwindow *, double, double yoffset) {
    orbitDistance -= static_cast<float>(yoffset) * 0.5f;
    orbitDistance = glm::clamp(orbitDistance, 7.0f, 17.0f);
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

std::vector<float> sphereVertices()
{
    //球体经纬线,加两边
    constexpr int segments = 64;
    constexpr int rings = 32;

    std::vector<float> sphereVertices;
    sphereVertices.reserve((segments + 1) * (rings + 1) * 6);

    for(int ring=0;ring<=rings;ring++)
    {
        float v = static_cast<float>(ring) / rings;
        float phi = v * glm::pi<float>();
        for (int segment = 0; segment <= segments; ++segment) {
            float u = static_cast<float>(segment) / segments;
            float theta = u * glm::two_pi<float>();
            glm::vec3 position(std::sin(phi) * std::cos(theta), std::cos(phi),std::sin(phi) * std::sin(theta));
            sphereVertices.insert(sphereVertices.end(),{position.x, position.y, position.z,position.x, position.y, position.z});
        }
    }


    return sphereVertices;
}

std::vector<unsigned int> sphereIndices()
{
    constexpr int segments = 64;
    constexpr int rings = 32;

    std::vector<unsigned int> sphereIndices;
    sphereIndices.reserve(segments * rings * 6);

    for (int ring = 0; ring < rings; ++ring) {
        for (int segment = 0; segment < segments; ++segment) {
            unsigned int current = ring * (segments + 1) + segment;
            unsigned int next = current + segments + 1;
            sphereIndices.insert(sphereIndices.end(),{current, next, current + 1, current + 1, next,next + 1});
        }
    }

    return sphereIndices;
}


int main() {
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(W, H, "Mini-Marmoset", nullptr, nullptr);

    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwTerminate();
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, resize);
    glfwSetCursorPosCallback(window, mouse);
    glfwSetMouseButtonCallback(window, mouseButton);
    glfwSetScrollCallback(window, scroll);
    glViewport(0, 0, W, H);
    glEnable(GL_DEPTH_TEST);

    Shader pbrShader(SHADER_DIR "/pbrShader.vert",SHADER_DIR "/pbrShader.frag");
    Shader lightShader(SHADER_DIR "/lightShader.vert",SHADER_DIR "/lightShader.frag");

    //创建球体顶点数据
    std::vector<float> sphereVerticles = sphereVertices();
    std::vector<unsigned int> sphereIndicles = sphereIndices();

    unsigned int sphereVAO = 0, sphereVBO = 0, sphereEBO = 0;
    glGenVertexArrays(1,&sphereVAO);
    glGenBuffers(1,&sphereVBO);
    glGenBuffers(1,&sphereEBO);

    glBindVertexArray(sphereVAO);

    glBindBuffer(GL_ARRAY_BUFFER,sphereVBO);
    glBufferData(GL_ARRAY_BUFFER,sphereVerticles.size() * sizeof(float),sphereVerticles.data(),GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sphereIndicles.size() * sizeof(unsigned int),sphereIndicles.data(),GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6 * sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);


    //四个点光源的位置和颜色
    const std::array<glm::vec3, 4> lightPositions ={
        glm::vec3(-4.5f, 4.5f, 4.0f),
        glm::vec3(4.5f, 4.5f, 4.0f),
        glm::vec3(-3.0f, -1.65f, 2.8f),
        glm::vec3(3.0f, -1.65f, 2.8f)
    };

    const std::array<glm::vec3, 4> lightColors = {
        glm::vec3(150.0f, 116.0f, 78.0f),
        glm::vec3(130.0f, 118.0f, 94.0f),
        glm::vec3(48.0f, 32.0f, 23.0f),
        glm::vec3(48.0f, 32.0f, 23.0f)
    };

    //5个球的位置和材质
    const std::array<glm::vec3, 25> spherePositions = {
        glm::vec3(-3.2f, -4.0f, 0.0f),
        glm::vec3(-1.6f, -4.0f, 0.0f),
        glm::vec3(0.0f, -4.0f, 0.0f),
        glm::vec3(1.6f, -4.0f, 0.0f),
        glm::vec3(3.2f, -4.0f, 0.0f),

        glm::vec3(-3.2f, -2.0f, 0.0f),
        glm::vec3(-1.6f, -2.0f, 0.0f),
        glm::vec3(0.0f, -2.0f, 0.0f),
        glm::vec3(1.6f, -2.0f, 0.0f),
        glm::vec3(3.2f, -2.0f, 0.0f),

        glm::vec3(-3.2f, 0.0f, 0.0f),
        glm::vec3(-1.6f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.6f, 0.0f, 0.0f),
        glm::vec3(3.2f, 0.0f, 0.0f),

        glm::vec3(-3.2f, 2.0f, 0.0f),
        glm::vec3(-1.6f, 2.0f, 0.0f),
        glm::vec3(0.0f, 2.0f, 0.0f),
        glm::vec3(1.6f, 2.0f, 0.0f),
        glm::vec3(3.2f, 2.0f, 0.0f),

        glm::vec3(-3.2f, 4.0f, 0.0f),
        glm::vec3(-1.6f, 4.0f, 0.0f),
        glm::vec3(0.0f, 4.0f, 0.0f),
        glm::vec3(1.6f, 4.0f, 0.0f),
        glm::vec3(3.2f, 4.0f, 0.0f),
    };

    const std::array<Material, 25> materials = {
       Material{glm::vec3(0.23f, 0.10f, 0.055f), 0.0f, 0.82f, 0},
       Material{glm::vec3(0.52f, 0.055f, 0.025f), 0.0f, 0.23f, 1},
       Material{glm::vec3(0.035f, 0.28f, 0.055f), 0.0f, 0.92f, 2},
       Material{glm::vec3(1.0f, 0.71f, 0.22f), 1.0f, 0.12f, 3},
       Material{glm::vec3(0.42f, 0.16f, 0.12f), 0.0f, 0.48f, 4},

        Material{glm::vec3(0.23f, 0.10f, 0.055f), 0.0f, 0.82f, 0},
        Material{glm::vec3(0.52f, 0.055f, 0.025f), 0.0f, 0.23f, 1},
        Material{glm::vec3(0.035f, 0.28f, 0.055f), 0.0f, 0.92f, 2},
        Material{glm::vec3(1.0f, 0.71f, 0.22f), 1.0f, 0.12f, 3},
        Material{glm::vec3(0.42f, 0.16f, 0.12f), 0.0f, 0.48f, 4},

        Material{glm::vec3(0.23f, 0.10f, 0.055f), 0.0f, 0.82f, 0},
        Material{glm::vec3(0.52f, 0.055f, 0.025f), 0.0f, 0.23f, 1},
        Material{glm::vec3(0.035f, 0.28f, 0.055f), 0.0f, 0.92f, 2},
        Material{glm::vec3(1.0f, 0.71f, 0.22f), 1.0f, 0.12f, 3},
        Material{glm::vec3(0.42f, 0.16f, 0.12f), 0.0f, 0.48f, 4},

        Material{glm::vec3(0.23f, 0.10f, 0.055f), 0.0f, 0.82f, 0},
        Material{glm::vec3(0.52f, 0.055f, 0.025f), 0.0f, 0.23f, 1},
        Material{glm::vec3(0.035f, 0.28f, 0.055f), 0.0f, 0.92f, 2},
        Material{glm::vec3(1.0f, 0.71f, 0.22f), 1.0f, 0.12f, 3},
        Material{glm::vec3(0.42f, 0.16f, 0.12f), 0.0f, 0.48f, 4},

        Material{glm::vec3(0.23f, 0.10f, 0.055f), 0.0f, 0.82f, 0},
        Material{glm::vec3(0.52f, 0.055f, 0.025f), 0.0f, 0.23f, 1},
        Material{glm::vec3(0.035f, 0.28f, 0.055f), 0.0f, 0.92f, 2},
        Material{glm::vec3(1.0f, 0.71f, 0.22f), 1.0f, 0.12f, 3},
        Material{glm::vec3(0.42f, 0.16f, 0.12f), 0.0f, 0.48f, 4},
    };


    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        //鼠标交互逻辑
        float yaw = glm::radians(orbitYaw);
        float pitch = glm::radians(orbitPitch);
        glm::vec3 orbitOffset(orbitDistance * std::cos(pitch) * std::cos(yaw),orbitDistance * std::sin(pitch),orbitDistance * std::cos(pitch) * std::sin(yaw));
        camera.Position = orbitOffset;

        //变换矩阵
        glm::mat4 model = glm::translate(glm::mat4(1.0f),glm::vec3(0,0,0));
        glm::mat4 view = glm::lookAt(camera.Position, glm::vec3(0.0f),glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(W) / H, 0.1f, 100.0f);

        glClearColor(0.56f, 0.43f, 0.31f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //绘制球体
        pbrShader.use();

        //上传属性值
        pbrShader.setMat4("model",model);
        pbrShader.setMat4("view",view);
        pbrShader.setMat4("projection",projection);
        pbrShader.setVec3("cameraPos",camera.Position);
        pbrShader.setFloat("ao", 1.0f);

        for(int i=0; i < lightPositions.size(); i++)
        {
            pbrShader.setVec3("lightPositions[" + std::to_string(i) + "]",lightPositions[i]);
            pbrShader.setVec3("lightColors[" + std::to_string(i) + "]",lightColors[i]);
        }

        //绘制多个球体
        glBindVertexArray(sphereVAO);
        for (std::size_t i = 0; i < materials.size(); ++i) {
            glm::mat4 model =glm::translate(glm::mat4(1.0f), spherePositions[i]);
            model = glm::scale(model, glm::vec3(0.82f));
            pbrShader.setMat4("model", model);
            pbrShader.setVec3("albedo", materials[i].albedo);
            pbrShader.setFloat("metallic", materials[i].metallic);
            pbrShader.setFloat("roughness", materials[i].roughness);
            pbrShader.setInt("materialType", materials[i].type);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sphereIndicles.size()),GL_UNSIGNED_INT, nullptr);
        }

        //绘制光源
        lightShader.use();
        lightShader.setMat4("model",model);
        lightShader.setMat4("view", view);
        lightShader.setMat4("projection", projection);

        for (int i = 2; i < 4; ++i) {
            glm::mat4 model =
                glm::translate(glm::mat4(1.0f), lightPositions[i]);
            model = glm::scale(model, glm::vec3(0.14f));
            lightShader.setMat4("model", model);
            lightShader.setVec3("lightColor", lightColors[i] * 0.025f);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sphereIndicles.size()),GL_UNSIGNED_INT, nullptr);
        }


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
