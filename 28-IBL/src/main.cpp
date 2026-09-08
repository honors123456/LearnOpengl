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
#include <stdexcept>
#include <vector>

GLuint loadHDRTexture(const char *path)
{
  cv::Mat image = cv::imread(path, cv::IMREAD_ANYDEPTH | cv::IMREAD_COLOR);
  if (image.empty() || image.type() != CV_32FC3)
    throw std::runtime_error(std::string("Failed to load HDR image: ") + path);
  cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
  cv::flip(image, image, 0);
  GLuint texture = 0;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, image.cols, image.rows, 0,
               GL_RGB, GL_FLOAT, image.ptr<float>());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  return texture;
}

GLuint loadTexture(const char *path, bool srgb)
{
  cv::Mat image = cv::imread(path, cv::IMREAD_UNCHANGED);
  if (image.empty())
    throw std::runtime_error(std::string("Failed to load texture: ") + path);
  cv::flip(image, image, 0);
  GLenum format = GL_RED;
  GLenum internalFormat = GL_R8;
  if (image.channels() == 3) {
    cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
    format = GL_RGB;
    internalFormat = srgb ? GL_SRGB8 : GL_RGB8;
  } else if (image.channels() == 4) {
    cv::cvtColor(image, image, cv::COLOR_BGRA2RGBA);
    format = GL_RGBA;
    internalFormat = srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;
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
  return texture;
}

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

GLuint createCubemap(int size, bool mipmapped)
{
  GLuint texture = 0;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
  for (int face = 0; face < 6; ++face) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB16F,
                 size, size, 0, GL_RGB, GL_FLOAT, nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                  mipmapped ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  return texture;
}

void renderCube()
{
  static GLuint vao = 0, vbo = 0;
  if (vao == 0) {
    const float vertices[] = {
      -1,-1,-1, 1,-1,-1, 1,1,-1, 1,1,-1,-1,1,-1,-1,-1,-1,
      -1,-1,1, 1,-1,1, 1,1,1, 1,1,1,-1,1,1,-1,-1,1,
      -1,1,1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,1,1,
      1,1,1,1,1,-1,1,-1,-1,1,-1,-1,1,-1,1,1,1,1,
      -1,-1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,-1,-1,
      -1,1,-1,1,1,-1,1,1,1,1,1,1,-1,1,1,-1,1,-1};
    glGenVertexArrays(1, &vao); glGenBuffers(1, &vbo);
    glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);
  }
  glBindVertexArray(vao); glDrawArrays(GL_TRIANGLES,0,36); glBindVertexArray(0);
}

void renderQuad()
{
  static GLuint vao = 0, vbo = 0;
  if (vao == 0) {
    const float vertices[] = {-1,-1, 1,-1, 1,1, -1,-1, 1,1, -1,1};
    glGenVertexArrays(1,&vao); glGenBuffers(1,&vbo); glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER,vbo); glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,2*sizeof(float),nullptr);
  }
  glBindVertexArray(vao); glDrawArrays(GL_TRIANGLES,0,6); glBindVertexArray(0);
}

int main() {
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

  Shader pbrShader(SHADER_DIR "/pbrObjectShader.vert",
                   SHADER_DIR "/pbrObjectShader.frag");
  Shader lightShader(SHADER_DIR "/lightShader.vert",
                     SHADER_DIR "/lightShader.frag");
  GLuint irradianceMap = 0;
  GLuint prefilterMap = 0;
  GLuint hdrEnvironment = 0;
  try {
    hdrEnvironment = loadHDRTexture(SHADER_DIR "/../res/newport_loft.hdr");
  } catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
  }
  if (hdrEnvironment == 0) { glfwTerminate(); return -1; }

  Shader equirectShader(SHADER_DIR "/captureShader.vert", SHADER_DIR "/equirectangularToCubemapShader.frag");
  Shader irradianceShader(SHADER_DIR "/captureShader.vert", SHADER_DIR "/irradianceShader.frag");
  Shader prefilterShader(SHADER_DIR "/captureShader.vert", SHADER_DIR "/prefilterShader.frag");
  Shader brdfShader(SHADER_DIR "/brdfShader.vert", SHADER_DIR "/brdfShader.frag");
  Shader backgroundShader(SHADER_DIR "/backgroundShader.vert", SHADER_DIR "/backgroundShader.frag");

  GLuint fbo = 0, rbo = 0;
  glGenFramebuffers(1, &fbo); glGenRenderbuffers(1, &rbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo); glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
  glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
  std::array<glm::mat4, 6> captureViews = {
    glm::lookAt(glm::vec3(0),glm::vec3(1,0,0),glm::vec3(0,-1,0)), glm::lookAt(glm::vec3(0),glm::vec3(-1,0,0),glm::vec3(0,-1,0)),
    glm::lookAt(glm::vec3(0),glm::vec3(0,1,0),glm::vec3(0,0,1)), glm::lookAt(glm::vec3(0),glm::vec3(0,-1,0),glm::vec3(0,0,-1)),
    glm::lookAt(glm::vec3(0),glm::vec3(0,0,1),glm::vec3(0,-1,0)), glm::lookAt(glm::vec3(0),glm::vec3(0,0,-1),glm::vec3(0,-1,0))};
  GLuint envCubemap = createCubemap(512, true);
  glViewport(0,0,512,512); glBindFramebuffer(GL_FRAMEBUFFER,fbo);
  equirectShader.use(); equirectShader.setInt("equirectangularMap",0); equirectShader.setMat4("projection",captureProjection);
  glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D,hdrEnvironment);
  for (int i=0;i<6;++i) { equirectShader.setMat4("view",captureViews[i]); glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,envCubemap,0); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); renderCube(); }
  glBindTexture(GL_TEXTURE_CUBE_MAP,envCubemap); glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
  glBindFramebuffer(GL_FRAMEBUFFER,0); glViewport(0,0,W,H);
  GLuint brdfLUT=0; glGenTextures(1,&brdfLUT); glBindTexture(GL_TEXTURE_2D,brdfLUT); glTexImage2D(GL_TEXTURE_2D,0,GL_RG16F,512,512,0,GL_RG,GL_FLOAT,nullptr); glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); glBindFramebuffer(GL_FRAMEBUFFER,fbo); glBindRenderbuffer(GL_RENDERBUFFER,rbo); glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,512,512); glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,brdfLUT,0); glViewport(0,0,512,512); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); brdfShader.use(); renderQuad(); glBindFramebuffer(GL_FRAMEBUFFER,0); glViewport(0,0,W,H);
  irradianceMap=createCubemap(32,false);
  glBindRenderbuffer(GL_RENDERBUFFER,rbo); glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,32,32);
  irradianceShader.use(); irradianceShader.setInt("environmentMap",0); irradianceShader.setMat4("projection",captureProjection); glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_CUBE_MAP,envCubemap); glBindFramebuffer(GL_FRAMEBUFFER,fbo); glViewport(0,0,32,32);
  for(int i=0;i<6;++i){ irradianceShader.setMat4("view",captureViews[i]); glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,irradianceMap,0); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); renderCube(); }
  prefilterMap=createCubemap(128,true); glBindTexture(GL_TEXTURE_CUBE_MAP,prefilterMap); glGenerateMipmap(GL_TEXTURE_CUBE_MAP); prefilterShader.use(); prefilterShader.setInt("environmentMap",0); prefilterShader.setMat4("projection",captureProjection); glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_CUBE_MAP,envCubemap);
  for(int mip=0;mip<5;++mip){ int size=128>>mip; glBindRenderbuffer(GL_RENDERBUFFER,rbo); glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,size,size); glViewport(0,0,size,size); prefilterShader.setFloat("roughness",float(mip)/4.0f); for(int i=0;i<6;++i){ prefilterShader.setMat4("view",captureViews[i]); glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,prefilterMap,mip); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); renderCube(); }}
  glBindFramebuffer(GL_FRAMEBUFFER,0); glViewport(0,0,W,H);

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

  //球体顶点数据，只有位置信息和法线
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

  auto loadMaterial = [](const std::string &name) {
    const std::string root = std::string(SHADER_DIR) + "/../res/pbr/" + name + "/";
    return PBRMaterial{
        loadTexture((root + "albedo.png").c_str(), true),
        loadTexture((root + "normal.png").c_str(), false),
        loadTexture((root + "metallic.png").c_str(), false),
        loadTexture((root + "roughness.png").c_str(), false),
        loadTexture((root + "ao.png").c_str(), false)};
  };
  const std::array<PBRMaterial, 5> materials = {
      loadMaterial("rusted_iron"), loadMaterial("gold"),
      loadMaterial("grass"), loadMaterial("plastic"),
      loadMaterial("wall")};

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

  while (!glfwWindowShouldClose(window)) {
    processInput(window);

    float yaw = glm::radians(orbitYaw);
    float pitch = glm::radians(orbitPitch);
    glm::vec3 orbitOffset(
        orbitDistance * std::cos(pitch) * std::cos(yaw),
        orbitDistance * std::sin(pitch),
        orbitDistance * std::cos(pitch) * std::sin(yaw));
    camera.Position = orbitOffset;

    //观察矩阵和投影矩阵
    glm::mat4 view = glm::lookAt(camera.Position, glm::vec3(0.0f),glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(W) / H, 0.1f, 100.0f);

    glClearColor(0.56f, 0.43f, 0.31f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pbrShader.use();
    pbrShader.setMat4("view", view);
    pbrShader.setMat4("projection", projection);
    pbrShader.setVec3("camPos", camera.Position);
    pbrShader.setBool("enableDirectLights", enableDirectLights);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    pbrShader.setInt("irradianceMap", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    pbrShader.setInt("prefilterMap", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, brdfLUT);
    pbrShader.setInt("brdfLUT", 2);
    pbrShader.setInt("albedoMap", 3);
    pbrShader.setInt("normalMap", 4);
    pbrShader.setInt("metallicMap", 5);
    pbrShader.setInt("roughnessMap", 6);
    pbrShader.setInt("aoMap", 7);
    for (int i = 0; i < 4; ++i) {
      pbrShader.setVec3("lightPositions[" + std::to_string(i) + "]", lightPositions[i]);
      pbrShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);
    }

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

    glDepthFunc(GL_LEQUAL);
    backgroundShader.use();
    backgroundShader.setMat4("projection", projection);
    backgroundShader.setMat4("view", glm::mat4(glm::mat3(view)));
    backgroundShader.setInt("environmentMap", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    renderCube();
    glDepthFunc(GL_LESS);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
