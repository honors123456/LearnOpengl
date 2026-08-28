#include "../camera/Camera.h"
#include "../shader/shader.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <opencv2/opencv.hpp>

static constexpr int W = 800, H = 600;
Camera camera(glm::vec3(0, 0, 3));
float dt = 0, last = 0, lastX = W / 2.0f, lastY = H / 2.0f;
bool firstMouse = true, rotating = false;

// 窗口大小变化时，更新 OpenGL 视口。
void resize(GLFWwindow *, int w, int h) { glViewport(0, 0, w, h); }
void mouse(GLFWwindow *, double x, double y) {
  if (!rotating) {
    firstMouse = true;
    return;
  }
  float fx = (float)x, fy = (float)y;
  if (firstMouse) {
    lastX = fx;
    lastY = fy;
    firstMouse = false;
  }
  camera.ProcessMouseMovement(fx - lastX, lastY - fy);
  lastX = fx;
  lastY = fy;
}
void button(GLFWwindow *, int b, int a, int) {
  if (b == GLFW_MOUSE_BUTTON_LEFT) {
    rotating = a == GLFW_PRESS;
    firstMouse = true;
  }
}
void scroll(GLFWwindow *, double, double y) {
  camera.ProcessMouseScroll((float)y);
}
void input(GLFWwindow *w) {
  if (glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(w, true);
  if (glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS)
    camera.ProcessKeyboard(FORWARD, dt);
  if (glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS)
    camera.ProcessKeyboard(BACKWARD, dt);
  if (glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS)
    camera.ProcessKeyboard(LEFT, dt);
  if (glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS)
    camera.ProcessKeyboard(RIGHT, dt);
}

int main() {

  if (!glfwInit())
    return -1;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(W, H, "HDR & Bloom", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    return -1;

  glfwSetFramebufferSizeCallback(window, resize);
  glfwSetCursorPosCallback(window, mouse);
  glfwSetMouseButtonCallback(window, button);
  glfwSetScrollCallback(window, scroll);

  int framebufferWidth = 0;
  int framebufferHeight = 0;
  glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
  glViewport(0, 0, framebufferWidth, framebufferHeight);

  // 开启深度测试，保证场景中前方物体遮挡后方物体。
  glEnable(GL_DEPTH_TEST);

  // 立方体着色器和光源着色器
  Shader objShder(SHADER_DIR "/objectShader.vert", SHADER_DIR "/objectShader.frag");
  Shader lightShader(SHADER_DIR "/lightShader.vert", SHADER_DIR "/lightShader.frag");

  // 模糊着色器负责对亮部纹理执行水平和垂直高斯模糊。
  Shader blurShader(SHADER_DIR "/blurShader.vert", SHADER_DIR "/blurShader.frag");
  Shader compositeSHader(SHADER_DIR "/finalShader.vert",SHADER_DIR "/finalShader.frag");

  //
  // 全屏四边形覆盖整个屏幕，后处理阶段通过它采样输入纹理。
  float quad[] = {-1, 1, 0, 1, -1, -1, 0, 0, 1, -1, 1, 0,
                  -1, 1, 0, 1, 1,  -1, 1, 0, 1, 1,  1, 1};
  GLuint qvao, qvbo;
  glGenVertexArrays(1, &qvao);
  glGenBuffers(1, &qvbo);
  glBindVertexArray(qvao);
  glBindBuffer(GL_ARRAY_BUFFER, qvbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
  // 全屏四边形每个顶点包含两个屏幕坐标和两个纹理坐标。
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // 场景箱子：每个顶点包含位置、法线和纹理坐标，共 6 个面、36 个顶点。
  float cubeVertices[] = {
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
  GLuint vao, vbo;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  // 可见光源：用一个点表示 lightPos。
  GLuint lightVAO, lightVBO;
  glGenVertexArrays(1, &lightVAO);
  glGenBuffers(1, &lightVBO);
  glBindVertexArray(lightVAO);
  glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
  float lightVertex[] = {0.0f, 0.0f, 0.0f};
  glBufferData(GL_ARRAY_BUFFER, sizeof(lightVertex), lightVertex,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  cv::Mat image = cv::imread(DIFFUSE_TEXTURE_FILE, cv::IMREAD_COLOR);
  if (image.empty()) {
    std::cerr << "Failed to load texture: " << DIFFUSE_TEXTURE_FILE << '\n';
    glfwTerminate();
    return -1;
  }
  cv::flip(image, image, 0);
  cv::cvtColor(image, image, cv::COLOR_BGR2RGB);

  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0, GL_RGB,
               GL_UNSIGNED_BYTE, image.data);
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // HDR 帧缓冲：颜色附件 0 保存完整场景，颜色附件 1 保存亮部。
  GLuint hdrFBO, colors[2], depth;

  // 创建 HDR 帧缓冲
  glGenFramebuffers(1, &hdrFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
  
  // 创建2个颜色纹理附件
  for (int i = 0; i < 2; ++i) {
    glGenTextures(1, &colors[i]);
    glBindTexture(GL_TEXTURE_2D, colors[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, W, H, 0, GL_RGBA, GL_FLOAT,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                           GL_TEXTURE_2D, colors[i], 0);
  }

  // 创建深度缓冲附件
  glGenRenderbuffers(1, &depth);
  glBindRenderbuffer(GL_RENDERBUFFER, depth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, W, H);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, depth);

  //创建了两个颜色附件，默认只启用第一个，此时启用两个颜色附件，实现一次绘制同时输出完整场景和亮部。
  GLenum targets[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  //location 0 → GL_COLOR_ATTACHMENT0
  //location 1 → GL_COLOR_ATTACHMENT1
  //opengl以及glsl规范规定
  //glVertexAttribPointer → 配置顶点属性 location 
  //glDrawBuffers         → 配置片段颜色输出 location
  glDrawBuffers(2, targets);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cerr << "HDR framebuffer incomplete\n";

  //**Ping-Pong FBO 迭代：** 借助两个交替的 FBO 来回倒腾 3~5 次，让光晕极其平滑、均匀地向四周扩散开。
  // 创建 ping FBO
  GLuint ping[2], ptex[2];
  glGenFramebuffers(2, ping);
  glGenTextures(2, ptex);

  // 创建两个颜色附件：完整场景和亮部提取结果。
  for (int i = 0; i < 2; ++i) {
    glBindFramebuffer(GL_FRAMEBUFFER, ping[i]);
    glBindTexture(GL_TEXTURE_2D, ptex[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, W, H, 0, GL_RGBA, GL_FLOAT,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           ptex[i], 0);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glm::vec3 lightPos(1.4f, 1.2f, 1.5f);

  while (!glfwWindowShouldClose(window)) {

    float now = (float)glfwGetTime();
    dt = now - last;
    last = now;
    input(window);

    glm::mat4 view = camera.GetViewMatrix(),
              proj = glm::perspective(glm::radians(camera.Zoom), float(W) / H,
                                      0.1f, 100.0f);

    // 第一阶段：把 3D 场景渲染到 HDR 帧缓冲。
    // 阶段一：绑定 HDR FBO，开始渲染场景。
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    objShder.use();
    objShder.setMat4("view", view);
    objShder.setMat4("projection", proj);
    glm::mat4 cubeModel = glm::rotate(glm::mat4(1.0f), glm::radians(25.0f),
                                      glm::vec3(1.0f, 0.0f, 0.0f));
    cubeModel = glm::rotate(cubeModel, glm::radians(-30.0f),
                            glm::vec3(0.0f, 1.0f, 0.0f));
    cubeModel = glm::scale(cubeModel, glm::vec3(0.6f));
    objShder.setMat4("model", cubeModel);
    objShder.setVec3("viewPos", camera.Position);
    objShder.setVec3("lightPos", lightPos);
    objShder.setInt("diffuseMap", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    // 光源也写入 HDR FBO，因此它的高亮会参与 Bloom。
    lightShader.use();
    lightShader.setMat4("model", glm::translate(glm::mat4(1.0f), lightPos));
    lightShader.setMat4("view", view);
    lightShader.setMat4("projection", proj);
    glPointSize(24.0f);
    glBindVertexArray(lightVAO);
    glDrawArrays(GL_POINTS, 0, 1);

    // 第二阶段：从亮部纹理开始，交替执行水平和垂直模糊。
    // 阶段二：交替执行水平模糊和垂直模糊。
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    bool horizontal = true, first = true;
    blurShader.use();
    blurShader.setInt("image", 0);
    for (int i = 0; i < 10; ++i) {
      glBindFramebuffer(GL_FRAMEBUFFER, ping[horizontal]);
      blurShader.setInt("horizontal", horizontal);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, first ? colors[1] : ptex[!horizontal]);
      glBindVertexArray(qvao);
      glDrawArrays(GL_TRIANGLES, 0, 6);
      horizontal = !horizontal;
      first = false;
    }

    // 第三阶段：回到默认帧缓冲，合成 HDR 场景和 Bloom 光晕。
    // 阶段三：绑定默认帧缓冲，把结果显示到窗口。
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    compositeSHader.use();
    compositeSHader.setInt("hdrScene", 0);
    compositeSHader.setInt("bloomBlur", 1);
    // 曝光值控制 HDR 颜色进入色调映射前的整体亮度。
    compositeSHader.setFloat("exposure", 1.0f);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colors[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, ptex[!horizontal]);
    glBindVertexArray(qvao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
