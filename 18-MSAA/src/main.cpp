#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

float cubeYaw = 20.0f;
float cubePitch = 20.0f;
float cubeScale = 2.0f;
bool draggingCube = false;
double lastMouseX = 0.0;
double lastMouseY = 0.0;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if(!draggingCube)
    {
        return;
    }

    float xoffset = static_cast<float>(xpos - lastMouseX);
    float yoffset = static_cast<float>(ypos - lastMouseY);
    lastMouseX = xpos;
    lastMouseY = ypos;

    const float sensitivity = 0.2f;
    cubeYaw += xoffset * sensitivity;
    cubePitch += yoffset * sensitivity;

    if(cubePitch > 89.0f)
    {
        cubePitch = 89.0f;
    }
    if(cubePitch < -89.0f)
    {
        cubePitch = -89.0f;
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if(button != GLFW_MOUSE_BUTTON_LEFT)
    {
        return;
    }

    if(action == GLFW_PRESS)
    {
        draggingCube = true;
        glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
    }
    else if(action == GLFW_RELEASE)
    {
        draggingCube = false;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    cubeScale += static_cast<float>(yoffset) * 0.2f;

    if(cubeScale < 0.2f)
    {
        cubeScale = 0.2f;
    }
    if(cubeScale > 20.0f)
    {
        cubeScale = 20.0f;
    }
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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // 通过 GLFW 获取 OpenGL 函数地址，并交给 GLAD 加载。
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // 顶点着色器：接收位置、颜色、纹理坐标，并应用 MVP 矩阵变换。
    const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n"
        "layout (location = 2) in vec2 aTexture;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "out vec3 ourColor;\n"
        "out vec2 TexCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        "    ourColor = aColor;\n"
        "    TexCoord = aTexture;\n"
        "}\n";

    // 片段着色器：当前只输出顶点颜色，暂时没有采样纹理。
    const char* fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec3 ourColor;\n"
        "in vec2 TexCoord;\n"
        "void main()\n"
        "{\n"
        "    FragColor = vec4(ourColor, 1.0);\n"
        "}\n";

    // 编译顶点着色器。
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    // 编译片段着色器。
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // 创建着色器程序，并把两个着色器链接成可使用的 GPU 程序。
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // 顶点数据：每个顶点依次包含 position、color、texture。
    float vertices[] = {
        // position           // color           // texture
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,

         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.5f, 0.5f, 0.5f,  0.0f, 1.0f,

        -0.5f, -0.5f, -0.5f,  1.0f, 0.5f, 0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.5f, 1.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.5f, 1.0f, 0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.5f,  0.0f, 1.0f,

         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.5f, 0.0f, 1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 0.5f, 0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.5f, 1.0f, 0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 1.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.5f, 1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.5f, 1.0f, 1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.5f,  0.0f, 1.0f,

        -0.5f, -0.5f, -0.5f,  0.2f, 0.4f, 0.8f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.8f, 0.4f, 0.2f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.2f, 0.8f, 0.4f,  1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.8f, 0.2f, 0.4f,  0.0f, 1.0f
    };

    // 索引数据：6 个面，每个面 2 个三角形，共 36 个索引。
    unsigned int indices[] = {
         0,  1,  2,   2,  3,  0,
         4,  5,  6,   6,  7,  4,
         8,  9, 10,  10, 11,  8,
        12, 13, 14,  14, 15, 12,
        16, 17, 18,  18, 19, 16,
        20, 21, 22,  22, 23, 20
    };

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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indices),indices,GL_STATIC_DRAW);

    // 告诉 OpenGL 如何从每 8 个 float 中解析位置、颜色和纹理坐标。
    glVertexAttribPointer(0,3,GL_FLOAT,false,8*sizeof(float),(void*)0);
    glVertexAttribPointer(1,3,GL_FLOAT,false,8*sizeof(float),(void*)(3*sizeof(float)));
    glVertexAttribPointer(2,2,GL_FLOAT,false,8*sizeof(float),(void*)(6*sizeof(float)));

    // 启用三个顶点属性。
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    // 解绑 VAO，避免后续误改它记录的状态。
    glBindVertexArray(0);

    //多重采样帧缓冲离屏渲染
    unsigned int multisampleFBO;
    glGenFramebuffers(1, &multisampleFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, multisampleFBO);

    // 1. 创建多重采样颜色纹理附件 (注意目标是 GL_TEXTURE_2D_MULTISAMPLE)
    unsigned int texColorBufferMulti;
    glGenTextures(1, &texColorBufferMulti);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texColorBufferMulti);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB8, 800, 600, GL_TRUE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texColorBufferMulti, 0);

    // 2. 创建多重采样渲染缓冲对象 (RBO) 用于深度和模板测试
    unsigned int rboMulti;
    glGenRenderbuffers(1, &rboMulti);
    glBindRenderbuffer(GL_RENDERBUFFER, rboMulti);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, 800, 600);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboMulti);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    while(!glfwWindowShouldClose(window))
    {
        // Pass 1渲染到多重采样帧缓冲，让场景在离屏中完成 MSAA。
        glBindFramebuffer(GL_FRAMEBUFFER, multisampleFBO);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 开启深度测试，让近处片段遮挡远处片段。
        glEnable(GL_DEPTH_TEST);

        // 使用已经链接好的着色器程序。
        glUseProgram(shaderProgram);

        // model：把模型从局部空间变换到世界空间。
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        //glm::radians(50.0f)旋转角度，glm::vec3(0.5f, 1.0f, 0.0f)一根旋转轴的方向向量。正方体是绕这根斜着的轴旋转
        model = glm::rotate(model, glm::radians(cubePitch), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(cubeYaw), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model,glm::vec3(cubeScale, cubeScale, cubeScale));

        // view：把世界空间变换到观察空间，相当于把摄像机向后拉。
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f));

        // projection：把观察空间变换到裁剪空间，形成透视效果。
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f/600.0f, 0.1f, 100.0f);

        // 把三个矩阵传给顶点着色器里的 uniform。
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // 绑定 VAO 并按索引绘制正方体。
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES,sizeof(indices)/sizeof(int),GL_UNSIGNED_INT,0);

        glBindVertexArray(0);

        //Pass 2 把多重采样结果 resolve（解析）到默认 framebuffer 显示。
        glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampleFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, 800, 600, 0, 0, 800, 600, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
