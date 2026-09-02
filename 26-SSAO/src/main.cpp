// ============================================================
// 26-SSAO：屏幕空间环境光遮蔽（SSAO）演示
// ------------------------------------------------------------
// 每帧渲染流程（5 个 Pass）：
//   1. 几何 Pass  —— 把观察空间位置/法线/漫反射/高光写入 G-Buffer
//   2. SSAO Pass  —— 在观察空间对 64 个半球采样点做深度比较，
//                    输出带随机噪声的单通道原始遮蔽值
//   3. Blur Pass  —— 平滑 SSAO 的随机颗粒
//   4. 光照 Pass  —— 环境光乘以模糊后的 AO，点光源直接光不受遮挡
//   5. 光源标记  —— 复制 G-Buffer 深度后绘制发光小球
// 交互：按住左键拖拽旋转视角，滚轮缩放距离；
//       数字键 1-4 切换显示模式（含义见 displayMode 注释）。
// ============================================================

// 项目内部封装：相机与着色器工具
#include "../camera/Camera.h"
#include "../shader/shader.h"
// 第三方库：窗口、OpenGL 函数加载、数学库
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
// 标准库
#include <array>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

// 窗口分辨率（G-Buffer 与 SSAO 纹理均使用该尺寸）
static constexpr int W = 800;
static constexpr int H = 600;

// 相机对象：位置每帧由 orbit* 轨道参数重建
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = W * 0.5f;                    // 鼠标上一帧的 X 坐标
float lastY = H * 0.5f;                    // 鼠标上一帧的 Y 坐标
bool firstMouse = true;                    // 是否处于首次采样，避免视角跳变
bool rotating = false;                     // 左键是否按住（进入旋转模式）
glm::vec3 orbitTarget(0.0f, -0.3f, -2.2f); // 轨道中心，即场景焦点
float orbitYaw = 90.0f;                    // 轨道方位角（度）
float orbitPitch = 8.0f;                   // 轨道俯仰角（度）
float orbitDistance = 10.0f;               // 相机到焦点的距离
// 显示模式：0=关闭 AO（纯直接光） 1=完整 SSAO 2=原始 AO 3=模糊后 AO
int displayMode = 1;

// 窗口尺寸变化时更新视口
void resize(GLFWwindow *, int width, int height) {
  glViewport(0, 0, width, height);
}

// 按住左键拖动时旋转轨道相机：横向偏移改方位角，纵向偏移改俯仰角
void mouse(GLFWwindow *, double x, double y) {
  // 未按住左键时不响应，并重置首次采样标志
  if (!rotating) {
    firstMouse = true;
    return;
  }
  float currentX = static_cast<float>(x);
  float currentY = static_cast<float>(y);
  // 按下后的首帧直接记录起点，避免从旧位置产生大跳变
  if (firstMouse) {
    lastX = currentX;
    lastY = currentY;
    firstMouse = false;
  }
  float xoffset = currentX - lastX;
  float yoffset = lastY - currentY;
  orbitYaw += xoffset * 0.2f;                  // 每像素 0.2 度的旋转灵敏度
  orbitPitch += yoffset * 0.2f;
  orbitPitch = glm::clamp(orbitPitch, -80.0f, 80.0f); // 限制俯仰角避免翻转
  lastX = currentX;
  lastY = currentY;
}

// 左键按下/松开时切换旋转模式
void mouseButton(GLFWwindow *, int button, int action, int) {
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    rotating = action == GLFW_PRESS;
    firstMouse = true; // 每次切换都重置，重新计算拖动起点
  }
}

// 滚轮缩放轨道距离，限制在 2.5~15 之间
void scroll(GLFWwindow *, double, double y) {
  orbitDistance -= static_cast<float>(y) * 0.5f;
  orbitDistance = glm::clamp(orbitDistance, 2.5f, 15.0f);
}

// 每帧轮询键盘输入：Esc 退出，数字键 1-4 切换显示模式
void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    displayMode = 0;
  if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    displayMode = 1;
  if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    displayMode = 2;
  if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
    displayMode = 3;
}

int main() {
  // 初始化 GLFW
  if (!glfwInit())
    return -1;

  // 请求 OpenGL 3.3 核心上下文
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // 创建窗口
  GLFWwindow *window =
      glfwCreateWindow(W, H, "SSAO", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return -1;
  }

  // 绑定当前上下文并加载 OpenGL 函数
  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    return -1;
  // 注册窗口事件回调
  glfwSetFramebufferSizeCallback(window, resize);
  glfwSetCursorPosCallback(window, mouse);
  glfwSetMouseButtonCallback(window, mouseButton);
  glfwSetScrollCallback(window, scroll);
  glViewport(0, 0, W, H);
  glEnable(GL_DEPTH_TEST); // 几何 Pass 需要深度剔除

  // 编译并链接各 Pass 使用的着色器
  Shader objectShader(SHADER_DIR "/objectShader.vert",
                      SHADER_DIR "/objectShader.frag");
  Shader lightingShader(SHADER_DIR "/finalShader.vert",
                        SHADER_DIR "/finalShader.frag");
  Shader ssaoShader(SHADER_DIR "/ssaoShader.vert",
                    SHADER_DIR "/ssaoShader.frag");
  Shader ssaoBlurShader(SHADER_DIR "/ssaoBlurShader.vert",
                        SHADER_DIR "/ssaoBlurShader.frag");
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
      -1, -1, -1,  0,  0, -1, 0, 0,
       1,  1, -1,  0,  0, -1, 1, 1,
       1, -1, -1,  0,  0, -1, 1, 0,
      -1, -1, -1,  0,  0, -1, 0, 0,
      -1,  1, -1,  0,  0, -1, 0, 1,
       1,  1, -1,  0,  0, -1, 1, 1,
      -1, -1,  1,  0,  0,  1, 0, 0,
       1, -1,  1,  0,  0,  1, 1, 0,
       1,  1,  1,  0,  0,  1, 1, 1,
      -1, -1,  1,  0,  0,  1, 0, 0,
       1,  1,  1,  0,  0,  1, 1, 1,
      -1,  1,  1,  0,  0,  1, 0, 1,
      -1,  1,  1, -1,  0,  0, 1, 0,
      -1,  1, -1, -1,  0,  0, 1, 1,
      -1, -1, -1, -1,  0,  0, 0, 1,
      -1,  1,  1, -1,  0,  0, 1, 0,
      -1, -1, -1, -1,  0,  0, 0, 1,
      -1, -1,  1, -1,  0,  0, 0, 0,
       1,  1,  1,  1,  0,  0, 1, 0,
       1, -1, -1,  1,  0,  0, 0, 1,
       1,  1, -1,  1,  0,  0, 1, 1,
       1,  1,  1,  1,  0,  0, 1, 0,
       1, -1,  1,  1,  0,  0, 0, 0,
       1, -1, -1,  1,  0,  0, 0, 1,
      -1, -1, -1,  0, -1,  0, 0, 1,
       1, -1, -1,  0, -1,  0, 1, 1,
       1, -1,  1,  0, -1,  0, 1, 0,
      -1, -1, -1,  0, -1,  0, 0, 1,
       1, -1,  1,  0, -1,  0, 1, 0,
      -1, -1,  1,  0, -1,  0, 0, 0,
      -1,  1, -1,  0,  1,  0, 0, 1,
       1,  1,  1,  0,  1,  0, 1, 0,
       1,  1, -1,  0,  1,  0, 1, 1,
      -1,  1, -1,  0,  1,  0, 0, 1,
      -1,  1,  1,  0,  1,  0, 0, 0,
       1,  1,  1,  0,  1,  0, 1, 0};

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

  // UV 球体既用于墙角遮挡参照，也用于显示点光源。
  constexpr int sphereSegments = 32;
  constexpr int sphereRings = 20;
  std::vector<float> sphereVertices;
  std::vector<unsigned int> sphereIndices;
  for (int ring = 0; ring <= sphereRings; ++ring) {
    float v = static_cast<float>(ring) / sphereRings;
    float phi = v * glm::pi<float>();
    for (int segment = 0; segment <= sphereSegments; ++segment) {
      float u = static_cast<float>(segment) / sphereSegments;
      float theta = u * glm::two_pi<float>();
      glm::vec3 position(std::sin(phi) * std::cos(theta), std::cos(phi),
                         std::sin(phi) * std::sin(theta));
      sphereVertices.insert(sphereVertices.end(),
                            {position.x, position.y, position.z,
                             position.x, position.y, position.z});
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
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

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

  // SSAO Pass 输出单通道环境遮蔽值；Blur Pass 输出去噪后的结果。
  GLuint ssaoFBO = 0, ssaoBlurFBO = 0;
  GLuint ssaoColorBuffer = 0, ssaoColorBufferBlur = 0;
  glGenFramebuffers(1, &ssaoFBO);
  glGenFramebuffers(1, &ssaoBlurFBO);
  glGenTextures(1, &ssaoColorBuffer);
  glGenTextures(1, &ssaoColorBufferBlur);

  glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
  glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, W, H, 0, GL_RED, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         ssaoColorBuffer, 0);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "SSAO framebuffer incomplete\n";
    return -1;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
  glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, W, H, 0, GL_RED, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         ssaoColorBufferBlur, 0);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "SSAO blur framebuffer incomplete\n";
    return -1;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // 生成 64 个朝向法线半球的采样点。越靠近中心，采样点越密集。
  std::vector<glm::vec3> ssaoKernel;
  ssaoKernel.reserve(64);
  std::mt19937 ssaoRandomEngine(20260901u);
  std::uniform_real_distribution<float> randomZeroToOne(0.0f, 1.0f);
  for (int i = 0; i < 64; ++i) {
    glm::vec3 sample(randomZeroToOne(ssaoRandomEngine) * 2.0f - 1.0f,
                     randomZeroToOne(ssaoRandomEngine) * 2.0f - 1.0f,
                     randomZeroToOne(ssaoRandomEngine));
    sample = glm::normalize(sample);
    sample *= randomZeroToOne(ssaoRandomEngine);
    float scale = static_cast<float>(i) / 64.0f;
    scale = 0.1f + 0.9f * scale * scale;
    ssaoKernel.push_back(sample * scale);
  }

  // 4x4 随机旋转向量只位于切线平面，着色器会把它平铺到整个屏幕。
  std::vector<glm::vec3> ssaoNoise;
  ssaoNoise.reserve(16);
  for (int i = 0; i < 16; ++i) {
    ssaoNoise.emplace_back(randomZeroToOne(ssaoRandomEngine) * 2.0f - 1.0f,randomZeroToOne(ssaoRandomEngine) * 2.0f - 1.0f,0.0f);
  }
  
  GLuint noiseTexture = 0;
  glGenTextures(1, &noiseTexture);
  glBindTexture(GL_TEXTURE_2D, noiseTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT,ssaoNoise.data());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  //设置采样器对应的纹理
  lightingShader.use();
  lightingShader.setInt("gPosition", 0);
  lightingShader.setInt("gNormal", 1);
  lightingShader.setInt("gAlbedo", 2);
  lightingShader.setInt("gSpecular", 3);
  lightingShader.setInt("ssaoBlur", 4); // 单元4 → ssaoColorBufferBlur（模糊遮蔽）
  lightingShader.setInt("ssao", 5);     // 单元5 → ssaoColorBuffer（原始遮蔽）

  ssaoShader.use();
  ssaoShader.setInt("gPosition", 0);
  ssaoShader.setInt("gNormal", 1);
  ssaoShader.setInt("texNoise", 2);
  for (int i = 0; i < 64; ++i) {
    ssaoShader.setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
  }

  ssaoBlurShader.use();
  ssaoBlurShader.setInt("ssaoInput", 0);

  // 点光源配置：位置与颜色（颜色为 HDR 数值，>1 才能产生高光）
  static constexpr int LIGHT_COUNT = 1;
  std::array<glm::vec3, LIGHT_COUNT> lightPositions{};
  std::array<glm::vec3, LIGHT_COUNT> lightColors{};

  lightPositions[0] = glm::vec3(0.0f, 2.4f, 0.5f);
  lightColors[0] = glm::vec3(7.0f, 5.7f, 4.5f);

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

    // 1. 几何 Pass：把观察空间位置、法线和材质写入 G-Buffer。
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClearColor(0.02f, 0.02f, 0.02f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    objectShader.use();
    objectShader.setMat4("view", view);
    objectShader.setMat4("projection", projection);
    glBindVertexArray(cubeVAO);
    objectShader.setBool("useTexture", false);
    objectShader.setFloat("specularStrength", 0.18f);

    // 封装立方体绘制：只需传入模型矩阵与漫反射颜色
    auto drawCube = [&](const glm::mat4 &model, const glm::vec3 &color) {
      objectShader.setMat4("model", model);
      objectShader.setVec3("baseColor", color);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    };

    // 封闭房间的墙体
    const glm::vec3 roomColor(0.62f, 0.60f, 0.56f);
    drawCube(glm::scale(glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, -2.1f, -2.6f)),glm::vec3(4.0f, 0.1f, 4.0f)),roomColor);
    drawCube(glm::scale(glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, -6.5f)),glm::vec3(4.0f, 2.1f, 0.1f)),roomColor);
    drawCube(glm::scale(glm::translate(glm::mat4(1.0f),glm::vec3(-4.0f, 0.0f, -2.6f)),glm::vec3(0.1f, 2.1f, 4.0f)),roomColor);
    drawCube(glm::scale(glm::translate(glm::mat4(1.0f),glm::vec3(4.0f, 0.0f, -2.6f)),glm::vec3(0.1f, 2.1f, 4.0f)),roomColor);
    drawCube(glm::scale(glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 2.1f, -2.6f)),glm::vec3(4.0f, 0.1f, 4.0f)),roomColor);

    //大的立方体盒子
    glm::mat4 largeBox = glm::translate(glm::mat4(1.0f),glm::vec3(-0.9f, -1.15f, -3.1f));
    largeBox = glm::rotate(largeBox, glm::radians(24.0f),glm::vec3(0.0f, 1.0f, 0.0f));
    largeBox = glm::scale(largeBox, glm::vec3(0.9f));
    drawCube(largeBox, glm::vec3(0.52f, 0.37f, 0.26f));

    //小的立方体盒子
    glm::mat4 smallBox = glm::translate(glm::mat4(1.0f),glm::vec3(1.0f, -1.5f, -2.3f));
    smallBox = glm::rotate(smallBox, glm::radians(-18.0f),glm::vec3(0.0f, 1.0f, 0.0f));
    smallBox = glm::scale(smallBox, glm::vec3(0.55f));
    drawCube(smallBox, glm::vec3(0.36f, 0.44f, 0.48f));

    // 墙角球体
    glBindVertexArray(sphereVAO);
    objectShader.setVec3("baseColor", glm::vec3(0.45f, 0.31f, 0.21f));
    glm::mat4 cornerSphere = glm::translate(glm::mat4(1.0f), glm::vec3(2.65f, -1.35f, -5.55f));
    cornerSphere = glm::scale(cornerSphere, glm::vec3(0.72f));
    objectShader.setMat4("model", cornerSphere);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sphereIndices.size()),GL_UNSIGNED_INT, nullptr);

    // 2. SSAO Pass：在观察空间中比较邻域深度，得到原始遮蔽纹理。
    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    glClear(GL_COLOR_BUFFER_BIT);
    ssaoShader.use();
    ssaoShader.setMat4("projection", projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // 3. Blur Pass：平滑原始 SSAO 的随机颗粒。
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glClear(GL_COLOR_BUFFER_BIT);
    ssaoBlurShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // 4. 光照 Pass：环境光乘以 AO，直接光照保持不变。
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    lightingShader.use();
    lightingShader.setInt("displayMode", displayMode);
    for (int i = 0; i < LIGHT_COUNT; ++i) {
      glm::vec3 lightPositionView =
          glm::vec3(view * glm::vec4(lightPositions[i], 1.0f));
      lightingShader.setVec3("lights[" + std::to_string(i) + "].position",
                             lightPositionView);
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
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // 5. 复制深度后再画光源，光源标记才能被场景物体正确遮挡。
    //为什么要拷贝Buffer?
    //上面的状态是绘制全屏 quad，glClear之后，屏幕 FBO 的深度缓冲此刻是空的，glEnable(GL_DEPTH_TEST)也没开过
    //如果直接用这个空深度缓冲开深度测试，小球永远找不到"比自己更近的墙"——于是会透过墙壁显示出来，穿帮。
    //解决办法就是把gBuffer的正确景深度，搬到屏幕 FBO 的深度缓冲
    //一句话总结：光照 pass 用后处理 quad 上屏时丢掉了场景深度，而光源小球是真正的 3D 物体，需要深度测试来决定该不该被墙挡住
    //所以把几何 pass 留在 gBuffer 里的深度 blit 到屏幕，补齐这个缺口
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, W, H, 0, 0, W, H, GL_DEPTH_BUFFER_BIT,
                      GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_DEPTH_TEST);
    // 用发光小球标记每个点光源的位置
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

  // 释放 GLFW 资源并退出
  glfwTerminate();
  return 0;
}
