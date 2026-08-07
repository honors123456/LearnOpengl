#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
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
bool dragging = false; // 左键是否按住（按住才旋转视角）
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

// 1. 鼠标移动回调函数：仅按住左键时旋转相机视角。
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    (void)window;

    if (!dragging) {
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

// 2. 鼠标按键回调：左键按下时捕获并隐藏光标开始拖动，松开后恢复。
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    (void)window;
    (void)mods;

    if (button != GLFW_MOUSE_BUTTON_LEFT) {
        return;
    }

    if (action == GLFW_PRESS) {
        dragging = true;
        firstMouse = true; // 按下瞬间重置基准，避免视角跳变
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else if (action == GLFW_RELEASE) {
        dragging = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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
    GLFWwindow* window=glfwCreateWindow(800,600,"Cube",nullptr,nullptr);
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
    GLenum specularFormat = specularImage.channels() == 4 ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(specularFormat),
                 specularImage.cols, specularImage.rows, 0, specularFormat,
                 GL_UNSIGNED_BYTE, specularImage.data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    {

    //物体着色器
    Shader objShader(SHADER_DIR "/objectShader.vert", SHADER_DIR "/objectShader.frag");

    //光源着色器（画一个白色小方块标记光源位置，便于观察点光源/聚光灯）
    Shader lightShader(SHADER_DIR "/lightShader.vert", SHADER_DIR "/lightShader.frag");

    //描边着色器（纯色 + 沿法线外扩，用于模板测试 PASS 2 画轮廓）
    Shader singleColorShader(SHADER_DIR "/singleColor.vert", SHADER_DIR "/singleColor.frag");

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

    // 光源位置小方块的 VAO（复用同一个 VBO，只需位置属性）
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glEnable(GL_DEPTH_TEST);

    // 光源位置（4 个点光源；平行光用方向，聚光灯跟随相机）
    glm::vec3 pointLightPositions[] = {
        glm::vec3( 0.7f,  0.2f,  2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3( 0.0f,  0.0f, -3.0f)
    };

    lastFrame = static_cast<float>(glfwGetTime());
    while(!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //开启深度测试和模板测试
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);

        // 0. 初始化与清空三大缓冲区
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glStencilMask(0xFF);	//开启所有位的写入权限,允许 OpenGL 把数据正常写入或清空模板缓冲区。
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        processInput(window);

        //两个物体都是同一个摄像机，同一个窗口进行观察，所以共用view和projection
        glm::mat4 view = camera.GetViewMatrix();
        float aspect = framebufferHeight > 0
            ? static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight)
            : 1.0f;
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspect, 0.1f, 100.0f);

        //光源对象
        lightShader.use();
        lightShader.setMat4("view", view);
        lightShader.setMat4("projection", projection);
        
        //物体对象
        objShader.use();
        objShader.setMat4("view",view);
        objShader.setMat4("projection",projection);
        objShader.setVec3("cameraPos",camera.Position);

        // 平行光（DirLight）
        objShader.setVec3("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
        objShader.setVec3("dirLight.ambient",  glm::vec3(0.05f, 0.05f, 0.05f));
        objShader.setVec3("dirLight.diffuse",  glm::vec3(0.4f, 0.4f, 0.4f));
        objShader.setVec3("dirLight.specular", glm::vec3(0.5f, 0.5f, 0.5f));

        // 4 个点光源（PointLight）
        for (int i = 0; i < 4; ++i) {
            std::string p = "pointLights[" + std::to_string(i) + "].";
            objShader.setVec3(p + "position", pointLightPositions[i]);
            objShader.setVec3(p + "ambient",  glm::vec3(0.05f, 0.05f, 0.05f));
            objShader.setVec3(p + "diffuse",  glm::vec3(0.8f, 0.8f, 0.8f));
            objShader.setVec3(p + "specular", glm::vec3(1.0f, 1.0f, 1.0f));
            objShader.setFloat(p + "constant",  1.0f);
            objShader.setFloat(p + "linear",    0.09f);
            objShader.setFloat(p + "quadratic", 0.032f);
        }

        // 聚光灯（SpotLight）：跟随相机，模拟手电筒
        objShader.setVec3("spotLight.position", camera.Position);
        objShader.setVec3("spotLight.direction", camera.Front);
        objShader.setFloat("spotLight.cutOff",      glm::cos(glm::radians(12.5f)));
        objShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
        objShader.setFloat("spotLight.constant",  1.0f);
        objShader.setFloat("spotLight.linear",    0.09f);
        objShader.setFloat("spotLight.quadratic", 0.032f);
        objShader.setVec3("spotLight.ambient",  glm::vec3(0.0f, 0.0f, 0.0f));
        objShader.setVec3("spotLight.diffuse",  glm::vec3(1.0f, 1.0f, 1.0f));
        objShader.setVec3("spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

        // 设置 Material 结构体
        objShader.setInt("material.diffuse", 0);
        objShader.setInt("material.specular", 1);
        objShader.setFloat("material.shininess", 64.0f);

        //设置纹理
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);

        //描边轮廓对象
        singleColorShader.use();
        glm::mat4 outlineModel = glm::mat4(1.0f);
        outlineModel = glm::translate(outlineModel, glm::vec3(0.0f, 0.0f, 0.0f));
        outlineModel = glm::rotate(outlineModel, glm::radians(20.0f), glm::vec3(1.0f, 0.3f, 0.5f));
        singleColorShader.setMat4("model", outlineModel);
        singleColorShader.setMat4("view", view);
        singleColorShader.setMat4("projection", projection);
        singleColorShader.setVec3("outlineColor", glm::vec3(0.98f, 0.28f, 0.26f)); // 深青色描边

        //pass 1
    	//如果模板测试和深度测试同时通过，将模板缓冲区对应位置的值替换（REPLACE）为 glStencilFunc 中指定的 ref 值。
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);	// 规则：无条件通过，指定 ref = 1
        glStencilMask(0xFF); 				// 允许向模板缓冲区写入数据

        // PASS 1
    	//绘制光源：光源小方块也写入模板值 1，描边会跳过光源区域，避免轮廓盖住光源
        lightShader.use();
        glBindVertexArray(lightCubeVAO);
        for (int i = 0; i < 4; ++i) {
            glm::mat4 lightModel = glm::mat4(1.0f);
            lightModel = glm::translate(lightModel, pointLightPositions[i]);
            lightModel = glm::scale(lightModel, glm::vec3(0.2f));
            lightShader.setMat4("model", lightModel);
            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        }
        glBindVertexArray(0);

    	//pass 1
        //绘制立方体，该立方体写入模板值1
        objShader.use();
        glBindVertexArray(VAO);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f,0.0f,0.0f));
        model = glm::rotate(model, glm::radians(20.0f), glm::vec3(1.0f, 0.3f, 0.5f));
        objShader.setMat4("model", model);

        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
        objShader.setMat3("normalMatrix", normalMatrix);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // PASS 2: 
    	//GPU 在绘制 Pass 2 的几何体时，会去检查对应位置的模板值。只有模板值【不等于 1】的像素才准画，等于 1 的直接丢弃（Discard）。
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);//禁用所有位的写入权限，无论渲染命令成功与否，或者 glStencilOp 设置了什么操作，模板缓冲区里的任何像素值都不会改变。
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    	
        glDisable(GL_DEPTH_TEST); // 关闭禁用深度测试，描边不被自身深度裁剪

		//pass 2
        //绘制纯色放大版模型（描边），仅在模板值 != 1 的区域渲染
        singleColorShader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &diffuseMap);
    glDeleteTextures(1, &specularMap);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
