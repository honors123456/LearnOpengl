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



## 第六章：**摄像机（Camera）与动态视角**

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
