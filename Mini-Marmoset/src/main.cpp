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


    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        float yaw = glm::radians(orbitYaw);
        float pitch = glm::radians(orbitPitch);
        glm::vec3 orbitOffset(orbitDistance * std::cos(pitch) * std::cos(yaw),orbitDistance * std::sin(pitch),orbitDistance * std::cos(pitch) * std::sin(yaw));
        camera.Position = orbitOffset;

        glm::mat4 model = glm::translate(glm::mat4(1.0f),glm::vec3(0,0,0));
        glm::mat4 view = glm::lookAt(camera.Position, glm::vec3(0.0f),glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(W) / H, 0.1f, 100.0f);

        glClearColor(0.56f, 0.43f, 0.31f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //绘制球体
        pbrShader.use();
        pbrShader.setMat4("model",model);
        pbrShader.setMat4("view",view);
        pbrShader.setMat4("projection",projection);

        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sphereIndicles.size()),GL_UNSIGNED_INT, nullptr);


        //绘制光源
        lightShader.use();
        lightShader.setMat4("model",model);
        lightShader.setMat4("view", view);
        lightShader.setMat4("projection", projection);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
