#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <opencv2/opencv.hpp>

int main(){
    //初始化
    if(glfwInit() == GLFW_FALSE){
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    //设置版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //创建窗口
    GLFWwindow* window=glfwCreateWindow(800,600,"LearnOpenGL",nullptr,nullptr);
    if(window == nullptr)
    {
        std::cout << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    //初始化加载函数
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    //顶点数组
    float vertices[] = {
        // 位置(x,y,z)       // 颜色(r,g,b)      // UV(u,v)
        0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // 右上
        0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // 右下
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // 左下
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // 左上
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    //着色器
    const char* vertexShaderSource = "#version 330 core\n"
                "layout (location = 0) in vec3 aPos;\n"
                "layout (location = 1) in vec3 aColor;\n"
                "layout (location = 2) in vec2 aTexture;\n"
                "out vec3 ourColor;\n"
                "out vec2 TexCoord;\n"
                "void main(){\n"
                "   gl_Position = vec4(aPos,1.0);\n"
                "   ourColor = aColor;\n"
                "   TexCoord = aTexture;\n"
                "}" ;

    const char* fragShaderSource = "#version 330 core\n"
                "out vec4 FragColor;\n"
                "in vec3 ourColor;\n"
                "in vec2 TexCoord;\n"
                "uniform sampler2D ourTexture;\n"
                "void main(){\n"
                "   FragColor = texture(ourTexture, TexCoord) * vec4(ourColor, 1.0);\n"
                "}";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader,1,&vertexShaderSource,nullptr);
    glCompileShader(vertexShader);

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader,1,&fragShaderSource,nullptr);
    glCompileShader(fragShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram,vertexShader);
    glAttachShader(shaderProgram,fragShader);
    glLinkProgram(shaderProgram);


    GLuint VAO,VBO,EBO,texture;
    //向显存传递顶点数组
    glGenVertexArrays(1,&VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1,&VBO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);

    glGenBuffers(1,&EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indices),indices,GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,false,8*sizeof(float),(void*)0);
    glVertexAttribPointer(1,3,GL_FLOAT,false,8*sizeof(float),(void*)(3*sizeof(float)));
    glVertexAttribPointer(2,2,GL_FLOAT,false,8*sizeof(float),(void*)(6*sizeof(float)));
    //启动3个属性
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    //向显存传递纹理数据
    glGenTextures(1,&texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,texture);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    cv::Mat image = cv::imread("../res/KJLogo.png",cv::IMREAD_UNCHANGED);
    if(image.empty())
    {
        std::cerr<<" load image file failed!"<<std::endl;
        glfwTerminate();
        return -1;
    }

    cv::flip(image,image,0);
    cv::cvtColor(image,image,cv::COLOR_BGRA2RGB);

    int width,height,channels;
    width = image.cols;
    height = image.rows;
    channels = image.channels();

    if(!image.empty())
    {
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,00,GL_RGB,GL_UNSIGNED_BYTE,image.data);

        //自动触发gpu进行渐远级采样
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else{
        std::cout<<" failed to load texture "<<std::endl;
    }

    image.release();


    while(!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(shaderProgram, "ourTexture"), 0);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
