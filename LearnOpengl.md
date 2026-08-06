



# LearnOpengl



## 第一章：OpenGL 核心概念与管线全貌 (OpenGL Core & Pipeline)



#### 1.Opengl的本质

opengl本质是一个状态机。

opengl状态，状态设置函数，状态应用函数。

**关键思维转变**：在 OpenGL 中，你几乎永远在重复这个模式 —— **绑定某个对象/设置状态 -> 调用绘制 -> 解绑对象**。



#### 2.现代图形渲染管线

**渲染管线** 指的是：将你给出的 3D 坐标数据，一步步经过 GPU 计算，最终转换成屏幕上 2D 像素阵列的全过程。

现代 OpenGL 使用的是**可编程管线（Programmable Pipeline）**。这意味着管线中的关键步骤允许你写自定义的小程序（即 **着色器 Shader**）来控制。

```glsl
[ 顶点数据 (Vertex Data) ]
          │
          ▼
┌──────────────────┐
│ 顶点着色器 (VS)  │  <-- 【可编程】计算顶点的最终坐标 (MVP 变换)
└─────────┬────────┘
          │
          ▼
┌──────────────────┐
│ 图元装配 (Assembly)│  将顶点组合成点、线或三角形
└─────────┬────────┘
          │
          ▼
┌──────────────────┐
│ 光栅化 (Raster)  │  将三角形离散化为屏幕上的“片段/像素 (Fragments)”
└─────────┬────────┘
          │
          ▼
┌──────────────────┐
│ 片段着色器 (FS)  │  <-- 【可编程】计算每个像素的最终颜色 (光照、纹理)
└─────────┬────────┘
          │
          ▼
┌──────────────────┐
│ 测试与混合 (Tests)│  深度测试、Alpha 混合，最终写入帧缓冲区 (Framebuffer)
└──────────────────┘
```



##### 两个最核心的 Shader：

1. ###### **顶点着色器 (Vertex Shader, VS)**：

   - **输入**：单个顶点的属性（位置、法线、纹理坐标等）。
   - **输出**：该顶点在剪裁空间（Clip Space）中的位置（赋值给内置变量 `gl_Position`）。
   - **作用**：处理 3D 空间变换（例如让模型旋转、平移、透视缩放）。

2. ###### **片段着色器 (Fragment Shader, FS)**：

   - **输入**：光栅化插值后得到的像素片段信息。
   - **输出**：该片段的最终 RGBA 颜色（输出到屏幕）。
   - **作用**：计算光照、采样纹理、阴影融合等几乎所有视觉效果。



#### 3.Opengl的对象与“绑定”机制

OpenGL 用 **对象（Objects）** 来管理 GPU 内存中的资源（如顶点数据、纹理、着色器）。

```c++
在 C++ 中，OpenGL 对象通常用一个 GLuint（无符号整型 ID）来表示。它的使用范式极其固定：

GLuint objectId = 0;

// 1. 创建/生成对象
glGenBuffers(1, &objectId);

// 2. 绑定对象到当前上下文（告诉 OpenGL：接下来对这个目标的操作都作用在 objectId 上）
glBindBuffer(GL_ARRAY_BUFFER, objectId);

// 3. 配置/传输数据
glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

// 4. 解绑对象（防止后续代码意外修改）
glBindBuffer(GL_ARRAY_BUFFER, 0);
```



#### 4.GPU上的三大内存缓冲区

| **简称**      | **全称**                              | **作用与比喻**                                               |
| ------------- | ------------------------------------- | ------------------------------------------------------------ |
| **VBO**       | Vertex Buffer Object（顶点缓冲对象）  | **显存里的原始数据块**。存储顶点的位置、颜色、法线等原始浮点数数组。 |
| **EBO / IBO** | Element Buffer Object（索引缓冲对象） | **顶点的绘制顺序清单**。存储顶点索引，避免重复定义公共顶点（省显存）。 |
| **VAO**       | Vertex Array Object（顶点数组对象）   | **数据解释说明书**。记录“哪一个 VBO 绑定在哪，数据应该怎么解读，对应的属性位置在哪”。 |



## 第二章：工程环境

#### 1.依赖库

**GLFW (Graphics Library Framework)**：

- **作用**：OpenGL 是跨平台的，它自己**不包含任何创建窗口或处理键盘鼠标输入的代码**。GLFW 帮我们屏蔽了 Windows (Win32 API)、macOS (Cocoa) 和 Linux (X11/Wayland) 的底层差异，用来创建渲染窗口和上下文（OpenGL Context）。

**GLAD (OpenGL Loading Library)**：

- **作用**：由于 OpenGL 驱动函数的位置在编译期无法确定（由显卡驱动在运行时动态提供），我们需要在运行时获取函数的内存地址。GLAD 帮你自动定位并加载所有 `gl*` 开头的函数指针（替代了老旧的 GLEW）。

**GLM (OpenGL Mathematics)**（后续章节引入）：

- **作用**：专门用于图形学数学计算的 Header-only 库，其语法与 GLSL 着色器语言高度保持一致。



#### 2.项目结构

```c++
MyOpenGLProject/
├── CMakeLists.txt
├── external/
│   ├── glad/            # 包含 glad.c 和 glad/KHR 头文件
│   └── glfw/            # GLFW 库或源码
└── src/
    └── main.cpp
    
    
cmake_minimum_required(VERSION 3.16)
project(OpenGL_Learning CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. 引入 GLFW (假设已下载预编译包或作为子目录)
find_package(glfw3 3.3 REQUIRED)

# 2. 编译 GLAD (glad.c 需要被编译为源文件)
add_library(glad external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# 3. 主可执行文件
add_executable(OpenGLApp src/main.cpp)

# 4. 链接依赖库
target_link_libraries(OpenGLApp PRIVATE glad glfw)
```



```c++
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// 窗口尺寸改变时的回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // 告诉 OpenGL 渲染视口的大小（左下角 x, y, 宽度, 高度）
    glViewport(0, 0, width, height);
}

// 处理键盘输入
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true); // 按下 Esc 键关闭窗口
    }
}

int main() {
    // -------------------------------------------------------------
    // 1. 初始化 GLFW 并配置 OpenGL 上下文版本
    // -------------------------------------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // 主版本号 3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // 次版本号 3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 核心模式 (Core-profile)

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // macOS 必需
#endif

    // -------------------------------------------------------------
    // 2. 创建窗口对象
    // -------------------------------------------------------------
    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL - Chapter 2", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // 将该窗口的上下文设置为当前线程的主上下文
    glfwMakeContextCurrent(window);
    // 注册窗口大小改变的回调
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // -------------------------------------------------------------
    // 3. 初始化 GLAD (必须在获取 OpenGL 上下文之后调用!)
    // -------------------------------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // -------------------------------------------------------------
    // 4. 渲染主循环 (Render Loop)
    // -------------------------------------------------------------
    while (!glfwWindowShouldClose(window)) {
        // A. 处理输入事件
        processInput(window);

        // B. 渲染指令 (State-using)
        // 设置清屏颜色 (State-setting: RGBA, 墨绿色)
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        // 清除颜色缓冲区，应用上面设置的颜色
        glClear(GL_COLOR_BUFFER_BIT);

        // C. 交换前后缓冲区 (Double Buffering)
        glfwSwapBuffers(window);
        // D. 轮询并处理操作系统事件（按键、鼠标移动、窗口移动等）
        glfwPollEvents();
    }

    // -------------------------------------------------------------
    // 5. 释放资源
    // -------------------------------------------------------------
    glfwTerminate();
    return 0;
}
```



#### 3.核心机制深度拆解

##### 1. 什么是 OpenGL Context（上下文）？

- OpenGL 上下文是一个存储了 **OpenGL 状态机所有变量和数据** 的结构体。
- `glfwMakeContextCurrent(window)` 的作用就是把当前窗口绑定的上下文“绑定到当前 C++ 线程”。**所有的 OpenGL API 调用，本质上都是在修改或读取当前线程绑定的这个上下文**。如果你在没有 Context 的线程直接调用 `glClear`，程序会立马崩溃。

##### 2. 为什么需要双缓冲区（Double Buffering）？

- **单缓冲区的隐患**：如果程序直接在显示器正在显示的图像上逐像素绘制，用户会看到画面从上到下绘制的闪烁和撕裂（Tearing）。
- **双缓冲区原理**：
  - **前缓冲区（Front Buffer）**：包含了当前屏幕上显示的最终图像。
  - **后缓冲区（Back Buffer）**：所有的渲染指令（如 `glClear`、绘制三角形）都在这个不可见的缓冲区中秘密进行。
  - **`glfwSwapBuffers(window)`**：当后缓冲区绘制完毕后，瞬间交换前后缓冲区指针，把完整绘制好的新帧呈现给用户。





## 第三章：显存分配、顶点属性与 GLSL 着色器



#### 1.标准化设备坐标 (NDC)

OpenGL 是一个 3D 渲染引擎，但屏幕是 2D 的。OpenGL 只会在 **标准化设备坐标（Normalized Device Coordinates, NDC）** 范围内的顶点进行渲染：

- $x, y, z$ 轴的取值范围均为 **$[-1.0, 1.0]$**。
- 任何超出这个范围的顶点都会被 GPU 裁剪（Clipped）掉。

```c++
float vertices[] = {
    // x      y     z
    -0.5f, -0.5f, 0.0f, // 左下角
     0.5f, -0.5f, 0.0f, // 右下角
     0.0f,  0.5f, 0.0f  // 顶角
};
```



#### 2.GPU 内存分配：创建与充填 VBO

数据现在在 CPU 的 RAM 中，我们需要把它发送到 **GPU 的显存（VRAM）** 中。这就需要用到 **VBO (Vertex Buffer Object)**。显存里面的数据块。

```c++
GLuint VBO;
// 1. 生成 1 个缓冲对象 ID，在gpu创建一块内存
glGenBuffers(1, &VBO);

// 2. 将 VBO 绑定到 GL_ARRAY_BUFFER 目标上
glBindBuffer(GL_ARRAY_BUFFER, VBO);

// 3. 将 CPU 内存的数据拷贝到 GPU 显存
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
```



##### 深入理解 `glBufferData` 的第四个参数：

GPU 会根据你给的显存提示（Usage Hint）来决定把这块内存放在显存的哪个区域（例如速度极快但容量小的区域，还是慢一点的共享内存）：

- **`GL_STATIC_DRAW`**：数据**几乎不会改变**（如房子、山体）。GPU 会将其优化放置以提高读取效率。
- **`GL_DYNAMIC_DRAW`**：数据**频繁改变**（如每帧都在变形的粒子/角色）。数据会反复更新和使用。
- **`GL_STREAM_DRAW`**：数据**每次绘制都要改变**。数据更新一次，只使用一次。



#### 3.编写 GLSL 着色器代码

数据进了显存，但 GPU 还不知道该怎么处理它。我们需要编写两个极简的着色器。

（1）顶点着色器 (Vertex Shader)

```glsl
#version 330 core
layout (location = 0) in vec3 aPos; // 位置变量的属性位置值为 0

void main() {
    // gl_Position 是 GLSL 内置的输出变量，必须赋给它一个 vec4 向量 (x, y, z, w)
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
```

（2）片段着色器 (Fragment Shader)

```glsl
#version 330 core
out vec4 FragColor; // 最终输出给屏幕像素的 RGBA 颜色

void main() {
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f); // 输出橙色 (R:1.0, G:0.5, B:0.2, A:1.0)
}
```



（3）动态编译与链接着色器 (C++ 端)

GLSL 代码是以字符串形式传递给 OpenGL 在**运行时**编译的：

```c++
// A. 编译顶点着色器
GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
glCompileShader(vertexShader);
// (建议检查编译错误：glGetShaderiv)

// B. 编译片段着色器
GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
glCompileShader(fragmentShader);

// C. 链接两个着色器到一个着色器程序 (Shader Program) 中
GLuint shaderProgram = glCreateProgram();
glAttachShader(shaderProgram, vertexShader);
glAttachShader(shaderProgram, fragmentShader);
glLinkProgram(shaderProgram);

// D. 链接完成后，原有的单独着色器对象就可以释放了
glDeleteShader(vertexShader);
glDeleteShader(fragmentShader);
```



（4）配置VAO & 顶点属性

这是图形学初学者最容易迷糊的一步。我们已经把连续的浮点数送进了 VBO 显存，但 GPU 不知道这串 `0101` 字节流代表什么。

`aPos` 在 GLSL 里声明为 `vec3`，包含 3 个 `float`。我们需要告诉 OpenGL 如何解析显存中的数据：

```
VBO 数据流（内存连续存着 9 个 float）:
[ -0.5, -0.5,  0.0,   0.5, -0.5,  0.0,   0.0,  0.5,  0.0 ]
  |<-- 顶点 0 -->|   |<-- 顶点 1 -->|   |<-- 顶点 2 -->|
  |<----- 步长 (Stride) = 3 * sizeof(float) ----->|
```

属性指针的配置

```c++
GLuint VAO;
glGenVertexArrays(1, &VAO);

// 1. 绑定 VAO，接下来所有的 VBO 绑定和 glVertexAttribPointer 配置都会被存入这个 VAO 中
glBindVertexArray(VAO);

// 2. 绑定 VBO 并充填数据
glBindBuffer(GL_ARRAY_BUFFER, VBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

// 3. 告诉 OpenGL 怎么解析这个属性,定义解包规则
glVertexAttribPointer(
    0,                  // 属性位置 (location = 0，与 Vertex Shader 中的 location = 0 对应)
    3,                  // 属性的大小 (vec3，由 3 个值组成)
    GL_FLOAT,           // 数据类型
    GL_FALSE,           // 是否希望数据被标准化 (Normalized，如 map 到 [0,1])
    3 * sizeof(float),  // 步长 (Stride)：连续两个顶点属性组之间的间隔字节数
    (void*)0            // 偏移量 (Offset)：数据在缓冲区中起始位置的字节偏移
);

// 4. 启用该顶点属性 (默认是禁用的)
glEnableVertexAttribArray(0);

// 5. 解绑 VAO（养成好习惯）
glBindVertexArray(0);
```

**`glBufferData`** 负责：把 CPU 的数据**拷贝到 GPU 显存（VBO）**。

**`glVertexAttribPointer`** 负责：定义**解包规则**，并把规则与当前 VBO 关联。

**`VAO`** 负责：把这些解包规则、属性启用状态、以及关联的 VBO/EBO 句柄**打包成一个快照**。



详细解释下：

VAO是顶点数组对象，类似于一个结构体：

```c++
// 概念模型：VAO 内部的状态数据结构
struct VAO_State {
    bool Enabled;                  // 是否启用（由 glEnableVertexAttribArray 设置）
    int Size;                      // 元素数量（如 vec3 就是 3）
    GLenum Type;                   // 数据类型（如 GL_FLOAT）
    GLsizei Stride;                // 步长（如 3 * sizeof(float)）
    const void* Pointer;           // 偏移量 (void*)0
    
    // 关键点！VAO 会隐式记录：配置这一步时，GL_ARRAY_BUFFER 上绑定的是哪个 VBO 的 ID！
    GLuint BoundVBO_ID;            
    
    // 如果有 EBO，EBO 的 ID 也会被保存在这里
    GLuint BoundEBO_ID;            
};
```

glGenVertexArrays(1, &VAO); 创建VAO这个“结构体”

`glVertexAttribPointer(0, ...)` ，OpenGL **不仅**记录了数据大小、类型、步长和偏移量，还会**将当前绑定在 `GL_ARRAY_BUFFER` 上的 VBO 句柄（ID）与属性槽位 0 强行绑定在一起，并永久保存在 VAO 中！**

##### 所以，VAO类似于一个解析记录，存储了一些解析信息。告诉gpu怎么解析顶点信息。

解绑（`glBindVertexArray(0)`）只是在“配置阶段”关掉录制开关；而到了“绘制阶段”，我们还会重新把它绑定回来！



(4)索引缓冲区（EBO）

我们为了直观，主要用的是 `glDrawArrays`（直接按顶点列表顺序画）。但当我们要画一个**矩形**时，矩形由 2 个三角形组成（共 6 个顶点），如果用 `glDrawArrays`，就需要定义 6 个顶点的数据，其中有 2 个顶点是完全重复的。为了**不浪费显存**，图形学引入了 **EBO（Element Buffer Object，元素缓冲区对象 / 索引缓冲区）** 和 **`glDrawElements`**。

假设要画一个矩形：

- **不用 EBO**：你需要传 6 个顶点：`[左下, 右下, 上,  右下, 右上, 上]`。重复传了 2 个顶点的数据（位置、颜色、UV 等），浪费显存和带宽。
- **使用 EBO**：你只需要传 **4 个独立的顶点**（0, 1, 2, 3），然后再传一个简单的索引数组：`[0, 1, 2,  1, 3, 2]`，告诉 GPU 按什么顺序去组合这 4 个顶点画出 2 个三角形。

```c++
// 1. 定义 4 个顶点（独立数据，无重复）
float vertices[] = {
     0.5f,  0.5f, 0.0f,  // 0: 右上
     0.5f, -0.5f, 0.0f,  // 1: 右下
    -0.5f, -0.5f, 0.0f,  // 2: 左下
    -0.5f,  0.5f, 0.0f   // 3: 左上
};

// 2. 定义索引数组（6个索引，组成2个三角形）
unsigned int indices[] = {
    0, 1, 3,  // 第一个三角形：右上 -> 右下 -> 左上
    1, 2, 3   // 第二个三角形：右下 -> 左下 -> 左上
};

GLuint VAO, VBO, EBO;
glGenVertexArrays(1, &VAO);
glGenBuffers(1, &VBO);
glGenBuffers(1, &EBO); // A. 申请 EBO 句柄

// B. 开门 (绑定 VAO)
glBindVertexArray(VAO);

// C. 绑定并填充 VBO (顶点数据)
glBindBuffer(GL_ARRAY_BUFFER, VBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

// D. 绑定并填充 EBO (索引数据)
// ⚠️ 注意目标是 GL_ELEMENT_ARRAY_BUFFER！
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

// E. 配置顶点属性指针 (位置等)
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);

// F. 关门 (解绑 VAO)
glBindVertexArray(0); 

// ⚠️ 极其关键的细节：
// VAO 会记录当前绑定的 GL_ELEMENT_ARRAY_BUFFER (EBO)。
// 所以在关门 (解绑 VAO) 之前，绝对不能手动解绑 EBO！
// 否则 VAO 就会把 EBO 的绑定关系给“丢掉”！
```



（5）最终绘制流程

主循环中

```c++
// 渲染循环内：
glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
glClear(GL_COLOR_BUFFER_BIT);

// 1. 激活着色器程序
glUseProgram(shaderProgram);

// 2. 绑定带有顶点格式记录的 VAO
glBindVertexArray(VAO);

// 3. 执行绘制图元指令
// 参数: 图元类型 (三角形), 起始索引, 顶点总数
glDrawArrays(GL_TRIANGLES, 0, 3);

//或者使用索引来绘制
glDrawElements(
        GL_TRIANGLES,      // 绘制图元类型
        6,                 // 要绘制的索引总数量 (2个三角形 * 3个顶点 = 6)
        GL_UNSIGNED_INT,   // 索引数据的类型 (必须与 indices 数组类型一致)
        0                  // EBO 内部的内存偏移量 (通常从 0 开始)
    );

// 4. 解绑
glBindVertexArray(0);
```



## 第四章：深入 GLSL 与着色器管道 (Shading & Color Interpolation)



#### 1.数据通道一：Uniform（CPU 到 GPU 的广播）

**Uniform** 是一种从 CPU 中的 C++ 代码向 GPU 中的着色器发送数据的方式。

##### Uniform 的三大铁律：

1. **全局性（Global）**：Uniform 变量在每个着色器程序对象中都是独一无二的，可以在任何着色器阶段（VS / FS）直接访问。
2. **只读性（Read-only）**：GLSL 代码不能修改 Uniform 的值，它只能由 CPU 写入。
3. **常量性（Uniform across primitives）**：在一次 `glDraw*` 调用中，所有顶点和像素看到的 Uniform 值是**完全一致的**（除非你在 C++ 端重新给它赋值）。



```glsl
#version 330 core
out vec4 FragColor;

// 在 C++ 端动态更新这个 uniform 变量
uniform vec4 ourColor;

void main() {
    FragColor = ourColor;
}
```



```c++
// A. 必须先激活着色器程序！
glUseProgram(shaderProgram);

// B. 动态计算颜色 (利用正弦函数使 green 值在 [0, 1] 之间循环)
float timeValue = glfwGetTime();
float greenValue = (sin(timeValue) / 2.0f) + 0.5f;

// C. 查询 uniform 变量在 Shader 中的内存位置 (Location)
int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");

// D. 更新 uniform 的值 (4f 代表接收 4 个 float 参数)
glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);

// E. 绘制（开门 -> 画画 -> 关门）
glBindVertexArray(VAO);
glDrawArrays(GL_TRIANGLES, 0, 3);
glBindVertexArray(0);
```



#### 2.数据通道二：In/Out 变量与光栅化插值（Rasterization Interpolation）

如果我们想让三角形的 3 个顶点分别拥有 **红、绿、蓝** 三种颜色，并且在三角形内部呈现平滑过渡的渐变效果，该怎么做？

这就需要用到 GLSL 的 **`in` / `out` 关键字**，以及 GPU **光栅化阶段的重心坐标插值（Barycentric Interpolation）**。

```c++
float vertices[] = {
    // 位置 (x,y,z)        // 颜色 (r,g,b)
     0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // 右下：纯红
    -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // 左下：纯绿
     0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // 顶角：纯蓝
};
```

```glsl
#version 330 core
layout (location = 0) in vec3 aPos;   // 位置属性
layout (location = 1) in vec3 aColor; // 颜色属性

out vec3 ourColor; // 向片段着色器输出颜色

void main() {
    gl_Position = vec4(aPos, 1.0);
    ourColor = aColor; // 将输入的颜色直接传给下一个阶段
}


#version 330 core
out vec4 FragColor;

in vec3 ourColor; // 必须与顶点着色器的 out 变量同名、同类型！

void main() {
    FragColor = vec4(ourColor, 1.0); // 最终像素颜色
}
```

![image-20260730091924936](D:\Code\LearnOpengl\image-20260730091924936.png)

##### 直觉上以为顶点数组，右下纯红，左下纯绿，顶上纯蓝，会均分这个三角形，但是却是出现了渐变颜色，为什么？

##### 这就是重心坐标插值的底层原理！

##### 在光栅化阶段，GPU 会计算每个像素中心点 $P$ 相对三角形三个顶点 $A, B, C$ 的 **重心坐标（Barycentric Coordinates） $(\alpha, \beta, \gamma)$**：

$$P = \alpha A + \beta B + \gamma C$$

且满足 $\alpha + \beta + \gamma = 1$。

GPU 的硬件插值单元会自动对所有标注了 `out` 的变量应用这个公式：

$$\text{Color}_P = \alpha \cdot \text{Color}_A + \beta \cdot \text{Color}_B + \gamma \cdot \text{Color}_C$$

当像素 $P$ 位于三角形正中央时，$\alpha = \beta = \gamma = \frac{1}{3}$，此时该像素得到的颜色就是红、绿、蓝按 1:1:1 混合出来的**灰色**！

硬件光栅化器 (Rasterizer) 根据重心坐标自动插值每个像素!



##### 如果我想让上面这个带有“红绿蓝渐变”的彩色三角形，**整体随时间变暗或变亮**，结合 Uniform 与插值出来的 `ourColor`，Uniform float alpha，

##### FragColor = vec4(ourColor, alpha); 能实现效果吗？

##### 	不能！这里直接写 `vec4(ourColor, alpha)` 会发现**没有透明变化**（或者完全没效果），这涉及到了两个极具代表性的图形学机制问题：

##### 机制一：管线后端的“测试与混合”机制 (Tests & Blending)

​	**本质问题**：**Alpha 值本身并不等于透明度，它只是一个普通的 4D 向量通道（$w$ 分量）。**

​	**底层原理**：在片段着色器计算完 `FragColor = vec4(RGB, A)` 后，数据会被送往管线的最后一步——**混合阶段（Blending Stage）**。

​	**默认没效果**：OpenGL 默认关闭了 `GL_BLEND`。在未开启混合时，GPU 会执行默认的 **写入覆盖** 操作——直接用当前的 RGB 覆盖掉帧缓冲区（Screen Buffer）里的像素，直接**忽略了 Alpha 分量**。

​	**关键认知**：要让 Alpha 产生透明效果，必须显式启用 `glEnable(GL_BLEND)` 并指定像素混合方程（`glBlendFunc`），让 GPU 将“新绘制像素”与“背景已有像素”按 Alpha 比例进行数学计算。

##### 机制二：着色器中的“颜色空间与光照数学” (Color Math)

- **本质问题**：**混淆了“透明度（Opacity/Alpha）”与“漫反射亮度（Brightness/Luminance）”的数学含义。**
- **底层原理**：
  - **透明度控制**：改变 Alpha 影响的是**物体与背景的叠加比例**（物体透过了背景色），背景是什么颜色，透出来的就是什么颜色。
  - **亮度控制**：改变亮度是**对 RGB 颜色向量进行标量乘法缩放**（$C_{\text{out}} = C_{\text{in}} \cdot k$）。
- **关键认知**：在图形学中，控制强弱、明暗、能量缩放的操作，本质上都是在 Shader 内部对 RGB 分量进行乘法（Multiply）运算；而 Alpha 只负责控制覆盖比例。

所以实现方法：

（1）开启glEnable(GL_BLEND)	

（2）FragColor = vec4(ourColor * brightness, 1.0);  在 Shader 中用 Uniform 因子乘以 RGB，brightness亮度因子





## 第五章：纹理贴图 (Textures) 与 GPU 采样机制

#### 1.纹理坐标系统 (UV Coordinates)

为了把一张 2D 图像贴到 3D 网格上，顶点数据中需要包含 **纹理坐标（Texture Coordinates）**，通常称为 **UV 坐标**。

- **UV 坐标的范围**：规定在 **$[0.0, 1.0]$** 之间。
- **原点位置**：
  - **OpenGL 规范**：原点 $(0.0, 0.0)$ 在图像的 **左下角**，右上角为 $(1.0, 1.0)$。
  - **大部分图像格式（如 PNG/JPG）**：原点在 **左上角**。

```
(0,1) ┌──────────────────┐ (1,1)
       │                  │
       │     2D 图像      │
       │                  │
 (0,0) └──────────────────┘ (1,0)
```

 当我们在顶点属性里定义了 3 个顶点的 UV 坐标后，GPU 的光栅化阶段会像插值颜色一样，自动为三角形内部的每一个像素插值出精准的 $(u, v)$ 坐标。



#### 2.采样机制（Sampling）

纹理上的基本单位叫 **纹像素（Texel）**，而屏幕上的基本单位叫 **像素（Pixel）**。它们几乎不可能一模一样大。

当一张 $256 \times 256$ 的小图贴在一个很大的墙面上（被放大了），或者贴在一个很远的物体上（被缩小了），GPU 必须决定**如何根据 UV 坐标去计算出最终的颜色**。这就是 **纹理过滤（Texture Filtering）**。

##### (1) 两种基础纹理过滤算法

- ##### **邻近过滤 (`GL_NEAREST`)**：

  - **原理**：选择与当前 UV 坐标最近的那个 Texel 的颜色。
  - **视觉表现**：形成明显的**马赛克/像素风**（Minecraft 的风格）。

- **线性过滤 (`GL_LINEAR`)**：

  - **原理**：根据 UV 坐标周围的 4 个 Texel 进行双线性插值（Bilinear Interpolation）。
  - **视觉表现**：边缘更平滑，但放大过度时会显得**平淡模糊**。

```c++
// 在 C++ 端为当前纹理设置过滤模式
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // 缩小模式
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  // 放大模式
```



##### (2) 解决远距离走样：多级渐远纹理 (Mipmapping)

假设你在远方渲染一座由 100 万像素纹理贴图构成的房子，在屏幕上它只占 $10 \times 10$ 个像素。GPU 在计算这 100 个像素时，如果简单采样，会导致严重的闪烁和锯齿（走样/Aliasing）。

**Mipmap 原理**： 在初始化时，GPU 会自动为原始图像生成一组尺寸依次减半的图像链（$256\times 256 \rightarrow 128\times 128 \rightarrow 64\times 64 \dots \rightarrow 1\times 1$）。根据物体距离相机的远近，GPU 会**自动选择合适分辨率的 Mipmap 层级**去采样。

```c++
// 配合 Mipmap 的常用缩小过滤：双线性插值采样 Mipmap + 在相邻 Mipmap 层级间进行线性混合（三线性过滤）
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
```



#### 3.环绕方式 (Wrapping)：当 UV 超出 $[0.0, 1.0]$ 范围时

如果顶点的 UV 坐标传了 $(2.0, 2.0)$，超越了 $[0, 1]$ 范围，OpenGL 提供了四种处理模式：

| **模式**                 | **表现行为**                                       |
| ------------------------ | -------------------------------------------------- |
| **`GL_REPEAT`**          | 默认行为。重复平铺图像（用于地砖、墙面重复贴图）。 |
| **`GL_MIRRORED_REPEAT`** | 镜像重复平铺。                                     |
| **`GL_CLAMP_TO_EDGE`**   | 边缘超出的部分强制拉伸边缘像素的颜色。             |
| **`GL_CLAMP_TO_BORDER`** | 超出部分直接填充用户指定的自定义边框颜色。         |



#### 4.C++ 加载图像与创建 OpenGL 纹理对象

##### (1) 顶点数组和图像加载

```c++
float vertices[] = {
    // 位置(x,y,z)       // 颜色(r,g,b)      // UV(u,v)
     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // 右上
     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // 右下
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // 左下
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // 左上
};
```

```c++
#include <opencv2/opencv.hpp>

GLuint texture;
glGenTextures(1, &texture);	//gpu中开辟显存
glBindTexture(GL_TEXTURE_2D, texture);

//设置纹理环绕与过滤参数,配置当前绑定的纹理在渲染时该如何处理边界和缩放。
//置超出[0.0, 1.0]范围时的“包裹/环绕模式（Wrapping）”,在 S 轴（水平）和 T 轴（垂直）上都设置为 GL_REPEAT（重复模式）。
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // S 轴相当于 U 轴
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // T 轴相当于 V 轴

//纹理被“缩小”时的采样过滤策略，指定采用 三线性过滤（Trilinear Filtering）。
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//纹理被“放大”时的采样过滤策略,指定采用 双线性插值（Bilinear Interpolation）。
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

// 解决图像上下颠倒问题
int width, height, nrChannels;
cv::Mat image= cv::imread("container.jpg", cv::IMREAD_UNCHANGED);
width = image.cols;
height = image.rows;
nrChannels = image.channels();
//翻转
cv::flip(image,image,0);

//cv::Mat默认BGR
cv::cvtColor(
image,
image,
cv::COLOR_BGR2RGB
);

if (!image.empty()) {
    // 将图像数据传输到 GPU 显存
    glTexImage2D(
        GL_TEXTURE_2D, 
        0,                  // Mipmap 层级，0 代表 Base Level
        GL_RGB,             // GPU 存储格式
        width, height, 0, 
        GL_RGB,             // 源图数据格式 (RGBA/RGB)
        GL_UNSIGNED_BYTE,   // 数据类型 (8 bit unsigned char)
        image.data                // 图像内存指针
    );
    
    // 自动触发 GPU 生成 Mipmap 链
    glGenerateMipmap(GL_TEXTURE_2D);
} else {
    std::cout << "Failed to load texture" << std::endl;
}
//释放内存
image.release();
```

##### 注意： 加载 JPG（RGB 格式）或者某些奇数分辨率的图片时，纹理渲染出来严重变形、倾斜，甚至程序直接 Crash！

##### 原因：OpenGL 默认假设每行像素数据的字节数是 **4 字节对齐（4-byte alignment）** 的（比如 RGBA 每像素 4 字节，刚好对齐）。

##### 			但如果图像是 **RGB 格式**（每像素 3 字节），且图像宽度不是 4 的倍数（比如 $99 \times 99$ 分辨率），每行字节数就无法被 4 整除。GPU 解析内存就会错位！

**解决方案**：在调用 `glTexImage2D` 传 RGB 数据之前，手动把解包对齐改为 **1 字节对齐**：

```c++
// 告诉 OpenGL 图像内存字节是紧密排列的（按 1 字节对齐）
glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
```



##### （2）GLSL中采样纹理

在 GLSL 中，采样纹理使用的是内置的专有类型 **`sampler2D`**。它本质上也是一个 **Uniform**！

```glsl
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec3 ourColor;
out vec2 TexCoord;

void main() {
    gl_Position = vec4(aPos, 1.0);
    ourColor = aColor;
    TexCoord = aTexCoord; // 将 UV 坐标原样传给 FS
}


#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

// 纹理采样器 (Uniform)
uniform sampler2D ourTexture;

void main() {
    // 内置函数 texture(采样器, 插值后的UV坐标) 返回一个 vec4 RGBA 颜色
    // 我们还可以将其与顶点颜色 ourColor 进行相乘混合！
    //如果直接写 FragColor = texture(ourTexture, TexCoord);，渲染出来的就是图片的原汁原味（100% 贴图颜色）。
    //但是在实际游戏和图形开发中，顶点颜色常常用来做 着色叠加（Tinting/Color Modulation） 或 特殊效果遮罩。
    //数学本质：颜色乘法本质是色彩滤波。任何颜色乘以白色 (1,1,1) 保持不变；乘以黑色 (0,0,0) 变成纯黑；乘以红色 (1,0,0) 则只保留纹理中的红色分量。
    FragColor = texture(ourTexture, TexCoord) * vec4(ourColor, 1.0);
    //顶点数组的颜色属性是红，绿，蓝，黄 ，因为 GPU 的光栅化阶段会对 ourColor 进行重心坐标插值，你会看到图片贴图被叠加了一层平滑过渡的彩虹渐变滤镜
}
```

##### 为什么代码里没有写 `glUniform1i` 设置 ourTexture？

**因为 OpenGL 有一个默认偷懒机制：槽位 0 是默认激活的！**

1. **纹理单元（Texture Unit）的概念**： GPU 内部有一排专门挂载纹理的“插槽”，编号为 `0, 1, 2, 3...`（即 `GL_TEXTURE0`, `GL_TEXTURE1`...）。
2. **Shader 中 Uniform 的默认值**： GLSL 里的 `uniform sampler2D` 如果你不显式去赋值，它的初始默认值就是 **`0`**（代表指向 `GL_TEXTURE0` 这个插槽）。
3. **OpenGL 默认激活的插槽**： 当程序启动时，OpenGL 默认激活的就是 `GL_TEXTURE0`（相当于隐式执行了 `glActiveTexture(GL_TEXTURE0)`）。

**连起来看发生的全过程：**

- 你在 CPU 端 `glBindTexture(GL_TEXTURE_2D, texture)`，因为默认处于 `GL_TEXTURE0` 槽位，所以你的图片数据被挂到了 **0 号槽位**上。
- 在 Shader 端，`uniform sampler2D ourTexture;` 默认读取的就是 **0 号槽位**。
- **两边恰好对上了！** 所以即使你一行 Uniform 设置代码都没写，图片也能正常渲染出来！

如果同时使用多张纹理，就必须显式设置texture了。



##### (3) 多纹理单元与纹理绑定机制（Texture Units）

如果你想在一个 Shader 里同时叠加两张纹理（比如：底层墙面纹理 + 表层木纹/笑脸纹理），该怎么做？

这就是 **纹理单元（Texture Unit）** 的作用。

- GPU 硬件拥有多个纹理槽位：`GL_TEXTURE0`, `GL_TEXTURE1`, `GL_TEXTURE2` ...
- `sampler2D` 的 Uniform 值，存储的就是**这个采样器绑定在哪个纹理槽位上**（整数 `0`, `1`, `2`...）。

```c++
// 在渲染主循环里绑定多张纹理：

// 1. 激活 0 号纹理槽位，并绑定 texture1
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, texture1);

// 2. 激活 1 号纹理槽位，并绑定 texture2
glActiveTexture(GL_TEXTURE1);
glBindTexture(GL_TEXTURE_2D, texture2);

// 3. 在 Shader 中将槽位分配给对应的 sampler2D 变量
glUseProgram(shaderProgram);
glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0); // 告诉 sampler1 去 0 号槽位采样
glUniform1i(glGetUniformLocation(shaderProgram, "texture2"), 1); // 告诉 sampler2 去 1 号槽位采样
```

##### (4)最终渲染流程

```c++
while(!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1)
    
    	glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    	//绘制完毕，解绑 VAO
    	glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
```



## 第六章：3D 坐标变换与 MVP 矩阵 (MVP Matrix Pipeline)

在前面的章节中，我们画的所有东西都死死地钉在 $[-1, 1]$ 的二维屏幕空间里。但在 3D 图形学中，我们要把一个三维物体的坐标（如 $x, y, z$），最终投影并映射到二维屏幕的像素点上。

#### 1.五大坐标空间与流水线全景

在 OpenGL 中，一个顶点数据从写在 C++ 数组里，到最终变成屏幕上的像素，需要经历 **5 个不同的坐标空间（Coordinate Spaces）**：

```c++
[ 1. 模型/局部空间 (Model/Local Space) ] 
       │ 
       ▼  × Model 矩阵 (平移, 旋转, 缩放)
[ 2. 世界空间 (World Space) ] 
       │ 
       ▼  × View 矩阵 (摄像机位置/朝向变换)
[ 3. 观察空间 (View/Camera Space) ] 
       │ 
       ▼  × Projection 矩阵 (透视/正交投影)
[ 4. 裁剪空间 (Clip Space) ] ── (齐次坐标 [x, y, z, w]，裁剪掉超出 [-w, w] 的顶点)
       │ 
       ▼  【GPU 硬件自动执行】：透视除法 (Perspective Division, 即 xyz / w)
[ 标准设备坐标 (NDC Space) ] ── (坐标强行挤压到 [-1.0, 1.0] 的标准化立方体)
       │ 
       ▼  【GPU 硬件自动执行】：视口变换 (glViewport)
[ 5. 屏幕空间 (Screen Space) ] ── (映射到屏幕像素分辨率, 如 1920x1080)
```



其实图形学里的“空间”，**本质上就是“以谁为坐标原点（参考系）”**。

为了彻底搞懂这五大空间，我们可以用一个**非常接地气的现实例子**：

👉 **假设我们要在一个电影片场里，拍一张小熊玩偶放在桌子上的照片。**

我们将跟着这个小熊玩偶，看看它是怎么一步步被记录到你的手机屏幕上的！

##### 第一步：局部空间（Local Space / Object Space）

- **现实类比**：玩具厂在生产这个“小熊玩偶”时用的设计图纸。

- **原点 $(0,0,0)$ 在哪里**：在小熊自己的几何中心（比如小熊的肚脐眼位置）。

- **为什么需要它**：

  在工厂做模型时，设计师根本不需要知道这个小熊将来会被卖到哪里、放在哪张桌子上。设计师只关心：“小熊的鼻子距离肚脐眼往上 5 厘米（$+Y$），右手距离肚脐眼往右 10 厘米（$+X$）”。

- **顶点坐标**：我们在 C++ 代码里手写的那个 `vertices[]` 数组，里面的坐标就是**局部坐标**。

##### 第二步：世界空间（World Space）

- **现实类比**：把小熊摆到电影片场的某张桌子上。

- **原点 $(0,0,0)$ 在哪里**：整个片场的绝对中心（比如片场舞台正中央的地板点）。

- **为什么要变换**：

  场景里除了小熊，还有桌子、椅子、房子。如果大家都用自己的肚脐眼当原点，它们就会全部重叠在同一个地方！我们需要把它们摆到整个大场景（世界）的各个位置上。

- **怎么变过来的（Model 矩阵）**：

  我们拿着小熊，做了三件事：

  1. **缩放（Scale）**：觉得小熊太大了，缩小一半。
  2. **旋转（Rotate）**：把小熊面向门口。
  3. **平移（Translate）**：把小熊搬到房间东北角的桌子上（比如世界坐标 `(5, 2, -10)`）。

  **这组“放大缩小 + 旋转 + 搬运”的操作打包起来，就是 Model（模型）矩阵！**

##### 第三步：观察空间（View Space / Camera Space）

- **现实类比**：摄像师扛着摄影机，走到了小熊面前准备拍照。

- **原点 $(0,0,0)$ 在哪里**：**摄像机的镜头中心！**

- **为什么要变换**：

  世界原点在哪，摄像机根本不在乎。摄像机只关心：“这个小熊距离我的**镜头前方**有多远？在镜头偏左还是偏右？”

- **怎么变过来的（View 矩阵）**：

  在计算机里，并没有真正的“相机”。所谓的“把相机向后移动 5 米”，在数学上，等于**把整个片场里所有的东西（小熊、桌子、房子）全部向前拉近 5 米**！

  **这种“以摄像机镜头为中心，把全世界所有东西都重新量一遍距离”的操作，就是 View（观察）矩阵！**

  现在，小熊的坐标变成了“在镜头前方 3 米，偏左 1 米”。

##### 第四步：裁剪空间（Clip Space）

- **现实类比**：摄影机取景框内部的视野范围。

- **原点 $(0,0,0)$ 在哪里**：取景框的正中央。

- **为什么要变换**：

  片场太大了，但摄影机的镜头视野是有限的（就像一个金字塔形状的“视锥体”）。**视野之外的东西（比如在你身后的墙壁），我们根本不需要渲染！**

- **怎么变过来的（Projection 矩阵）**：

  我们用 **Projection（投影）矩阵** 做了一件神奇的事：它把那个像金字塔一样的视野空间，**强行挤压成了一个标准的正方体（Box）**。

  - 只要在这个正方体内部的顶点，保留！
  - 超出这个正方体边缘的顶点，**直接剪掉（Clipping）**！这就是“裁剪空间”名称的由来。

##### 💡 核心补充：这里 $w$ 分量悄悄发挥了作用！

经过投影矩阵计算后，每个顶点的四维坐标变成了 $(x, y, z, w)$。

此时的 **$w$ 分量记录的正是该点距离摄像机镜头的实际物理距离（Depth）**！距离越远，$w$ 值越大。

##### 硬件自动执行步骤：透视除法（NDC 空间）

这一步**不需要你写代码**，GPU 硬件会自动对每一个顶点做一件事：

把坐标除以 $w$：

$$\text{新坐标} = \begin{pmatrix} x / w \\ y / w \\ z / w \end{pmatrix}$$

**为什么必须除以 $w$？这就是“近大远小”的秘密！**

- 远处的山，距离镜头 100 米 $\to$ $w = 100$。坐标 $(x, y)$ 除以 100，瞬间缩成了极小的一点。
- 嘴边的苹果，距离镜头 1 米 $\to$ $w = 1$。坐标 $(x, y)$ 除以 1，大小几乎不变。

做完除法后，所有还在视野内的顶点，它们的 $x, y, z$ 坐标都会被精准地拉伸/压缩到 **[-1.0, 1.0]** 的区间里。这个标准化的小立方体空间，就叫做 **NDC 空间（Standardized Device Coordinates）**。

##### 第五步：屏幕空间（Screen Space）

- **现实类比**：照片最终打印在纸上，或者显示在你的手机屏幕上（比如 $1920 \times 1080$ 像素）。

- **原点 $(0,0)$ 在哪里**：屏幕的左下角（或左上角）。

- **怎么变过来的（Viewport 视口变换）**：

  GPU 拿着刚刚那个 $[-1, 1]$ 的 NDC 立方体，做最后一步映射：

  - $x$ 从 $[-1, 1]$ 映射到 $[0, 1920]$ 像素。
  - $y$ 从 $[-1, 1]$ 映射到 $[0, 1080]$ 像素。
  - $z$ 被存进**深度缓冲区（Z-Buffer）**，用来判断“谁挡住了谁”。

##### 极简总结：小熊的一生

1. **局部空间**：小熊在工厂图纸上的原始尺寸。（原点：肚脐眼）

2. **世界空间**：小熊被摆在了房间的哪个位置。（原点：房间中心）

3. **观察空间**：小熊在摄像机镜头前方的什么位置。（原点：摄像机镜头）

4. **裁剪空间**：小熊有没有在摄像机的视野框里，并在齐次坐标 $w$ 里记下距离。

5. **NDC 空间**：（GPU 自动除以 $w$）把视野里的所有东西压缩进一个 $[-1, 1]$ 的标准立方体，产生“近大远小”。

6. **屏幕空间**：（GPU 自动映射）把 $[-1, 1]$ 转换成具体的显示器像素（比如第 500 行，第 300 列）。

   

#### 2.矩阵

##### 1. 向量基础运算（Vectors）

向量 $\vec{v} = \begin{bmatrix} x \\ y \end{bmatrix}$ 在代数上是一列数，在空间中是从原点出发的**带有方向和长度的箭头**。

##### ① 向量加法 ($\vec{u} + \vec{v}$)

- **代数计算：** 对应分量相加。$\begin{bmatrix} u_1 \\ u_2 \end{bmatrix} + \begin{bmatrix} v_1 \\ v_2 \end{bmatrix} = \begin{bmatrix} u_1 + v_1 \\ u_2 + v_2 \end{bmatrix}$

- **空间解释：** **平移叠加（首尾相连法 / 平行四边形法则）**。

  - 先沿着 $\vec{u}$ 走，再从 $\vec{u}$ 的终点出发沿着 $\vec{v}$ 的方向和距离继续走，最终到达的位置就是 $\vec{u} + \vec{v}$。

    

##### ② 标量乘法 / 数乘 ($c\vec{v}$)

- **代数计算：** 用常数 $c$ 乘以向量的每个分量。$c \begin{bmatrix} x \\ y \end{bmatrix} = \begin{bmatrix} cx \\ cy \end{bmatrix}$

- ##### **空间解释：** **缩放（Scaling）**。

  - $c > 1$：沿着原方向拉伸；

  - $0 < c < 1$：沿着原方向压缩；

  - $c < 0$：方向反转并发生缩放。

  - “向量（Vector）”这个词的英文词根也有“载体/引导”的意思，而标量（Scalar）的词源就是“Scaling（缩放）”。

    

##### 2. 向量的乘法：点积与叉积

向量之间没有常规的除法，乘法也有两种完全不同的形式，分别对应不同的几何测量：

##### ① 内积 / 点积（Dot Product: $\vec{u} \cdot \vec{v}$）

- **代数计算：** 对应分量乘积之和。

  $$\vec{u} \cdot \vec{v} = u_1 v_1 + u_2 v_2 + \dots + u_n v_n$$

- **几何公式：** $\vec{u} \cdot \vec{v} = \Vert{}\vec{u}\Vert{} \Vert{}\vec{v}\Vert{} \cos\theta$ （$\theta$ 为夹角）。

- **空间解释：** **投影与相似度度量**。

  - 几何上，它等于“$\vec{u}$ 在 $\vec{v}$ 方向上的投影长度”乘以“$\vec{v}$ 的长度”。

  - **符号的物理/几何含义：**

    - 点积 $> 0$：夹角为锐角，方向大致相同（正向促进）；

    - 点积 $= 0$：夹角为 $90^\circ$，**两向量正交（垂直）**；

    - 点积 $< 0$：夹角为钝角，方向大致相反。

      

##### ② 外积 / 叉积（Cross Product: $\vec{u} \times \vec{v}$）

*(主要针对三维空间)*

- **代数计算：** 结构为行列式形式：

  $$\vec{u} \times \vec{v} = \begin{bmatrix} u_2 v_3 - u_3 v_2 \\ u_3 v_1 - u_1 v_3 \\ u_1 v_2 - u_2 v_1 \end{bmatrix}$$

- **空间解释：** **定向面积与法向量**。

  - 结果是一个**全新的向量**，其方向垂直于 $\vec{u}$ 和 $\vec{v}$ 构成的平面（符合右手定则）。

  - 它的**长度** $\Vert{}\vec{u} \times \vec{v}\Vert{} = \Vert{}\vec{u}\Vert{} \Vert{}\vec{v}\Vert{} \sin\theta$，正好等于 $\vec{u}$ 和 $\vec{v}$ 张成的**平行四边形的面积**。

    

##### 3. 矩阵与空间变换（Matrices）

矩阵 $A = \begin{bmatrix} a & b \\ c & d \end{bmatrix}$ 代数上是二维数组；**几何上是空间线性变换的“记录卡”**。

- **第一列 $\begin{bmatrix}a \\ c\end{bmatrix}$**：基底 $\hat{i}(1,0)$ 变换后的新落点。
- **第二列 $\begin{bmatrix}b \\ d\end{bmatrix}$**：基底 $\hat{j}(0,1)$ 变换后的新落点。



##### ① 矩阵加法 ($A + B$)

- **代数计算：** 对应位置元素相加（要求维度完全一致）。
- **几何解释：** **变换效果的并行叠加**。
  - 如果 $A$ 是将空间向右拉伸的力，$B$ 是向上推的力，那么 $A+B$ 就是两个空间变换效果在同一时刻的线性组合。



##### ② 矩阵数乘 ($k A$)

- **代数计算：** 常数 $k$ 乘以矩阵中的每一个元素。
- **几何解释：** **整体变换强度的缩放**。
  - 把整个矩阵所代表的空间变形幅度统一放大或缩小 $k$ 倍。



##### ③ 矩阵乘向量 ($A \vec{v}$)

- **代数计算：** “行乘列”或“列的线性组合”。

  $$A \vec{v} = \begin{bmatrix} a & b \\ c & d \end{bmatrix} \begin{bmatrix} x \\ y \end{bmatrix} = x \begin{bmatrix} a \\ c \end{bmatrix} + y \begin{bmatrix} b \\ d \end{bmatrix}$$

- **几何解释：** **对向量施加空间变换**。

  - 向量 $\vec{v}$ 在原坐标系中的相对权重 $(x,y)$ 保持不变，作用到变换后的新基底上，计算出它在变形后空间的新坐标。



##### ④ 矩阵乘矩阵 ($A \cdot B$)

- **代数计算：** 矩阵 $A$ 的第 $i$ 行点乘矩阵 $B$ 的第 $j$ 列，得到新矩阵的 $(i, j)$ 位置元素。
- **几何解释：** **复合空间变换（从右向左依次应用）**。
  - $A \cdot B$ 意味着：**先对空间应用 $B$ 变换，再在变形后的空间上应用 $A$ 变换**。
  - **为什么不满足交换律 ($AB \neq BA$)？**
    - 几何上极其直观：“先顺时针旋转 $90^\circ$ 再向右剪切”，与“先向右剪切再顺时针旋转 $90^\circ$”，最终空间被扭曲成的形状完全不同。



##### 4. 矩阵的特征量与几何性质

##### ① 行列式 ($\det(A)$)

- **代数计算：** 对于 $2 \times 2$ 矩阵，$\det(A) = ad - bc$。
- **几何解释：** **空间的体积/面积缩放因子**。
  - $\det(A) = 2$：变换后，单位网格的面积放大了 2 倍；
  - $\det(A) = 0$：空间被**降维压缩**（例如把二维平面压成一条线或一个点），这种变换丢失了信息，无法还原；
  - $\det(A) < 0$：空间发生了**翻转**（镜像颠倒）。



##### ② 逆矩阵 ($A^{-1}$)

- **代数性质：** $A^{-1} A = I$ （$I$ 为单位矩阵，即不做任何改变的恒等变换）。
- **几何解释：** **变换的“撤销键”**。
  - 将被 $A$ 扭曲的空间反向还原回初始状态。如果 $\det(A) = 0$（已被压扁降维），就无法还原，因此退化矩阵没有逆矩阵。



##### ③ 迹（Trace: $\text{tr}(A)$）

- **代数计算：** 矩阵**主对角线元素之和**。

  $$\text{tr}(A) = a_{11} + a_{22} + \dots + a_{nn}$$

- **几何解释：** **空间在各维度上的微小变化率 / 总体伸缩和**。

  - 在连续介质力学与微分几何中，迹代表空间变换在原点附近的**体膨胀率**（或者变流场中的“散度”）。
  - **重要代数性质与几何联系：** 矩阵的迹等于其所有特征值之和（$\text{tr}(A) = \sum \lambda_i$），它描述了各个主轴方向伸缩量的累计总和。



##### ④ 特征值与特征向量 ($A\vec{v} = \lambda \vec{v}$)

- **几何解释：** 空间被矩阵 $A$ 整体扭曲挤压时，绝大多数向量的方向都会发生偏转。但**极少数特殊的轴线**（特征向量 $\vec{v}$），在变换后**方向保持在同一条直线上不动**，仅仅发生了长度上的缩放（缩放倍率即特征值 $\lambda$）。



#### 3.齐次坐标（Homogeneous Coordinates）与 $w$ 分量的魔法

##### 1. 为什么用 4x4 矩阵而不是 3x3？

3x3 矩阵只能表示**旋转**和**缩放**（线性变换），**无法用 3x3 矩阵乘法直接表示平移（Translation）**！

**缩放与旋转**满足线性映射公理 $\rightarrow$ 它们是线性映射 $\rightarrow$ **它们可以被写成 $3 \times 3$ 矩阵的形式**。

**矩阵乘法**满足线性映射公理 $\rightarrow$ 它代表线性映射 $\rightarrow$ **它涵盖了包括旋转、缩放、剪切、镜像、投影在内的所有线性变换**。

**平移**不满足 $T(\mathbf{0}) = \mathbf{0}$（违反齐次性） $\rightarrow$ 它不是线性映射 $\rightarrow$ **它绝对不可能写成传统的 $3 \times 3$ 矩阵乘法 $A\vec{v}$ 的形式**。

空间中每个点都是一个向量，缩放可以理解为，向量的方向不变，改变长度；旋转可以理解为，向量的基点和长度不变，改变方向。



为了把平移、旋转、缩放统一成**同一个矩阵乘法**，图形学引入了 **齐次坐标**：

- **当 $w = 1.0$ 时**：四维向量 $(x, y, z, 1.0)$ 代表 3D 空间中的一个 **点（Point）**。平移矩阵对它生效。
- **当 $w = 0.0$ 时**：四维向量 $(x, y, z, 0.0)$ 代表 3D 空间中的一个 **方向/向量（Vector）**。它不受任何平移影响（向量只有方向和长度，没有位置）。



##### 2. $w$ 分量在透视投影中的关键作用（透视除法）

当顶点乘以投影矩阵后，结果向量的 $w$ 分量就不再是 $1.0$ 了，而是**变成了该顶点距离相机的深度值（通常与 $-z$ 成正比）**。

随后，GPU 硬件会自动对每一个顶点执行 **透视除法（Perspective Division）**：

$$\text{NDC Position} = \begin{pmatrix} x / w \\ y / w \\ z / w \end{pmatrix}$$

##### **这正是“近大远小”的数学本质**：

- 距离相机越远的物体，它的 $z$ 越大 $\rightarrow$ $w$ 越大。
- $x$ 和 $y$ 除以一个很大的 $w$ 后，数值迅速变小，物体就在屏幕上缩成了极小的一点！



#### 4.MVP 三大矩阵详解与组合法则

我们需要在 CPU 端用数学库（GLM）构建三个矩阵，然后传递给 GPU。



##### 1. 模型矩阵（Model Matrix）

将顶点从**局部空间**转换到**世界空间**。

包含：**缩放（S）、旋转（R）、平移（T）**。

> ⚠️ **死记硬背级规则（矩阵乘法顺序）**：
>
> 矩阵乘法**不满足交换律**！在 GLSL 和 C++ 中，矩阵运算是从右往左进行的：
>
> $$\mathbf{Model} = \mathbf{T} \cdot \mathbf{R} \cdot \mathbf{S}$$
>
> **含义**：对顶点**先做缩放 $\to$ 再做旋转 $\to$ 最后平移**。
>
> *如果颠倒了（比如先平移再旋转），物体就会绕着世界原点公转，而不是绕着自身中心自转！*



##### 2. 观察矩阵（View Matrix）

将世界坐标系中的一切转换到以**摄像机为原点**的坐标系中。

在图形学中，**并没有真正的“相机”**。所谓的“移动相机”，本质上是**将整个世界中的所有物体向相反方向移动**！

（例如：相机向后退 5 个单位 $+Z$，相当于把整个世界向前推 5 个单位 $-Z$）。



##### 3. 投影矩阵（Projection Matrix）

将观察空间中的 3D 坐标映射到 **裁剪空间**。

- **透视投影（Perspective）**：模拟人眼“近大远小”。视锥体是个截头体（Frustum）。
- **正交投影（Orthographic）**：平行线永远平行，没有近大远小（用于 2D 游戏、UI、CAD 软件）。



```c++
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// A. 构建 Model 矩阵：绕 (1, 0, 0) 轴旋转 -55 度
glm::mat4 model = glm::mat4(1.0f); // 初始化为 4x4 单位矩阵
model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));

// B. 构建 View 矩阵：相机沿 Z 轴向后退 3 个单位 (即整个世界往 -Z 移动 3)
glm::mat4 view = glm::mat4(1.0f);
view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

// C. 构建 Projection 矩阵：45度垂直视角 (FOV)，宽高比 800/600，近平面 0.1，远平面 100.0
glm::mat4 projection = glm::perspective(
    glm::radians(45.0f), 
    800.0f / 600.0f, 
    0.1f, 
    100.0f
);

// D. 传输给 Shader (在渲染主循环内或初始化时)
glUseProgram(shaderProgram);

int modelLoc = glGetUniformLocation(shaderProgram, "model");
glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

int viewLoc = glGetUniformLocation(shaderProgram, "view");
glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

int projLoc = glGetUniformLocation(shaderProgram, "projection");
glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
```



```glsl
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

// 接收来自 C++ 的 MVP 矩阵
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // ⚠️ 正确顺序：P * V * M * Pos
    // 相当于：Projection * (View * (Model * vec4(aPos, 1.0)))
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    
    TexCoord = aTexCoord;
}
```

**glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);**

这个就是把观察空间裁剪为视锥体并映射到二维平面上

glm::radians(45.0f) ：垂直视野角。`45°` 表示镜头上下能看到的角度范围是 45 度

800.0f / 600.0f 宽高比

0.1 近裁剪面，near plane。距离摄像机小于 `0.1` 的东西会被裁掉。

100 远裁剪面，far plane。距离摄像机大于 `100.0` 的东西会被裁掉。



**注意：**

**垂直视野角（Vertical Field of View，简称 Vertical FOV）**，**把它想象成你眼睛（或摄像机镜头）在垂直方向上张开的“张角”**。

当然也有**水平视野角（Horizontal FOV）**。但为什么在 OpenGL 的矩阵 API（`glm::perspective`）里，**第一个参数强制要求传垂直 FOV（fovy）** 呢？

**因为屏宽比（Aspect Ratio）是会动态变化的！**

1. 假设你把游戏窗口从 $16:9$ 的宽屏，拖拽拉长成了 $21:9$ 的带鱼屏。

2. **OpenGL 的设计哲学是：保持垂直方向能看到的内容高度不变（垂直 FOV 锁死）。**

3. 水平方向能看到多少内容，直接通过公式由垂直 FOV 和宽高比算出来：

   $$\text{Horizontal FOV} = 2 \cdot \arctan\left( \tan\left(\frac{\theta_{fovy}}{2}\right) \times \text{Aspect Ratio} \right)$$

4. 这样一来，无论用户怎么拉伸窗口宽度，画面垂直方向的透视关系都不会扭曲变态，只需要向左右两侧“扩充”或“裁切”画面即可。



#### 5.深度测试

一旦我们引入了 3D 空间，不同顶点的 $z$ 轴深度就会产生前后遮挡。

OpenGL 默认使用的是 **画家算法（Painter's Algorithm）**：先画的在下面，后画的直接覆盖在上面。如果没有开启深度测试，**远处的物体如果后绘制，就会把近处的物体挡住！**

**在程序初始化阶段开启深度测试**：**glEnable(GL_DEPTH_TEST);**

**在渲染主循环每帧开始时，【必须】清空深度缓冲区**： **glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);**



#### 6.渲染一个正方体

```c++
float vertices[] = {
    // 位置 (x, y, z)     // 纹理坐标 (u, v)
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

// 初始化阶段开启深度测试
glEnable(GL_DEPTH_TEST);

while (!glfwWindowShouldClose(window)) {
    // 1. 清屏 (颜色 + 深度)
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 2. 激活 Shader & 绑定纹理
    glUseProgram(shaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);

    // 3. 动态更新 Model 矩阵（让立方体随时间沿 (0.5, 1.0, 0.0) 轴旋转）
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(
        model, 
        (float)glfwGetTime() * glm::radians(50.0f), 
        glm::vec3(0.5f, 1.0f, 0.0f)
    );

    // 4. 更新 View 与 Projection 矩阵
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f/600.0f, 0.1f, 100.0f);

    // 5. 将矩阵传入 Shader
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // 6. 绘制 36 个顶点
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glfwSwapBuffers(window);
    glfwPollEvents();
}
```



## 第七章：**摄像机（Camera）与动态视角**

之前我们画出了 3D 盒子并用矩阵将它投射到了屏幕上。但那时相机是固定不动的。**第7章的核心任务，就是给渲染引擎加上摄像机控制系统，让你能像玩 FPS 第一人称射击游戏（如 CS/Minecraft）一样，用鼠标和键盘自由穿梭在 3D 世界中。**



#### 1.摄像机的几何本质：做一套自己的坐标系

在现代 OpenGL 中，并没有一个内置的 `Camera` 结构体。**所谓摄像机，本质上就是定义在世界空间中的一个“局部标架”（3 个互相垂直的方向向量 + 1 个位置点）。**

要确定一个摄像机在世界中的姿态，我们需要定义 4 个向量：

1. **相机位置（Position）**：相机在世界空间中的三维坐标点 $\vec{P}$。

2. **相机前方**（**Front**）**：相机镜头正对着的方向向量 $\vec{F}$。

3. **相机右方（Right）**：指向相机右侧的方向向量 $\vec{R}$。

4. **相机上方（Up）**：指向相机头顶正上方方向的向量 $\vec{U}$。

   

在 C++ 代码中，我们不需要手动去推导这 3 个互相垂直的方向。利用向量叉乘（两个向量的叉乘是），只要知道**相机位置**、**目标点**和**世界默认上方向 $(0,1,0)$**，就能快速推导出相机的观察标架：

- $\text{相机方向向量 } \vec{D} = \text{normalize}(\vec{P} - \text{Target})$（注意指向镜头后方）
- $\text{相机右向量 } \vec{R} = \text{normalize}(\text{WorldUp} \times \vec{D})$
- $\text{相机上向量 } \vec{U} = \vec{D} \times \vec{R}$

这三个基向量组合在一起，构成了 **LookAt 矩阵** 的数学基础！



#### 2.观察矩阵（LookAt 矩阵）的数学形式

GLM 库直接为我们封装好了 `glm::lookAt` 函数。它的数学公式结构如下：

$$LookAt = \begin{bmatrix} R_x & R_y & R_z & 0 \\ U_x & U_y & U_z & 0 \\ D_x & D_y & D_z & 0 \\ 0 & 0 & 0 & 1 \end{bmatrix} \begin{bmatrix} 1 & 0 & 0 & -P_x \\ 0 & 1 & 0 & -P_y \\ 0 & 0 & 1 & -P_z \\ 0 & 0 & 0 & 1 \end{bmatrix}$$

- **左边矩阵**：旋转矩阵，将相机的方向对齐到标准的世界坐标轴。
- **右边矩阵**：平移矩阵，将整个世界平移，使得相机位置落在世界原点 $(0,0,0)$。



#### 3.完整 C++ 摄像机类实现

为了便于在主循环里直接调用，我们将相机逻辑打包成一个符合专业工程习惯的 `Camera` 类。

```c++
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// 默认相机参数设定
const float YAW         = -90.0f; // 初始偏航角指向 -Z 轴
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;

class Camera {
public:
    // 1. 空间姿态向量（Pos + 3个基向量 + 世界天空方向）
    glm::vec3 Position; // 相机在世界里的绝对位置 (x, y, z)
    glm::vec3 Front;	// 相机正对着的方向（镜头指向）
    glm::vec3 Up;		// 相机头顶指向的方向
    glm::vec3 Right;	// 相机右手臂指向的方向
    glm::vec3 WorldUp;	// 世界的绝对天空方向，通常固定为 (0.0, 1.0, 0.0)

    // 2. 姿态角（欧拉角）
    float Yaw;		// 偏航角：控制左右看
    float Pitch;	// 俯仰角：控制上下看

    // 3. 交互与控制参数
    float MovementSpeed;		// 键盘移动速度
    float MouseSensitivity;		// 鼠标灵敏度
    float Zoom;					// 视野大小 (FOV)，控制缩放

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f), 
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), 
           float yaw = YAW, float pitch = PITCH) 
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // 获取用于传递给着色器的 View 矩阵
    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // 处理键盘输入 (WASD)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
    }

    // 处理鼠标移动 (俯仰角 Pitch 与偏航角 Yaw)
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw   += xoffset;
        Pitch += yoffset;

        // 视角翻转锁：防止 Pitch 超过 80 度导致摄像机颠倒
        if (constrainPitch) {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        updateCameraVectors();
    }

    // 处理鼠标滚轮缩放 (FOV 调整)
    void ProcessMouseScroll(float yoffset) {
        Zoom -= yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

private:
    // 根据当前 Pitch / Yaw 角度重新计算相机方向向量
    void updateCameraVectors() {
        // 【第一步】：用三角函数，把角度（Yaw/Pitch）还原成三维空间的方向向量 (Front)
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        
        // 【第二步】：叉乘魔法 —— 自动计算相机的 Right 和 Up
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};
```

**最重要的是******updateCameraVectors****：

##### 几何含义拆解：

1. **`Front` 的计算**：这是球面坐标系到笛卡尔坐标系的转换公式。通过 `Yaw`（左右）和 `Pitch`（上下）这两个角度，在单位球面上精准锁定一个点的三维方向。
2. **`Right` 的叉乘**：$\text{Front} \times \text{WorldUp}$。利用右手定则，镜头方向与世界天空方向构成的平面的法线，**正好就是相机的右手臂方向**。
3. **`Up` 的二次叉乘**：$\text{Right} \times \text{Front}$。知道了右边和前面，再次叉乘，就能精准求出严格垂直于这俩向量的“相机真正头顶方向”。



**glm**::**mat4** **GetViewMatrix**() **const** {    **return** **glm**::**lookAt**(**Position**, **Position** + **Front**, **Up**); }

**`Position`**：眼睛在哪里。

**`Position + Front`**：眼睛在看向哪一点（目标点）。

**`Up`**：眼睛的头顶朝哪里。



#### 4.在渲染循环（Render Loop）中整合 GLFW 回调与帧率补偿

在实际主循环中使用相机时，有两个工程要点：

1. **帧率无关移动（DeltaTime）**：不能直接每帧移动固定的距离，否则高帧率电脑上移动极快，低帧率电脑上极慢。必须乘上上帧耗时 `deltaTime`。
2. **GLFW 鼠标抓取**：使用 `glfwSetInputMode` 隐藏鼠标光标并锁定在窗口内，实现第一人称视角接管。

```c++
#include <GLFW/glfw3.h>

// 全局相机实例
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = 800.0f / 2.0f;
float lastY = 600.0f / 2.0f;
bool firstMouse = true;

// 时间补偿变量
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// 1. 鼠标移动回调函数
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
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

// 2. 滚轮回调函数
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// 3. 键盘输入检测
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

int main() {
    // ... 初始化 GLFW、绑定窗口与回调 ...
    // glfwSetCursorPosCallback(window, mouse_callback);
    // glfwSetScrollCallback(window, scroll_callback);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // 锁定捕获鼠标指针

    // 渲染循环内部
    while (!glfwWindowShouldClose(window)) {
        // 计算 DeltaTime
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 响应按键输入
        processInput(window);

        // 清屏
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 激活着色器并传输变换矩阵
        ourShader.use();

        // 根据相机的 Zoom 动态生成投影矩阵 (FOV 变换实现倍镜放大效果)
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);

        // 获取并传参相机 View 矩阵
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("view", view);

        // ... 绑定 VAO/VBO/纹理并调用 glDrawArrays / glDrawElements ...

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
```



## 第八章.基础光照模型

​	光照的物理本质是**光子碰撞物体表面后的吸收、折射与反射**。在实时渲染（Real-time Rendering）的极早期，由于 GPU 算力极其有限，无法像光线追踪（Ray Tracing）那样去追踪每条光子的路径。

为了在 GPU 上以每秒 60+ 帧的速度跑出逼真的光影，物理学家 Binn-Phong 和 Phong 提出了著名的 **Phong 基础光照模型（Phong Reflection Model）**。

它把极其复杂的现实光照，高度经验化地拆解为 **3 种独立的成分相加**：

$$\text{最终颜色} = \text{环境光 (Ambient)} + \text{漫反射 (Diffuse)} + \text{高光 (Specular)}$$



#### 1.三大成分的物理本质与数学推导

这三个项描述的是**光对物体颜色的影响**，而不是物体自身的颜色，最后观察到颜色是还要将这个影响因子x物体表面颜色。

```
环境光 (Ambient)               漫反射 (Diffuse)               高光 (Specular)
      (无方向，全局基础亮度)          (取决于入射角 θ)             (取决于反射光与观察者夹角 ϕ)

          │                            \  N 向量                     \  N 向量  / R 反射光
          │                             \ │                           \ │  /   
          ▼                              \│                            \│/    ↙ V 观察方向
     ┌─────────┐                      ────┴────                     ────┴────  👁 观察者
     └─────────┘                         物体                           物体
```

##### 1. 环境光（Ambient Light）：模拟间接光照

- **物理本质**：在现实世界中，即便太阳被遮挡，房间里也不会是纯黑的，因为光线在墙壁、地板、天花板之间经过了成千上万次**间接漫反射**。

- **Phong 模型的简化**：计算无数次的间接反射太贵了，Phong 模型直接用一个**全局常量**来暴力模拟它。

- **数学公式**：

  $$\mathbf{I}_{ambient} = k_a \times \mathbf{L}_{color}$$

  （其中 $k_a$ 是环境光强度系数，例如 $0.1$；$\mathbf{L}_{color}$ 是光源颜色）。



##### 2. 漫反射光（Diffuse Light）：方向性散射（Lambert 余弦定律）

- **物理本质**：当光线照射到粗糙物体表面（如木头、水泥）时，光子会向四面八方均匀散射。**无论你从哪个角度看，这个点的漫反射亮度都是一样的**。

- **决定因素**：亮不亮**只取决于光线射入的角度**。直射（ $90^\circ$ 垂直入射）最亮，斜射最暗。

- **数学推导（Lambertian Reflectance）**：

  定义两个**单位向量**：

  1. 法线向量 $\mathbf{\vec{N}}$（Normal）：垂直于物体表面的单位向量。
  2. 光照向量 $\mathbf{\vec{L}}$（Light Dir）：从物体表面某点指向光源的单位向量。

  根据向量点积的几何意义：

  $$\mathbf{\vec{N}} \cdot \mathbf{\vec{L}} = \Vert{}\mathbf{\vec{N}}\Vert{} \Vert{}\mathbf{\vec{L}}\Vert{} \cos(\theta) = \cos(\theta)$$

  - 当光线直射时，$\theta = 0^\circ \implies \cos(0^\circ) = 1$（最亮）。
  - 当光线斜射 $60^\circ$ 时，$\cos(60^\circ) = 0.5$（亮度减半）。
  - 当光线从背面射来时，$\cos(\theta) < 0$，用 `max(dot(N, L), 0.0)` 把负值截断为 0，防止物体背面出现“负光照”。

- **数学公式**：

  $$\mathbf{I}_{diffuse} = \max(\mathbf{\vec{N}} \cdot \mathbf{\vec{L}}, 0.0) \times k_d \times \mathbf{L}_{color}$$

**一个物体表面的每一个采样点都有一个法线向量 N 和一个光照方向向量 L，GPU逐点计算它们的夹角，从而得到局部亮度。**



##### 3. 高光（Specular Light）：镜面反射光斑

- **物理本质**：当物体表面比较光滑（如金属、塑料、大理石）时，光线会沿着镜面反射方向集中射出。如果你的**眼睛刚好凑在这个反射光线附近**，就会看到极其刺眼的“亮斑”。

- **决定因素**：高光的亮暗**强烈依赖于观察者（相机）的位置**！

- **数学推导**：

  定义另外两个单位向量：

  1. 反射向量 $\mathbf{\vec{R}}$（Reflect）：光线经过法线 $\mathbf{\vec{N}}$ 镜面反射后的向量。GLSL 提供了内置函数 `reflect(-L, N)`（注意传入的是从光源指向物体的向量 `-L`）。
  2. 观察向量 $\mathbf{\vec{V}}$（View Dir）：从物体表面指向相机眼睛的单位向量。

  高光强度取决于 $\mathbf{\vec{R}}$ 与 $\mathbf{\vec{V}}$ 的夹角 $\phi$ 的余弦值：$\max(\mathbf{\vec{V}} \cdot \mathbf{\vec{R}}, 0.0)$。

  为了表现不同材质的粗糙/光滑程度，我们引入 **高光反光度（Shininess / Specular Power）** 指数 $p$：

  $$\text{Specular Intensity} = (\max(\mathbf{\vec{V}} \cdot \mathbf{\vec{R}}, 0.0))^p$$

  > **$p$ 指数（Shininess）的作用**：
  >
  > 任何 $0 \sim 1$ 之间的浮点数，次方数越大，衰减越剧烈！
  >
  > - $p = 8$：高光点很大、很软（像未经打磨的木头）。
  > - $p = 32$：标准塑料/漆面。
  > - $p = 256$：高光点极小、极其锐利（像高度抛光的金属或镜子）。

- **数学公式**：

  $$\mathbf{I}_{specular} = (\max(\mathbf{\vec{V}} \cdot \mathbf{\vec{R}}, 0.0))^p \times k_s \times \mathbf{L}_{color}$$



#### 2.核心数学工具：法线矩阵（Normal Matrix）

这是 3D 图形学中最容易被忽略、却最致命的一个线性代数坑！

顶点着色器（Vertex Shader）中，我们通常会用 Model 矩阵把顶点坐标从局部空间变换到世界空间：

$$\mathbf{P}_{world} = \mathbf{M}_{model} \cdot \mathbf{P}_{local}$$

**那么，法线向量 $\mathbf{\vec{N}}$ 也可以直接乘以 Model 矩阵转换到世界空间吗？**

答案是：绝对不行！（除非 Model 矩阵只包含等比例缩放和旋转），在几何上，如果 Model 矩阵包含了非等比例缩放，**原本垂直于表面的法线乘以该矩阵后，不再垂直于表面！** 这会导致渲染出来的光照发生严重的扭曲和阴影错乱。

##### 所以引入法线矩阵（Normal Matrix）：

数学推导证明（利用切线与法线的正交性 $\mathbf{\vec{T}} \cdot \mathbf{\vec{N}} = 0$）：

**能够正确变换法线的矩阵，是 Model 矩阵左上角 $3 \times 3$ 部分的“逆矩阵的转置”（Transpose of the Inverse）！**

$$\mathbf{M}_{normal} = \left( \mathbf{M}_{model, 3\times 3}^{-1} \right)^T$$

在 C++ 应用层中，我们在传模型矩阵给 GPU 前，需要这样计算法线矩阵：

```c++
glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
```

数学证明：

N：法线
T：任意表面切向量
N⋅T=0

假设切向量经过模型矩阵变换：T′ = MT
假设法线经过另个矩阵K变换：N′ = AN

因为	法线 dot 切向量 = 0
	N′⋅T′=0
所以	(AN)⋅(MT)=0

​		(AN)^T^ (MT)=0

​		N^T^A^T^MT=0

又因为 N⋅T=0；N^T^T = 0;

所以A^T^M = I; A = (M^-1^)^T^



#### 3.完整可运行代码实现

##### 1. 顶点着色器 (`shader.vs`)

一般光照条件下需要两套shader，光源一套，物体一套。

法线向量和顶点坐标都必须变换到 **世界空间（World Space）** 下计算，因为光源位置 `lightPos` 通常是用世界坐标给出的。

```glsl
#version 330 core
layout (location = 0) in vec3 aCoord;   //顶点
layout (location = 1) in vec3 aNormal;  //法线


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;  //逆转置法线矩阵

out vec3 normal;    //世界空间下的法线向量
out vec3 fragPos;   //世界空间下的片段坐标

void main()
{
    //局部空间转为世界空间
    fragPos = vec3(model * vec4(aCoord,1.0));

    //法线
    normal = normalMatrix * aNormal;

    //转换为裁剪空间坐标
    gl_Position = projection * view * model * vec4(aCoord,1.0);
}
```

```glsl
#点光源顶点着色器
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos,1.0);
}
```



##### 2. 片段着色器 (`shader.fs`)—— Phong 光照核心逻辑

所有光照计算都在片段着色器中按像素（Per-pixel Light / Phong Shading）逐点计算。

```glsl
#version 330 core

out vec4 FragColor;

in vec3 fragPos;
in vec3 normal;

//对象物体表面颜色
uniform vec3 objectColor;

//光源颜色
uniform vec3 lightColor;

//光源位置
uniform vec3 lightPos;

//摄像机位置
uniform vec3 cameraPos;

void main()
{
    //物体最终显示颜色 = 光源影响因子 * 物体表面颜色;
    //光源影响因子 = 环境光 + 漫反射 + 高光

    //方向向量
    vec3 N = normalize(normal);
    vec3 L = normalize(lightPos - fragPos); //入射光向量的反向量
    vec3 R = reflect(-L,N);                 //反射光向量
    vec3 V = normalize(cameraPos - fragPos);//相机方向向量的反向量
    vec3 H = normalize(L+V);          //半角向量

    //环境光
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    //漫反射
    float diff  = max(dot(N,L),0.0);  //入射光和法线夹角
    vec3 diffuse = diff * lightColor;

    //高光
    float specularStrength = 0.5;
    //float spec = pow(max(dot(V,R),0.0),32);  //反射光和相机夹角,使用pow进行光线集中收束
    //Phong模型,反射光和相机夹角,在高光夹角大于90时，有断层切线，所以引入Blinn-Phong模型，使用半角向量和法线的夹角
    float spec = pow(max(dot(N,H),0.0),64);
    vec3 specular = specularStrength * spec * lightColor;

    //物体最终显示颜色
    vec3 objColor = (ambient + diffuse + specular) * objectColor;

    FragColor = vec4(objColor,1.0);
}
```



```glsl
#点光源片段着色器
#version 330 core
out vec4 FragColor;

void main() {
    FragColor = vec4(1.0); // 始终输出白光
}
```



3. C++ 渲染主循环（Application Side）

```glsl
// 渲染主循环 (Render Loop)
while(!glfwWindowShouldClose(window))
{
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    processInput(window);

    //两个物体都是同一个摄像机，同一个窗口进行观察，所以共用view和projection
    glm::mat4 view = camera.GetViewMatrix();
    float aspect = framebufferHeight > 0
        ? static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight)
        : 1.0f;
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspect, 0.1f, 100.0f);

    //光源
    lightShader.use();
    glm::mat4 lightModel = glm::mat4(1.0f);
    lightModel = glm::translate(lightModel, lightPos);
    lightModel = glm::scale(lightModel, glm::vec3(0.2f));

    lightShader.setMat4("model",lightModel);
    lightShader.setMat4("view",view);
    lightShader.setMat4("projection",projection);

    //绘制光源正方体
    glBindVertexArray(lightCubeVAO);
    glDrawElements(GL_TRIANGLES,indexCount,GL_UNSIGNED_INT,0);
    glBindVertexArray(0);

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
    objShader.setVec3("lightColor",lightColor);
    objShader.setVec3("lightPos",lightPos);
    objShader.setVec3("objectColor",objectColor);
    objShader.setVec3("cameraPos",camera.Position);

    //绘制物体正方体
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES,indexCount,GL_UNSIGNED_INT,0);
    glBindVertexArray(0);

    glfwSwapBuffers(window);
    glfwPollEvents();
}
```



##### 4.Phong 与 Blinn-Phong 的演进关系

如果你运行上面的代码，并把视角转到高光区域边缘，你可能会注意到一个**瑕疵**：

当高光夹角 $\phi > 90^\circ$ 时，$\mathbf{\vec{V}} \cdot \mathbf{\vec{R}}$ 会瞬间截断为 0，导致高光边缘有一条非常不自然的**断层切线**。

为了解决这个问题，吉姆·布林（Jim Blinn）对 Phong 模型做了一改进，诞生了 **Blinn-Phong 光照模型**（现代渲染引擎更常用）：

```
Phong 模型                          Blinn-Phong 模型
    计算 View 与 Reflect 的夹角              计算 Normal 与 Halfway 的夹角
          \  N  / R 反射光                         \  N  /
           \ │ /                                    \ │ /   H 半角向量
            \│/    ↙ V                               \│/ ↗  (N 与 H 夹角永远 ≤ 90°)
          ───┴───  👁                                ───┴─── 👁
```

1. 不去计算复杂的反射向量 $\mathbf{\vec{R}}$，而是计算**半角向量（Halfway Vector）$\mathbf{\vec{H}}$**：

   $$\mathbf{\vec{H}} = \text{normalize}(\mathbf{\vec{L}} + \mathbf{\vec{V}})$$

2. 高光改为计算法线 $\mathbf{\vec{N}}$ 与半角向量 $\mathbf{\vec{H}}$ 的点积：

   $$\text{Specular} = (\max(\mathbf{\vec{N}} \cdot \mathbf{\vec{H}}, 0.0))^{p_{blinn}}$$

在代码中只需将片段着色器的高光计算改写为一行：

```glsl
// Blinn-Phong 高光计算 (性能更好，表现更平滑)
vec3 halfwayDir = normalize(lightDir + viewDir);
float spec = pow(max(dot(norm, halfwayDir), 0.0), 64); // Blinn-Phong 指数通常取 Phong 的 2~4 倍
```





## 第九章.物体材质

在上一章中，物体的颜色是由 `objectColor` 一笔带过的；但在现实世界中，**橡皮泥、金属、钢铁和塑料**对环境光、漫反射和高光的反应是截然不同的。第 9 章的核心目的，就是用结构体（Struct）把物体与光源的属性彻底抽象化和模块化。



#### 1.材质结构体

定义物体对各种光照成分的**反射率/吸收率**以及**表面光滑度**：

```glsl
struct Material {
    vec3 ambient;   // 物体在环境光下反射什么颜色（通常与 diffuse 一致）
    vec3 diffuse;   // 物体在漫反射光下的固有色（例如珊瑚红）
    vec3 specular;  // 物体高光斑点的颜色（金属通常带有自身颜色，塑料高光多为白色）
    float shininess;// 高光反光度/散射半径（数值越大，高光斑点越小、越锐利）
};

uniform Material material;
```



#### 2.光源结构体

定义光源发射出的**不同分量强度**（光不再只是一道单一白光，它可以有很强的漫反射强度，但极弱的环境光强度）：

```glsl
struct Light {
    vec3 position;  // 光源位置

    vec3 ambient;   // 光源的环境光强度（通常设低一点，如 vec3(0.2)）
    vec3 diffuse;   // 光源的漫反射强度（通常为光源的主色调，如 vec3(0.5)）
    vec3 specular;  // 光源的高光强度（通常设为全强，如 vec3(1.0)）
};

uniform Light light;
```



#### 3.光照着色器的升维

```glsl
#version 330 core

out vec4 FragColor;

in vec3 fragPos;
in vec3 normal;


struct Light {
    vec3 position;  // 光源位置
    vec3 ambient;   // 光源的环境光强度（通常设低一点，如 vec3(0.2)）
    vec3 diffuse;   // 光源的漫反射强度（通常为光源的主色调，如 vec3(0.5)）
    vec3 specular;  // 光源的高光强度（通常设为全强，如 vec3(1.0)）
};
    
struct Material {
    vec3 ambient;   // 物体在环境光下反射什么颜色（通常与 diffuse 一致）
    vec3 diffuse;   // 物体在漫反射光下的固有色（例如珊瑚红）
    vec3 specular;  // 物体高光斑点的颜色（金属通常带有自身颜色，塑料高光多为白色）
    float shininess;// 高光反光度/散射半径（数值越大，高光斑点越小、越锐利）
};

//材质颜色
uniform Material material;
//光源颜色
uniform Light light;

//光源位置
uniform vec3 lightPos;

//摄像机位置
uniform vec3 cameraPos;

void main()
{
    //物体最终显示颜色 = 光源影响因子 * 物体表面颜色;
    //光源影响因子 = 环境光 + 漫反射 + 高光

    //方向向量
    vec3 N = normalize(normal);
    vec3 L = normalize(light.position - fragPos); //入射光向量的反向量
    vec3 R = reflect(-L,N);                 //反射光向量
    vec3 V = normalize(cameraPos - fragPos);//相机方向向量的反向量
    vec3 H = normalize(L+V);          //半角向量

    //环境光
    vec3 ambient = light.ambient * material.ambient;

    //漫反射
    float diff  = max(dot(N,L),0.0);  //入射光和法线夹角
    vec3 diffuse = light.diffuse * (diff * material.diffuse);

    //高光
    float spec = pow(max(dot(V,R),0.0),material.shininess);  //反射光和相机夹角,使用pow进行光线集中收束
    vec3 specular = light.specular * (spec * material.specular);

    //物体最终显示颜色
    vec3 objColor = ambient + diffuse + specular;

    FragColor = vec4(objColor,1.0);
}
```



#### 4.c++传递Struct Uniform

在 C++ 主程序中，GLSL 的结构体成员变量是通过点号字符串（`struct.member`）逐个设置的：

```c++
// 设置 Material 结构体（以黄金材质为例）
lightingShader.use();
lightingShader.setVec3("material.ambient",  0.24725f, 0.1995f, 0.0745f);
lightingShader.setVec3("material.diffuse",  0.75164f, 0.60648f, 0.22648f);
lightingShader.setVec3("material.specular", 0.62828f, 0.5558f, 0.36606f);
lightingShader.setFloat("material.shininess", 51.2f);

// 设置 Light 结构体
lightingShader.setVec3("light.position", lightPos);
lightingShader.setVec3("light.ambient",  0.2f, 0.2f, 0.2f);
lightingShader.setVec3("light.diffuse",  0.5f, 0.5f, 0.5f); // 调暗漫反射试一下效果
lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
```



## 第十章.光照贴图

在现实世界中，一个物体的表面很少是材质纯一的。

以一个**带金属边框和铁锁的木箱**为例：

- **木板区域**：漫反射呈木纹颜色，几乎没有镜面高光。
- **钢铁边框/螺丝**：漫反射呈暗灰色，但对光线非常敏感，高光极强且锐利。

如果只用第 9 章的方法，你只能把整个箱子设为“木头”或者“金属”。而第 10 章的核心思路，就是**把材质属性拆成一张张按像素（UV 坐标）采样的纹理图片**。

本章的核心是引入两种贴图：**漫反射贴图（Diffuse Map）** 和 **高光贴图（Specular Map）**。



#### 1. 漫反射贴图 (Diffuse Map)

- **本质**：就是我们前面学的普通纹理图片（Texture），它定义了物体表面每个像素点在没有高光时的**固有颜色**。

- **Shader 变化**：把 `material.diffuse` 从 `vec3` 改为 `sampler2D`。

- **物理含义**：如果不开灯，只用漫无边际的微光照亮物体，你看到的那个**表面图样**是什么？

  **作用**：它决定了物体**基础颜色**的分布。

  **现实类比**：

  - 一个木箱子：上面有棕色的木纹、黑色的缝隙。
  - 漫反射贴图就是把这个**木纹图**直接贴在 3D 模型表面。

  **代码逻辑**：`diffColor` 告诉 GPU：“这里是棕色，那里是灰色”。



#### 2. 高光贴图 (Specular Map)

- **本质**：通常是一张**黑白/灰度图**，用来定义物体表面**各个区域的反光强度**。

  - **黑色像素（0.0）**：代表木头、皮革、布料——**不产生高光**。
  - **白色像素（1.0）**：代表抛光金属、玻璃——**产生最强高光**。
  - **灰色像素（0.5）**：代表生锈金属、湿润表面——**产生中等高光**。

- **Shader 变化**：把 `material.specular` 也改为 `sampler2D`。

- **物理含义**：当手电筒照在箱子上时，**哪些地方能晃你的眼睛（产生强光斑）？哪些地方吸光不反光？**

  **作用**：它用来控制**高光（Specular）的强弱分布**。

  **现实类比**：

  - 假设这个木箱子的边缘包裹着**铁条**，中间是**粗糙的木头**。
  - 手电筒照过去，**铁条**会反射出刺眼的白光；而**木头**只会漫反射，不会有亮眼的光斑。

  **高光贴图长什么样？**：

  - 它通常是一张**黑白图**。
  - **铁条/螺丝对应的位置**：画成**白色**（RGB 值为 1.0）。
  - **木头对应的位置**：画成**黑色**（RGB 值为 0.0）。



#### 3.GLSL 片段着色器（FS）的重构

```glsl
#version 330 core

out vec4 FragColor;

in vec3 fragPos;
in vec3 normal;
in vec2 TexCoords; // 从顶点着色器传过来的 UV 坐标

struct Light {
    vec3 position;  // 光源位置
    vec3 ambient;   // 光源的环境光强度（通常设低一点，如 vec3(0.2)）
    vec3 diffuse;   // 光源的漫反射强度（通常为光源的主色调，如 vec3(0.5)）
    vec3 specular;  // 光源的高光强度（通常设为全强，如 vec3(1.0)）
};
    
/*struct Material {
    vec3 ambient;   // 物体在环境光下反射什么颜色（通常与 diffuse 一致）
    vec3 diffuse;   // 物体在漫反射光下的固有色（例如珊瑚红）
    vec3 specular;  // 物体高光斑点的颜色（金属通常带有自身颜色，塑料高光多为白色）
    float shininess;// 高光反光度/散射半径（数值越大，高光斑点越小、越锐利）
};*/
    
struct Material {
    sampler2D diffuse;   // 漫反射贴图（替代了原先的 vec3 diffuse）
    sampler2D specular;  // 高光贴图（替代了原先的 vec3 specular）
    float     shininess; // 高光散射半径/粗糙度
};
//材质
uniform Material material;

//光源颜色
uniform Light light;

//光源位置
uniform vec3 lightPos;

//摄像机位置
uniform vec3 cameraPos;

void main()
{
    //物体最终显示颜色 = 光源影响因子 * 物体表面颜色;
    //光源影响因子 = 环境光 + 漫反射 + 高光

    //方向向量
    vec3 N = normalize(normal);
    vec3 L = normalize(light.position - fragPos); //入射光向量的反向量
    vec3 R = reflect(-L,N);                 //反射光向量
    vec3 V = normalize(cameraPos - fragPos);//相机方向向量的反向量
    vec3 H = normalize(L+V);          //半角向量
    vec3 diffColor = vec3(texture(material.diffuse, TexCoords)); //从漫反射贴图中采样出基础颜色

    //环境光，通常直接使用漫反射贴图的颜色
    vec3 ambient = light.ambient * diffColor;

    //漫反射
    float diff  = max(dot(N,L),0.0);  //入射光和法线夹角
    vec3 diffuse = light.diffuse * (diff * diffColor);

    //镜面高光 (Specular)：根据高光贴图采样值，决定当前像素能产生多少高光
    float spec = pow(max(dot(V,R),0.0),material.shininess);  //反射光和相机夹角,使用pow进行光线集中收束
    // 重点：从高光贴图（specular map）采样
    vec3 specMask = vec3(texture(material.specular, TexCoords));
    vec3 specular = light.specular * (spec * specMask);

    //物体最终显示颜色
    vec3 objColor = ambient + diffuse + specular;

    FragColor = vec4(objColor,1.0);
}
```



#### 4.C++ 端的纹理绑定

既然 `material.diffuse` 和 `material.specular` 变成了纹理，在 C++ 渲染循环中就需要使用**不同的纹理单元（Texture Units）**

```glsl
// 1. 激活并绑定漫反射贴图到 GL_TEXTURE0
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, diffuseMap);
lightingShader.setInt("material.diffuse", 0);

// 2. 激活并绑定高光贴图到 GL_TEXTURE1
glActiveTexture(GL_TEXTURE1);
glBindTexture(GL_TEXTURE_2D, specularMap);
lightingShader.setInt("material.specular", 1);

// 3. 绘制物体
glBindVertexArray(cubeVAO);
glDrawArrays(GL_TRIANGLES, 0, 36);
```



#### 5.放射贴图（Emission Map）

```glsl
#version 330 core

out vec4 FragColor;

in vec3 fragPos;
in vec3 normal;
in vec2 TexCoords;

struct Light {
    vec3 position;  // 光源位置
    vec3 ambient;   // 光源的环境光强度（通常设低一点，如 vec3(0.2)）
    vec3 diffuse;   // 光源的漫反射强度（通常为光源的主色调，如 vec3(0.5)）
    vec3 specular;  // 光源的高光强度（通常设为全强，如 vec3(1.0)）
};
    
struct Material{
    sampler2D diffuse; //漫反射贴图
    sampler2D specular; //高光贴图
    sampler2D emission; //放射贴图（自发光）
    float     shininess;//高光散射半径/粗糙度
};


//材质颜色
uniform Material material;
//光源颜色
uniform Light light;
//摄像机位置
uniform vec3 cameraPos;
//时间（秒，用于灯带单向流动）
uniform float time;

void main()
{
    //物体最终显示颜色 = 光源影响因子 * 物体表面颜色;
    //光源影响因子 = 环境光 + 漫反射 + 高光

    //方向向量
    vec3 N = normalize(normal);
    vec3 L = normalize(light.position - fragPos); //入射光向量的反向量
    vec3 R = reflect(-L,N);                 //反射光向量
    vec3 V = normalize(cameraPos - fragPos);//相机方向向量的反向量

    vec3 diffuseColor = texture(material.diffuse, TexCoords).rgb;
    vec3 specularColor = texture(material.specular, TexCoords).rgb;
    vec3 emissiveColor = texture(material.emission, TexCoords).rgb;

    // 单向流动：y 随时间单调下移，配合 GL_REPEAT 自然循环（0.2 = 约 5 秒滚完一个周期）
    vec2 emissiveUV = vec2(TexCoords.x, TexCoords.y - time * 0.2);
    emissiveColor = texture(material.emission, emissiveUV).rgb;

    //环境光
    vec3 ambient = light.ambient * diffuseColor;

    //漫反射
    float diff  = max(dot(N,L),0.0);  //入射光和法线夹角
    vec3 diffuse = light.diffuse * diff * diffuseColor;

    //高光
    float spec = pow(max(dot(V,R),0.0),material.shininess);  //反射光和相机夹角,使用pow进行光线集中收束
    vec3 specular = light.specular * spec * specularColor;

    //物体最终显示颜色
    vec3 objColor = ambient + diffuse + specular + emissiveColor;

    FragColor = vec4(objColor,1.0);
}
```



## 第十一章.投光物（Light Casters)

在前面几章中，我们使用的光源都是一个**固定位置的点**，发出的光均匀地向四周辐射。但在真实世界和游戏开发中，光源的形式极其丰富。第 11 章的核心目标，就是将光照系统升级，能够模拟现实世界中的**三种基本光源**：

1. **平行光 / 定向光（Directional Light）**：模拟太阳光。
2. **点光源（Point Light）**：模拟灯泡，带有距离衰减（Attenuation）。
3. **聚光灯（Spotlight）**：模拟手电筒/聚光手电，带有圆锥视锥与边缘平滑打光（Soft Edges）。



#### 1.定向光 / 平行光 (Directional Light)

当光源距离场景**无限远**时（最典型的就是太阳），它发出的所有光线到达受照物体时，彼此之间可以近似看作是**完全平行的**。

- **区别于点光源**：点光源的入射方向 $L$ 取决于片段的位置（`normalize(light.position - FragPos)`）；而定向光的入射方向 $L$ 对场景里的**所有像素都是恒定不变的**。
- **没有位置，只有方向**：定向光不需要 `position` 变量，只需要一个 `direction`（光线射向的方向）。

```glsl
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float     shininess;
};

// 平行光结构体：只有方向，没有位置，也没有距离衰减
struct DirLight {
    vec3 direction; // 光线照射的方向 (例如：vec3(-0.2, -1.0, -0.3))

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform DirLight dirLight;
uniform vec3 viewPos;

void main() {
    
    //工具向量
    vec3 N = normalize(normal);
    vec3 L = normalize(-dirLight.diretion);	// 注意：dirLight.direction 是“光源射出的方向”
    vec3 V = reflect(viewPos - fragPos);
    vec3 R = reflect(-L,N);
    
    // 1. 采样纹理
    vec3 diffColor = vec3(texture(material.diffuse, TexCoords));
    vec3 specMask  = vec3(texture(material.specular, TexCoords));

    // 2. 环境光
    vec3 ambient = dirLight.ambient * diffColor;

    // 3. 漫反射
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = dirLight.diffuse * (diff * diffColor);

    // 4. 镜面高光
    float spec = pow(max(dot(V, R), 0.0), material.shininess);
    vec3 specular = dirLight.specular * (spec * specMask);

    // 5. 合成最终颜色
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
```



#### 2.点光源与衰减 (Point Light & Attenuation)

##### 物理与数学原理

现实中的点光源（如蜡烛、灯泡）向四面八方辐射光线，能量随距离增加而向周围球面扩散。根据物理学逆平方定律，**光强与距离的平方成反比**。

在实时图形学中，纯粹的逆平方衰减在距离很近时会导致光强过高过曝，在远距离时又衰减得不够自然。工业界普遍使用**二次多项式衰减公式**：

$$F_{\text{att}} = \frac{1.0}{K_c + K_l \cdot d + K_q \cdot d^2}$$

- $d$：片段与光源之间的距离：`length(light.position - FragPos)`。
- $K_c$ (Constant 常数项)：通常设为 `1.0`，保证分母永远不小于 1，防止在极近距离时光强爆发。
- $K_l$ (Linear 线性项)：控制较近距离内的线性衰减。
- $K_q$ (Quadratic 二次项)：控制远距离的指数级快速衰减。

```glsl
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float     shininess;
};

// 点光源结构体：需要位置 + 衰减系数
struct PointLight {
    vec3 position;

    // 衰减系数
    float constant;  // Kc
    float linear;    // Kl
    float quadratic; // Kq

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform PointLight pointLight;
uniform vec3 viewPos;

void main() {
    // 1. 采样纹理
    vec3 diffColor = vec3(texture(material.diffuse, TexCoords));
    vec3 specMask  = vec3(texture(material.specular, TexCoords));

    // 2. 计算距离与衰减因子 (Attenuation)
    float distance    = length(pointLight.position - FragPos);
    float attenuation = 1.0 / (pointLight.constant + 
                               pointLight.linear * distance + 
                               pointLight.quadratic * (distance * distance));

    // 3. 环境光 (环境光通常也乘衰减，防止远处的物体被环境光过度照亮)
    vec3 ambient = pointLight.ambient * diffColor;

    // 4. 漫反射
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(pointLight.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = pointLight.diffuse * (diff * diffColor);

    // 5. 镜面高光
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = pointLight.specular * (spec * specMask);

    // 6. 应用衰减因子
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
```



#### 3.聚光灯与边缘平滑羽化 (Spotlight & Soft Edges)

##### 物理与数学原理

聚光灯（如手电筒、汽车大灯）只向某个特定方向的**锥体（Cone）** 范围内发射光线。

我们用两个切角（Cutoff Angle）来定义这个圆锥：

1. **内切角 $\phi$ (Inner Cutoff)**：内圆锥范围，处于此区域内的像素接收 100% 的聚光照射。
2. **外切角 $\gamma$ (Outer Cutoff)**：外圆锥范围。内角与外角之间的区域，光强从 1.0 平滑过渡到 0.0（实现边缘羽化/软边缘效果）。

为了避免在 Shader 里频繁计算开销昂贵的反三角函数（`acos`），我们**直接比较向量的点积（余弦值 $\cos$）**：

$$\theta = \text{dot}(\text{lightDir}, \text{normalize}(-\text{spotLight.direction}))$$

其中：

- $\theta$ 为当前片段与聚光灯中轴线的夹角余弦值。
- 注意：**角度越大，余弦值反而越小**！因此 $\cos(\text{inner}) > \cos(\text{outer})$。

边缘羽化插值强度 $I$ 的计算公式为：

$$I = \text{clamp}\left(\frac{\theta - \text{outerCutOff}}{\text{cutOff} - \text{outerCutOff}}, \,0.0,\, 1.0\right)$$

- 当 $\theta \ge \text{cutOff}$（在内切角内部）：$I = 1.0$（全亮）。
- 当 $\theta \le \text{outerCutOff}$（在外切角外部）：$I = 0.0$（全暗）。
- 当处于内外切角之间时：$I$ 在 $0.0 \sim 1.0$ 之间平滑过渡。

```glsl
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float     shininess;
};

struct SpotLight {
    vec3 position;  // 聚光灯位置 (手电筒位置)
    vec3 direction; // 聚光灯朝向 (手电筒照射方向)

    // 传进来的是角度的余弦值 cos(angle)
    float cutOff;      // 内切角余弦值 (如 cos(12.5°))
    float outerCutOff; // 外切角余弦值 (如 cos(17.5°))

    // 聚光灯同样有距离衰减
    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform SpotLight spotLight;
uniform vec3 viewPos;

void main() {
    // 1. 采样纹理
    vec3 diffColor = vec3(texture(material.diffuse, TexCoords));
    vec3 specMask  = vec3(texture(material.specular, TexCoords));

    // 2. 计算光线方向和距离衰减
    vec3 lightDir = normalize(spotLight.position - FragPos);
    float distance = length(spotLight.position - FragPos);
    float attenuation = 1.0 / (spotLight.constant + 
                               spotLight.linear * distance + 
                               spotLight.quadratic * (distance * distance));

    // 3. 计算聚光灯边缘平滑强度 (Intensity)
    // theta 是当前光线 lightDir 与聚光灯中心方向 (-spotLight.direction) 的夹角余弦值
    float theta = dot(lightDir, normalize(-spotLight.direction));
    float epsilon = spotLight.cutOff - spotLight.outerCutOff;
    // clamp 函数把结果限制在 [0.0, 1.0] 范围内
    float intensity = clamp((theta - spotLight.outerCutOff) / epsilon, 0.0, 1.0);

    // 4. 环境光 (不受聚光灯切角强度影响，确保锥体外不是绝对死黑，但受距离衰减影响)
    vec3 ambient = spotLight.ambient * diffColor * attenuation;

    // 5. 漫反射
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = spotLight.diffuse * (diff * diffColor) * attenuation * intensity;

    // 6. 镜面高光
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = spotLight.specular * (spec * specMask) * attenuation * intensity;

    // 7. 最终合成
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
```



4.c++侧传递数据

```glsl
// 在 Render Loop 中：

// 1. 将聚光灯位置与摄像机绑定（实现手电筒效果）
lightingShader.setVec3("spotLight.position", camera.Position);
lightingShader.setVec3("spotLight.direction", camera.Front);

// 2. 传入切角的余弦值（千万不要在 CPU 端传角度度数给 Shader，要传 cos 值！）
lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));

// 3. 设置距离衰减系数 (以 50 米覆盖范围为例)
lightingShader.setFloat("spotLight.constant", 1.0f);
lightingShader.setFloat("spotLight.linear", 0.09f);
lightingShader.setFloat("spotLight.quadratic", 0.032f);
```



## 第十二章.多光源

在实际游戏或 3D 引擎中，场景中很少只有一个光源。通常会有一个太阳光（平行光）、若干个路灯或火把（点光源）、以及主角持有的手电筒（聚光灯）。

第 12 章的核心，就是**将第 11 章的三种光源模块化**，在一个片段着色器（Fragment Shader）中，把所有光源对当前像素的贡献全部累加起来。



#### 1.核心架构设计

为了保持 Shader 代码的整洁与高效，我们需要运用 **GLSL 函数化** 的思路：

1. 为每种光源类型编写一个独立的计算函数：
   - `vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)`
   - `vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)`
   - `vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)`
2. 在 `main()` 函数中初始化 `vec3 output = vec3(0.0)`。
3. 依次调用各光源函数，将返回的 RGB 颜色叠加（Additivie Blending）到 `output` 中。

```glsl
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// ------------------ 结构体定义 ------------------
struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float     shininess;
};

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    
    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define NR_POINT_LIGHTS 4 // 定义点光源数量

// ------------------ Uniform 变量 ------------------
uniform vec3 viewPos;
uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

// ------------------ 函数声明 ------------------
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {
    // 属性准备 (全局计算一次，传给各个函数)
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // 1. 累加定向光
    vec3 result = CalcDirLight(dirLight, norm, viewDir);

    // 2. 累加所有点光源
    for(int i = 0; i < NR_POINT_LIGHTS; i++) {
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    }

    // 3. 累加聚光灯 (手电筒)
    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

    FragColor = vec4(result, 1.0);
}

// ------------------ 函数实现 ------------------

// 计算平行光
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    
    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    // 镜面高光
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    // 采样贴图
    vec3 ambientColor  = vec3(texture(material.ambient, TexCoords));
    vec3 diffuseColor  = vec3(texture(material.diffuse, TexCoords));
    vec3 specularColor = vec3(texture(material.specular, TexCoords));
    
    // 合成
    vec3 ambient  = light.ambient * ambientColor;
    vec3 diffuse  = light.diffuse * diff * diffuseColor;
    vec3 specular = light.specular * spec * specularColor;
    
    return (ambient + diffuse + specular);
}

// 计算点光源
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    // 镜面高光
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    // 距离衰减
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    // 采样贴图
    vec3 ambientColor  = vec3(texture(material.diffuse, TexCoords));
    vec3 diffuseColor  = vec3(texture(material.diffuse, TexCoords));
    vec3 specularColor = vec3(texture(material.specular, TexCoords));
    
    // 合成并乘衰减
    vec3 ambient  = light.ambient * ambientColor * attenuation;
    vec3 diffuse  = light.diffuse * diff * diffuseColor * attenuation;
    vec3 specular = light.specular * spec * specularColor * attenuation;
    
    return (ambient + diffuse + specular);
}

// 计算聚光灯
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    // 镜面高光
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    // 衰减
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    // 聚光灯边缘平滑 (Intensity)
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    // 采样贴图
    vec3 ambientColor  = vec3(texture(material.diffuse, TexCoords));
    vec3 diffuseColor  = vec3(texture(material.diffuse, TexCoords));
    vec3 specularColor = vec3(texture(material.specular, TexCoords));
    
    // 合成
    vec3 ambient  = light.ambient * ambientColor * attenuation;
    vec3 diffuse  = light.diffuse * diff * diffuseColor * attenuation * intensity;
    vec3 specular = light.specular * spec * specularColor * attenuation * intensity;
    
    return (ambient + diffuse + specular);
}
```



2.C++ 端的数组 Uniform 设置方法

```c++
// 在 Render Loop 中设置 4 个点光源：
glm::vec3 pointLightPositions[] = {
    glm::vec3( 0.7f,  0.2f,  2.0f),
    glm::vec3(2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f,  2.0f, -12.0f),
    glm::vec3( 0.0f,  0.0f, -3.0f)
};

for (unsigned int i = 0; i < 4; i++) {
    std::string baseName = "pointLights[" + std::to_string(i) + "]";
    
    lightingShader.setVec3(baseName + ".position", pointLightPositions[i]);
    lightingShader.setVec3(baseName + ".ambient", 0.05f, 0.05f, 0.05f);
    lightingShader.setVec3(baseName + ".diffuse", 0.8f, 0.8f, 0.8f);
    lightingShader.setVec3(baseName + ".specular", 1.0f, 1.0f, 1.0f);
    lightingShader.setFloat(baseName + ".constant", 1.0f);
    lightingShader.setFloat(baseName + ".linear", 0.09f);
    lightingShader.setFloat(baseName + ".quadratic", 0.032f);
}
```



## 第十三章.深度测试(Depth Testing)和深度冲突(Z-Fighting)

在上一个阶段，我们完成了光照系统的搭建。但进入更复杂的 3D 场景时，你会发现一个新的基本问题：**渲染顺序**。如果不告诉 GPU 哪个物体在前面、哪个在后面，后画的物体就会直接遮挡先画的物体，哪怕它实际上应该被挡在后面。

解决这个问题的机制，就是 **深度测试（Depth Testing）**。



#### 1.深度测试

##### 1. 深度缓冲区 (Depth Buffer / Z-Buffer)

- **本质**：与颜色缓冲区（Color Buffer，存储 RGB 颜色）类似，深度缓冲区是一个由窗口系统（或 GLFW）自动创建的 2D 缓冲阵列。

- **数据格式**：通常每个像素占用 24 位（3 字节）精度。

- **数值范围**：所有深度值在屏幕空间中都被归一化到了 $[0.0, 1.0]$ 区间：

  - `0.0` 代表紧贴着**近平面（Near Plane）**。

  - `1.0` 代表位于**远平面（Far Plane）**。

    

在没有开启深度测试时，OpenGL 绘制物体完全遵循 **“画家算法”（Painter's Algorithm）** —— 后画的像素会直接覆盖先画的像素。

**但这在工程上是不可行的**：

1. **性能极差**：每一帧对几十万个三角形做 CPU 排序，CPU 帧率直接暴跌。
2. **互相穿插无解**：如果两个三角形在空间中**互相穿插/交叉**（A 穿过 B），你根本无法定义“谁在前面、谁在后面”。

因此，GPU 在硬件层面引入了 **深度缓冲区（Depth Buffer / Z-Buffer）**。

深度缓冲区是一个与屏幕分辨率完全对应的 2D 数组（比如 1920×1080）。每个像素位置不存 RGB，只存一个浮点数 $Z$（通常是 24-bit 精度）。



在 FS 里拿到的 `gl_FragCoord.z`，**绝对不是真实的物理距离 $z$**，而是一个经过剧烈压缩的**非线性值**。视图空间中，Z是线性的，但是3D到2D的过程，会进行透视转换和映射到[0,1]中，这是一种反比双曲线的变换关系。意味着，**近处（0.1m ~ 1.0m）**：占据了深度缓冲区高达 **90%** 的浮点数精度！**远处（1.0m ~ 100.0m）**：剩下的 **99% 的空间**，只能可怜地挤在 **10%** 的精度区间里！

验证：

```glsl
// 片段着色器
//在 FS 里直接把 gl_FragCoord.z 当成颜色输出：
void main() {
    // 直接输出 gl_FragCoord.z
    FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
}

//把非线性深度还原为线性距离的公式
//可以在 Shader 里做逆运算，把非线性的 gl_FragCoord.z 还原回真正的物理距离：
#version 330 core
out vec4 FragColor;

float near = 0.1; 
float far  = 100.0; 

// 将非线性深度值 [0, 1] 还原为线性的物理距离 [near, far]
float LinearizeDepth(float depth) {
    // 1. 转回 NDC 空间 [-1, 1]
    float z = depth * 2.0 - 1.0; 
    // 2. 执行逆透视投影计算
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main() {
    float depth = LinearizeDepth(gl_FragCoord.z); // 得到真实的物理米数
    // 归一化到 [0, 1] 用于颜色显示（除以 far）
    FragColor = vec4(vec3(depth / far), 1.0);
}
```



##### 2. 测试管线流程

深度测试在片段着色器执行之后、写入颜色缓冲区之前进行（即属于 **Early Depth Testing** 前置测试或**屏幕空间测试**阶段）：

1. 当渲染一个片段时，GPU 将该片段的 $Z$ 值（$z \in [0.0, 1.0]$）与深度缓冲区中**同一坐标位置现有的 Z 值**进行比较。
2. **测试通过**：该片段比原有的片段更靠近摄像机，GPU 更新颜色缓冲区，并将当前片段的 $Z$ 值写入深度缓冲区。
3. **测试失败**：该片段被遮挡，GPU **直接丢弃（Discard）** 该片段，不更新任何缓冲区。



#### 2.API配置

```c++
// 1. 开启深度测试
glEnable(GL_DEPTH_TEST);

// 2. 在渲染循环（Render Loop）的每一帧开始时，必须同时清空颜色缓冲区和深度缓冲区！
glClear(GL_COLOR_BUFFER_MAX | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// 3. (可选) 修改深度测试比较函数，默认是 GL_LESS
// 即：只有当前片段 Z 值 < 缓冲区现有 Z 值时才通过测试
glDepthFunc(GL_LESS);
```



#### 常见的 `glDepthFunc` 测试规则：

| **函数标志** | **通过条件**                        | **适用场景**                 |
| ------------ | ----------------------------------- | ---------------------------- |
| `GL_LESS`    | $Z_{\text{new}} < Z_{\text{old}}$   | **默认选项**。渲染不透明物体 |
| `GL_LEQUAL`  | $Z_{\text{new}} \le Z_{\text{old}}$ | 天空盒（Skybox）渲染         |
| `GL_ALWAYS`  | 无条件通过                          | 调试或画 HUD UI 元素         |
| `GL_NEVER`   | 永远不通过                          | 调试                         |

你还可以使用 `glDepthMask(GL_FALSE)` 来**禁用深度写入**（只读取、不更新深度缓冲区），这在后面渲染**半透明物体（Alpha Blending）** 时至关重要。



#### 3.Z-Fighting（深度冲突）问题与解决方案

在远处（比如 50 米开外），深度缓冲区的精度被压缩到了极致。可能 $50.0$ 米和 $50.01$ 米算出来的 $z_{\text{depth}}$ 值为：

- 50.00m $\rightarrow$ `0.99999988`
- 50.01m $\rightarrow$ `0.99999989`

由于 24 位单精度浮点数的尾数有限，GPU **在数学上根本无法区分这两个数值的大小**！

这导致场景微小移动或旋转时，GPU 一会儿判定 A 在前，一会儿判定 B 在前，画面呈现出**剧烈的斑驳闪烁**。



##### 3 种工程解法（按推荐程度排序）：

1.不让多边形物理重合（最根本）

​	在 3D 建模或场景布置时，绝对不要把海报/地毯直接贴在墙面/地面同一平面上，手动偏移 `0.001` 单位。

2.合理设置近平面 `Near`（性价比最高）

​	看前面的公式可知，`near` 越小，精度的拉伸越畸形！

- **错误示范**：`glm::perspective(45.0f, aspect, 0.0001f, 1000.0f);`（`0.0001` 会瞬间吸走 99.9% 的精度，远景直接崩塌）。
- **正确做法**：将 `near` 提高到 `0.1f` 或 `1.0f`，Z-Fighting 立即消除大半。

3.开启多边形偏移（Polygon Offset）

当绘制贴纸、阴影贴图（Shadow Maps）或网格线框时，利用 GPU 的渲染偏移机制：

```c++
// 1. 绘制主体地面
DrawGround();

// 2. 绘制地面的跑道贴纸线
glEnable(GL_POLYGON_OFFSET_FILL);
// 两个参数：factor 影响斜率，units 影响最小可分分辨率
// 负值意味着将深度向“靠近摄像机”的方向推拉
glPolygonOffset(-1.0f, -1.0f); 

DrawRunwayLines(); // 渲染贴纸，绝对不会与地面发生 Z-Fighting

glDisable(GL_POLYGON_OFFSET_FILL);
```



#### 4.c++侧代码

```c++
int main() {
    // ... 初始化 GLFW/GLAD ...

    // 1. 全局开启深度测试
    glEnable(GL_DEPTH_TEST);
    
    // 设置深度比较函数 (默认就是 GL_LESS，即新 Z < 旧 Z 时通过)
    glDepthFunc(GL_LESS);

    // 渲染循环
    while (!glfwWindowShouldClose(window)) {
        // 2. 极其重要：每一帧清空颜色缓冲区的同时，必须清空深度缓冲区！
        // 如果漏掉 GL_DEPTH_BUFFER_BIT，画面会卡死在第一帧的深度状态
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 3. 正常激活 Shader、绑定 VAO 并绘制
        ourShader.use();
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
```



## 第十四章.**模板测试（Stencil Testing）与物体轮廓描边（Object Outlining）**

在上一章中，我们用深度缓冲区解决了“谁遮挡了谁”的问题。而 **模板缓冲区（Stencil Buffer）** 则是另一个极具表现力的 2D 掩码缓冲区。它最经典的应用场景之一，就是**游戏和 3D 编辑器中的物体选中/高亮轮廓描边（Outlining/Highlighting）**。



#### 1.模板缓冲区

. 模板缓冲区的硬件本质：像素级“画板遮罩”。你可以把 **颜色缓冲区（Color Buffer）** 想象成最终展示的画布，把 **深度缓冲区（Depth Buffer）** 想象成记录“深度距离”的测量表，而 **模板缓冲区（Stencil Buffer）** 则是一张**你可以随时用剪刀裁切、随时用画笔标记的遮罩版纸（Masking Tape）**。



##### 内存布局与数据结构

- 模板缓冲区与颜色、深度缓冲区同宽高（比如 $1920 \times 1080$ 个像素点）。

- 每个像素通常占 **8 个 bit（1 个字节）**，这意味着它的取值范围是 **$0 \sim 255$（即 `0x00` 到 `0xFF`）**。

- 在初始化 OpenGL 窗口上下文（如 GLFW / Qt）时，只要申请了 `GLFW_STENCIL_BITS, 8`，这个缓冲区就在 GPU 显存中静静待命了。

  

##### 渲染管线中的精准位置

片段着色器执行完后，像素需要闯过**三重关卡**才能最终写入屏幕：

$$\text{Fragment Shader} \longrightarrow \mathbf{1.\ \text{Stencil Test}} \longrightarrow \mathbf{2.\ \text{Depth Test}} \longrightarrow \mathbf{3.\ \text{Alpha Blending}} \longrightarrow \text{Frame Buffer}$$



#### 2.深度拆解：模板测试的两大核心控规

就是什么样的模板能通过测试。

##### 1.比较控制：glStencilFunc(func, ref, mask)

##### 当前片段凭什么能通过测试？

GPU 在处理每个片段时，会执行如下的计算逻辑：

​	**`ref` (Reference Value)**：你设置的**参考值**（$0 \sim 255$）

**`	mask`**：**按位与（AND）掩码**。在比较之前，`ref` 和缓冲区现有的值都会先与 `mask` 相与。通常我们设为 `0xFF`（即全 1，不屏蔽任何 bit）。

**`func`**：比较条件（`GLenum`）：

- `GL_ALWAYS`：总是通过（无条件写入/测试）。
- `GL_NEVER`：永远不通过。
- `GL_EQUAL`：相等时通过。
- `GL_NOTEQUAL`：**不相等时通过**（物体描边算法的核心！）。
- `GL_LESS` / `GL_LEQUAL` / `GL_GREATER` / `GL_GEQUAL`：大小关系比较。



##### 2.更新控制：`glStencilOp(sfail, dpfail, dppass)`

##### 测试完之后，模板缓冲区里的值要变成什么？

因为像素在进入颜色缓冲区前还要过“深度测试”这一关，所以 GPU 划分了 **3 种不同命运**：

| **参数名**   | **发生情况**                                           | **常见选择与含义**                          |
| ------------ | ------------------------------------------------------ | ------------------------------------------- |
| **`sfail`**  | **模板测试失败**（Stencil Fail）                       | `GL_KEEP`（保持原值不动）                   |
| **`dpfail`** | 模板测试通过，但**深度测试失败**（Depth Fail）         | `GL_KEEP`（保持原值不动）                   |
| **`dppass`** | 模板测试通过，且**深度测试通过**（Depth/Stencil Pass） | `GL_REPLACE`（将 `ref` 强行写入模板缓冲区） |



##### 3.写入开关

```c++
glStencilMask(0xFF); // 允许写入模板缓冲区（8个 bit 全部开启）
glStencilMask(0x00); // 禁止写入模板缓冲区（只读模式，锁定保护）
```



#### 3.实战推导：物体选中高亮描边（Object Outlining）算法

这套机制可以实现游戏引擎（如 Unity / Unreal）里选中文本、模型时的**描边轮廓效果**。

核心算法思想：两遍绘制（Two-Pass Rendering）

```
【Pass 1: 标记原模型】
  1. 开启模板测试，设置条件为 GL_ALWAYS，ref = 1。
  2. 正常绘制物体。
  3. 结果：屏幕上物体占用的像素，模板值全部变成了 1。
            ┌─────────┐
            │  Stencil│
            │   = 1   │
            └─────────┘

【Pass 2: 绘制放大的轮廓】
  1. 模板条件改为 GL_NOTEQUAL, ref = 1 (只有 Stencil != 1 的地方才能画！)。
  2. 禁用模板写入 (glStencilMask(0x00))，关闭深度测试 (glDisable(GL_DEPTH_TEST))。
  3. 将顶点在相机视角下膨胀放大 1.05 倍，用纯色 Shader 绘制。
  4. 结果：
      ┌───────────────┐
      │ 纯色描边 (放大)│
      │ ┌───────────┐ │
      │ │被遮挡丢弃! │ │  <- 此处 Stencil == 1，测试失败！
      │ │(Stencil=1)│ │
      │ └───────────┘ │
      └───────────────┘
```



##### 绘制放大的轮廓:

方案 ：在 Vertex Shader 中沿法线挤出（Extrude Along Normal，专业做法）,在顶点着色器里，将顶点的局部坐标按照**法线方向**微量推开.

```glsl
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float outlineWidth = 0.03; // 沿着法线挤出的外扩距离

void main() {
    // 将顶点沿法线方向外扩
    vec3 extrudedPos = aPos + aNormal * outlineWidth;
    gl_Position = projection * view * model * vec4(extrudedPos, 1.0);
}
```

