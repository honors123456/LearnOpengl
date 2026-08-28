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
#include <opencv2/opencv.hpp>
#include <random>
#include <vector>

static constexpr int W = 800;
static constexpr int H = 600;

Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = W * 0.5f;
float lastY = H * 0.5f;
bool firstMouse = true;
bool rotating = false;
glm::vec3 orbitTarget(0.0f, -0.5f, -0.8f);
float orbitYaw = 90.0f;
float orbitPitch = 12.0f;
float orbitDistance = 7.0f;

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
  float xoffset = currentX - lastX;
  float yoffset = lastY - currentY;
  orbitYaw += xoffset * 0.2f;
  orbitPitch += yoffset * 0.2f;
  orbitPitch = glm::clamp(orbitPitch, -80.0f, 80.0f);
  lastX = currentX;
  lastY = currentY;
}

void mouseButton(GLFWwindow *, int button, int action, int) {
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    rotating = action == GLFW_PRESS;
    firstMouse = true;
  }
}

void scroll(GLFWwindow *, double, double y) {
  orbitDistance -= static_cast<float>(y) * 0.5f;
  orbitDistance = glm::clamp(orbitDistance, 2.5f, 15.0f);
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

int main() {
  if (!glfwInit())
    return -1;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow *window =
      glfwCreateWindow(W, H, "Deferred Shading", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    return -1;
  glfwSetFramebufferSizeCallback(window, resize);
  glfwSetCursorPosCallback(window, mouse);
  glfwSetMouseButtonCallback(window, mouseButton);
  glfwSetScrollCallback(window, scroll);
  glViewport(0, 0, W, H);
  glEnable(GL_DEPTH_TEST);

  Shader geometryShader(SHADER_DIR "/objectShader.vert",
                        SHADER_DIR "/objectShader.frag");
  Shader lightingShader(SHADER_DIR "/finalShader.vert",
                        SHADER_DIR "/finalShader.frag");
  Shader lightShader(SHADER_DIR "/lightShader.vert",
                     SHADER_DIR "/lightShader.frag");

  //全屏渲染2D平面
  float quad[] = {-1, 1, 0, 1, -1, -1, 0, 0, 1, -1, 1, 0,
                  -1, 1, 0, 1, 1,  -1, 1, 0, 1, 1,  1, 1};
  GLuint quadVAO = 0, quadVBO = 0;
  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);

  //立方体
  float cube[] = {
      -1,-1,-1, 0,0,-1, 0,0,  1, 1,-1, 0,0,-1, 1,1,  1,-1,-1, 0,0,-1, 1,0,
      -1,-1,-1, 0,0,-1, 0,0, -1, 1,-1, 0,0,-1, 0,1,  1, 1,-1, 0,0,-1, 1,1,
      -1,-1, 1, 0,0, 1, 0,0,  1,-1, 1, 0,0, 1, 1,0,  1, 1, 1, 0,0, 1, 1,1,
      -1,-1, 1, 0,0, 1, 0,0,  1, 1, 1, 0,0, 1, 1,1, -1, 1, 1, 0,0, 1, 0,1,
      -1, 1, 1,-1,0, 0, 1,0, -1, 1,-1,-1,0, 0, 1,1, -1,-1,-1,-1,0, 0, 0,1,
      -1, 1, 1,-1,0, 0, 1,0, -1,-1,-1,-1,0, 0, 0,1, -1,-1, 1,-1,0, 0, 0,0,
       1, 1, 1, 1,0, 0, 1,0,  1,-1,-1, 1,0, 0, 0,1,  1, 1,-1, 1,0, 0, 1,1,
       1, 1, 1, 1,0, 0, 1,0,  1,-1, 1, 1,0, 0, 0,0,  1,-1,-1, 1,0, 0, 0,1,
      -1,-1,-1, 0,-1,0, 0,1,  1,-1,-1, 0,-1,0, 1,1,  1,-1, 1, 0,-1,0, 1,0,
      -1,-1,-1, 0,-1,0, 0,1,  1,-1, 1, 0,-1,0, 1,0, -1,-1, 1, 0,-1,0, 0,0,
      -1, 1,-1, 0, 1,0, 0,1,  1, 1, 1, 0, 1,0, 1,0,  1, 1,-1, 0, 1,0, 1,1,
      -1, 1,-1, 0, 1,0, 0,1, -1, 1, 1, 0, 1,0, 0,0,  1, 1, 1, 0, 1,0, 1,0};

  GLuint cubeVAO = 0, cubeVBO = 0;
  glGenVertexArrays(1, &cubeVAO);
  glGenBuffers(1, &cubeVBO);
  glBindVertexArray(cubeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  //点光源
  //低面数 UV 球体只用于显示点光源位置。
  constexpr int sphereSegments = 16;
  constexpr int sphereRings = 12;
  std::vector<float> sphereVertices;
  std::vector<unsigned int> sphereIndices;
  for (int ring = 0; ring <= sphereRings; ++ring) {
    float v = static_cast<float>(ring) / sphereRings;
    float phi = v * glm::pi<float>();
    for (int segment = 0; segment <= sphereSegments; ++segment) {
      float u = static_cast<float>(segment) / sphereSegments;
      float theta = u * glm::two_pi<float>();
      sphereVertices.push_back(std::sin(phi) * std::cos(theta));
      sphereVertices.push_back(std::cos(phi));
      sphereVertices.push_back(std::sin(phi) * std::sin(theta));
    }
  }
  for (int ring = 0; ring < sphereRings; ++ring) {
    for (int segment = 0; segment < sphereSegments; ++segment) {
      unsigned int current = ring * (sphereSegments + 1) + segment;
      unsigned int next = current + sphereSegments + 1;
      sphereIndices.insert(sphereIndices.end(),
                           {current, next, current + 1, current + 1, next,
                            next + 1});
    }
  }
  GLuint sphereVAO = 0, sphereVBO = 0, sphereEBO = 0;
  glGenVertexArrays(1, &sphereVAO);
  glGenBuffers(1, &sphereVBO);
  glGenBuffers(1, &sphereEBO);
  glBindVertexArray(sphereVAO);
  glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
  glBufferData(GL_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(sphereVertices.size() * sizeof(float)),
               sphereVertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
  glBufferData(
      GL_ELEMENT_ARRAY_BUFFER,
      static_cast<GLsizeiptr>(sphereIndices.size() * sizeof(unsigned int)),
      sphereIndices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  //立方体纹理
  cv::Mat image = cv::imread(DIFFUSE_TEXTURE_FILE, cv::IMREAD_COLOR);
  if (image.empty()) {
    std::cerr << "Failed to load texture: " << DIFFUSE_TEXTURE_FILE << '\n';
    return -1;
  }
  cv::flip(image, image, 0);
  cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
  GLuint diffuseTexture = 0;
  glGenTextures(1, &diffuseTexture);
  glBindTexture(GL_TEXTURE_2D, diffuseTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0, GL_RGB,
               GL_UNSIGNED_BYTE, image.data);
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // G-Buffer：四张颜色纹理分别保存位置、法线、颜色和高光强度。
  GLuint gBuffer = 0;
  GLuint gPosition = 0, gNormal = 0, gAlbedo = 0, gSpecular = 0;
  glGenFramebuffers(1, &gBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

  GLuint gTextures[] = {gPosition, gNormal, gAlbedo, gSpecular};
  GLenum internalFormats[] = {GL_RGBA16F, GL_RGBA16F, GL_RGBA8, GL_RGBA8};
  GLenum dataTypes[] = {GL_FLOAT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE};
  for (int i = 0; i < 4; ++i) {
    glGenTextures(1, &gTextures[i]);
    glBindTexture(GL_TEXTURE_2D, gTextures[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormats[i], W, H, 0, GL_RGBA,
                 dataTypes[i], nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                           GL_TEXTURE_2D, gTextures[i], 0);
  }
  gPosition = gTextures[0];
  gNormal = gTextures[1];
  gAlbedo = gTextures[2];
  gSpecular = gTextures[3];

  //开启frag着色器的四个颜色纹理附件
  //当前绑定 FBO 的颜色输出目标”。
  GLenum attachments[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                          GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
  glDrawBuffers(4, attachments);

  //深度缓冲区附件
  GLuint depthRBO = 0;
  glGenRenderbuffers(1, &depthRBO);
  glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, W, H);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, depthRBO);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "G-Buffer framebuffer incomplete\n";
    return -1;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  geometryShader.use();
  geometryShader.setInt("diffuseMap", 0);

  //设置采样器对应的纹理
  lightingShader.use();
  lightingShader.setInt("gPosition", 0);
  lightingShader.setInt("gNormal", 1);
  lightingShader.setInt("gAlbedo", 2);
  lightingShader.setInt("gSpecular", 3);

  static constexpr int LIGHT_COUNT = 100;
  std::array<glm::vec3, LIGHT_COUNT> lightPositions{};
  std::array<glm::vec3, LIGHT_COUNT> lightColors{};

  // 使用固定种子生成随机位置：布局看起来随机，但每次启动保持一致，便于调试。
  std::mt19937 randomEngine(20260828u);
  std::uniform_real_distribution<float> randomX(-3.5f, 3.5f);
  std::uniform_real_distribution<float> randomY(-1.6f, 2.2f);
  std::uniform_real_distribution<float> randomZ(-4.5f, 2.5f);
  for (int i = 0; i < LIGHT_COUNT; ++i) {
    lightPositions[i] = glm::vec3(randomX(randomEngine), randomY(randomEngine),
                                  randomZ(randomEngine));
    lightColors[i] = glm::vec3(
        0.8f + 1.8f * static_cast<float>((i * 17) % 100) / 100.0f,
        0.8f + 1.8f * static_cast<float>((i * 43) % 100) / 100.0f,
        0.8f + 1.8f * static_cast<float>((i * 71) % 100) / 100.0f);
  }

  while (!glfwWindowShouldClose(window)) {
    processInput(window);

    // 轨道相机：相机始终看向场景中心，拖动改变方位，滚轮改变距离。
    float yaw = glm::radians(orbitYaw);
    float pitch = glm::radians(orbitPitch);
    glm::vec3 orbitOffset(
        orbitDistance * std::cos(pitch) * std::cos(yaw),
        orbitDistance * std::sin(pitch),
        orbitDistance * std::cos(pitch) * std::sin(yaw));
    camera.Position = orbitTarget + orbitOffset;
    glm::mat4 view = glm::lookAt(camera.Position, orbitTarget,
                                 glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(
        glm::radians(camera.Zoom), static_cast<float>(W) / H, 0.1f, 100.0f);

    // 1. 几何 Pass：只写入 G-Buffer，不在这里计算多个光源。
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClearColor(0.02f, 0.02f, 0.02f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    geometryShader.use();
    geometryShader.setMat4("view", view);
    geometryShader.setMat4("projection", projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseTexture);
    glBindVertexArray(cubeVAO);
    // 箱子使用 container2.png，保留适中的高光强度。
    geometryShader.setBool("useTexture", true);
    geometryShader.setFloat("specularStrength", 0.35f);
    std::array<glm::vec3, 3> objectPositions = {
        glm::vec3(-1.2f, 0.0f, 0.0f), glm::vec3(1.2f, 0.0f, -0.8f),
        glm::vec3(0.0f, -1.3f, -1.8f)};
    std::array<float, 3> objectAngles = {25.0f, -35.0f, 50.0f};
    for (std::size_t i = 0; i < objectPositions.size(); ++i) {
      glm::mat4 model =
          glm::translate(glm::mat4(1.0f), objectPositions[i]);
      model = glm::rotate(model, glm::radians(objectAngles[i]),
                          glm::normalize(glm::vec3(1.0f, 0.7f, 0.3f)));
      model = glm::scale(model, glm::vec3(0.55f));
      geometryShader.setMat4("model", model);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // 用一个压扁并放大的立方体作为地面，提供明显的空间和遮挡参照。
    // 黑褐色底色和较高高光强度用来表现深色金属材质。
    geometryShader.setBool("useTexture", false);
    geometryShader.setVec3("baseColor", glm::vec3(0.18f, 0.07f, 0.03f));
    geometryShader.setFloat("specularStrength", 0.9f);
    glm::mat4 floorModel =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.2f, -1.0f));
    floorModel = glm::scale(floorModel, glm::vec3(4.0f, 0.15f, 4.0f));
    geometryShader.setMat4("model", floorModel);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // 2. 光照 Pass：读取四张 G-Buffer 纹理，在全屏四边形上统一算光照。
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    lightingShader.use();
    lightingShader.setVec3("viewPos", camera.Position);
    for (int i = 0; i < LIGHT_COUNT; ++i) {
      lightingShader.setVec3("lights[" + std::to_string(i) + "].position",
                             lightPositions[i]);
      lightingShader.setVec3("lights[" + std::to_string(i) + "].color",
                             lightColors[i]);
    }
    //绘制之前的4个纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gSpecular);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // 3. 复制深度后再画光源，光源标记才能被场景物体正确遮挡。
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, W, H, 0, 0, W, H, GL_DEPTH_BUFFER_BIT,
                      GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    lightShader.use();
    lightShader.setMat4("view", view);
    lightShader.setMat4("projection", projection);
    glBindVertexArray(sphereVAO);
    for (int i = 0; i < LIGHT_COUNT; ++i) {
      glm::mat4 lightModel =
          glm::translate(glm::mat4(1.0f), lightPositions[i]);
      lightModel = glm::scale(lightModel, glm::vec3(0.055f));
      lightShader.setMat4("model", lightModel);
      lightShader.setVec3("lightColor", lightColors[i]);
      glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sphereIndices.size()),
                     GL_UNSIGNED_INT, nullptr);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
