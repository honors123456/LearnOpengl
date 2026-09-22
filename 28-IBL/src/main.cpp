// LearnOpenGL 风格 IBL 示例：
// 1. OpenCV 加载 HDR 全景图；2. 离屏转换 environment cubemap；
// 3. 生成 irradiance、prefilter 和 BRDF LUT；4. 使用 PBR 材质渲染物体和 skybox。
#include "../camera/Camera.h"
#include "../shader/shader.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <array>
#include <cmath>
#include <iostream>
#include <vector>

struct PBRMaterial {
  GLuint albedo;
  GLuint normal;
  GLuint metallic;
  GLuint roughness;
  GLuint ao;
};

static constexpr int W = 1000;
static constexpr int H = 650;

Camera camera(glm::vec3(0.0f, 0.0f, 11.0f));
float lastX = W * 0.5f;
float lastY = H * 0.5f;
bool firstMouse = true;
bool rotating = false;
bool enableDirectLights = true;
bool directLightKeyPressed = false;
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
  if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
    if (!directLightKeyPressed)
      enableDirectLights = !enableDirectLights;
    directLightKeyPressed = true;
  } else {
    directLightKeyPressed = false;
  }
}

int main() {
  // 初始化窗口和 OpenGL 上下文。
  if (!glfwInit())
    return -1;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow *window = glfwCreateWindow(W, H, "LearnOpenGL IBL", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    glfwTerminate();
    return -1;
  }
  glfwSetFramebufferSizeCallback(window, resize);
  glfwSetCursorPosCallback(window, mouse);
  glfwSetMouseButtonCallback(window, mouseButton);
  glfwSetScrollCallback(window, scroll);
  glViewport(0, 0, W, H);
  glEnable(GL_DEPTH_TEST);

  // 环境捕获与 skybox 共用的立方体几何体。
  const float cubeVertices[] = {
      -1,-1,-1, 1,-1,-1, 1,1,-1, 1,1,-1,-1,1,-1,-1,-1,-1,
      -1,-1,1, 1,-1,1, 1,1,1, 1,1,1,-1,1,1,-1,-1,1,
      -1,1,1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,1,1,
      1,1,1,1,1,-1,1,-1,-1,1,-1,-1,1,-1,1,1,1,1,
      -1,-1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,-1,-1,
      -1,1,-1,1,1,-1,1,1,1,1,1,1,-1,1,1,-1,1,-1};
  GLuint cubeVAO = 0, cubeVBO = 0;
  glGenVertexArrays(1, &cubeVAO);
  glGenBuffers(1, &cubeVBO);
  glBindVertexArray(cubeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

  //离屏计算使用的全屏 quad。
  const float quadVertices[] = {-1,-1, 1,-1, 1,1, -1,-1, 1,1, -1,1};
  GLuint quadVAO = 0, quadVBO = 0;
  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);

  // pbrObjectShader：最终物体渲染。读取五张 PBR 材质图、四个点光源和三份 IBL 预计算数据。
  Shader pbrShader(SHADER_DIR "/pbrObjectShader.vert",
                   SHADER_DIR "/pbrObjectShader.frag");
  // lightShader：把点光源位置绘制成可见的发光标记球，不参与物体 PBR 光照计算。
  Shader lightShader(SHADER_DIR "/lightShader.vert",
                     SHADER_DIR "/lightShader.frag");
  GLuint irradianceMap = 0;
  GLuint prefilterMap = 0;
  // 读取 HDR 全景图。IMREAD_ANYDEPTH 保留 .hdr 的 float RGB 数据。
  cv::Mat hdrImage = cv::imread(SHADER_DIR "/../res/newport_loft.hdr",
                                cv::IMREAD_ANYDEPTH | cv::IMREAD_COLOR);
  if (hdrImage.empty() || hdrImage.type() != CV_32FC3) {
    std::cerr << "Failed to load HDR image.\n";
    glfwTerminate();
    return -1;
  }
  // OpenCV 默认 BGR、左上原点；转换到 OpenGL 使用的 RGB、左下原点。
  cv::cvtColor(hdrImage, hdrImage, cv::COLOR_BGR2RGB);
  cv::flip(hdrImage, hdrImage, 0);
  GLuint hdrEnvironment = 0;
  glGenTextures(1, &hdrEnvironment);
  glBindTexture(GL_TEXTURE_2D, hdrEnvironment);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, hdrImage.cols, hdrImage.rows,
               0, GL_RGB, GL_FLOAT, hdrImage.ptr<float>());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // captureShader.vert：离屏立方体捕获共用顶点着色器，把单位 cube 投影到六个 cubemap 面。
  // equirectangularToCubemapShader.frag：输入 HDR 经纬度全景图，输出 environment cubemap。
  Shader equirectShader(SHADER_DIR "/captureShader.vert", SHADER_DIR "/equirectangularToCubemapShader.frag");
  // irradianceShader.frag：对 environment cubemap 做半球余弦卷积，输出漫反射环境光。
  Shader irradianceShader(SHADER_DIR "/captureShader.vert", SHADER_DIR "/irradianceShader.frag");
  // prefilterShader.frag：以 roughness 为条件执行 GGX importance sampling，写入镜面反射 mip 链。
  Shader prefilterShader(SHADER_DIR "/captureShader.vert", SHADER_DIR "/prefilterShader.frag");
  // brdfShader.vert/.frag：在全屏 quad 上积分 BRDF，输出 NdotV × roughness 的二维查找表。
  Shader brdfShader(SHADER_DIR "/brdfShader.vert", SHADER_DIR "/brdfShader.frag");
  // backgroundShader：采样 environment cubemap 并绘制最终 skybox。
  Shader backgroundShader(SHADER_DIR "/backgroundShader.vert", SHADER_DIR "/backgroundShader.frag");

  // capture FBO：所有 cubemap 面和 BRDF LUT 都通过这个离屏 framebuffer 生成。
  // RBO 只提供深度附件，尺寸会随各个预计算阶段调整。
  GLuint fbo = 0, rbo = 0;
  glGenFramebuffers(1, &fbo);
  glGenRenderbuffers(1, &rbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
  // 每个 cubemap 面都是 90 度视角、1:1 宽高比。
  glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
  // 分别朝向 +X、-X、+Y、-Y、+Z、-Z 的六个观察矩阵。
  std::array<glm::mat4, 6> captureViews = {
    glm::lookAt(glm::vec3(0),glm::vec3(1,0,0),glm::vec3(0,-1,0)), glm::lookAt(glm::vec3(0),glm::vec3(-1,0,0),glm::vec3(0,-1,0)),
    glm::lookAt(glm::vec3(0),glm::vec3(0,1,0),glm::vec3(0,0,1)), glm::lookAt(glm::vec3(0),glm::vec3(0,-1,0),glm::vec3(0,0,-1)),
    glm::lookAt(glm::vec3(0),glm::vec3(0,0,1),glm::vec3(0,-1,0)), glm::lookAt(glm::vec3(0),glm::vec3(0,0,-1),glm::vec3(0,-1,0))};
  // 阶段 1：创建 512×512 HDR environment cubemap，接收全景图转换结果。
  GLuint envCubemap = 0;
  glGenTextures(1, &envCubemap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
  for (int face = 0; face < 6; ++face)
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glViewport(0,0,512,512);
  glBindFramebuffer(GL_FRAMEBUFFER,fbo);
  equirectShader.use();
  equirectShader.setInt("equirectangularMap",0);
  equirectShader.setMat4("projection",captureProjection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D,hdrEnvironment);
  for (int i=0;i<6;++i) {
    equirectShader.setMat4("view",captureViews[i]);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,envCubemap,0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }
  glBindTexture(GL_TEXTURE_CUBE_MAP,envCubemap);
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
  glBindFramebuffer(GL_FRAMEBUFFER,0);
  glViewport(0,0,W,H);

  // 阶段 2：在全屏 quad 上计算 BRDF LUT，RG 存储 Fresnel 的 scale 与 bias。
  GLuint brdfLUT=0;
  glGenTextures(1,&brdfLUT);
  glBindTexture(GL_TEXTURE_2D,brdfLUT);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RG16F,512,512,0,GL_RG,GL_FLOAT,nullptr);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glBindFramebuffer(GL_FRAMEBUFFER,fbo);
  glBindRenderbuffer(GL_RENDERBUFFER,rbo);
  glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,512,512);
  glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,brdfLUT,0);
  glViewport(0,0,512,512);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  brdfShader.use(); glBindVertexArray(quadVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindFramebuffer(GL_FRAMEBUFFER,0);
  glViewport(0,0,W,H);
  // 阶段 3：32×32 irradiance cubemap，存储低频漫反射环境光。
  irradianceMap = 0;
  glGenTextures(1, &irradianceMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
  for (int face = 0; face < 6; ++face)
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glBindRenderbuffer(GL_RENDERBUFFER,rbo);
  glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,32,32);
  irradianceShader.use();
  irradianceShader.setInt("environmentMap",0);
  irradianceShader.setMat4("projection",captureProjection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP,envCubemap);
  glBindFramebuffer(GL_FRAMEBUFFER,fbo);
  glViewport(0,0,32,32);
  for(int i=0;i<6;++i){
    irradianceShader.setMat4("view",captureViews[i]);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,irradianceMap,0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }

  // 阶段 4：128×128 prefilter cubemap。
  // 其各个 mip 保存不同 roughness 的镜面环境反射，用于 textureLod 采样。
  prefilterMap = 0;
  glGenTextures(1, &prefilterMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
  for (int face = 0; face < 6; ++face)
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_CUBE_MAP,prefilterMap);
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
  prefilterShader.use();
  prefilterShader.setInt("environmentMap",0);
  prefilterShader.setMat4("projection",captureProjection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP,envCubemap);
  // mip 0 到 4 分别对应 roughness 0.0 到 1.0；每级都需要绘制六个 cubemap 面。
  for(int mip=0;mip<5;++mip){
      int size=128>>mip;
      glBindRenderbuffer(GL_RENDERBUFFER,rbo);
      glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,size,size);
      glViewport(0,0,size,size);
      prefilterShader.setFloat("roughness",float(mip)/4.0f);
      for(int i=0;i<6;++i){
        prefilterShader.setMat4("view",captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,prefilterMap,mip);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
      }
  }
  glBindFramebuffer(GL_FRAMEBUFFER,0);
  glViewport(0,0,W,H);

  // UV 球体：位置本身就是单位球法线，因此每个顶点存 position + normal。
  constexpr int segments = 64;
  constexpr int rings = 32;
  std::vector<float> sphereVertices;
  std::vector<unsigned int> sphereIndices;
  sphereVertices.reserve((segments + 1) * (rings + 1) * 8);
  sphereIndices.reserve(segments * rings * 6);

  for (int ring = 0; ring <= rings; ++ring) {
    float v = static_cast<float>(ring) / rings;
    float phi = v * glm::pi<float>();
    for (int segment = 0; segment <= segments; ++segment) {
      float u = static_cast<float>(segment) / segments;
      float theta = u * glm::two_pi<float>();
      glm::vec3 position(std::sin(phi) * std::cos(theta), std::cos(phi),
                         std::sin(phi) * std::sin(theta));
      sphereVertices.insert(sphereVertices.end(),
                            {position.x, position.y, position.z,
                             position.x, position.y, position.z, u, v});
    }
  }
  for (int ring = 0; ring < rings; ++ring) {
    for (int segment = 0; segment < segments; ++segment) {
      unsigned int current = ring * (segments + 1) + segment;
      unsigned int next = current + segments + 1;
      sphereIndices.insert(sphereIndices.end(),
                           {current, next, current + 1, current + 1, next,
                            next + 1});
    }
  }

  // 上传球体顶点数据：每顶点 8 个 float = position(3) + normal(3) + UV(2)。
  // location 0/1/2 必须与 pbrObjectShader.vert 的输入布局一一对应。
  GLuint sphereVAO = 0, sphereVBO = 0, sphereEBO = 0;
  glGenVertexArrays(1, &sphereVAO);
  glGenBuffers(1, &sphereVBO);
  glGenBuffers(1, &sphereEBO);
  glBindVertexArray(sphereVAO);
  glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
  glBufferData(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(sphereVertices.size() * sizeof(float)),sphereVertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,static_cast<GLsizeiptr>(sphereIndices.size() * sizeof(unsigned int)),sphereIndices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),(void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),(void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),(void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  // 依次加载五套材质的 albedo、normal、metallic、roughness 和 AO 贴图。
  // 这里保留五个明确的资源目录，便于对照教程和调试单个材质。
  const std::string materialRoot = std::string(SHADER_DIR) + "/../res/pbr/";
  const std::array<std::string, 5> materialNames =
      {"rusted_iron", "gold", "grass", "plastic", "wall"};
  const std::array<std::string, 5> textureNames =
      {"albedo.png", "normal.png", "metallic.png", "roughness.png", "ao.png"};
  std::array<PBRMaterial, 5> materials{};
  for (std::size_t i = 0; i < materials.size(); ++i) {
    const std::string root = materialRoot + materialNames[i] + "/";

    // 每套材质依次加载五张贴图。albedo 按 sRGB 解码，其余数据贴图保持线性。
    for (std::size_t textureIndex = 0; textureIndex < textureNames.size(); ++textureIndex) {
      const std::string texturePath = root + textureNames[textureIndex];
      cv::Mat image = cv::imread(texturePath, cv::IMREAD_UNCHANGED);
      if (image.empty()) {
        std::cerr << "Failed to load texture: " << texturePath << std::endl;
        glfwTerminate();
        return -1;
      }

      // OpenCV 图像原点位于左上角，OpenGL 纹理坐标原点位于左下角，因此上下翻转。
      cv::flip(image, image, 0);
      GLenum format = GL_RED;
      GLenum internalFormat = GL_R8;
      if (image.channels() == 3) {
        cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
        format = GL_RGB;
        internalFormat = textureIndex == 0 ? GL_SRGB8 : GL_RGB8;
      } else if (image.channels() == 4) {
        cv::cvtColor(image, image, cv::COLOR_BGRA2RGBA);
        format = GL_RGBA;
        internalFormat = textureIndex == 0 ? GL_SRGB8_ALPHA8 : GL_RGBA8;
      }

      GLuint texture = 0;
      glGenTextures(1, &texture);
      glBindTexture(GL_TEXTURE_2D, texture);
      glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.cols, image.rows, 0,
                   format, GL_UNSIGNED_BYTE, image.data);
      glGenerateMipmap(GL_TEXTURE_2D);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

      // 将本次生成的纹理对象写入对应的 PBR 材质通道。
      if (textureIndex == 0)
        materials[i].albedo = texture;
      else if (textureIndex == 1)
        materials[i].normal = texture;
      else if (textureIndex == 2)
        materials[i].metallic = texture;
      else if (textureIndex == 3)
        materials[i].roughness = texture;
      else
        materials[i].ao = texture;
    }
  }

  // 四个点光源位置：前两盏在场景上方左右两侧（主光），
  // 后两盏贴近底部左右两侧（近距暖色补光，主循环里会画标记小球）。
  const std::array<glm::vec3, 4> lightPositions = {
      glm::vec3(-10.0f, 10.0f, 10.0f), glm::vec3(10.0f, 10.0f, 10.0f),
      glm::vec3(-10.0f, -10.0f, 10.0f), glm::vec3(10.0f, -10.0f, 10.0f)};
  // 对应 lightPositions 的四盏灯颜色：前两盏是暖白主光，后两盏是暗暖补光。
  // 数值为 HDR 强度（远超 1），着色器里会按距离平方衰减。
  const std::array<glm::vec3, 4> lightColors = {
      glm::vec3(300.0f), glm::vec3(300.0f),
      glm::vec3(300.0f), glm::vec3(300.0f)};

  // 主渲染循环：先绘制 PBR 材质球和直接光源，再最后绘制深度为最远处的 skybox。
  while (!glfwWindowShouldClose(window)) {
    processInput(window);

    // 将鼠标更新的球坐标相机参数转换为世界空间相机位置。
    float yaw = glm::radians(orbitYaw);
    float pitch = glm::radians(orbitPitch);
    glm::vec3 orbitOffset(
        orbitDistance * std::cos(pitch) * std::cos(yaw),
        orbitDistance * std::sin(pitch),
        orbitDistance * std::cos(pitch) * std::sin(yaw));
    camera.Position = orbitOffset;

    // 观察矩阵把世界空间变换到相机空间；投影矩阵建立透视投影。
    glm::mat4 view = glm::lookAt(camera.Position, glm::vec3(0.0f),glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(W) / H, 0.1f, 100.0f);

    // 清除上一帧颜色和深度。skybox 会在本帧最后覆盖未被物体写入的像素。
    glClearColor(0.56f, 0.43f, 0.31f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // PBR 绘制准备：先写入相机、IBL 纹理、材质纹理和解析点光源 uniform。
    pbrShader.use();
    pbrShader.setMat4("view", view);
    pbrShader.setMat4("projection", projection);
    pbrShader.setVec3("camPos", camera.Position);
    pbrShader.setBool("enableDirectLights", enableDirectLights);
    // texture unit 0~2 是 IBL 预计算数据：漫反射 irradiance、镜面 prefilter、BRDF LUT。
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    pbrShader.setInt("irradianceMap", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    pbrShader.setInt("prefilterMap", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, brdfLUT);
    pbrShader.setInt("brdfLUT", 2);
    // texture unit 3~7 是每个材质球自己的 PBR 贴图。
    pbrShader.setInt("albedoMap", 3);
    pbrShader.setInt("normalMap", 4);
    pbrShader.setInt("metallicMap", 5);
    pbrShader.setInt("roughnessMap", 6);
    pbrShader.setInt("aoMap", 7);
    // 不论开关状态都上传灯光参数；shader 依据 enableDirectLights 决定是否累加直接光。
    for (int i = 0; i < 4; ++i) {
      pbrShader.setVec3("lightPositions[" + std::to_string(i) + "]", lightPositions[i]);
      pbrShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);
    }

    // 依次绘制五种 textured PBR 材质球。
    // 每次循环将该材质的五张贴图绑定到固定纹理单元，再设置球体的世界位置。
    glBindVertexArray(sphereVAO);
    constexpr float spacing = 2.5f;
    for (std::size_t i = 0; i < materials.size(); ++i) {
      glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, materials[i].albedo);
      glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, materials[i].normal);
      glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, materials[i].metallic);
      glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, materials[i].roughness);
      glActiveTexture(GL_TEXTURE7); glBindTexture(GL_TEXTURE_2D, materials[i].ao);
      glm::mat4 model = glm::translate(
          glm::mat4(1.0f),
          glm::vec3((static_cast<float>(i) - 2.0f) * spacing, 0.0f, 0.0f));
      pbrShader.setMat4("model", model);
      glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sphereIndices.size()),
                     GL_UNSIGNED_INT, nullptr);
    }

    // 直接光开关开启时，额外绘制四个小球作为点光源位置与颜色的可视化标记。
    if (enableDirectLights) {
      lightShader.use();
      lightShader.setMat4("view", view);
      lightShader.setMat4("projection", projection);
      glBindVertexArray(sphereVAO);
      for (int i = 0; i < 4; ++i) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), lightPositions[i]);
        model = glm::scale(model, glm::vec3(0.5f));
        lightShader.setMat4("model", model);
        lightShader.setVec3("lightColor", lightColors[i]);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sphereIndices.size()),
                       GL_UNSIGNED_INT, nullptr);
      }
    }

    // skybox 必须最后绘制。LEQUAL + backgroundShader 的 xyww 写法保证其处在最远深度。
    glDepthFunc(GL_LEQUAL);
    backgroundShader.use();
    backgroundShader.setMat4("projection", projection);
    backgroundShader.setMat4("view", glm::mat4(glm::mat3(view)));
    backgroundShader.setInt("environmentMap", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS);

    // 提交本帧画面并继续处理键鼠事件。
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
