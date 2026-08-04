#include <glad/glad.h>
#include <GLFW/glfw3.h> 
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // 当窗口大小发生变化时，调整 OpenGL 的视口大小。
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    // 如果按下 ESC 键，设置窗口应该关闭。
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main() {
    // 初始化 GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // 设置 OpenGL 版本为 3.3，并使用核心模式
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建一个窗口对象
    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // 将刚创建的窗口绑定为当前线程的 OpenGL 上下文。
    // 后续所有 OpenGL 调用都会作用在这个窗口对应的上下文上。
    glfwMakeContextCurrent(window);

    // 初始化 GLAD，加载当前显卡驱动提供的 OpenGL 函数地址。
    // glfwGetProcAddress 负责按函数名查询地址，GLAD 会把这些地址保存到对应的函数指针里。
    // 这一步必须在 glfwMakeContextCurrent 之后执行，否则没有当前上下文，GLAD 无法正确加载 OpenGL 函数。
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 渲染循环
    while (!glfwWindowShouldClose(window)) {

        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // 设置清空颜色缓冲区时使用的颜色

        // 清空颜色缓冲区
        glClear(GL_COLOR_BUFFER_BIT);

        // 交换前后缓冲区，把这一帧的绘制结果显示到窗口上。
        glfwSwapBuffers(window);

        // 处理窗口事件，例如关闭窗口、键盘输入、鼠标输入、窗口大小变化等。
        // 如果不调用它，窗口不会及时响应系统事件，可能表现为卡死或无法关闭。
        glfwPollEvents();
    }

    // 清理并退出
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
