#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../shader/shader.h"
#include "../Camera/Camera.h"

// 全局相机实例
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = 800.0f / 2.0f;
float lastY = 600.0f / 2.0f;
bool firstMouse = true;
bool draggingCamera = false;
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口，后续所有 OpenGL 绘制都会输出到这个窗口。
    GLFWwindow* window=glfwCreateWindow(800,600,"Cubemap",nullptr,nullptr);
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

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    {

    //物体着色器
    Shader objShader(SHADER_DIR "/objectShader.vert", SHADER_DIR "/objectShader.frag");

    //天空盒着色器
    Shader skyShader(SHADER_DIR "/skyBoxShader.vert", SHADER_DIR "/skyBoxShader.frag");

    objShader.use();
    objShader.setInt("material.diffuse", 0);
    objShader.setInt("material.specular", 1);
    objShader.setInt("skybox", 2);

    skyShader.use();
    skyShader.setInt("skybox", 0);

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

    //天空盒顶点属性
    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    const GLsizei skyboxVertexCount = static_cast<GLsizei>(sizeof(skyboxVertices) / (3 * sizeof(float)));

    //天空盒顶点数据
    GLuint skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    //天空盒纹理（全屏3D效果，类似于3D虚拟厅）
    std::vector<std::string> skyFaces
    {
        SKYBOX_DIR "/right.jpg",
        SKYBOX_DIR "/left.jpg",
        SKYBOX_DIR "/top.jpg",
        SKYBOX_DIR "/bottom.jpg",
        SKYBOX_DIR "/front.jpg",
        SKYBOX_DIR "/back.jpg"
    };

    unsigned int skyTexture;
    glGenTextures(1,&skyTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP,skyTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for(int i = 0; i < static_cast<int>(skyFaces.size()); i++)
    {
        cv::Mat image = cv::imread(skyFaces[i],cv::IMREAD_UNCHANGED);
        if(image.empty())
        {
            std::cerr<<" read image failed! with "<<skyFaces[i]<<std::endl;
            return -1;
        }

        if (image.channels() == 4) {
            cv::cvtColor(image, image, cv::COLOR_BGRA2RGBA);
        } else if (image.channels() == 3) {
            cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
        } else if (image.channels() == 1) {
            cv::cvtColor(image, image, cv::COLOR_GRAY2RGB);
        } else {
            std::cerr << "Unsupported skybox channel count: " << image.channels()
                      << " with " << skyFaces[i] << std::endl;
            return -1;
        }

        GLenum format = image.channels() == 4 ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, static_cast<GLint>(format),
                     image.cols, image.rows, 0, format, GL_UNSIGNED_BYTE, image.data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); // 3D 纹理第 3 维坐标 R

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //定义光源位置
    glm::vec3 lightPos(1.0f, 1.0f, 2.0f);     // 光源在世界空间中的坐标

    lastFrame = static_cast<float>(glfwGetTime());
    while(!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //1.正常渲染场景中的3D 实体模型
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        //当执行深度清屏时，OpenGL 默认会把深度缓冲区（Depth Buffer）里全屏每一个像素的值全部填满为 1.0（代表目前没有任何东西，全是最远距离）。
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        processInput(window);

        //两个物体都是同一个摄像机，同一个窗口进行观察，所以共用view和projection
        glm::mat4 view = camera.GetViewMatrix();
        float aspect = framebufferHeight > 0
            ? static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight)
            : 1.0f;
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspect, 0.1f, 100.0f);

        //物体对象
        objShader.use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(cubePitch), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(cubeYaw), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
        objShader.setMat4("model",model);
        objShader.setMat4("view",view);
        objShader.setMat4("projection",projection);
        objShader.setMat3("normalMatrix",normalMatrix);
        objShader.setVec3("cameraPos",camera.Position);

        // 设置 Light 结构体
        objShader.setVec3("light.position", lightPos);
        objShader.setVec3("light.ambient",  glm::vec3(0.2f, 0.2f, 0.2f));
        objShader.setVec3("light.diffuse",  glm::vec3(0.5f, 0.5f, 0.5f)); // 调暗漫反射试一下效果
        objShader.setVec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f));

        // 设置 Material 结构体
        objShader.setInt("material.diffuse", 0);
        objShader.setInt("material.specular", 1);
        objShader.setFloat("material.shininess", 64.0f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyTexture);

        //绘制物体正方体
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES,indexCount,GL_UNSIGNED_INT,0);
        glBindVertexArray(0);

        //2.渲染天空盒
        //修改深度测试条件：允许深度等于 1.0 的片段通过（因为默认清屏深度是 1.0）
        glDepthFunc(GL_LEQUAL);
        skyShader.use();
        skyShader.setMat4("view", view);
        skyShader.setMat4("projection", projection);

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyTexture);
        glDrawArrays(GL_TRIANGLES, 0, skyboxVertexCount);
        glBindVertexArray(0);

        //恢复默认深度测试比较条件
        glDepthFunc(GL_LESS);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteTextures(1, &diffuseMap);
    glDeleteTextures(1, &specularMap);
    glDeleteTextures(1, &skyTexture);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
