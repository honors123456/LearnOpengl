



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

模板缓冲区的硬件本质：像素级“画板遮罩”。你可以把 **颜色缓冲区（Color Buffer）** 想象成最终展示的画布，把 **深度缓冲区（Depth Buffer）** 想象成记录“深度距离”的测量表，而 **模板缓冲区（Stencil Buffer）** 则是一张**你可以随时用剪刀裁切、随时用画笔标记的遮罩版纸（Masking Tape）**

##### GPU 渲染的核心原则永远是：**只有模型（几何体）形状盖到的像素，GPU 才会去处理它。*



物理类比：喷漆与纸板遮罩（Stencil Template）,想象你是一个街头艺术家，要在墙上用喷漆喷出一个**星形图案**：

步骤 1：取一张完整的卡纸（代表模板缓冲区，初始全是 0）。
步骤 2：用剪刀在卡纸中间剪出一个星形洞（在星形区域写入 1）。
步骤 3：把卡纸贴在墙上，拿起红色喷漆对着墙一阵猛喷（渲染绘制）。

结果是什么？**只有卡纸上有洞的地方（值为 1 的区域），红色喷漆才能穿过去留在墙上；卡纸挡住的地方（值为 0 的区域），喷漆全被挡住了。**

在英语里，“Stencil” 本身的意思就是“用来喷漆/印花的镂空样板/刻字板”。

GPU 里的模板缓冲区，就是**显存里的一张数字刻字板**。



核心逻辑：从“墙”到 GPU 像素

假设你的屏幕分辨率是 4×4 的像素格子。模板缓冲区（Stencil Buffer）就是一张对应大小的表格，每个格子存一个数字（默认是 0）：

初始状态（模板缓冲区全为 0）：

```
[ 0 ][ 0 ][ 0 ][ 0 ]
[ 0 ][ 0 ][ 0 ][ 0 ]
[ 0 ][ 0 ][ 0 ][ 0 ]
[ 0 ][ 0 ][ 0 ][ 0 ]
```

##### 第一步：画一个 2x2 的正方形（Pass 1 - 抠洞/标记）

你跟 GPU 说：“我现在要画个正方形。但我**先不画颜色**，我只是要在正方形覆盖的地方，把模板缓冲区里的数字**改成 1**！”

绘制后，模板缓冲区变成了：

```c++
[ 0 ][ 0 ][ 0 ][ 0 ]
[ 0 ][ 1 ][ 1 ][ 0 ]  <-- 正方形占用的地方
[ 0 ][ 1 ][ 1 ][ 0 ]  <-- 被标记成了 1
[ 0 ][ 0 ][ 0 ][ 0 ]
```



##### 第二步：全屏喷橙色油漆（Pass 2 - 过滤绘制）

现在，你拿着橙色油漆准备喷满整个屏幕，但你给 GPU 设定了一条规则：

👉 **“只有模板值【不等于 1】的格子，才允许喷上橙色！”**

GPU 挨个像素检查：

- 边缘的 `0` 格子：不等于 1，**测试通过** $\rightarrow$ 喷上橙色！
- 中间的 `1` 格子：等于 1 了，**测试失败** $\rightarrow$ 丢弃！不准喷漆！

最终屏幕上的结果：

```c++
[ 橙 ][ 橙 ][ 橙 ][ 橙 ]
[ 橙 ][ 无 ][ 无 ][ 橙 ]  <-- 中间被刚才标记的 1 挡住了！
[ 橙 ][ 无 ][ 无 ][ 橙 ]
[ 橙 ][ 橙 ][ 橙 ][ 橙 ]
```



##### 那它是怎么实现“物体描边”的？

理解了上面的“刻字板”原理，描边就只有**极其简单的两步**：

1. **第一步（画原物体）**：

   你正常画一个角色/箱子。同时告诉 GPU：“凡是这个角色占用的像素，把模板缓冲区对应的格子都**改写成 1**”。

   *(此时角色画好了，显存里也记下了这个角色精准的轮廓形状 `1`)*

2. **第二步（画稍大的纯色外壳）**：

   你把角色**整体放大一圈**（或者沿法线挤出一点），换成纯橙色的 Shader 重新画一次，注意gpu只对物体进行渲染。

   但同时加上规则：**“只有模板值【不等于 1】的地方才能画！”**

**发生了什么？**

- 放大后的橙色角色，**中间绝大部分区域**都落在了刚才第一步标记的 `1` 里面 $\rightarrow$ 全部被遮罩挡住，**画不出来**。
- 只有放大后**多出来的边缘一圈**，落在了外部模板值为 `0` 的区域 $\rightarrow$ **成功画出橙色**！

这就是为什么中间不会被橙色涂满，**只在最外围露出一圈漂亮的橙色边框**！



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



#### 4.完整的c++ / opengl 工程实现

```c++
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
        glStencilMask(0x00)；//禁用所有位的写入权限，无论渲染命令成功与否，或者 glStencilOp 设置了什么操作，模板缓冲区里的任何像素值都不会改变。
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
```





## 第十五章.混合(Blending)与半透明渲染

如果你把之前的“模板测试”比作在画布上用剪刀切块，那“混合”就是**用水彩或彩色玻璃去一层层叠加颜色**。



#### 1.现实中的物理直觉

你为什么能看到玻璃背后的东西？

在现实世界里，当你透过的红色玻璃看后面的绿树时，你的眼睛接收到的光线其实是**两部分光的混合**：

1. **来自于玻璃本身反射/折射出来的红光**（源头，Source）。
2. **来自于玻璃后面绿树穿透出来的绿光**（背景，Destination）。

如果玻璃很厚、颜色很深（Alpha 接近 1.0），绿树的光线就被挡住了，你只能看到红玻璃；如果玻璃极薄、几乎全透明（Alpha 接近 0.0），绿树的光线大部分穿透进来，红玻璃的存在感极低。

GPU 在计算渲染颜色时，做的事情和这个物理过程一模一样！



#### 2.混合计算的数学核心：颜色融合公式

当一个半透明像素（比如玻璃）准备写进屏幕时，屏幕上原本已经画好了背景（比如天空或地面）。GPU 会通过一个**线性加权公式**将它们融合：

$$\text{最终颜色} = (\text{源像素颜色} \times \text{源因子}) + (\text{背景颜色} \times \text{目标因子})$$



在 OpenGL 中，这四个变量拥有专业的图形学术语：

**$\text{Source (源 / Src)}$**：**当前正在绘制的半透明片段**（例如：正在画的红玻璃，带 $\alpha$ 值）。

**$\text{Destination (目标 / Dst)}$**：**已经存储在颜色缓冲区里的背景像素**（例如：之前画好的蓝色天空）。

**$\text{Src Factor (源混合因子)}$**：给源颜色乘上的权重系数。

**$\text{Dst Factor (目标混合因子)}$**：给目标背景色乘上的权重系数。

##### 常用混合模式：透明玻璃（Alpha Blending），在代码中，最经典、最符合人类视觉直觉的混合设置只有一行：

```c++
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
```

我们把这个 API 设置代入上面的数学公式，它其实就是：

$$\text{FinalColor} = (\text{Color}_{\text{src}} \times \alpha_{\text{src}}) + (\text{Color}_{\text{dst}} \times (1 - \alpha_{\text{src}}))$$



**算一次就懂了：**

假设你想画一块红色的透明玻璃，颜色为 $\text{Red} = (1.0, 0.0, 0.0)$，Alpha 透明度 $\alpha = 0.6$（不透明度为 60%）。

此时屏幕背景已经是白色的天空，颜色为 $\text{White} = (1.0, 1.0, 1.0)$。

- **玻璃贡献**：$(1.0, 0.0, 0.0) \times 0.6 = (0.6, 0.0, 0.0)$
- **天空贡献**：$(1.0, 1.0, 1.0) \times (1 - 0.6) = (0.4, 0.4, 0.4)$
- **相加得到最终颜色**：$(0.6, 0.0, 0.0) + (0.4, 0.4, 0.4) = (1.0, 0.4, 0.4)$（一种透着浅白光的红）



#### 3.Alpha 裁剪（Alpha Testing） vs Alpha 混合（Alpha Blending）

处理带透明通道（Alpha）的贴图时，工业界有两种截然不同的路线。选错路线，不仅性能变差，画面还会出现严重瑕疵。

```c++
			   ┌───────────────────────────────┐
               │    贴图包含 Alpha 透明通道      │
               └──────────────┬────────────────┘
                              │
              ┌───────────────┴───────────────┐
              ▼                               ▼
    【路线 A：Alpha 裁剪】           【路线 B：Alpha 混合】
    (Discard / Cutout)              (Alpha Blending)
   Alpha 非 0 即 1 (硬边缘)         Alpha 在 0~1 之间 (连续过渡)
   示例：树叶、栅栏、铁丝网          示例：玻璃、烟雾、水面、火焰
```



##### 路线 A：Alpha 裁剪（Discard / 硬透明）

像**铁丝网**或者**栅栏**贴图，像素点的透明度要么是 `0.0`（镂空空隙），要么是 `1.0`（实体网丝）。你根本不需要 GPU 去做复杂的颜色比例加权。

处理方法极其简单粗暴——直接在 **Fragment Shader（片段着色器）** 里用 `discard` 关键字：

```glsl
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D texture_diffuse;

void main() {
    vec4 texColor = texture(texture_diffuse, TexCoords);
    
    // 如果 Alpha 太低（比如小于 0.1），说明是网格空隙，直接把这个片段扔掉！
    if(texColor.a < 0.1)
        discard;
        
    FragColor = texColor;
}
```

**优点**：不需要开启 `glEnable(GL_BLEND)`，也不需要关深度测试，甚至不需要对物体排序，性能极高。



##### 路线 B：Alpha 混合（真正的半透明）

当你需要渲染玻璃、玻璃杯、烟雾这种边缘柔和、具有连续渐变透明度的物体时，就必须开启 `glEnable(GL_BLEND)`。

但开启它的同时，你也引入了图形学中最令人头疼的问题——**深度冲突与渲染顺序陷阱**。



我们先看一个经典的图形学 Bug： 如果你先画了一块**离相机很近的玻璃**，再画**玻璃后方远处的箱子**，会发生什么？

【错误的画法逻辑】
1. 先画近处的半透明玻璃：
   - GPU 算了颜色，画上了半透明红。
   - 重点：深度缓冲区（Depth Buffer）更新！记录下了“此像素处有一个很近的物体（Z 值很小）”。

2. 再画远处的箱子：
   - GPU 准备绘制箱子时，先做【深度测试】。
   
   - GPU 发现：“诶？这箱子的 Z 值比刚才记录的 Z 值更大（更远），说明箱子被遮挡了！”
   
   - 结果：GPU 直接把箱子的片段丢弃（Discard）了！
   
     

【最终画面】
透过玻璃你看不到背后的箱子，箱子直接凭空消失了！

不透明物体无论先画谁、后画谁，**深度测试（Depth Test）** 都能自动保证近处物体盖住远处物体。但半透明物体要求**远处的物体必须先画在背景上**，近处的半透明物体才能拿到“背景颜色”去乘 $(1 - \alpha)$！

解决半透明穿帮的唯一黄金法则，就是遵循现实中画家的作画习惯（**画家算法 Painter's Algorithm**）：**永远先画远景，再画近景。**

在 OpenGL 引擎中，标准的半透明渲染流程分为 **6 个严密的步骤**：

```
┌─────────────────────────────────────────────────────────┐
│ 步骤 1：开启深度测试 (glEnable(GL_DEPTH_TEST))            │
└────────────────────────────┬────────────────────────────┘
                             │
┌────────────────────────────▼────────────────────────────┐
│ 步骤 2：先绘制场景中所有【不透明】物体（箱子、墙壁、地面）  │
└────────────────────────────┬────────────────────────────┘
                             │
┌────────────────────────────▼────────────────────────────┐
│ 步骤 3：根据到摄像机的距离，对所有【半透明】物体从远到近排序 │
└────────────────────────────┬────────────────────────────┘
                             │
┌────────────────────────────▼────────────────────────────┐
│ 步骤 4：开启混合 (glEnable(GL_BLEND))                   │
│         设置因子 glBlendFunc(GL_SRC_ALPHA, ...)         │
└────────────────────────────┬────────────────────────────┘
                             │
┌────────────────────────────▼────────────────────────────┐
│ 步骤 5：按【由远及近】的顺序，依次绘制所有半透明物体      │
└────────────────────────────┬────────────────────────────┘
                             │
┌────────────────────────────▼────────────────────────────┐
│ 步骤 6：渲染完毕，关闭混合 (glDisable(GL_BLEND))         │
└─────────────────────────────────────────────────────────┘
```



##### 4.C++ 现代 OpenGL 完整排序与渲染实现

我们可以利用 C++ 标准库中的 `std::map` 容器来自动帮你做**由远及近的距离排序**。

在 C++ 的 `std::map<Key, Value>` 中，`Key` 默认按从小到大（升序）**排列。如果我们把“距离相机的距离”作为 Key，使用**反向迭代器（Reverse Iterator）**遍历，就能轻而易举地获得**从大到小（由远及近）的渲染顺序！

```c++
#include <map>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// 假设场景里有 3 块玻璃窗户的位置
std::vector<glm::vec3> windowsPositions {
    glm::vec3(-1.5f, 0.0f, -0.48f),
    glm::vec3( 1.5f, 0.0f,  0.51f),
    glm::vec3( 0.0f, 0.0f,  0.7f)
};

while(!glfwWindowShouldClose(window))
{
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

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
    objShader.setVec3("cameraPos",camera.Position);

    // 设置 Light 结构体
    objShader.setVec3("light.position", lightPos);
    objShader.setVec3("light.ambient",  glm::vec3(0.2f, 0.2f, 0.2f));
    objShader.setVec3("light.diffuse",  glm::vec3(0.5f, 0.5f, 0.5f)); // 调暗漫反射试一下效果
    objShader.setVec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f));

    // 设置 Material 结构体
    objShader.setInt("material.diffuse", 0);
    objShader.setInt("material.specular", 1);
    objShader.setInt("material.emission", 2);
    objShader.setFloat("material.shininess", 64.0f);
    objShader.setFloat("time", static_cast<float>(glfwGetTime()));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specularMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, emissiveMap);

    //绘制物体正方体
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES,indexCount,GL_UNSIGNED_INT,0);
    glBindVertexArray(0);

    // 半透明物体最后绘制，并根据相机距离从远到近排序。
    std::vector<glm::vec3> sortedWindows = windowsPositions;
    std::sort(sortedWindows.begin(), sortedWindows.end(), [](const glm::vec3& left, const glm::vec3& right) {
        return glm::length(camera.Position - left) > glm::length(camera.Position - right);
    });

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE); // 仍做深度测试，但不让透明物体覆盖深度缓冲。

    windowShader.use();
    windowShader.setMat4("view", view);
    windowShader.setMat4("projection", projection);
    windowShader.setInt("windowTexture", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, windowTexture);
    glBindVertexArray(windowVAO);

    for (const glm::vec3& position : sortedWindows) {
        glm::mat4 windowModel = glm::mat4(1.0f);
        windowModel = glm::translate(windowModel, position);
        windowShader.setMat4("model", windowModel);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    glfwSwapBuffers(window);
    glfwPollEvents();
}
```





## 第十六章.面剔除(Face Culling)

如果把渲染管线比作生产线，之前的“深度测试”是在最后包装阶段检查产品好坏，而“面剔除”则是在**最前端就把根本不需要生产的半成品直接扔掉**。



#### 1.现实中的直觉：你看不到物体的“背面”

想象你桌上放着一个实心的木质立方体：

- 无论你站在哪个角度看它，你**最多只能同时看到它的 3 个面**。
- 另外 3 个面向背后的面（背面），完全被立方体自身的正面挡住了。

在 3D 渲染中，一个立方体由 12 个三角形拼成。如果我们把背对相机的 6 个三角形也让片段着色器（Fragment Shader）光栅化、计算光照、采样纹理，**这些计算出来的像素最终又会被深度测试丢弃**，这完全是对 GPU 算力的巨大浪费！

> **面剔除的核心目的**：在三角形刚刚投射到屏幕、准备开始算像素之前，**一瞬间把所有“背对相机”的三角形丢掉**。开启它，GPU 的光栅化和片段着色器工作量直接斩掉将近 **50%**！



#### 2.GPU 是怎么知道三角形“背对相机”的？

GPU 既不需要做复杂的光线追踪，也不需要计算法线点积。它判断“正反面”的方法高效到令人发指——只看**顶点的缠绕顺序（Winding Order）**。

当你在代码里定义一个三角形的三个顶点时（例如 $A \to B \to C$），它们在屏幕上的投影顺序只有两种可能：顺时针，逆时针。



假设你在 3D 空间中定义了一个三角形，从**正面**看它，它的三个顶点顺序是 **逆时针（A $\to$ B $\to$ C）**。

现在，**你走到这个三角形的背面去观察它**，猜猜看它的投影顺序变成了什么？

——**它在屏幕上变成了顺时针！**

正是利用这个空间透视几何的物理规律：**OpenGL 默认规定，屏幕上呈现逆时针（CCW）的三角形是正面，呈现顺时针（CW）的三角形是背面。** GPU 只需要看顶点在屏幕上的排列方向，就能在微秒级内判断它是正面还是背面！



#### 3.核心API

开启/关闭

```c++
glEnable(GL_CULL_FACE);  // 开启面剔除状态机
glDisable(GL_CULL_FACE); // 关闭面剔除
```



告诉gpu剔除哪一边

```c++
glCullFace(GL_BACK);          // 剔除背面（最常用！默认值）
glCullFace(GL_FRONT);         // 剔除正面（只绘制背面，某些特殊阴影算法会用）
glCullFace(GL_FRONT_AND_BACK);// 正背面全剔除（画面啥都不画）
```



改变正面的判定规则（可选）

默认情况下 `GL_CCW`（逆时针）是正面。如果你在使用 DirectX 的模型数据（DirectX 默认 `GL_CW` 顺时针为正面），你可以切换它：

```c++
glFrontFace(GL_CW);  // 把顺时针定义为正面
glFrontFace(GL_CCW); // 把逆时针定义为正面（OpenGL 默认）
```



#### 4.两个最容易踩到的“面剔除 Bug”

##### 坑 1：模型看起来“镂空”或者“法线反了”

**现象**：开启面剔除后，实心模型的某些面消失了，透过洞能看到模型内部。

**原因**：你在手写顶点数组（VAO/EBO）或者建模软件导出模型时，顶点的**索引顺序画反了**（比如原本该逆时针画的面写成了顺时针）。GPU 误以为正面的面是背面，直接给剔除掉了。



##### 坑 2：植物、草叶、栅栏转个身就消失了

**现象**：场景里的草叶、花朵或单面墙，从正面看好好的，摄像机绕到背面时突然全不见了。

**原因**：草叶和栅栏通常是**无厚度的单层平面（Single Quad）**。它只有正面没有背面。当你绕到它身后时，GPU 判定该四边形为“背面”，顺手就把它给剔除了！

```c++
【渲染无厚度物体（草、花、窗户贴图）的标准策略】

// 1. 绘制带有厚度的 3D 实体模型（房子、箱子、角色）
glEnable(GL_CULL_FACE);
drawSolidModels();

// 2. 绘制没有厚度的单面物体（草叶、双面图层）时，临时关闭面剔除！
glDisable(GL_CULL_FACE);
drawFoliageAndGrass();
```



#### 5.总结

手写代码时，你只需要记住**写顶点数据时按照逆时针（CCW）定义**，然后在渲染循环里加上这三行，面剔除就能完美工作：

```c++
glEnable(GL_CULL_FACE);
glCullFace(GL_BACK);
glFrontFace(GL_CCW);
```



## 第十七章.帧缓冲区（Framebuffer）与离屏渲染

如果把之前学的所有渲染技术比作在**画板**上画画，那么到目前为止，你的画笔一直只能直接画在“屏幕”（默认帧缓冲区）这块画板上。

而这一章，我们要带你拆掉这块屏幕，给 GPU 装上一张“隐形画板”！

#### 1.帧缓冲区

在 OpenGL 中，**帧缓冲区（FBO）** 就像是一个**隐形画架**。

- **默认帧缓冲区（Default Framebuffer）**：你初始化 GLFW / Qt 窗口时系统自动为你创建的。你在片段着色器中输出的 `FragColor`，最终会直接呈现在电脑显示器上。
- **自定义帧缓冲区（Custom FBO）**：你自己手动在 GPU 内存里开辟的一块“画板”。你在上面画的任何东西，屏幕上都**完全看不到**！这就叫 **离屏渲染（Off-screen Rendering）**。

离屏渲染画出来的不是死数据，而是一张**存放在 GPU 显存里的纹理贴图（Texture）**。我们可以把这张贴图拿过来，当成输入再喂给下一个着色器。**后处理特效（如夜视仪、黑白电影、模糊、 Bloom 光晕、阴影贴图 Shadow Map）全靠它！**



#### 2.帧缓冲区的附件(Attachments)

一个空的 FBO 就像是一个**没有装画纸的空画架**，它是不能直接用来绘画的。要想让一个自定义 FBO 生效，你必须给它挂载至少一个**附件（Attachment）**：

```c++
				+----------------------------------+
               |    自定义帧缓冲区 (FBO)           |
               +----------------------------------+
                               |
       +-----------------------+-----------------------+
       |                                               |
  【颜色附件】                                     【深度/模板附件】
Color Attachment                               Depth/Stencil Attachment
 (纹理 Texture)                              (渲染缓冲区 Renderbuffer - RBO)
       |                                               |
 存储最终画出来的颜色                           存储深度 Z-buffer 和 stencil 模板
（可以当贴图再次采样！）                        （只写不读，极速优化）
```



**颜色附件（Color Attachment）**：

- 用 **纹理（Texture）** 来充当。片段着色器算出来的颜色直接画进这张纹理里。
- **最大优势**：渲染完后，你可以把这张纹理绑定到别的 Shader 里，像普通图片一样去采样它！



**深度/模板附件（Depth / Stencil Attachment）**：

- 用 **渲染缓冲区对象（Renderbuffer Object - RBO）** 来充当。
- 专门用来存 `GL_DEPTH_TEST` 的深度值。RBO 是专门为只写不读（Write-Only）优化的显存格式，性能极高。



#### 3.代码实现

实现一个“反色 / 夜视仪”滤镜：

```c++
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture; // 离屏渲染存好的场景贴图

void main() {
    vec3 col = texture(screenTexture, TexCoords).rgb;
    
    // 效果 1：反色滤镜（Inversion）
    FragColor = vec4(1.0 - col, 1.0);
    
    // 效果 2：黑白电影滤镜（Grayscale）
    // float average = 0.2126 * col.r + 0.7152 * col.g + 0.0722 * col.b;
    // FragColor = vec4(vec3(average), 1.0);
}
```



滤镜着色器

```c++
//滤镜着色器
Shader filterShader(SHADER_DIR "/filterShader.vert", SHADER_DIR "/filterShader.frag");
```



滤镜顶点数据

```c++
// 全屏四边形：每个顶点包含屏幕位置和纹理坐标。
float quadVertices[] = {
    // position    // texture
    -1.0f,  1.0f,  0.0f, 1.0f,
    1.0f,  1.0f,  1.0f, 1.0f,
    1.0f, -1.0f,  1.0f, 0.0f,

    1.0f, -1.0f,  1.0f, 0.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
    -1.0f,  1.0f,  0.0f, 1.0f
};

unsigned int quadVAO,quadVBO;
glGenVertexArrays(1,&quadVAO);
glBindVertexArray(quadVAO);

glGenBuffers(1,&quadVBO);
glBindBuffer(GL_ARRAY_BUFFER,quadVBO);
glBufferData(GL_ARRAY_BUFFER,sizeof(quadVertices),quadVertices,GL_STATIC_DRAW);

glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);
glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
glEnableVertexAttribArray(1);
glBindVertexArray(0);

//1.创建并绑定帧缓冲区
GLuint FBO;
glGenFramebuffers(1,&FBO);
glBindFramebuffer(GL_FRAMEBUFFER,FBO);

//2.创建空的2D纹理(颜色附件)
GLuint textureColorBuffer;
glGenTextures(1,&textureColorBuffer);
glBindTexture(GL_TEXTURE_2D,textureColorBuffer);
//注意: 最后一个 NULL 代表现在不传图像数据，只开辟一块跟屏幕一样大的显存空间
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// 将纹理挂载到 FBO 的 COLOR_ATTACHMENT0
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);

//3.创建RBO(深度和模板附件)
GLuint RBO;
glGenRenderbuffers(1, &RBO);
glBindRenderbuffer(GL_RENDERBUFFER, RBO);
glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600); // 24位深度 + 8位模板
glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);   

//4.检查FBO是否完整配置成功
if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

//解绑帧缓冲区
glBindFramebuffer(GL_FRAMEBUFFER, 0);
```



渲染循环

```c++
while(!glfwWindowShouldClose(window))
{
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    //pass 1 离屏渲染 (在帧缓冲区中绘制目标场景)
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

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
    objShader.setVec3("cameraPos",camera.Position);

    // 设置 Light 结构体
    objShader.setVec3("light.position", lightPos);
    objShader.setVec3("light.ambient",  glm::vec3(0.2f, 0.2f, 0.2f));
    objShader.setVec3("light.diffuse",  glm::vec3(0.5f, 0.5f, 0.5f)); // 调暗漫反射试一下效果
    objShader.setVec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f));

    // 设置 Material 结构体
    objShader.setInt("material.diffuse", 0);
    objShader.setInt("material.specular", 1);
    objShader.setInt("material.emission", 2);
    objShader.setFloat("material.shininess", 64.0f);
    objShader.setFloat("time", static_cast<float>(glfwGetTime()));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specularMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, emissiveMap);

    //绘制物体正方体
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES,indexCount,GL_UNSIGNED_INT,0);
    glBindVertexArray(0);

    //Pass 2：后处理渲染（关闭帧缓冲区绘制，在当前缓冲区绘制滤镜）
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    glDisable(GL_DEPTH_TEST);   //全屏2d贴图不需要深度测试

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    //启用帧缓冲区的颜色附件，此时上面的绘制内容全在纹理中，类似于Qt中的QPainter(&QImage),在图片中绘制，然后再贴图
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer); 
    
    filterShader.use();
    filterShader.setInt("screenTexture", 3);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    glfwSwapBuffers(window);
    glfwPollEvents();
}
```





## 第十八章.立方体贴图和天空盒

#### 1.立方体贴图（Cubemap）

想象你拿了一个正方体盒子，把它套在你的头上。盒子的 6 个内表面（前、后、左、右、上、下）分别贴着一张全景相机的照片。当你转头看时，你就会感觉自己置身于一个真实的 3D 世界中——这就是**天空盒（Skybox）**。

在 OpenGL 中，立方体贴图是一种特殊的纹理类型（`GL_TEXTURE_CUBE_MAP`）。它由 **6 张独立的 2D 纹理** 组合而成，分别对应立方体的 6 个面。

```c++
				+----------+
                  |  Top (+Y)|
       +----------+----------+----------+----------+
       | Left (-X)| Front(+Z)|Right (+X)| Back (-Z)|
       +----------+----------+----------+----------+
                  |Bottom(-Y)|
                  +----------+
```



##### 采样机制的超酷改变：

普通的 2D 纹理使用 $(u, v)$ 坐标（取值 0 到 1）来采样像素。

而 Cubemap 完全不同！它使用一个 **3D 方向向量 $(x, y, z)$** 来采样。

> 💡 **原理**：想象立方体的中心是 $(0, 0, 0)$，你从中心发射一条**射线**指向方向 $(x, y, z)$。这条射线穿过立方体盒子哪个面的哪个像素，GPU 就会采样那个像素的值返回给你！



#### 2.在c++中加载Cubemap

加载 Cubemap 需要依次将 6 张图片的像素数据绑定到 `GL_TEXTURE_CUBE_MAP_POSITIVE_X` 到 `GL_TEXTURE_CUBE_MAP_NEGATIVE_Z` 这 6 个连续的枚举值上：

```c++
#include <glad/glad.h>
#include <vector>
#include <string>
#include <iostream>
#include "stb_image.h" // 常用图像加载库

unsigned int loadCubemap(std::vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        // 加载图片
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            // GL_TEXTURE_CUBE_MAP_POSITIVE_X + i 会按顺序遍历 6 个面：
            // Right(+X), Left(-X), Top(+Y), Bottom(-Y), Front(+Z), Back(-Z)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    // 设置环绕与过滤方式
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); // 3D 纹理第 3 维坐标 R

    return textureID;
}
```



#### 3.天空盒(SkyBox)的着色器

vertex shader:

```glsl
#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords; // 输出 3D 向量作为纹理采样坐标

uniform mat4 projection;
uniform mat4 view;

void main() {
    TexCoords = aPos; // 顶点的 3D 位置向量直接作为 Cubemap 的采样方向！
    
    // 技巧 1：移除 View 矩阵中的位移成分（只保留旋转），让天空盒永远跟着相机走！
    mat4 viewNoTranslation = mat4(mat3(view));
    
    vec4 pos = projection * viewNoTranslation * vec4(aPos, 1.0);
    
    // 技巧 2：将 gl_Position 的 z 分量设为 w，透视除法后 z/w = 1.0，强制让天空盒深度为最大值 (1.0)，确保它永远被场景中其他物体遮挡
   	// 为了保证天空盒永远在最远端,将z分量设置为w, 经过透视除法 z/w = 1.0后，标准化后z为1.0
    gl_Position = pos.xyww;
}
```



fragment shader:

```glsl
#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox; // 特殊采样器：samplerCube

void main() {
    // 直接用 3D 方向向量去采样 Cubemap
    FragColor = texture(skybox, TexCoords);
}
```



#### 4.主循环

天空盒要求模拟真实的天空，就要求场景里的东西必须挡住天空，而天空盒不能挡住任何物体。传统做法就是先绘制天空盒，再绘制物体。在gpu中就是绘制完所有东西之后，天空盒中那些被场景挡住的像素直接被覆盖了，相当于白绘制了，造成资源浪费。

为了极致的性能，Modern OpenGL 采用了一种**逆向思维**：把天空盒放在最后绘制！利用深度测试，让已经画好的房子和树木直接把天空盒剔除掉，这样 GPU 连天空盒被挡住部分的像素着色器都不用跑了。

因为我们在顶点着色器里把天空盒的深度设置成了最远的 `1.0`，所以在渲染循环中，我们需要配合调整 OpenGL 的深度测试条件.

```c++
//当执行深度清屏时，OpenGL 默认会把深度缓冲区（Depth Buffer）里全屏每一个像素的值全部填满为 1.0（代表目前没有任何东西，全是最远距离）
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// 1. 正常渲染场景中的 3D 实体模型（房子、玩家、树木）
glEnable(GL_DEPTH_TEST);
glDepthFunc(GL_LESS); // 默认比较条件,只有深度 < 当前 Depth Buffer 的像素才能画上去，例如各种3D物体

drawSceneObjects();

// 2. 渲染天空盒
// GL_LEQUAL（小于等于）,允许深度等于 1.0 的片段通过
// 当 GPU 遇到之前画过房子的地方（深度0.2）,1.0(天空盒) >= 0.2，不绘制天空盒。
// 当 GPU 遇到背景空白区域（深度1.0）：1.0(天空盒)= 1.0,绘制天空盒
glDepthFunc(GL_LEQUAL); 

skyboxShader.use();
skyboxShader.setMat4("view", camera.GetViewMatrix());
skyboxShader.setMat4("projection", camera.GetProjectionMatrix());

glBindVertexArray(skyboxVAO);
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
glDrawArrays(GL_TRIANGLES, 0, 36); // 绘制 36 个顶点构成的立方体

// 恢复默认深度测试比较条件
glDepthFunc(GL_LESS);
```



#### 5.Cubemap的进阶：环境映射（反射和折射）

Cubemap 不仅能画天空，还能作为**环境光照贴图**，让场景中的物体拥有质感满满的动态反射（Reflection）或折射（Refraction）效果！

##### 1.镜像反射

根据入射光向量 $I$（相机到物体的向量）和表面法线 $N$，计算出反射向量 $R = \text{reflect}(I, N)$，然后用 $R$ 去采样天空盒。

```glsl
#version 330 core
out vec4 FragColor;

in vec3 Normal; // 世界空间法线
in vec3 Position; // 世界空间顶点位置

uniform vec3 cameraPos; // 摄像机位置
uniform samplerCube skybox; // 天空盒

void main() {
    vec3 I = normalize(Position - cameraPos); // 入射光线
    vec3 N = normalize(Normal);               // 法线
    vec3 R = reflect(I, N);                  // 计算反射向量
    
    // 用反射向量去采样天空盒，实现铬合金/金属镜面效果！
    FragColor = vec4(texture(skybox, R).rgb, 1.0);
}
```



##### 2.玻璃折射

利用斯涅尔定律（Snell's Law），使用 GLSL 内置的 `refract()` 函数，传入折射率比值（例如空气到玻璃的折射率约 $\frac{1.00}{1.52} \approx 0.66$）。

```glsl
// 只需要把 reflect 换成 refract
float ratio = 1.00 / 1.52; // 空气 -> 玻璃
vec3 R = refract(I, normalize(Normal), ratio);
FragColor = vec4(texture(skybox, R).rgb, 1.0);
```



## 第十九章.高级数据和高级GLSL

在之前的渲染体验中，我们就像在使用“新手套装”：每次给 GPU 发送顶点，都是用 `glBufferData` 把数据一口气塞过去；每次给 Shader 传变量，都是用 `glUniformMatrix4fv` 逐个挨个喂。

但在大型游戏或者复杂场景中，这种“一次性、搬砖式”的数据传输会带来严重的 CPU 与 GPU 传输瓶颈。**本章我们将解锁 GPU 显存的“高级操作手册”，学会如何像外科医生一样精细手术显存，以及使用高阶 GLSL 技巧让多 Shader 共享内存！**



#### 1.显存的“微创手术”：`glBufferSubData` 与 `glMapBuffer`

##### 局部更新：`glBufferSubData`

想象一下，你创建了一个包含 $100$ 万个顶点的巨大 VBO。每一帧，你只需要修改其中 $10$ 个顶点的位置。

如果你继续调用 `glBufferData`，GPU 会做一件极其奢侈的事：**销毁整块旧显存，重新开辟显存，并把 $100$ 万个顶点重新搬运一遍！**

**`glBufferSubData`** 就像是显存的“微创手术”，它允许你在不重新分配显存的前提下，**只替换指定偏移量（Offset）和大小（Size）的内存数据**。

```c++
// 1. 初始化阶段：开辟 100 万个 float 的空间，但先不传数据 (传 NULL)
glBindBuffer(GL_ARRAY_BUFFER, VBO);
glBufferData(GL_ARRAY_BUFFER, 1000000 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

// 2. 渲染循环中：只更新第 100 到 103 个 float 的数据（零显存重新分配开销！）
float newPos[] = { 1.2f, 3.4f, 5.6f };
glBufferSubData(GL_ARRAY_BUFFER, 100 * sizeof(float), sizeof(newPos), newPos);
```



##### 显存指针：`glMapBuffer`

如果需要频繁对海量数据进行逻辑修改（比如在 CPU 上做布衣物理模拟、粒子位置更新），反复调用 `glBufferSubData` 依然会有大量的 API 函数开销。

**`glMapBuffer`** 允许你直接**拿到指向 GPU 显存的 C++ 内存指针**！你可以像操作普通 C++ 数组一样直接写内存：

```c++
glBindBuffer(GL_ARRAY_BUFFER, VBO);

// 1. 将显存“映射”到 CPU 可写的指针地址
float* ptr = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

if (ptr) {
    // 2. CPU 直接写显存（完全没有 API 函数调用开销）
    ptr[0] = 10.0f; // 修改第 0 个 float
    ptr[1] = 20.0f; // 修改第 1 个 float
    
    // 3. 修改完毕后必须解绑！GPU 才能安全读取这块显存
    glUnmapBuffer(GL_ARRAY_BUFFER);
}
```



#### 2.显存内的零拷贝：`glCopyBufferSubData`

如果你想把数据从 Buffer A 复制到 Buffer B，传统方式是：

$$\text{GPU (Buffer A)} \xrightarrow{\text{PCIe Bus}} \text{CPU RAM} \xrightarrow{\text{PCIe Bus}} \text{GPU (Buffer B)}$$

这种跨 PCIe 总线的传输极其缓慢！

**`glCopyBufferSubData`** 可以让数据**直接在 GPU 显存内部完成高速克隆**：

```c++
// 绑定源与目标缓冲区（使用专属的 COPY 绑定点，防止挤占普通的 ARRAY_BUFFER）
glBindBuffer(GL_COPY_READ_BUFFER, vboSource);
glBindBuffer(GL_COPY_WRITE_BUFFER, vboTarget);

// 将 vboSource 从 0 偏移开始的 1024 字节，直接克隆到 vboTarget 的 0 偏移处
glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, 1024);
```



#### 3.高级 GLSL：内嵌变量（Built-in Variables）

GLSL 为我们内置了一些非常实用的全局变量，熟练使用它们可以省去很多手写传递数据的麻烦。

##### 1. 顶点着色器内置变量

- **`gl_VertexID`**：当前正在处理的顶点的**索引编号**（当使用 `glDrawElements` 时它代表索引值，使用 `glDrawArrays` 时代表顶点的序号）。可以用来在 Shader 里根据 ID 动态生成 procedural 几何形状！
- **`gl_PointSize`**：如果开启了 `glEnable(GL_PROGRAM_POINT_SIZE);`，你可以直接在 VS 里动态控制**点图元渲染出来的像素像素大小**（非常适合做星空或动态粒子系统）。



##### 2. 片段着色器内置变量

- **`gl_FragCoord`**：当前片段在**屏幕窗口空间下的坐标**（$x, y, z$），$x, y$ 代表屏幕像素坐标（如 $1920 \times 1080$ 下的像素位置），$z$ 代表该片段的深度的 $0.0 \sim 1.0$ 值。可以用来做视口级别的扫描线、棋盘格或屏幕空间特效！
- **`gl_FrontFacing`**：布尔值，告诉你当前片段属于**正面（Front Face）还是背面（Back Face）**。在画双面透明物体（如双面发光的玻璃）时，你可以根据它动态翻转法线！



```c++
// 片段着色器：双面发光材质
#version 330 core
out vec4 FragColor;
in vec3 Normal;

void main() {
    vec3 N = normalize(Normal);
    if (!gl_FrontFacing) {
        N = -N; // 如果渲染的是背面，自动翻转法线方向！
    }
    // 正常计算光照...
}
```



#### 4.跨 Shader 终极共享内存：统一缓冲区对象（UBO）

##### 痛点：被 Uniform 传参支配的恐惧

假设你的场景里有 $10$ 个不同的 Shader 程序（有的渲染角色、有的渲染地形、有的渲染水体）。每一个 Shader 都需要用到摄像机的 **投影矩阵 (`projection`)** 和 **视图矩阵 (`view`)**。

传统做法是：在每一帧渲染循环里，你需要切 10 次 Shader，手写 20 次 `glUniformMatrix4fv`！这不仅代码冗长，而且 API 交互开销巨大。

##### 解法：Uniform Buffer Object (UBO)

UBO 允许我们在 GPU 里开辟一块**公共显存内存区**。所有 Shader 都能“挂载”到这块内存区。**你只需在 C++ 端更新一次 UBO，所有 Shader 就会瞬间同时收到最新的 View/Projection 矩阵！**



```glsl
#version 330 core
layout (location = 0) in vec3 aPos;

// 定义 Uniform 块
layout (std140) uniform Matrices {
    mat4 projection;
    mat4 view;
}; // 👈 所有 Shader 里这一段保持一模一样即可！

uniform mat4 model;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
```



```c++
// Step 1: 建立连接 - 将 Shader 的 Block 绑定到绑点 (Binding Point 0)
//glGetUniformBlockIndex检索shader里面的uniform块的索引
//glUniformBlockBinding将Uniform块索引和binding point通道号0绑定起来
unsigned int blockIndexA = glGetUniformBlockIndex(shaderA.ID, "Matrices");
glUniformBlockBinding(shaderA.ID, blockIndexA, 0); // 挂载到 Binding Point 0

unsigned int blockIndexB = glGetUniformBlockIndex(shaderB.ID, "Matrices");
glUniformBlockBinding(shaderB.ID, blockIndexB, 0); // 挂载到 Binding Point 0

// Step 2: 在 GPU 开辟 UBO 显存空间
unsigned int uboMatrices;
glGenBuffers(1, &uboMatrices);
glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
// 空间大小为 2 个 mat4 (projection + view)
glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
glBindBuffer(GL_UNIFORM_BUFFER, 0);

// 将我们创建的 UBO 绑定到 Binding Point 0 上！
//是打通“显存物理对象”和“抽象绑定点”的最后一步。它把真实的显存缓冲区（uboMatrices）**挂载（Bind）**到了指定的绑定槽位（通道 0）上，并且让这个缓冲区
//从头到尾（Base 代表从 0 偏移开始）都在向该通道提供数据。
glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboMatrices);

// Step 3: 初始化填充投影矩阵 Projection (假设投影矩阵几乎不变)
glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
// 从 0 偏移处填充 projection 矩阵
glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
glBindBuffer(GL_UNIFORM_BUFFER, 0);

// Step 4: 每帧渲染循环 - 只更新一次 View 矩阵！
glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
// 视图矩阵在第 2 个 mat4 位置，所以偏移量为 sizeof(glm::mat4)
glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
glBindBuffer(GL_UNIFORM_BUFFER, 0);

// 接下来直接切 Shader 绘制即可，再也不用调用 glUniformMatrix4fv("view") 了！
shaderA.use();
drawMeshA();

shaderB.use();
drawMeshB();
```



##### 比较重要得几点:

##### 1.Binding Point 0,**官方术语叫 Uniform Buffer Binding Point）既不存在于 CPU 内存里，也不存在于某一个具体的 Shader（着色器程序）内部，而是存在于 OpenGL 上下文（OpenGL Context，即 GPU 驱动在内存中维护的全局状态机）中。**

OpenGL 从诞生起就是一个巨大的**状态机（State Machine）**。你可以把它想象成一个摆满各种插槽、开关和寄存器的巨型控制台，这个控制台由 GPU 驱动在显卡/系统内存中维护。

在这个控制台里，有一排专门用来插 Uniform 缓冲区的**全局插槽（Binding Points）**：

- 显卡硬件通常会提供至少 **3636 个**这样的全局绑定槽位（在 OpenGL 规范中由 `GL_MAX_UNIFORM_BUFFER_BINDINGS` 保证，通常很多显卡支持 84 个甚至更多，索引从 `0` 到 `N-1`）。
- **通道号 `0`，就是这排全局插槽中的第 0 号插槽。**



##### 2.`glUniformBlockBinding(shaderA.ID, blockIndexA, 0);`

意思就是把shader内部那个uniform块映射到全局插槽（Binding Points）0中，表示从binging point 0 中取数据



##### 3.`glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboMatrices);`

- 这行代码更直接：它把真实的物理显存缓冲区对象 `uboMatrices`，直接**插入到了全局的第 0 号插槽中**。

所以在真实的物理显存缓冲区对象 `uboMatrices`中填充数据，shader根据glUniformBlockBinding之前绑定得关系，从全局的第 0 号插槽中取数据。



##### 4 GLSL 里定义 UBO 时必须加上 **`layout (std140)`**

c++里数据结构内存顺序和gpu数据结构显存顺序完全对不上就会出现问题





## 第二十章.几何着色器

在基础管线中，顶点着色器（VS）是一个“老实的传送带”：**输入 11 个顶点，只能输出 11 个顶点**，它无法创造新顶点，也无法改变几何拓扑。

而**几何着色器 (Geometry Shader, GS)** 位于顶点着色器与片段着色器之间。它的核心能力是：**接收一整组顶点（即一个完整的图元：点、线、三角形），并能凭空创造出全新的顶点/图元，或者直接丢弃它们！**



#### 1.几何管线得位置与数据流向

顶点着色器（vs）-------(图元组装器组装)--------->	几何着色器（gs）	----------（光栅化）--------->片段着色器（fs）



#### 2.几何着色器

```glsl
# version 330 core
// 1. 声明输入的图元类型（必须与 C++ 端 glDrawArrays 的图元匹配）
layout (triangles) in;

// 2. 声明输出的图元类型及最大顶点数量上限
layout (triangle_strip, max_vertices = 3) out;

// 3. 接收从 VS 传进来的接口块数组（因为输入的是一个图元，包含多个顶点！）
in VS_OUT {
    vec2 texCoords;
    vec3 normal;
} vsData[]; // 👈 必须是数组！triangles 对应 size=3，lines 对应 size=2，points 对应 size=1

// 4. 输出给 FS 的变量
out vec2 TexCoords;

void main() {
    // 遍历输入的 3 个顶点，原样发射出去
    for(int i = 0; i < 3; i++) {
        gl_Position = gl_in[i].gl_Position; // 内置变量 gl_in[] 包含了 VS 计算好的 gl_Position
        TexCoords = vsData[i].texCoords;
        EmitVertex(); // 👈 关键字 1：把当前设置好的顶点状态打包发射出去！
    }
    EndPrimitive();   // 👈 关键字 2：结束当前图元（三角形）的组装
}
```



##### 核心函数与内置变量拆解：

1. **`gl_in[]` 数组**：

   - 类型：内置结构体数组 `out gl_PerVertex { vec4 gl_Position; float gl_PointSize; ... } gl_in[];`
   - 含义：存储了上游顶点着色器（VS）写入的所有内置变量。例如 `gl_in[0].gl_Position` 代表输入图元的第 00 个顶点位置。

   ```glsl
   // GLSL 内部自动为你声明了以下结构体（你不需要在代码里重写它）
   in gl_PerVertex {
       vec4  gl_Position;     	// 顶点在裁剪空间下的坐标 (MVP 变换后的坐标)
       float gl_PointSize;    	// 点的大小（只有画点图元时有用）
       float gl_ClipDistance[];// 裁剪距离
       float gl_CullDistance[];// 剔除距离
   } gl_in[]; 					// 👈 注意：这里声明了数组对象 gl_in
   ```

   

2. **`EmitVertex()`**：

   - **函数含义**：告诉 GPU：“我已经把这个新顶点的 `gl_Position` 和各种 `out` 变量设置好了，请把这个顶点推入输出缓冲区。”

3. **`EndPrimitive()`**：

   - **函数含义**：告诉 GPU：“当前这一组顶点已经够组成一个完整图元（如 `triangle_strip`）了，请把它们打成一个包传给光栅化阶段，准备开始组装下一个新图元。”



#### 3.实战应用场景一：法线可视化（Normal Visualization）

在 3D 渲染调试中，我们经常遇到光照计算错误的问题。此时我们需要直观地在屏幕上看到模型的**法线向量**到底指向哪里。

让几何着色器接收模型原本的**三角形**，但在输出时，不仅画出原三角形，还在每个顶点处**凭空生成一条沿着法线方向延伸的小线段**！

vert

```glsl
#version 330 core

layout (location = 0) in vec3 aPos;     // 1. 输入：顶点的位置坐标 (Model Space)
layout (location = 1) in vec3 aNormal;  // 2. 输入：顶点的法线向量 (Model Space)

uniform mat4 model;        // 模型矩阵：将顶点从模型空间转到世界空间
uniform mat3 normalMatrix; // 法线矩阵：用于将法线正确变换到世界空间（避免非等比缩放导致法线变形）

// 定义输出到几何着色器 (GS) 的接口块 (Interface Block)
out VS_OUT {
    vec3 normal;
} vs_out;

void main()
{
    // 将顶点坐标变换到【世界空间】（注意：这里故意没乘 view 和 projection，留给 GS 处理）
    gl_Position = model * vec4(aPos, 1.0);
    
    // 将法线向量转到【世界空间】并归一化，传递给下一个阶段
    vs_out.normal = normalize(normalMatrix * aNormal);
}
```



geom:

```glsl
#version 330 core

layout (triangles) in;                  // 1. 声明输入：一次接收一个三角形（包含 3 个顶点）
layout (line_strip, max_vertices = 6) out; // 2. 声明输出：输出线段带，最多发射 6 个顶点（3 条线段 × 2 个端点）

// 接收从顶点着色器传过来的数据
// 注意：gs_in[] 必须是数组！因为输入是一个三角形，包含了 3 个顶点的属性
in VS_OUT {
    vec3 normal;
} gs_in[];

uniform mat4 view;       // 观察矩阵
uniform mat4 projection; // 投影矩阵

const float MAGNITUDE = 0.4; // 显示法线线段的长度

// 辅助函数：为指定的某个顶点生成一条法线线段
void GenerateLine(int index)
{
    // 获取当前顶点的世界坐标 (对应 VS 输出的 gl_Position)
    vec4 pos = gl_in[index].gl_Position; 
    
    // 获取当前顶点的世界空间法线，并确保归一化
    vec3 normal = normalize(gs_in[index].normal);

    // ---- 发射线段起点 ----
    // 顶点原本的位置，经过 View 和 Projection 矩阵变换到裁剪空间 (NDC)
    gl_Position = projection * view * pos;
    EmitVertex(); // 把这个起点吐给渲染管线

    // ---- 发射线段终点 ----
    // 起点位置 + (法线方向 * 长度)，同样变换到裁剪空间
    gl_Position = projection * view * (pos + vec4(normal * MAGNITUDE, 0.0));
    EmitVertex(); // 把这个终点吐给渲染管线

    // 结束当前线段图元（将刚发射的两个点连成一条线）
    EndPrimitive();
}

void main()
{
    GenerateLine(0); // 为三角形的第 0 个顶点画一条法线
    GenerateLine(1); // 为三角形的第 1 个顶点画一条法线
    GenerateLine(2); // 为三角形的第 2 个顶点画一条法线
}
```



##### 有个疑惑：当前已经有in VS_OUT{}gs_in[];来接收来自vertex shader的数据，那layout (triangles) in;,layout (line_strip, max_vertices = 6) out;是不是没有用？

##### 先弄清楚渲染管线：

##### （1）c++端发起绘制命令，glDrawArrays(GL_TRIANGLES, 0, 36)

##### （2）GPU 启动顶点着色器（VS），layout (location = 0) in vec3 aPos;layout (location = 1) in vec3 aNormal;读取并处理顶点数据

##### （3）图元组装器组装顶点数据， C++ 端传的 `GL_TRIANGLES` 指令，把 VS 处理好的顶点**3 个 3 个拦截打包**，组装成一个个完整的“三角形图元”。

##### （4）几何着色器，组装好的三角形被推进几何着色器（GS）。**`layout (triangles) in;`**是一个**校验与接收声明**，对 GPU 喊道：“我已经准备好了，请按 **`triangles`（三角形，即一次 3 个顶点）** 的规格把打包好的图元喂给我！”

##### `layout (triangles) in;` 的真正作用：

它决定了 GS 内部两个数组的大小（也就是对应前面说的 `gl_in[]` 和 `gs_in[]`）：

- 如果你声明 `layout (points) in;` →→ 代表输入是点，`gl_in` 的数组长度自动变为 **`1`**。
- 如果你声明 `layout (lines) in;` →→ 代表输入是线段，`gl_in` 的数组长度自动变为 **`2`**。
- 如果你声明 `layout (triangles) in;` →→ 代表输入是三角形，`gl_in` 的数组长度自动变为 **`3`**（所以你在代码里才能写 `gl_in[0]`, `gl_in[1]`, `gl_in[2]`）。

**编译约束：** *如果 C++ 端传的是* `GL_TRIANGLES`*，但你的 GS 里写了* `layout (lines) in;`*，OpenGL 会直接抛出驱动错误，因为***接口格式对不上了***！*

##### `layout (line_strip, max_vertices = 6) out;` 的真正作用：

1. **定义输出几何拓扑（`line_strip`）**：告诉光栅化器：“不管我输入的原本是三角形还是点，我接下来用 `EmitVertex()` 吐出来的点，请**按连续线段（Line Strip）**连接并画在屏幕上！”
2. **预分配 GPU 显存（`max_vertices = 6`）**： 因为 GS 是可以动态吐出顶点的，GPU 硬件必须提前知道你最多会吐出几个顶点，好在显存里为你分配硬件寄存器空间。你写 `max_vertices = 6`，GPU 就为你开辟 6 个顶点的寄存器缓冲，多了不给，少了浪费。

##### （5）光栅化处理

##### （6）片段着色器进行渲染





frag:

```glsl
#version 330 core

out vec4 FragColor; // 输出最终像素颜色

void main()
{
    // 将 GS 生成的所有线段都涂上纯黄色 (R=1.0, G=1.0, B=0.0, A=1.0)
    FragColor = vec4(1.0, 1.0, 0.0, 1.0);
}
```



#### 4.实战应用场景二：模型爆炸特效（Explosion）

vert:

```glsl
#version 330 core
layout (location = 0) in vec3 aCoord;   //顶点
layout (location = 1) in vec3 aNormal;  //法线
layout (location = 2) in vec2 aTexCoords; //纹理坐标


uniform mat4 model;
uniform mat3 normalMatrix;  //逆转置法线矩阵

out VS_OUT {
    vec3 normal;    //世界空间下的法线向量
    vec3 fragPos;   //世界空间下的片段坐标
    vec2 TexCoords;
} vs_out;

void main()
{
    //局部空间转为世界空间
    vec4 worldPos = model * vec4(aCoord,1.0);
    vs_out.fragPos = worldPos.xyz;

    //法线
    vs_out.normal = normalize(normalMatrix * aNormal);
    vs_out.TexCoords = aTexCoords;

    // 几何着色器需要在世界空间移动三角形，投影变换由下一阶段完成。
    gl_Position = worldPos;
}

```



geom:

```glsl
#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec3 normal;
    vec3 fragPos;
    vec2 TexCoords;
} gs_in[];

out GS_OUT {
    vec3 normal;
    vec3 fragPos;
    vec2 TexCoords;
} gs_out;

uniform mat4 view;
uniform mat4 projection;
uniform float time;

// 根据三角形三个顶点计算世界空间面法线。
vec3 GetFaceNormal()
{
    vec3 edge1 = gs_in[1].fragPos - gs_in[0].fragPos;
    vec3 edge2 = gs_in[2].fragPos - gs_in[0].fragPos;
    return normalize(cross(edge1, edge2));
}

void main()
{
    vec3 faceNormal = GetFaceNormal();

    // 0～0.6 之间周期变化，使模型循环爆炸并复原。
    float phase = sin(time) * 0.5 + 0.5;
    float distance = phase * phase * 0.6;
    vec3 offset = faceNormal * distance;

    for(int i = 0; i < 3; ++i)
    {
        vec3 explodedPos = gs_in[i].fragPos + offset;

        gs_out.fragPos = explodedPos;
        gs_out.normal = gs_in[i].normal;
        gs_out.TexCoords = gs_in[i].TexCoords;

        gl_Position = projection * view * vec4(explodedPos, 1.0);
        EmitVertex();
    }

    EndPrimitive();
}

```





frag:

```glsl
#version 330 core

out vec4 FragColor;

in GS_OUT {
    vec3 normal;
    vec3 fragPos;
    vec2 TexCoords;
} fs_in;

struct Light {
    vec3 position;  // 光源位置
    vec3 ambient;   // 光源的环境光强度（通常设低一点，如 vec3(0.2)）
    vec3 diffuse;   // 光源的漫反射强度（通常为光源的主色调，如 vec3(0.5)）
    vec3 specular;  // 光源的高光强度（通常设为全强，如 vec3(1.0)）
};
    
struct Material{
    sampler2D diffuse; //漫反射贴图
    sampler2D specular; //高光贴图
    float     shininess;//高光散射半径/粗糙度
};

uniform samplerCube skybox;


//材质颜色
uniform Material material;
//光源颜色
uniform Light light;
//摄像机位置
uniform vec3 cameraPos;

void main()
{
    //物体最终显示颜色 = 光源影响因子 * 物体表面颜色;
    //光源影响因子 = 环境光 + 漫反射 + 高光

    //方向向量
    vec3 N = normalize(fs_in.normal);
    vec3 L = normalize(light.position - fs_in.fragPos); //入射光向量的反向量
    vec3 R = reflect(-L,N);                       //光源入射方向对应的反射光向量
    vec3 V = normalize(cameraPos - fs_in.fragPos);      //从片段指向相机的观察方向

    vec3 diffuseColor = texture(material.diffuse, fs_in.TexCoords).rgb;
    vec3 specularColor = texture(material.specular, fs_in.TexCoords).rgb;

    //环境光
    vec3 ambient = light.ambient * diffuseColor;

    //漫反射
    float diff  = max(dot(N,L),0.0);  //入射光和法线夹角
    vec3 diffuse = light.diffuse * diff * diffuseColor;

    //高光
    float spec = pow(max(dot(V,R),0.0),material.shininess);  //反射光和相机夹角,使用pow进行光线集中收束
    vec3 specular = light.specular * spec * specularColor;

    //物体最终显示颜色
    vec3 objColor = ambient + diffuse + specular;

    FragColor = vec4(objColor.rgb, 1.0);
}
```



#### 5.C++ 端的配置与着色器编译流程

在 C++ 端，使用几何着色器只需要在传统的 `VS + FS` 管线中插入一个 **`GL_GEOMETRY_SHADER`** 阶段：

```glsl
// Step 1: 编译 Geometry Shader
const char* gShaderCode = loadShaderSource("explosion.gs");
unsigned int geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
glShaderSource(geometryShader, 1, &gShaderCode, NULL);
glCompileShader(geometryShader);

// 检查编译错误...
checkCompileErrors(geometryShader, "GEOMETRY");

// Step 2: 链接到 Shader Program
unsigned int shaderProgram = glCreateProgram();
glAttachShader(shaderProgram, vertexShader);
glAttachShader(shaderProgram, geometryShader); // 👈 挂载几何着色器！
glAttachShader(shaderProgram, fragmentShader);
glLinkProgram(shaderProgram);

// Step 3: 渲染循环中直接使用
glUseProgram(shaderProgram);
glUniform1f(glGetUniformLocation(shaderProgram, "time"), glfwGetTime()); // 更新时间变量

glBindVertexArray(modelVAO);
glDrawArrays(GL_TRIANGLES, 0, modelVertexCount); // 正常绘制即可，GS 会在 GPU 内部拦截并爆炸！
```



##### 性能警告与使用禁忌 (Performance Best Practices)

虽然几何着色器非常强大，但在现代游戏引擎（如 UnrealaEngine / Unity）中，**过度使用 GS 可能会导致严重的硬件性能卡顿**：

1. **并行效率较低**：GPU 显卡的架构是极其擅长处理大规模同质化数据（VS/FS）的。几何着色器由于需要动态开辟/变动内存（输出可变数量的顶点），会导致 GPU 的 SIMD（单指令多线程）调度器效率下降。

2. ##### 现代替代方案

   - 如果是为了画成千上万个相同几何体（如草地、粒子），优先使用 **实例化渲染 (Instanced Rendering)**。
   - 在现代 OpenGL (4.x+) 或 Vulkan/DirectX12 中，复杂几何体生成已更多交给 **细分曲面着色器 (Tessellation Shader)** 或 **网格着色器 (Mesh Shader)**。





## 第二十一章.实例化渲染(Instanced Rendering)

#### 1.核心痛点与解决思路

假设你要在场景里渲染一片包含 **$10,000$ 棵树** 的森林（每棵树的模型顶点完全相同，只是在空间中的位置、缩放、旋转不同）。

传统渲染模式：

```c++
for (int i = 0; i < 10000; i++) {
    shader.setMat4("model", modelMatrices[i]); // 切 Uniform，触发 API 开销
    glDrawArrays(GL_TRIANGLES, 0, 3600);        // 发送一次 Draw Call
}
```

GPU 擅长并行处理几十万个顶点，处理 $10,000$ 棵树易如反掌。**CPU 每秒向 GPU 发送 $10,000$ 次渲染命令（Draw Call）** ,调用 Draw Call 本身是有巨大开销的：CPU 需要切换渲染状态、刷新管线上下文、并通过 PCIe 总线通知 GPU。会引发巨大的 API 调用开销与 PCIe 总线阻塞，导致 CPU 直接卡死，帧率暴跌至个位数。



##### 解决办法：实例化渲染（Instanced Rendering）

**核心思想：** **“一次提交，批量绘制”**。

CPU 只需发送 **$1$ 次** 绘制指令，同时将 $10,000$ 个物体的变换数据（如 `mat4` 矩阵数组）通过一个 Buffer 一次性打包塞给 GPU。GPU 在内部并行启动 $10,000$ 个渲染实例，瞬间完成全场绘制！



#### 2.核心 API 与 GLSL 内置变量拆解

##### 1.GLSL 内置变量：`gl_InstanceID`

GLSL 在顶点着色器（VS）中提供了一个全局内置变量 **`gl_InstanceID`**：

- **定义**：`in int gl_InstanceID;`
- **含义**：代表当前正在绘制的**实例索引编号**（从 `0` 自动递增到 `InstanceCount - 1`）。

```glsl
#version 330 core
layout (location = 0) in vec2 aPos;

uniform vec2 offsets[100]; // 从 C++ 传入 100 个平移偏移量

void main() {
    // 利用 gl_InstanceID 索引当前实例对应的偏移量
    vec2 offset = offsets[gl_InstanceID];
    gl_Position = vec4(aPos + offset, 0.0, 1.0);
}
```



#### 2.规模化方案：实例化顶点属性（Instanced Arrays）与 `glVertexAttribDivisor`

当绘制数量达到 **$10,000$ ~ $100,000$** 时，Uniform 内存容量会超出上限（`GL_MAX_UNIFORM_BLOCK_SIZE` 限制）。

此时我们需要像配置普通的顶点属性（如位置、法线）一样，把 $10,000$ 个物体的变换矩阵（`mat4 model`）塞进一个普通的 VBO（称为 Instance VBO）中。

这里就需要用到核心 API：**`glVertexAttribDivisor`**。

```c++
void glVertexAttribDivisor(GLuint index, GLuint divisor);
```

##### 参数详解与更新机制：

- **`index`**：指定的顶点属性 Location 编号（对应 GLSL 中的 `layout (location = N)`）。
- `divisor`（属性更新频率 / 除数）
  - **`0`（默认值）**：**每处理 1 个顶点**，该属性更新一次。（普通顶点属性的规则）
  - **`1`**：**每处理 1 个实例（Instance）**，该属性才更新一次！（实例化属性的核心规则）
  - **`2`**：每处理 2 个实例才更新一次，以此类推。



#### 3.动态实例和GPU粒子系统

实例化渲染真正的威力，完全体现在**海量动态数据的实时处理**上！比如游戏里的**雪花暴风雪、枪口火花、魔法粒子系统**，或者**随风飘动的成千上万棵草**。

下面为你提供一套**高度实用、逻辑完整且极其震撼**的实战案例：**基于 GPU 实例化的动态粒子系统 (GPU Instanced Particle System)**！

在这个例子中：

1. **CPU 负责物理逻辑**：每一帧更新 $10,000$ 个粒子的位置、旋转和 Alpha 透明度衰减。
2. **GPU 显存局部更新**：使用我们前面第一章学的 **`glBufferSubData`** 零开销更新 Instance VBO。
3. **GPU 实例化渲染**：一次 Draw Call 渲染出 $10,000$ 个朝向相机的动态粒子！



##### 粒子结构体与数据内存布局设计

```c++
#include <glm/glm.hpp>
#include <vector>

// 1. 单个粒子的 CPU 端物理状态
struct Particle {
    glm::vec3 position; // 当前位置
    glm::vec3 velocity; // 速度向量
    glm::vec4 color;    // 颜色与 Alpha 透明度 (r, g, b, alpha)
    float size;         // 缩放大小
    float life;         // 剩余生命周期 (1.0 代表刚出生，0.0 代表死亡)
};

// 2. 传给 GPU Instance VBO 的“实例化顶点属性”（只存渲染相关的必要数据，省显存！）
struct ParticleInstanceData {
    glm::vec3 position; // 对应 Location 2
    glm::vec4 color;    // 对应 Location 3
    float scale;        // 对应 Location 4
};
```



##### 顶点着色器:

在顶点着色器中，我们接收 **Location 0, 1 的静态粒子网格数据**，以及 **Location 2, 3, 4 的动态实例化数据**：

```glsl
#version 330 core
// ---- 静态属性：所有粒子共享同一个四边形 (Quad) 网格 ----
layout (location = 0) in vec3 aQuadVertex; // 面板 4 个顶点的局部坐标 (-0.5 ~ 0.5)
layout (location = 1) in vec2 aTexCoords;  // 纹理坐标

// ---- 动态实例化属性：每个粒子独一无二的数据 (每 1 个实例更新一次) ----
layout (location = 2) in vec3 aInstancePos;   // 粒子当前世界坐标
layout (location = 3) in vec4 aInstanceColor; // 粒子当前颜色与 Alpha
layout (location = 4) in float aInstanceScale; // 粒子当前大小

out vec2 TexCoords;
out vec4 ParticleColor;

uniform mat4 projection;
uniform mat4 view;

void main() {
    TexCoords = aTexCoords;
    ParticleColor = aInstanceColor;

    // ---- 广告牌 (Billboarding) 技巧：让粒子永远面向摄像机 ----
    // 从 View 矩阵中提取摄像机的右向量 (Right) 和上向量 (Up)
    vec3 CameraRight_worldspace = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 CameraUp_worldspace    = vec3(view[0][1], view[1][1], view[2][1]);

    // 根据粒子位置、尺寸以及摄像机方向，动态计算顶点位置
    vec3 vertexWorldPos = aInstancePos
        + CameraRight_worldspace * aQuadVertex.x * aInstanceScale
        + CameraUp_worldspace    * aQuadVertex.y * aInstanceScale;

    gl_Position = projection * view * vec4(vertexWorldPos, 1.0);
}
```



##### 每个粒子就是一个小平面，如果摄像机移动，粒子就会消失，所以需要广告牌，无论摄像机怎么转，这个小平面永远正对摄像机。

**广告牌就是一个永远面向摄像机（Camera）的 2D 平面（Quad/矩形）**，广告牌的画面永远正对着你的眼睛**。**在 3D 渲染中，**绘制真实的 3D 模型极其昂贵**，而**绘制一个 2D 平面（只有 2 个三角形、4 个顶点）极其便宜**！

##### 广告牌（Billboard）顶点世界坐标推导公式。



##### 三维坐标系（X, Y, Z）的默认绑定顺序

计算机中，我们要用一个 3×3 的旋转矩阵来表示摄像机的姿态，那么：

- **矩阵的第 0 行/列**，自然对应的就是 X轴（Right 向量）；
- **矩阵的第 1 行/列**，对应的就是 Y轴（Up 向量）；
- **矩阵的第 2 行/列**，对应的就是 Z轴（Direction 向量）。



CameraRight_worldspace * aQuadVertex.x * aInstanceScale

CameraUp_worldspace    * aQuadVertex.y * aInstanceScale

CameraRight_worldspace ，CameraUp_worldspace    都是单位向量，

aQuadVertex 粒子平面的顶点坐标

CameraRight_worldspace * aQuadVertex.x * aInstanceScale，摄像机右方向的偏移

CameraUp_worldspace    * aQuadVertex.y * aInstanceScale 摄像机上方向的偏移

`vertexWorldPos` 计算出来的是：**当前广告牌（粒子）的 4 个顶点，在 3D 世界空间（World Space）中的绝对坐标 (X,Y,Z)(\*X\*,\*Y\*,\*Z\*)**

- **输入：** 粒子的中心位置 `aInstancePos`、广告牌 2D 网格局部坐标 `aQuadVertex`、摄像机的两个眼睛方向。
- **输出：** 4 个顶点在 3D 世界里**经过缩放、且已经对齐摄像机视角后**的绝对 3D 物理位置。



片段着色器

片段着色器负责渲染柔和的粒子贴图，并结合生命周期带来的 Alpha 渐变：

```glsl
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec4 ParticleColor;

void main() {
    // 用纹理坐标画一个软边圆：中心不透明，边缘平滑淡出，模拟雪花的柔边。
    float dist = length(TexCoords - vec2(0.5)); //粒子方块的纹理中心坐标是(0,5,0.5)，计算的是当前像素到中心点之间的欧氏距离
    float alpha = 1.0 - smoothstep(0.0, 0.5, dist);
    alpha *= ParticleColor.a;

    // 几乎透明的片段直接丢弃，避免无效写入。
    if (alpha < 0.01)
        discard;

    FragColor = vec4(ParticleColor.rgb, alpha);
}
```



float dist = length(TexCoords - vec2(0.5)); //粒子方块的纹理中心坐标是(0,5,0.5)，计算的是当前像素到中心点之间的欧氏距离



float alpha = 1.0 - smoothstep(0.0, 0.5, dist);**计算当前像素距离中心的远近，离中心越近透明度越高（实心亮斑），离边缘越近透明度越低（平滑羽化消失）**

##### smoothstep：是 GLSL 的内置平滑阶跃函数。它的标准语法是： `smoothstep(下限, 上限, 当前值)`

- **当当前值小于等于下限（`0.0`）时：** 函数返回 `0.0`。
- **当当前值大于等于上限（`0.5`）时：** 函数返回 `1.0`。
- **当当前值在下限和上限之间（`0.0` 到 `0.5` 之间）时：** 函数会返回一个 **`0.0` 到 `1.0` 之间平滑渐变的 S 型曲线数值**。



c++端

**我们不需要重新创建 VBO，而是使用 `glBufferSubData` 在原显存位置上局部重写！**

```c++
// 雪花着色器：顶点着色器负责广告牌，片元着色器画出软边圆。
Shader particleShader(SHADER_DIR "/particleSystemShader.vert", SHADER_DIR "/particleSystemShader.frag");

// 单个广告牌四边形：4 个顶点，用三角形带拼成两个三角形。
// 每顶点依次：局部坐标(-0.5~0.5) + 纹理坐标(0~1)
float quadVertices[] = {
    // 位置              纹理坐标
    -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
    0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
    0.5f, -0.5f, 0.0f,  1.0f, 0.0f
};

GLuint quadVAO, quadVBO;
glGenVertexArrays(1, &quadVAO);
glGenBuffers(1, &quadVBO);
glBindVertexArray(quadVAO);
glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
// location 0：四边形局部坐标
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);
// location 1：纹理坐标
glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
glEnableVertexAttribArray(1);

// 实例数据缓冲：每帧用 CPU 更新，上传所有雪花的位置/颜色/大小。
const unsigned int PARTICLE_COUNT = 1000;
GLuint instanceVBO;
glGenBuffers(1, &instanceVBO);
glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
glBufferData(GL_ARRAY_BUFFER, PARTICLE_COUNT * 8 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
// location 2：实例位置
glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
glEnableVertexAttribArray(2);
glVertexAttribDivisor(2, 1);// 👈 告知 GPU：每绘制完 1 颗雪花，位置更新一次！
// location 3：实例颜色
glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
glEnableVertexAttribArray(3);
glVertexAttribDivisor(3, 1);// 👈 告知 GPU：每绘制完 1 颗雪花，颜色更新一次！
// location 4：实例大小
glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(7 * sizeof(float)));
glEnableVertexAttribArray(4);
glVertexAttribDivisor(4, 1);// 👈 告知 GPU：每绘制完 1 颗雪花，缩放大小更新一次！
glBindVertexArray(0);// 解绑 VAO 保护数据

// 初始化雪花粒子。
struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec4 color;
    float scale;
};
std::vector<Particle> particles(PARTICLE_COUNT);

std::mt19937 rng(12345);
std::uniform_real_distribution<float> distX(-15.0f, 15.0f);
std::uniform_real_distribution<float> distY(-10.0f, 10.0f);
std::uniform_real_distribution<float> distZ(-15.0f, 15.0f);
std::uniform_real_distribution<float> distFall(0.5f, 1.5f);    // 下落速度
std::uniform_real_distribution<float> distDrift(-0.3f, 0.3f);  // 水平漂移速度
std::uniform_real_distribution<float> distScale(0.08f, 0.35f); // 雪花大小
std::uniform_real_distribution<float> distAlpha(0.2f, 0.7f);   // 透明度

for (auto& p : particles) {
    p.position = glm::vec3(distX(rng), distY(rng), distZ(rng));
    p.velocity = glm::vec3(distDrift(rng), -distFall(rng), distDrift(rng));
    p.color = glm::vec4(1.0f, 1.0f, 1.0f, distAlpha(rng));
    p.scale = distScale(rng);
}

// 雪花半透明：加法混合叠加增亮，且与绘制顺序无关，不需要排序。
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE);

// 每帧要上传的实例数据：位置(3) + 颜色(4) + 大小(1) = 8 个 float。
std::vector<float> instanceData(PARTICLE_COUNT * 8);
```



**`glVertexAttribDivisor(index, divisor)` 是实例化渲染（Instanced Rendering）的“步进频率控制器”。**

它的作用是告诉 GPU 硬件：**“当我在绘制海量重复物体时，这个顶点属性（Attribute）应该每隔多少个实例（Instance）才在 VBO 里往前跳一步（更新一次数据）？”**

**`divisor = 0`（默认值）**：**按顶点步进**。GPU 每处理 1 个顶点，该 Location 对应的属性指针就向后移动一次。（用于 `quadVertices` 这种网格自身的几何数据）。

**`divisor = 1`**：**按实例步进**。GPU 处理完一个**完整物体的所有顶点**（比如整张四边形的 4 个顶点）后，该 Location 对应的属性指针才向后移动一次！

**`divisor = N`**：每绘制完 $N$ 个完整实例，指针才移动一次。





while循环

```c++
while(!glfwWindowShouldClose(window))
{
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window);

    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // 更新雪花：下落 + 漂移 + 水平摆动；掉出底部后从顶部重新出现。
    for (unsigned int i = 0; i < PARTICLE_COUNT; ++i) {
        Particle& p = particles[i];
        p.position += p.velocity * deltaTime;
        p.position.x += std::sin(currentFrame * 0.8f + i * 0.7f) * 0.01f;
        p.position.z += std::cos(currentFrame * 0.6f + i * 0.9f) * 0.01f;

        if (p.position.y < -10.0f) {
            p.position.y = 10.0f;
            p.position.x = distX(rng);
            p.position.z = distZ(rng);
        }

        float* dst = &instanceData[i * 8];
        dst[0] = p.position.x;
        dst[1] = p.position.y;
        dst[2] = p.position.z;
        dst[3] = p.color.r;
        dst[4] = p.color.g;
        dst[5] = p.color.b;
        dst[6] = p.color.a;
        dst[7] = p.scale;
    }

    // 上传实例数据。
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(instanceData.size() * sizeof(float)), instanceData.data());

    glm::mat4 view = camera.GetViewMatrix();
    float aspect = framebufferHeight > 0
        ? static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight)
        : 1.0f;
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspect, 0.1f, 100.0f);

    particleShader.use();
    particleShader.setMat4("view", view);
    particleShader.setMat4("projection", projection);

    glBindVertexArray(quadVAO);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, PARTICLE_COUNT);
    glBindVertexArray(0);

    glfwSwapBuffers(window);
    glfwPollEvents();
}
```

梳理下每个雪花的绘制流程：

glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1000);执行后，GPU 的硬件调度器（Rasterizer & Thread Scheduler）就会立刻启动。

为了渲染这 1000 颗雪花（每颗雪花由 2 个三角形、6 个顶点组成），GPU 实际上需要并行运行 **$6000$ 次顶点着色器（Vertex Shader）**。



##### 1.硬件索引计算公式

在顶点着色器内部，有两个硬件自动维持的计数器：

1. **`gl_VertexID`**：当前绘制的是当前实例的第几个顶点（取值范围：`0` 到 `5`）。
2. **`gl_InstanceID`**：当前绘制的是第几颗雪花（取值范围：`0` 到 `999`）。

GPU 的硬件顶点拉取器（Vertex Fetcher）在给 Shader 变量赋值时，使用的是如下通用计算公式：

$$\text{数据内存偏移} = \text{起始偏移 (Offset)} + \text{步长 (Stride)} \times \text{当前索引}$$

其中：

- 如果 `Divisor == 0`：$\text{当前索引} = \text{gl\_VertexID}$（按顶点切换）
- 如果 `Divisor == 1`：$\text{当前索引} = \text{gl\_InstanceID}$（按雪花实例切换）



##### 2.5 个属性的实时赋值推演表

假设现在 GPU 正在并行处理**第 42 颗雪花（`gl_InstanceID = 42`）** 的**第 2 个顶点（`gl_VertexID = 2`）**，5 个属性的值是如何被精准送入 Shader 的：

| **属性名称 (Location)** | **数据来源**  | **Divisor** | **GPU 算出的内存索引** | **最终送入 Shader 的值**                        |
| ----------------------- | ------------- | ----------- | ---------------------- | ----------------------------------------------- |
| **0: `aQuadVertex`**    | `quadVBO`     | **`0`**     | `gl_VertexID` ($2$)    | 取出 `quadVBO` 中的**第 2 个顶点坐标**          |
| **1: `aTexCoords`**     | `quadVBO`     | **`0`**     | `gl_VertexID` ($2$)    | 取出 `quadVBO` 中的**第 2 个纹理坐标**          |
| **2: `aInstancePos`**   | `instanceVBO` | **`1`**     | `gl_InstanceID` ($42$) | 取出 `instanceVBO` 中**第 42 颗雪花的世界坐标** |
| **3: `aInstanceColor`** | `instanceVBO` | **`1`**     | `gl_InstanceID` ($42$) | 取出 `instanceVBO` 中**第 42 颗雪花的颜色**     |
| **4: `aInstanceScale`** | `instanceVBO` | **`1`**     | `gl_InstanceID` ($42$) | 取出 `instanceVBO` 中**第 42 颗雪花的大小**     |



无论处理这第 42 颗雪花的哪一个顶点（`gl_VertexID` 是 $0, 1, 2, 3, 4, 5$ 中的哪一个）：

- **Location 0 和 1** 会随着 `gl_VertexID` 的改变，不断切换四边形 4 个角落的局部坐标和纹理贴图坐标。
- **Location 2、3 和 4** 会被**锁死在第 42 份内存区域上保持不变**，确保这 6 个顶点都共享这一颗雪花的位置、颜色和大小！



```
CPU 发起指令: glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1000)
                              │
                              ▼
        ┌─────────────────────────────────────────┐
        │  GPU 硬件循环：InstanceID 0 到 999        │
        └─────────────────────┬───────────────────┘
                              │
           ┌──────────────────┴──────────────────┐
           │ （以第 42 颗雪花为例：gl_InstanceID = 42） │
           └──────────────────┬──────────────────┘
                              │
        ┌─────────────────────┴─────────────────────┐
        │  GPU 内部并发派发 6 个 Vertex Shader 线程    │
        │  (分别对应 gl_VertexID = 0, 1, 2, 3, 4, 5) │
        └─────────────────────┬─────────────────────┘
                              │
          ┌───────────────────┼───────────────────┐
          │                   │                   │
  VertexThread 0      VertexThread 1     ... VertexThread 5
(gl_VertexID = 0)   (gl_VertexID = 1)      (gl_VertexID = 5)
  ┌───────────┐       ┌───────────┐          ┌───────────┐
  │Loc 0:点 0 │       │Loc 0:点 1 │          │Loc 0:点 5 │ <-- 随 VertexID 变
  │Loc 1:UV 0 │       │Loc 1:UV 1 │          │Loc 1:UV 5 │ <-- 随 VertexID 变
  │Loc 2:Pos42│       │Loc 2:Pos42│          │Loc 2:Pos42│ <-- 锁定 Instance 42
  │Loc 3:Col42│       │Loc 3:Col42│          │Loc 3:Col42│ <-- 锁定 Instance 42
  │Loc 4:Scl42│       │Loc 4:Scl42│          │Loc 4:Scl42│ <-- 锁定 Instance 42
  └─────┬─────┘       └─────┬─────┘          └─────┬─────┘
        │                   │                      │
        └───────────────────┼──────────────────────┘
                            ▼
               [ 输出 6 个变换后的顶点坐标 ]
                            │
                            ▼
               [ 光栅化为三角形，送入 FS 涂色 ]
                            │
                            ▼
              第 42 颗雪花成功绘制到屏幕上！
```

**`Divisor = 0` 的属性**（四边形形状）：就像印章的**图案刻痕**，印 1000 次，每次盖印章（绘制一个 Instance）时印章本身的那 6 个点图案是不变的，但在盖印章的内部（画这 6 个点时）需要按点切换。

**`Divisor = 1` 的属性**（雪花属性）：就像盖印章时**印在纸上的不同位置和颜色**，盖第 1 个印章用红色在左上角，盖第 2 个印章用蓝色在右下角。印章盖完一次，位置和颜色才换下一个。



##### 注意：

在 GPU 开始画图之前，所有的属性数据就已经同时准备好了。并不是画完 6 个顶点之后才去赋值实例属性，而是在画每一个顶点时，GPU 就已经同时把这 5 个属性全部传给了 Shader！

**错误理解**：先取点0坐标 $\to$ 画点0 $\to$ 取点1坐标 $\to$ 画点1 $\to \dots \to$ 画完 6 个点 $\to$ **才去读取**实例的位置/颜色/缩放。

**真实流程**：在画点 0 的那一瞬间，**顶点坐标、纹理坐标、实例位置、实例颜色、实例缩放这 5 个属性是同时被送进 Shader 的**！



##### 真实的硬件执行时序（以第 42 颗雪花为例）

##### 1. 确定当前上下文 (Context)

GPU 准备开始画第 42 颗雪花（`gl_InstanceID = 42`）。

此时，硬件指针会**直接定位并锁定**在 `instanceVBO` 的第 42 份数据上：

- **`aInstancePos`** = `instanceVBO[42]` 的位置
- **`aInstanceColor`** = `instanceVBO[42]` 的颜色
- **`aInstanceScale`** = `instanceVBO[42]` 的缩放

这3个实例属性在画这颗雪花的整个过程中**保持静止不变**。

##### 2. 处理这颗雪花的 6 个顶点（5 个属性同时送入）

- ##### **处理第 0 个顶点**：

  GPU 提取 `quadVBO[0]` 的坐标和纹理，**同时组合**上面锁定的第 42 份实例数据，打包送入 Vertex Shader：

  - `aQuadVertex` $\leftarrow$ `quadVBO[0].pos` (切变)
  - `aTexCoords` $\leftarrow$ `quadVBO[0].uv` (切变)
  - `aInstancePos` $\leftarrow$ `instanceVBO[42].pos` (不变)
  - `aInstanceColor` $\leftarrow$ `instanceVBO[42].color` (不变)
  - `aInstanceScale` $\leftarrow$ `instanceVBO[42].scale` (不变)
  - **执行 Shader 代码，算完点 0 坐标。**

- **处理第 1 个顶点**：

  - `aQuadVertex` $\leftarrow$ `quadVBO[1].pos` (切变)
  - `aTexCoords` $\leftarrow$ `quadVBO[1].uv` (切变)
  - `aInstancePos` $\leftarrow$ `instanceVBO[42].pos` (不变)
  - `aInstanceColor` $\leftarrow$ `instanceVBO[42].color` (不变)
  - `aInstanceScale` $\leftarrow$ `instanceVBO[42].scale` (不变)
  - **执行 Shader 代码，算完点 1 坐标。**

- **……依此类推，直到处理完第 5 个顶点。**

##### 3. 切换到下一颗雪花

6 个顶点全部计算完毕后，GPU 的 `gl_InstanceID` 变成 `43`。

此时，硬件指针才会滑动到 `instanceVBO[43]`，开始绘制第 43 颗雪花。







## 第二十二章.抗锯齿 (Anti-Aliasing) 与多重采样 (MSAA)

我们在屏幕上渲染 3D 几何体时，屏幕是由一个个正方形的**像素（Pixel）**组成的。 当一个三角形的斜边只覆盖了某个像素的一半时，光栅化器（Rasterizer）必须做一个艰难的决定：**这个像素要么全涂上颜色，要么全不涂（变成背景色）。** 这导致斜边边缘呈现出明显的、粗糙的阶梯状方块——这就是臭名昭著的 **走样（Aliasing，俗称“狗牙”）**。

#### 1.传统抗锯齿方案

1. 超采样抗锯齿 (SSAA, Super-Sampling Anti-Aliasing)：
   - **做法：** 把整个屏幕的分辨率开到 4 倍（比如 4K4*K* 屏幕），渲染完后再强行缩小回 1080P1080*P*。
   - **缺点：** **极其奢侈、暴力！** 显卡需要多承担 4 倍的像素着色（Fragment Shader）计算量，帧率直接暴跌。
2. 后处理抗锯齿 (FXAA / SMAA)：
   - **做法：** 纯靠 2D 图像处理算法去“识别”屏幕上的锯齿边缘并做模糊平滑。
   - **缺点：** 无法真正增加几何精度，常常把画面边缘弄得过于模糊（Blurry）。



#### 2.MSAA（多重采样抗锯齿）

**MSAA（Multi-Sample Anti-Aliasing）** 是现代 GPU 硬件级别支持的抗锯齿方案。它的核心哲学是：**“我们不对每个像素做 4 次昂贵的片元着色，而是对几何边缘做 4 次精细的深度/模板/覆盖测试！”**

##### 1.MSAA 的底层数据结构

假设开启了 **4×MSAA**：

- 每一个像素内部，不再只存 **1 个颜色值和 1 个深度值**。

- GPU 在该像素内部开辟了 **4 个独立的子采样点（Sample Points）**。

- 每一个子采样点都有自己的：

  - **几何覆盖标记**（这个子点被三角形盖住了吗？）
  - **深度值 / 模板值**

  

##### 2. MSAA 的硬件工作流程：

1. **顶点与光栅化阶段：** 当三角形边缘划过一个像素时，GPU 不再看像素中心，而是**精确检查这 4 个子采样点中有几个被三角形覆盖了**（比如 4 个点里有 2 个被盖住，覆盖率就是 50%50%）。

2. **片元着色阶段（极其省性能的关键点！）：** **不管这 4 个子点里几个被覆盖，片元着色器（FS）在这个像素上【只执行一次】！** 算出来的颜色被同时分发给这几个被覆盖的子采样点。

3. ##### 颜色解析阶段（Resolve）：

   在渲染到屏幕的最后一步，GPU 把这 4 个子点的颜色

   做平均加权混合（例如 50%50% 三角形颜色 + 50%50% 背景色）

   - 如果像素全在三角形内部：4 个子点全命中 →→ 纯三角形颜色。
   - 如果像素在斜边边缘：只有 2 个子点命中 →→ 颜色被半透明融合，**阶梯状的“狗牙”瞬间被平滑过渡！**



#### 3.现代opengl中的MSAA

在 OpenGL 中，MSAA 不能用普通的默认帧缓冲（Default Framebuffer），必须通过**多重采样帧缓冲（Multisample FBO）**来配置。

整个配置流程分为 4 步：

##### 1: 在 GLFW 窗口创建时请求多重采样抗锯齿

```c++
glfwWindowHint(GLFW_SAMPLES, 4); // 👈 请求 4x MSAA
GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
```

##### 2.开启 OpenGL 硬件多重采样开关

在渲染主循环开始前，手动开启 MSAA 硬件开关：

```c++
glEnable(GL_MULTISAMPLE); // 默认其实是开启的，但显式写出来更安全
```

##### 3.创建多重采样帧缓冲（Multisampled FBO）

如果我们要对画面做后期处理（比如加滤镜），必须创建一个带有抗锯齿纹理附件的 FBO：

```c++
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
glStorageRenderbufferMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, 800, 600);
glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboMulti);

if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
glBindFramebuffer(GL_FRAMEBUFFER, 0);
```

##### 4: 关键的“Blit（内存块拷贝/降采样）”操作

因为多重采样纹理（`GL_TEXTURE_2D_MULTISAMPLE`）无法直接贴到普通的 2D 屏幕或后期处理 Quad 上，你必须把多重采样的 FBO **“Blit（降采样拷贝）”** 到一个普通的标准 FBO 里：

```c++
// 1. 绑定我们的多重采样 FBO，正常绘制 3D 场景
glBindFramebuffer(GL_FRAMEBUFFER, multisampleFBO);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
drawScene();

// 2. 绑定目标普通 FBO (或者默认屏幕 0)
glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampleFBO); // 读源
glBindFramebuffer(GL_DRAW_FRAMEBUFFER, normalFBO);       // 写目标

// 3. 核心函数：glBlitFramebuffer —— 硬件自动把 4x MSAA 降采样融合成普通 2D 图像！
glBlitFramebuffer(0, 0, 800, 600, 0, 0, 800, 600, GL_COLOR_BUFFER_BIT, GL_NEAREST);

glBindFramebuffer(GL_FRAMEBUFFER, 0);
```

#### 4.高级拓展：在 GLSL 中直接采样多重采样纹理

如果你不想用 `glBlitFramebuffer`，而是想在 Shader 里**手动控制**每个采样点（例如实现自定义的图像边缘检测，或者复杂抗锯齿混合），GLSL 提供了专门的采样器：

```glsl
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2DMS screenTextureMS; // 👈 专门的多重采样采样器
uniform int sampleCount = 4;

void main() {
    ivec2 texCoord = ivec2(textureSize(screenTextureMS) * TexCoords);
    vec4 color = vec4(0.0);

    // 手动遍历并叠加该像素下的所有子采样点 (Sub-samples)
    for(int i = 0; i < sampleCount; i++) {
        color += texelFetch(screenTextureMS, texCoord, i); // 👈 获取第 i 个采样点的数据
    }

    // 求平均值
    FragColor = color / float(sampleCount);
}
```



#### 5.MSAA 核心总结

1. ##### **高性价比：** MSAA 只对**几何边缘的子采样点**进行多次覆盖测试，而片元着色器（FS）依然只运行一次，完美避开了 SSAA 那种成倍暴涨的性能灾难。

2. **硬件级支持：** 现代 GPU 架构原生支持多重采样缓冲区（`GL_TEXTURE_2D_MULTISAMPLE`），速度极快。

3. **管线限制：** 多重采样纹理不能直接采样，必须通过 `glBlitFramebuffer` 降采样（Resolve）后才能交给后期处理着色器。





## 第二十三章.Gamma校正

前面的基础光照章节，写过冯氏光照（Phong Lighting），计算过衰减公式，但是渲染出来的灯光效果往往看起来怪怪的：

- 光源的衰减边界非常生硬，像一个突兀的光圈。
- 暗部细节全黑成一团，亮部瞬间过曝。

**原因并不是你的物理公式写错了，而是因为你忽略了“显示器”和“人眼”对光强的非线性反应！**



#### 1.物理现实

如果现实世界里的光子数量翻倍（比如开两盏灯），人眼感受到的亮度并不是“翻倍”，而是觉得“稍微亮了一点点”。人眼对亮度的感知呈**对数曲线（Logarithmic）**。

在上世纪的显像管（CRT）时代，电子枪打在荧光屏上的物理电压和最终发出的亮度，**并不是线性关系，而是呈指数约 2.22.2 的幂函数关系**。

也就是说，如果你给显示器输入 `0.5` 的电压（本该是 50% 的亮度），由于物理特性，屏幕实际表现出来的亮度只有大约 `0.22`（0.52.2≈0.2170.52.2≈0.217）。画面整体变得极其昏暗！

为了抵消显示器的这种暗淡效应，早期的艺术家和操作系统在制作和保存图片（如 JPG/PNG 纹理）时，**在输出端故意把亮度做了一次“压暗（幂次 1/2.2≈0.451/2.2≈0.45）”的预补偿**。

- 当这张被压暗的图片送到 CRT 显示器上时，显示器刚好用 2.22.2 次幂把它“放大”回来，最终在人眼里呈现出正常的线性视觉效果。
- 这个标准的值大约是 **γ=2.2\*γ\*=2.2**。



**计算机在计算 3D 光照数学公式时，必须在纯粹的【物理线性空间（Linear Space）】中进行！**

但你的程序却引入了两个混乱的来源：

1. **美术贴图（如漫反射贴图）：** 美术人员用相机拍的、或者在 Photoshop 里画的 JPG 贴图，**全部都是已经被做过 γ=2.2\*γ\*=2.2 预补偿的非线性数据**。
2. **光照数学计算：** 你的顶点着色器和片段着色器里的加减乘除、向量点积、光照衰减，**是在绝对线性的物理世界里计算的**。

**灾难发生了：** 你把一个非线性编码的贴图颜色（比如 `0.5`，实际上在物理世界它只代表 `0.217` 的光强），直接放进线性光照公式里去乘以光源衰减。**你用错误的数据，算出了错误的物理结果！** 这就是为什么暗部死黑、光照衰减生硬的根本原因。



#### 2.解决方案

想要得到电影级、真实物理级别的光照，现代渲染管线必须遵循以下铁律：



##### 输入端：把采样出的颜色转换回线性空间

当你在片段着色器（FS）中采样一张漫反射贴图时，必须手动把它转回线性的物理世界（即做一次 γ=2.2*γ*=2.2 的幂运算）：

```c++
// 手动做 Gamma 校正的逆运算（转回线性空间）
vec3 diffuseColor = pow(texture(diffuseTexture, TexCoords).rgb, vec3(2.2));
```



##### 中间段：在绝对线性空间中计算光照

所有的光照计算、冯氏模型、阴影、混合，都在这个线性空间里稳稳计算，保证物理结果绝对精确。



##### 输出端：在最后一步做 Gamma 编码 (Gamma Correction)

在片段着色器的最后，把最终算出来的线性光照颜色，**重新做一次 γ=1/2.2≈0.45\*γ\*=1/2.2≈0.45 的幂运算（即 Gamma 校正）**，然后再输出给屏幕显示：

```c++
// 片段着色器的最后一步：Gamma 校正
vec3 finalColor = pow(lightingResult, vec3(1.0 / 2.2));
FragColor = vec4(finalColor, 1.0);
```





## 第二十四章.阴影映射(Shadow Mapping)

在这之前，我们的 3D 渲染里虽然有耀眼的光源和精美的材质，但你会发现一个极其诡异的物理现象：**物体和地面之间没有任何阴影！** 悬空的箱子仿佛漂浮在空中，没有任何立体感和真实感。

而 **Shadow Mapping（阴影映射）**，就是现代 3D 游戏（从《马里奥》到《赛博朋克 2077》）用来解决这个问题的标准工业级技术。



#### 1.阴影

在物理世界中，阴影的本质其实非常简单：

> **“从光源的视角看过去，所有能被光源直接看到的物体表面都是亮的（被照亮）；而那些被挡在后面的物体表面，就是光线照不到的阴影区。”**

基于这个伟大的物理直觉，现代显卡发明了 **Shadow Mapping（阴影映射技术）**。它巧妙地将阴影计算拆成了 **两步走** 的策略。



#### 2.核心架构：两步绘制法（The Two-Pass Algorithm）

为了在屏幕上画出阴影，显卡（GPU）在每一帧里必须**把整个场景画两遍（Two Passes）**：



##### 第一步（Pass 1）：光源当相机，拍一张“深度照片”

1. **摆放相机：** 把摄像机强行挪到光源的位置（比如太阳的位置）。
2. **只看距离，不看颜色：** 从光源视角看过去，我们不需要知道物体是红的还是绿的，**我们只关心“每个像素距离光源有多远”**。
3. **记录存盘：** 显卡把这些距离（深度值 Depth）存进一张黑白纹理图片里。这张图片就叫 **深度贴图（Depth Map / Shadow Map）**。

> **这张照片的含义：** 照片里记录的每一个像素值，代表了**从光源出发，沿着这个方向能撞到的“第一个障碍物”的最近距离（Closest Depth）**。



##### 第二步（Pass 2）：玩家当相机，正式画画与“查账”

把摄像机切回玩家眼睛的位置，正常给场景上色。

当 GPU 准备画地面上的某个像素点 P时，它在后台悄悄做了一场**“跨时空查账”**：

1. **坐标转换：** 把像素 P的 3D 世界坐标，投射回 Pass 1 那张“光源深度照片”的对应位置上。
2. 获取两个深度值：
   - **真实距离 (Dcurrent\*D\*current)：** 点 P此时此刻距离光源的真实物理距离。
   - **照片距离 (Dclosest\*D\*closest)：** 查一下 Pass 1 照片里，在这个方向上记录的最近距离。
3. 灵魂对比：
   - 如果 Dcurrent≈Dclosest*D*current≈*D*closest：说明点 P*P* 就是当年光源撞到的那个第一个物体，光线直接照到了它 →→ **它是亮部！**
   - 如果 Dcurrent>Dclosest*D*current>*D*closest：说明在点 P*P* 和光源之间，**早就有一个更近的物体把光线挡住了！** →→ **点 P\*P\* 陷落于阴影中！**



#### 3.代码:从 0 到 1 构建阴影管线

##### 1. 配置 Pass 1 的离屏“深度容器”（Depth Framebuffer）

我们在 C++ 端配置一个专门用来存深度的帧缓冲（FBO），注意：**我们只需要深度附件（Depth Attachment），不需要颜色附件！**

```c++
// 1. 创建深度帧缓冲 (FBO)
unsigned int depthMapFBO;
glGenFramebuffers(1, &depthMapFBO);

// 2. 创建 2D 深度纹理 (1024x1024 分辨率)
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
unsigned int depthMap;
glGenTextures(1, &depthMap);
glBindTexture(GL_TEXTURE_2D, depthMap);
// 注意：内存格式为 GL_DEPTH_COMPONENT，只分配 32 位浮点数存深度！
glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

// 3. 把深度纹理挂载到 FBO 上
glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
glDrawBuffer(GL_NONE); // 显式告诉 OpenGL：我们不需要写入任何颜色！
glReadBuffer(GL_NONE);
glBindFramebuffer(GL_FRAMEBUFFER, 0);
```



##### 2. Pass 1 着色器：极速生成深度图

因为 Pass 1 只需要算深度，Shader 简洁到了极致：

##### **Pass 1 顶点着色器 (`shadow_depth.vs`)：**

```c++
# version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix; // 👈 光源视角矩阵 (Light Projection * Light View)
uniform mat4 model;

void main() {
    // 把顶点从模型空间直接转换到“光源视角下的裁剪空间”
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}
```

```glsl
# version 330 core
void main() {
    // 啥都不用写！GPU 硬件光栅化器会自动把 gl_Position.z 写入深度贴图！
}
```



##### 3. Pass 2 着色器：正式查账与阴影计算

##### 片段着色器（FS）拿到 Pass 1 拍好的 `shadowMap` 纹理，开始比对深度：

在正式渲染场景时，我们在片段着色器中通过 **光源空间矩阵（LightSpaceMatrix）** 把当前片元投影到光源的视角下，去查那张深度贴图。

```glsl
#version 330 core
layout (location = 0) in vec3 aCoord;   //顶点
layout (location = 1) in vec3 aNormal;  //法线
layout (location = 2) in vec2 aTexCoords; //纹理坐标

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;  //逆转置法线矩阵
uniform mat4 lightSpaceMatrix; // 光源的 Projection * View 矩阵（阴影映射用）

out VS_OUT {
    vec3 FragPos;           //世界空间下的片段坐标
    vec3 Normal;            //世界空间下的法线向量
    vec2 TexCoords;         //世界空间下的纹理坐标
    vec4 FragPosLightSpace; //当前片段在光源视角下的裁剪空间坐标
} vs_out;

void main()
{
    //局部空间转为世界空间
    vs_out.FragPos = vec3(model * vec4(aCoord,1.0));

    //法线
    vs_out.Normal = normalMatrix * aNormal;
    vs_out.TexCoords = aTexCoords;

    //将世界坐标变换到光源裁剪空间（供片元阶段做阴影比较）
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);

    //转换为裁剪空间坐标
    gl_Position = projection * view * model * vec4(aCoord,1.0);
}

```



```glsl
#version 330 core

out vec4 FragColor;

struct Light {
    vec3 direction;      // 平行光照射方向（从光源射向场景）
    vec3 ambient;        // 环境光
    vec3 diffuse;        // 漫反射
    vec3 specular;       // 高光
};

struct Material {
    sampler2D diffuse;   // 漫反射贴图
    sampler2D specular;  // 高光贴图
    float shininess;     // 高光锐利度
};

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace; //当前像素在光源视角下的裁剪空间坐标
} fs_in;


//材质颜色
uniform Material material;
//光源颜色
uniform Light light;
//摄像机位置
uniform vec3 cameraPos;
//深度纹理
uniform sampler2D shadowMap;

void main()
{
    // ========== 1. 基础向量 ==========
    // N：片元法线（世界空间，插值后）
    vec3 N = normalize(fs_in.Normal);
    // V：片元指向相机的方向
    vec3 V = normalize(cameraPos - fs_in.FragPos);

    // ========== 2. 纹理颜色（Gamma 逆校正：sRGB -> 线性空间）==========
    vec3 diffuseColor  = pow(texture(material.diffuse,  fs_in.TexCoords).rgb, vec3(2.2));
    vec3 specularColor = texture(material.specular, fs_in.TexCoords).rgb;

    // ========== 3. 环境光：恒定，不受阴影遮挡 ==========
    vec3 ambient = light.ambient * diffuseColor;

    // ========== 4. 漫反射（直接光）==========
    vec3 L = normalize(-light.direction);          // 片元指向光源的方向
    float diff = max(dot(N, L), 0.0);              // 法线与光线的夹角余弦
    vec3 diffuse = light.diffuse * diff * diffuseColor;

    // ========== 5. 高光（直接光）==========
    vec3 R = reflect(-L, N);                       // 光线关于法线的反射方向
    float spec = pow(max(dot(V, R), 0.0), material.shininess);
    vec3 specular = light.specular * spec * specularColor;

    // 直接光 = 漫反射 + 高光（这部分会被阴影遮挡）
    vec3 direct = diffuse + specular;

    // ========== 6. 阴影判断：与深度贴图比较 ==========
    // 6.1 透视除法：光源裁剪空间 -> NDC [-1, 1]
    vec3 projCoords = fs_in.FragPosLightSpace.xyz / fs_in.FragPosLightSpace.w;
    // 6.2 NDC -> 纹理采样坐标 [0, 1]
    projCoords = projCoords * 0.5 + 0.5;
    // 6.3 当前片元在光源视角下的深度
    float currentDepth = projCoords.z;
    // 6.4 与深度贴图比较：加 bias 消除自阴影（Shadow Acne），3x3 PCF 柔化边缘
    float bias = 0.005;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0); // 单个纹素的大小
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            // 采样周围 3x3 邻域的最近深度,邻域采样,projCoords.xy当前位置，vec2(x, y) * texelSize 3x3偏移值
            float closestDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            // 当前深度比记录值更远 -> 被遮挡 -> 记 1，否则记 0
            shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0; // 取 9 个采样点的平均值，得到柔化的阴影系数
    // 超出阴影贴图范围的部分不判定为阴影（避免物体外被误判成黑）
    if (projCoords.z > 1.0)
        shadow = 0.0;

    // ========== 7. 合成颜色 ==========
    // 环境光恒定 + 直接光按阴影系数衰减
    vec3 lighting = ambient + (1.0 - shadow) * direct;
    // Gamma 校正：线性空间 -> sRGB
    vec3 finalColor = pow(lighting, vec3(1.0 / 2.2));

    FragColor = vec4(finalColor, 1.0);
}
```



```c++
while(!glfwWindowShouldClose(window))
{
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window);

    //两个物体都是同一个摄像机，同一个窗口进行观察，所以共用view和projection
    glm::mat4 view = camera.GetViewMatrix();
    float aspect = framebufferHeight > 0
        ? static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight)
        : 1.0f;
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspect, 0.1f, 100.0f);

    // 计算物体的模型矩阵（旋转正方体）
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(cubePitch), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(cubeYaw), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

    // 计算光源空间矩阵（平行光：正交投影，阴影覆盖一个长方体区域）
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);
    glm::vec3 lightDir(-0.2f, -1.0f, -0.3f);
    glm::mat4 lightView = glm::lookAt(lightPos, lightPos + lightDir, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    // ============ 第一遍：从光源视角渲染深度贴图（Shadow Map）============
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT); // 与深度纹理尺寸一致
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    shadowShader.use();
    shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    // 画正方体
    shadowShader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    // 画地面
    glBindVertexArray(groundVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ============ 第二遍：从相机视角正常渲染 ============
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //绘制光源位置小方块
    lightShader.use();
    glm::mat4 lightModel = glm::mat4(1.0f);
    lightModel = glm::translate(lightModel, lightPos);
    lightModel = glm::scale(lightModel, glm::vec3(0.2f));
    lightShader.setMat4("model", lightModel);
    lightShader.setMat4("view", view);
    lightShader.setMat4("projection", projection);
    glBindVertexArray(lightCubeVAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    //物体对象
    objShader.use();

    objShader.setMat4("model",model);
    objShader.setMat4("view",view);
    objShader.setMat4("projection",projection);
    objShader.setMat3("normalMatrix",normalMatrix);
    objShader.setVec3("cameraPos",camera.Position);

    // 阴影映射相关 uniform
    objShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    objShader.setInt("shadowMap", 2);

    // 设置平行光参数（方向、颜色）
    objShader.setVec3("light.direction", glm::vec3(-0.2f, -1.0f, -0.3f)); // 固定斜上方照射
    objShader.setVec3("light.ambient",  glm::vec3(0.2f, 0.2f, 0.2f));
    objShader.setVec3("light.diffuse",  glm::vec3(0.8f, 0.8f, 0.8f));
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
    glBindTexture(GL_TEXTURE_2D, depthMap); // 深度贴图绑定到纹理单元 2（对应 uniform shadowMap）

    //绘制物体正方体
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES,indexCount,GL_UNSIGNED_INT,0);
    //绘制地面（同样接收阴影）
    glBindVertexArray(groundVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glfwSwapBuffers(window);
    glfwPollEvents();
}
```



#### 4.工业级深度：避坑指南与四大“魔鬼细节”

真实渲染中必须处理以下 **4 个工业级魔鬼细节**：

##### 1.阴影粉刺（Shadow Acne）与斜率偏置

- **现象：** 地面上全是像斑马线一样、密密麻麻的黑白交错条纹。
- **原因：** 深度贴图分辨率有限（比如 1024×10241024×1024）。当光线斜着照射到地面时，多个相邻像素去查深度图，查出来的都是同一个离散深度值；而像素自己的真实深度是连续变化的。**浮点数精度稍微错位一点点，像素就会误以为自己被自己挡住了（Self-Shadowing）！**
- **工业级解决方案（动态斜率偏置 Slope-Scaled Bias）：** 在比对深度时，主动让当前深度**扣减一个小的偏移量（Bias）**。而且这个 Bias 必须根据**光线与表面的倾斜角度动态计算**

shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;



##### 2.悬浮现象（Peter Panning）

- **现象：** 加了 Bias 之后，粉刺消失了，但阴影和物体底部脱节了！箱子看起来像悬浮在半空中。
- **原因：** Bias 给了太大了，把阴影整体往外推得太远。
- **工业级解决方案（Pass 1 剔除正面 Front-Face Culling）：** 在 Pass 1 渲染深度图时，开启 **`glCullFace(GL_FRONT);`**，只把物体的**背面**写入深度图！ 由于实心物体有厚度，背面的深度天然比正面深得多，Pass 2 渲染正面时绝对不会和背面发生精度争用，**这样可以用极小的 Bias 彻底杀死粉刺，同时完全避免悬浮！**



##### 3.视锥体泄漏与边缘假阴影

- **现象：** 玩家看向远方超出光源照射范围的地方，屏幕边缘全部变成了假阴影；或者超出深度图边界的地方一片漆黑。

- 工业级解决方案：

  1. 在 C++ 端把深度纹理的环绕模式设为GL_CLAMP_TO_BORDER，并将边界颜色强行设为 `1.0`（最大深度，代表无遮挡）：

  ```c++
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  ```

  2.在 GLSL 加入防御性截断：

  if (projCoords.z > 1.0)
          shadow = 0.0;

  

##### 4.硬锯齿与 PCF 软阴影核算法 (Percentage-Closer Filtering)

- **现象：** 阴影边缘全是粗糙的锯齿方块（硬阴影 Hard Shadows）。
- **原因：** 深度图采样出来的结果非 00 即 11，没有半影区（Penumbra）。
- **工业级解决方案（PCF 3x3 滤波核）：** 不要只采样 1 个像素！**去目标 UV 周边的 3×33×3 邻域内采样 9 个点进行深度对比，然后把 9 次对比的结果求平均值！**

```glsl
float shadow = 0.0;
// 计算单个纹理像素 (Texel) 的 UV 尺寸
vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

// 遍历周边 3x3 邻域
for(int x = -1; x <= 1; ++x) {
    for(int y = -1; y <= 1; ++y) {
        float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
        shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
    }    
}
// 取 9 次对比的平均值，瞬间得到边缘极度丝滑柔和的半影软阴影！
shadow /= 9.0;
```



#### 5.总结

这就是现代游戏引擎中阴影映射（Shadow Mapping）的全貌：

1. **核心逻辑：** 两次绘制。Pass 1 从光源视角拍深度照片；Pass 2 从玩家视角查照片比大小。

2. **判断准则：** 当前像素真实距离 Dcurrent−Bias>*D*current−Bias> 深度照片记录距离 Dclosest  ⟹  *D*closest⟹ **阴影！**

3. ##### 三大神器：

   - **动态斜率 Bias** 杀死阴影粉刺；
   - **正面剔除 (Front-Face Culling)** 杀死悬浮；
   - **PCF 3x3 滤波** 杀死边缘锯齿！





## 第二十五章.法线贴图(Normal Mapping)

在之前的渲染中，如果你想画一面“凹凸不平的破旧砖墙”，唯一的方法是往模型里**塞入成千上万个三角形顶点**去雕刻砖缝。这会导致显卡顶点数量暴胀，帧率断崖式下跌。

而 **法线贴图（Normal Mapping）** 的伟大之处在于：**它允许我们使用一个只有 2 个三角形的完全平整的平面，通过“欺骗光照计算”，呈现出极其惊人的凹凸立体感！**



#### 1.核心思想

平整的墙为什么看起来像凸起的？ 物理学告诉你：**人眼是个极其愚蠢的器官。我们的大脑根本看不清 3D 物体的真实几何形状，我们大脑判断凹凸的唯一依据，是【光影的明暗交替】！**



在冯氏光照模型中，决定一个像素点是亮还是暗的核心公式是**点积（Dot Product）**：

##### 									亮度=max(*N*⋅*L*,0)

- N⃗*N*（Normal）：表面的法线方向（决定脸朝哪个方向）。
- L⃗*L*（Light）：指向光源的方向。



如果 N⃗*N* 永远朝向正上方 (0,0,1)(0,0,1)： 不管光怎么照，平面上每个点算出来的亮度都一模一样，人眼一看：“切，不就是张毫无生气的平整纸片吗？”

**妙计：** 如果我们能偷梁换柱，**让平整平面上每一个像素的“假法线 N”稍微歪一点点**：

- 砖块边缘的法线向左歪一点 →→ 光照打上去立刻产生阴影。
- 砖缝深处的法线向右歪一点 →→ 产生高光。



##### 只需改变法线 N，平整的纸片瞬间就能呈现出凹凸万丈的立体感！这就是法线贴图的灵魂。

每一个像素的法线向量来源于哪里？**直接将每个像素的rgb当作法线向量**



妙计虽好，但立刻遇到了一个**物理学灾难**。

我们在着色器里计算光照时，所有的向量（光线方向 L⃗*L*、视角方向 V⃗*V*）都是在**世界空间（World Space）**里算得好好的。 但是，你的砖墙贴图是贴在平整面上的，贴图里的法线向量（比如红色代表 X\*X\* 轴右，绿色代表 Y\*Y\* 轴上）是基于**图片自身局部平面（UV 空间）**的。

- **灾难：** 如果把法线贴图直接丢进世界空间里去算： 当玩家走到墙的背面，或者把这堵墙旋转 90 度贴在天花板上时，原本指“上”的绿色法线突然变成了指“前”，整堵墙的光影瞬间崩塌、发黑、穿帮！

**如何解决？** 我们需要发明一个**“万能局部坐标系”**——不管这堵墙怎么旋转、怎么扭曲、甚至贴在会变形的角色手臂上，这个局部坐标系永远紧紧贴在砖块的表面。 这个伟大的局部坐标系，就叫 **切线空间（Tangent Space）**！



#### 2.核心架构

想象你手里有一张平整的布（或者一张世界地图）。 这张布有两个方向的坐标：

1. **横向的 U 方向**（向右延伸）
2. **纵向的 V 方向**（向上延伸）

当你把这张布**贴在一个 3D 的曲面（比如球体或者捏扁揉圆的角色模型）**上时：

- **切线（Tangent, T⃗\*T\*）：** 就是**当你在 3D 空间里沿着贴图的 U（横向）方向走一步时，3D 空间里的坐标会朝哪个方向变化**。它永远平行于纹理的横向。
- **副切线（Bitangent, B⃗\*B\*）：** 就是**当你在 3D 空间里沿着贴图的 V（纵向）方向走一步时，3D 空间里的坐标会朝哪个方向变化**。它永远平行于纹理的纵向。

再加上法线，就形成了在切线空间里，每一个顶点都有三个互相垂直的亲兄弟轴：

1. Z轴（Normal, N）：永远垂直于表面，指向你的眼睛（法线）。
2. X轴（Tangent, T）：永远顺着纹理贴图的 **U（横向）** 方向。
3. Y轴（Bitangent, B）：永远顺着纹理贴图的 **V（纵向）** 方向。

这三个向量组合在一起，就构成了大名鼎鼎的 **TBN 矩阵（Tangent-Bitangent-Normal Matrix）**。 它的特工使命极其纯粹：**充当翻译官。它能把世界空间的光线方向，瞬间翻译成切线空间里的方向；或者把法线贴图里的局部法线，翻译回世界空间！**



#### 3.TBN矩阵传输

在 C++ 端，为了在顶点着色器中构建 TBN 矩阵，我们除了位置、法线、UV 之外，还必须在导入模型时**计算出每一个顶点的切线（Tangent）和副切线（Bitangent）**：



在 3D 软件里，模型是由无数个**三角形**组成的。每个三角形有 3 个顶点（P1,P2,P3），并且每个顶点都有对应的纹理坐标（UV1,UV2,UV3）。

我们能不能利用这个三角形在 **3D 空间里的位移** 和它在 **2D 纹理里的 UV 差值**，反推出切线 T 呢？



假设在三角形表面，3D 空间的位置变化量（ΔPΔ*P*）是由 2D 纹理坐标的变化量（ΔUΔ*U* 和 ΔVΔ*V*）线性组合出来的：

- 向量边 1：E*1=*P*2−*P*1，它对应的 UV 变化是 Δ*U*1 和 Δ*V1。
- 向量边 2：E*2=*P*3−*P*1，它对应的 UV 变化是 Δ*U*2 和 Δ*V2。



根据线性关系，我们可以写出这样一个矩阵方程：

$$
\begin{aligned}
\vec{E}_1 &= \Delta U_1 \vec{T} + \Delta V_1 \vec{B}_{uv} \\
\vec{E}_2 &= \Delta U_2 \vec{T} + \Delta V_2 \vec{B}_{uv}
\end{aligned}
$$

如果先按向量形式写成矩阵乘法，就是：

$$
\begin{bmatrix}
\vec{E}_1 \\
\vec{E}_2
\end{bmatrix}
=
\begin{bmatrix}
\Delta U_1 & \Delta V_1 \\
\Delta U_2 & \Delta V_2
\end{bmatrix}
\begin{bmatrix}
\vec{T} \\
\vec{B}_{uv}
\end{bmatrix}
$$

由于 $\vec{E}_1$、$\vec{E}_2$、$\vec{T}$、$\vec{B}_{uv}$ 都是三维向量，真正计算时可以把它展开成 x、y、z 三个分量：

$$
\begin{bmatrix}
E_{1x} & E_{1y} & E_{1z} \\
E_{2x} & E_{2y} & E_{2z}
\end{bmatrix}
=
\begin{bmatrix}
\Delta U_1 & \Delta V_1 \\
\Delta U_2 & \Delta V_2
\end{bmatrix}
\begin{bmatrix}
T_x & T_y & T_z \\
B_{uvx} & B_{uvy} & B_{uvz}
\end{bmatrix}
$$

我们的目标是先求出 $\vec{T}$（切线）和原始的 $\vec{B}_{uv}$（由 UV 的 V 方向解出的副切线参考方向）。根据线性代数，只要在方程两边**乘以 2D 纹理矩阵的逆矩阵（Inverse Matrix）**，就能把它们解出来。

先把 2D 纹理矩阵记为 $A$：

$$
A =
\begin{bmatrix}
\Delta U_1 & \Delta V_1 \\
\Delta U_2 & \Delta V_2
\end{bmatrix}
$$

它的行列式为：

$$
\det(A) = \Delta U_1 \Delta V_2 - \Delta U_2 \Delta V_1
$$

所以它的逆矩阵为：

$$
A^{-1}
=
\frac{1}{\Delta U_1 \Delta V_2 - \Delta U_2 \Delta V_1}
\begin{bmatrix}
\Delta V_2 & -\Delta V_1 \\
-\Delta U_2 & \Delta U_1
\end{bmatrix}
$$

两边左乘 $A^{-1}$：

$$
\begin{bmatrix}
T_x & T_y & T_z \\
B_{uvx} & B_{uvy} & B_{uvz}
\end{bmatrix}
=
\frac{1}{\Delta U_1 \Delta V_2 - \Delta U_2 \Delta V_1}
\begin{bmatrix}
\Delta V_2 & -\Delta V_1 \\
-\Delta U_2 & \Delta U_1
\end{bmatrix}
\begin{bmatrix}
E_{1x} & E_{1y} & E_{1z} \\
E_{2x} & E_{2y} & E_{2z}
\end{bmatrix}
$$

因此可以得到 UV 方程下的向量求解式：

$$
\begin{aligned}
\vec{T}
&=
\frac{1}{\Delta U_1 \Delta V_2 - \Delta U_2 \Delta V_1}
(\Delta V_2 \vec{E}_1 - \Delta V_1 \vec{E}_2) \\
\vec{B}_{uv}
&=
\frac{1}{\Delta U_1 \Delta V_2 - \Delta U_2 \Delta V_1}
(-\Delta U_2 \vec{E}_1 + \Delta U_1 \vec{E}_2)
\end{aligned}
$$

但是最终用于 TBN 矩阵的副切线，一般不直接使用这个原始 $\vec{B}_{uv}$。更稳妥的做法是先把切线相对法线正交化，再用法线和切线叉乘得到真正参与 TBN 的 $\vec{B}$：

$$
\begin{aligned}
\vec{T} &= normalize(\vec{T} - (\vec{N} \cdot \vec{T})\vec{N}) \\
\vec{B} &= normalize(cross(\vec{N}, \vec{T}))
\end{aligned}
$$

对应到代码里的 `f`，就是：

$$
\begin{aligned}
f &= \frac{1}{\Delta U_1 \Delta V_2 - \Delta U_2 \Delta V_1} \\
\vec{T} &= f(\Delta V_2 \vec{E}_1 - \Delta V_1 \vec{E}_2) \\
\vec{B} &= normalize(cross(\vec{N}, \vec{T}))
\end{aligned}
$$



```c++
//假设我们在 C++ 中手动为一个三角形计算 Tangent 和 Bitangent
glm::vec3 edge1 = pos2 - pos1;
glm::vec3 edge2 = pos3 - pos1;
glm::vec2 deltaUV1 = uv2 - uv1;
glm::vec2 deltaUV2 = uv3 - uv1;
glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

glm::vec3 tangent;
tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
tangent = tangent - normal * glm::dot(normal, tangent);
tangent = glm::normalize(tangent);

glm::vec3 bitangent = glm::normalize(glm::cross(normal, tangent));

//把Tangent 和 Bitangent 像普通 VBO 属性一样送入显卡，绑定到 Location 3 和 4
```



#### 4.Shader源码实现：在切线空间中计算光照

在法线贴图的实现中，有两种常见策略：

1. **方案 A：** 把光线方向（LightDir）和视线方向（ViewDir）通过 TBN 矩阵**变换到切线空间**中，与法线贴图直接点积。
2. **方案 B：** 把法线贴图解包出的法线通过 TBN*TBN* 矩阵**变换到世界空间**中。

在现代游戏引擎中，**方案 A（把光照向量转换到切线空间）** 效率最高，因为所有的顶点都转换好了，片段着色器直接拿来用。

```glsl
# version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;   // 👈 切线向量
layout (location = 4) in vec3 aBitangent; // 👈 副切线向量

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    vs_out.FragPos = vec3(worldPos);
    vs_out.TexCoords = aTexCoords;

    // 1. 将法线、切线、副切线变换到世界空间，并消除不等比缩放的影响
    //因为当模型发生“非等比缩放（Non-uniform Scaling）”或者“剪切变形”时，切线和副切线也会和法线一样发生方向畸变！它们必须享受同等待遇的数学矫正。
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 N = normalize(normalMatrix * aNormal); //这个是之前的基础光照的世界空间的法线求解，同样的T,B和N同处一个局部空间，所以同乘一个矩阵
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 B = normalize(normalMatrix * aBitangent);
    
    // 2. 构造正交归一化的 TBN 矩阵 (World-to-Tangent 矩阵)
    mat3 TBN = transpose(mat3(T, B, N)); 

    // 3. 将 光源位置、观察位置、当前片段位置 全部变换到【切线空间】中！
    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos  = TBN * viewPos;
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;

    gl_Position = projection * view * worldPos;
}
```



```glsl
#version 330 core

out vec4 FragColor;

// 光照参数结构体：本 demo 只用了一个点光源。
struct Light {
    vec3 ambient;   // 环境光强度（不随距离衰减）
    vec3 diffuse;   // 漫反射强度
    vec3 specular;  // 镜面高光强度
};

// 材质参数结构体。
struct Material {
    sampler2D normalMap; // 法线贴图：用颜色编码法线方向，制造表面凹凸
    vec3 diffuseColor;   // 漫反射基础颜色（本 demo 为纯色砖红）
    float shininess;     // 高光锐度（越大，高光越集中）
};

// 顶点着色器传来的数据（都是已经变换到切线空间的值）。
// 在切线空间计算光照，才能让法线贴图中的法线与光线、视线处于同一坐标系。
in VS_OUT {
    vec3 FragPos;           // 片元世界坐标（本 demo 未直接使用）
    vec2 TexCoords;         // 纹理坐标，用于采样法线贴图
    vec3 TangentLightPos;   // 光源位置（切线空间）
    vec3 TangentCameraPos;  // 相机位置（切线空间）
    vec3 TangentFragPos;    // 片元位置（切线空间）
} fs_in;

uniform Material material;
uniform Light light;

void main()
{
    // 1. 从法线贴图中采样颜色 (RGB 范围是 0.0 ~ 1.0),
    vec3 N = texture(material.normalMap, fs_in.TexCoords).rgb;

    // 2. 解包 (Unpack)：把 [0, 1] 的颜色映射回 [-1, 1] 的真实法线向量空间
    N = normalize(N * 2.0 - 1.0);

    // 3. 获取切线空间下的光线方向与视线方向
    vec3 L = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    vec3 V = normalize(fs_in.TangentCameraPos - fs_in.TangentFragPos);
    vec3 H = normalize(L + V);

    // 4. 计算光照分量：
    //    先对漫反射颜色做 gamma 逆校正（pow 2.2），因为最终输出还会做一次 gamma 校正，
    //    中间的光照计算需要在线性空间进行，避免颜色失真。
    vec3 diffuseColor = pow(material.diffuseColor, vec3(2.2));

    //    diffuse：法线与光线的夹角越接近 0°，漫反射越强（Lambert 定律）。
    float diff = max(dot(N, L), 0.0);

    //    spec：Blinn-Phong 模型，用半程向量 H 与法线 N 的点积计算镜面高光。
    float spec = pow(max(dot(N, H), 0.0), material.shininess);

    // 5. 点光源距离衰减：光强随距离增大按二次曲线快速减弱。
    float lightDistance = length(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float attenuation = 1.0 / (1.0 + 0.22 * lightDistance + 0.20 * lightDistance * lightDistance);

    // 6. 合成光照：环境光 + (漫反射 + 高光) × 距离衰减。
    vec3 ambient = light.ambient * diffuseColor;
    vec3 diffuse = light.diffuse * diff * diffuseColor;
    vec3 specular = light.specular * spec;
    vec3 lighting = ambient + (diffuse + specular) * attenuation;

    // 7. gamma 校正：把线性光照结果映射回 sRGB 显示空间（pow 1/2.2）。
    vec3 finalColor = pow(lighting, vec3(1.0 / 2.2));

    // 8. 输出最终颜色，alpha 恒为 1.0（不透明）。
    FragColor = vec4(finalColor, 1.0);
}
```



##### 法线贴图核心总结

1. **核心原理：** 

   欺骗光照。用一张记录了微小向量偏移的纹理（Normal Map），代替原本平整的几何面法线参与 `dot(N, L)` 计算。

   我们把因果链条梳理顺：

   1. **源头（法线贴图）：** 它是现成的纹理，里面存了成千上万个微小的假法线（比如砖缝深处法线歪了）。
   2. **桥梁（TBN 矩阵）：** 因为光线和视线在世界空间，法线贴图在局部空间。我们用 TBN 把光线和视线“翻译”进切线空间。
   3. **审判（光照计算）：** 在每一个像素点上，我们用**法线贴图里解包出来的假法线**，去和翻译过来的光线、视线做点积。

   **结果：** 每一个像素点因为法线方向各不相同，算出来的亮度和高光也完全不同，最终在毫无起伏的纸片平面上，骗过了我们的大脑，造出了完美的立体凹凸感（法线贴图渲染）！

​		

​	法线贴图（Normal Map）里到底长什么样？

​	想象一下，你用 Photoshop 打开了一张法线贴图（比如砖墙的 Normal Map）。

- 这张图片长得极其诡异，通体呈现出一种**耀眼的浅蓝色（RGB 颜色大约是 `R=128, G=128, B=255`）**。
- 为什么是蓝色的？ 因为在图片的每一个像素点（Pixel）里：
  - **R 通道（红色）** 代表 X 轴的偏转。
  - **G 通道（绿色）** 代表 Y 轴的偏转。
  - **B 通道（蓝色）** 代表 Z 轴（永远朝外）。
  - 纯正的浅蓝色代表法线毫无偏转（即 `(0, 0, 1)`）。
  - **而当遇到砖缝或凹陷处时，图片的像素颜色会变成紫色、粉色或青色！** 这代表这个像素处的法线故意向左或向下歪了。

​	一张 1024×10241024×1024 的法线贴图，里面有 **100 万个像素格子**，每一个格子都存着一个不同的微小偏转向量。**这就是“成千上万个微小的假法线”的真正藏身之处！**，就是把像素点的RGB当作法线



2.关键步骤：

- **解包（Unpack）：** `normal = normalize(texture(normalMap, UV).rgb * 2.0 - 1.0);` 把显存里的 [[0](http://localhost:3782/home/unified_1786503632384_b865d44c#references), [1](http://localhost:3782/home/unified_1786503632384_b865d44c#references)] 颜色还原成数学上的 `[-1, 1]` 向量。
- **TBN 矩阵：** 把世界空间的坐标转换到局部的切线空间，使得法线贴图能够完美适应任意旋转和复杂弯曲的 3D 模型。





## 第二十六章.视差贴图(Parallax Mapping)

**视差贴图 (Parallax Mapping)**（也叫高度贴图 / Relief Mapping）

​		在法线贴图中，虽然我们通过假法线骗过了光照（有了明暗凹凸），但**如果你把镜头贴近砖墙去看它的边缘，你会发现它依然是一个惨不忍睹的绝对平整纸片！** 砖块不会挡住后面的砖块，也没有真正的物理错位。

而 **视差贴图** 允许我们在片段着色器中，**通过一张黑白的“高度图（Height Map）”，动态偏移 UV 采样坐标**，让像素产生真实的“遮挡位移”。



#### 1.核心思想

- **法线贴图的局限：** 法线贴图只改变了光照计算中的 N⃗*N*（法线），但**没有改变纹理采样的 UV 坐标**。 当你走到侧面观察时，你会发现砖墙边缘的轮廓依然是笔直的一条线，毫无立体结构可言。
- **视差贴图的野望：** 我们希望给每个像素赋予一张**高度图（Height Map，黑白图，白色代表凸起，黑色代表凹陷）**。当摄像机斜着看过去时，**视觉视线（View Direction）会和高度图相交，从而在视觉上把 UV 坐标“往前或往后拉伸”**，让凹陷处的像素显示出被遮挡的深处纹理！



#### 2.核心架构

##### 核心数学原理：视差偏移（Parallax Offset Calculation）



想象一下：你在现实中站在一口深井的边缘，往下看井底的石头。

- 如果你**正对着井口向下看**（视线垂直于地面），石头就是它原本的位置，没有任何位移。

- 如果你

  走到井边，斜着眼睛往井底深处看

  ：

  - 因为井有深度（h*h*），你的视线被井壁挡了一段。
  - 在你的眼里，井底的石头会产生一个**“视觉上的水平错位（偏移量 P**P**）”**——石头看起来好像向旁边移了一段距离。

视差贴图的数学公式，就是在计算机里**完美复现这种“斜眼看深坑时的视觉错位”**！



1. 已知条件：

   - 视线方向 V⃗*V*（在切线空间中，由视线向量乘 TBN 矩阵得到）。

   - 当前像素的纹理坐标 
     $$
     UVcurrent
     $$

   - 高度贴图采样出的高度值 
     $$
     h=texture(heightMap, UV).r
     $$
     （范围 0.0∼1.00.0∼1.0，11 代表最高，00 代表最深凹陷）。

2. **偏移公式：** 我们希望凹陷越深（h*h* 越小），UV 偏移得越厉害。同时，视线越倾斜（夹角越小），偏移量也应该越大。 视差偏移量的计算公式为：

$$
heightScale = 0.1 \quad (\text{控制凹陷深度强度})
$$


$$
P = \frac{\vec{V}_{xy}}{\vec{V}_z} \times (h \times heightScale)
$$

> **分量展开**：$\vec{V}_{xy}$ 是视线向量 $\vec{V}$ 的水平分量（沿贴图切平面方向），$\vec{V}_z$ 是它的深度分量（沿表面法线方向）。GLSL 里写作 `viewDir.xy / viewDir.z`，即逐分量相除：
> $$
> \frac{\vec{V}_{xy}}{\vec{V}_z} = \left( \frac{V_x}{V_z},\ \frac{V_y}{V_z} \right)
> $$

$$
UV_{final} = UV_{current} -P
$$

- *h*：从高度图中采样出来的当前点的高度值（范围 0.0∼1.00.0∼1.0）。
- **heightScale**：控制整体凹陷强度的放大系数（比如 `0.1`）。
- **含义：** h×heightScale 代表当前这个像素对应的**实际物理下陷深度**。坑越深，等会儿视线偏移得越厉害。

这里的 V是**视线方向向量（View Direction）**，并且是在**切线空间**中计算的。

- *V*z：视线在垂直方向（朝外）的分量。
- *V**x**y*：视线在水平方向（平面上）的分量。
- **为什么要用 Vxy/Vz 做除法？** 这是一个完美的初中几何相似三角形原理：
  - 当你**垂直向下看**时（视线几乎平行于 Z 轴）：V**x**y≈0，分子为 0，所以偏移量 **P**=0。**（正着看，没有视差错位！）**
  - 当你极度斜着看时（视线几乎平行于平面）：Vz变得非常接近 0，分母变成一个极小的数。 分母越小，整体商Vxy/Vz就会变得巨大无比！
    - **物理意义：** 视线越平缓（斜着看），同一个深度的坑在视觉上产生的水平错位就会**拉得越长**！这就是为什么斜着看时视差效果最强烈的原因。



#### 3.代码

在片段着色器中，我们在采样漫反射和法线贴图之前，**先用高度图对 UV 坐标进行一次“视差偏移”**：



```c++
#version 330 core
layout (location = 0) in vec3 aPos;       // 顶点位置
layout (location = 1) in vec3 aNormal;    // 法线
layout (location = 2) in vec2 aTexCoords; // 纹理坐标
layout (location = 3) in vec3 aTangent;   // 切线
layout (location = 4) in vec3 aBitangent; // 副切线

uniform mat4 projection;  // 投影矩阵（相机 -> 裁剪）
uniform mat4 view;        // 视图矩阵（世界 -> 相机）
uniform mat4 model;       // 模型矩阵（局部 -> 世界）
uniform vec3 viewPos;     // 相机位置（世界空间）
uniform vec3 lightPos;    // 光源位置（世界空间）

// 传给片段着色器的接口块：所有向量都变换到【切线空间】。
out VS_OUT {
    vec3 FragPos;          // 片元世界坐标
    vec2 TexCoords;        // 纹理坐标
    vec3 TangentLightPos;  // 光源位置（切线空间）
    vec3 TangentViewPos;   // 相机位置（切线空间）
    vec3 TangentFragPos;   // 片元位置（切线空间）
} vs_out;

void main()
{
    // 把顶点变换到世界空间，供后面 TBN 变换和光照使用。
    vec3 fragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.FragPos = fragPos;
    vs_out.TexCoords = aTexCoords;

    // 法线、切线、副切线变换到世界空间（normalMatrix 防止非等比缩放扭曲方向）。
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 N = normalize(normalMatrix * aNormal);
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 B = normalize(normalMatrix * aBitangent);

    // 构造世界 -> 切线的 TBN 矩阵：把光线、视线、片元位置翻译进切线空间，
    // 以便片段着色器里能用法线贴图（存于切线空间）直接做点积。
    mat3 TBN = transpose(mat3(T, B, N));

    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos  = TBN * viewPos;
    vs_out.TangentFragPos  = TBN * fragPos;

    gl_Position = projection * view * vec4(fragPos, 1.0);
}
```



```glsl
#version 330 core

out vec4 FragColor;

// 光照参数结构体：本 demo 只用了一个点光源。
// 注意：光源方向不用这里传，顶点着色器已把 lightPos 变换到切线空间（fs_in.TangentLightPos）。
struct Light {
    vec3 ambient;   // 环境光强度
    vec3 diffuse;   // 漫反射强度
    vec3 specular;  // 镜面高光强度
};

// 材质参数结构体：三张贴图 + 视差高度缩放。
struct Material {
    sampler2D diffuse;   // 漫反射贴图：物体固有颜色
    sampler2D normalMap; // 法线贴图：颜色编码的切线空间法线
    sampler2D depthMap;  // 高度图（黑白）：R 通道存表面高度
    float shininess;     // 高光锐度（越大越集中）
};

// 顶点着色器传来的数据（都在切线空间）。
in VS_OUT {
    vec3 FragPos;          // 片元世界坐标
    vec2 TexCoords;        // 纹理坐标（视差偏移会修改它）
    vec3 TangentLightPos;  // 光源位置（切线空间）
    vec3 TangentViewPos;   // 相机位置（切线空间）
    vec3 TangentFragPos;   // 片元位置（切线空间）
} fs_in;

uniform Material material;
uniform Light light;
uniform float heightScale; // 视差强度（越大凹陷越深，用 Q/E 键调节）

void main()
{
    // 0. 计算切线空间视线方向（片元指向相机）。
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);

    // 1. 视差贴图核心：根据高度图偏移 UV（全部内联，不再封装函数）。
    //    原理：视线越倾斜、高度越低（凹陷越深），UV 偏移越大，
    //    让平面看起来像有真实的深度遮挡（砖块互相错位）。
    // 1.1 采样当前 UV 处的高度（0 = 最深凹陷，1 = 最高凸起）。
    float height = texture(material.depthMap, fs_in.TexCoords).r;

    // 1.2 视线方向在切线空间下的偏移量：viewDir.xy / viewDir.z 是视线倾斜程度。
    //     视线越斜（viewDir.xy 大 / viewDir.z 小），或高度越低，偏移越大。
    vec2 p = viewDir.xy / viewDir.z * (height * heightScale);

    // 1.3 用偏移后的 UV 采样其他贴图，实现“假深度遮挡”。
    vec2 texCoords = fs_in.TexCoords - p;

    // 2. 采样法线贴图并解包成 [-1, 1] 的切线空间法线。
    vec3 N = texture(material.normalMap, texCoords).rgb;
    N = normalize(N * 2.0 - 1.0);

    // 3. 计算切线空间光线/视线/半程向量。
    vec3 L = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    vec3 V = normalize(fs_in.TangentViewPos  - fs_in.TangentFragPos);
    vec3 H = normalize(L + V);

    // 4. 采样漫反射贴图，做 gamma 逆校正（转回线性空间）。
    vec3 diffuseColor = pow(texture(material.diffuse, texCoords).rgb, vec3(2.2));

    // 5. 漫反射（Lambert 定律）与高光（Blinn-Phong）。
    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(N, H), 0.0), material.shininess);

    // 6. 点光源距离衰减。
    float lightDistance = length(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float attenuation = 1.0 / (1.0 + 0.22 * lightDistance + 0.20 * lightDistance * lightDistance);

    // 7. 合成光照：环境光 + (漫反射 + 高光) × 衰减。
    vec3 ambient = light.ambient * diffuseColor;
    vec3 diffuse = light.diffuse * diff * diffuseColor;
    vec3 specular = light.specular * spec;
    vec3 lighting = ambient + (diffuse + specular) * attenuation;

    // 8. gamma 校正：线性光照结果映射回 sRGB 显示空间。
    vec3 finalColor = pow(lighting, vec3(1.0 / 2.2));

    FragColor = vec4(finalColor, 1.0);
}
```





#### 4.从基础视差到“陡峭视差 (Steep Parallax)”与“视差遮挡 (Parallax Occlusion)”

基础视差贴图有一个致命缺点：**当高度图的凹陷非常深、或者视线极度倾斜时，单次采样计算出来的偏移会严重失真，产生类似锯齿状的撕裂。**

为了解决这个问题，现代图形学演化出了两大高级变体：

##### 1. 陡峭视差映射 (Steep Parallax Mapping)

- **原理：** 既然单次采样不准，我们就把视线光线**切成 N\*N\* 个等长的步长（比如 10 层）**。
- **过程：** 在循环中，一层一层往下走，直到某一层的高度低于当前层的深度值，立刻停下，把这个交点作为最终的 UV。
- **效果：** 凹陷处的几何遮挡变得非常真实，砖缝里能看到真正的阴影层次。

##### 2. 视差遮挡映射 (Parallax Occlusion Mapping, POM)

- **原理：** 结合了陡峭视差的“多层步进”，并在找到前后两层（穿透点前后）之后，**在两层之间进行一次线性插值（Linear Interpolation）**。
- **效果：** 彻底消除了陡峭视差的分层阶梯感，达到了近乎 100% 逼真的微观几何错位效果（3A 游戏标准配置）。

------



#### 💡 视差贴图核心总结

1. **核心目标：** 解决法线贴图“边缘依然是平整纸片”的穿帮问题。
2. **底层武器：** 一张黑白**高度图（Height Map）**。
3. **实现秘诀：** 在片段着色器中，利用**视线方向与高度值**动态偏移 UV 坐标，让像素在视觉上发生“物理错位”。





## 第二十七章.HDR 与 Bloom（泛光特效）

在现实世界中，太阳光的亮度可以是烛光的几万倍，人眼可以极其舒适地适应从极黑到极亮的跨度。但在传统的电脑渲染中，我们的颜色值被死死锁死在 `[0.0, 1.0]` 的狭窄区间内（过曝的强光直接被粗暴截断为纯白色）。

而 **HDR 渲染** 允许我们用真正的浮点数来计算超高亮度的光照；紧接着的 **Bloom（泛光特效）** 则能让太阳、车灯、爆炸等强光源产生刺眼、耀眼的光晕扩散效果！



#### 1.核心思想

在默认情况下，我们的片段着色器输出的颜色（RGB）上限是 **`1.0`**：

- 黑暗的角落：`0.1`
- 普通的白墙：`0.8`
- 太阳 / 强力聚光灯：本该是 `10.0` 或 `50.0` 的极高光强。
- **灾难发生：** 因为输出上限只有 `1.0`，显卡把所有大于 `1.0` 的光照统统**“强行截断（Clamp）”**成了 `1.0`。
- **后果：** 画面中 100 瓦的灯泡和 10000 瓦的太阳看起来没有任何区别，全是一片惨白、毫无层次感的“塑料假白”。



解决方案：

- **第一步（浮点缓冲）：** 允许光照颜色突破 `1.0`（比如输出 `vec3(15.0, 15.0, 15.0)`），在创建 FBO 颜色纹理时，使用 16 位或 32 位浮点数格式（如 **`GL_RGB16F`**）来存储超高亮度，绝对不截断！
- **第二步（色调映射 Tone Mapping）：** 因为我们的电脑显示器只能显示 `0.0 ~ 1.0` 的标准 LDR 范围，所以在最后一步，我们通过数学公式，**把超高亮度的 HDR 颜色“压缩”回 `[0.0, 1.0]` 的显示范围内，同时完美保留暗部细节和亮部层次！**



#### 2.HDR浮点帧缓冲（Float FBO）

要玩转 HDR，第一步是创建一个支持浮点数的 FBO：

```c++
unsigned int hdrFBO;
glGenFramebuffers(1, &hdrFBO);
glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

// 1. 创建浮点数颜色纹理附件-保存场景和物体 (关键：内部格式使用 GL_RGB16F 代替普通的 GL_RGB8)
unsigned int colorBuffer;
glGenTextures(1, &colorBuffer);
glBindTexture(GL_TEXTURE_2D, colorBuffer);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);

//2.创建浮点数颜色纹理附件-保存亮度
unsigned int brightnessBuffer;
glGenTextures(1, &brightnessBuffer);
glBindTexture(GL_TEXTURE_2D, brightnessBuffer);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, brightnessBuffer, 0);

// 3. 创建标准的深度缓冲 RBO
unsigned int rboDepth;
glGenRenderbuffers(1, &rboDepth);
glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
glStorageRenderbuffer(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

//同时启用两个颜色纹理附件
//opengl以及glsl规范规定
//glVertexAttribPointer → 配置顶点属性 location 
//glDrawBuffers         → 配置片段颜色输出 location
GLenum targets[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
glDrawBuffers(2, targets);

if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "HDR Framebuffer not complete!" << std::endl;
glBindFramebuffer(GL_FRAMEBUFFER, 0);
```



#### 3.色调映射（Tone Mapping）

在最后一步全屏渲染（Screen Quad）的片段着色器中，我们对超高亮度的 HDR 颜色进行压缩：

```glsl
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D hdrBuffer;
uniform float exposure; // 曝光度控制

void main() {
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
    
    // 1. 色调映射公式 (Tone Mapping) - 这里使用最经典的 Reinhard 算子，或者高级的 ACES 曲线
    // 它的数学本质：把任何极大的数 x，映射成 x / (x + 1)，让它永远无限趋近于 1.0 却永不溢出！
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    
    // 2. 结合曝光度调整 (Exposure)
    // mapped = vec3(1.0) - exp(-hdrColor * exposure); // 曝光公式
    
    // 3. 经典的 Gamma 校正 (2.2)
    mapped = pow(mapped, vec3(1.0 / 2.2));
    
    FragColor = vec4(mapped, 1.0);
}
```



#### 4.Bloom（泛光特效）

有了 HDR 之后，太阳的亮度变成了 `20.0`。但如果直接显示在屏幕上，它依然只是一个很亮的像素点。 **Bloom 的使命，就是让这个强光点“晕染扩散开来”，变成刺眼的光晕。**

- **亮部提取（Bright Pass）：** 扫描整张 HDR 画面，把亮度超过阈值（如亮度 >1.0>1.0）的强光挑出来，其余暗部抹成纯黑。
- **降采样（Downsampling）：** 把亮部图缩小（比如缩到原图的 1441），极大降低模糊时的显卡采样开销。
- **可分离高斯模糊（Separable Gaussian Blur）：** 利用高斯函数的数学特性，将 N×N的复杂计算拆解为**“先做一次水平模糊，再做一次垂直模糊”**。
- **Ping-Pong FBO 迭代：** 借助两个交替的 FBO 来回倒腾 3~5 次，让光晕极其平滑、均匀地向四周扩散开。



##### Ping-pong FBO

```c++
//创建两个FBO,两个纹理
GLuint ping[2], ptex[2];
glGenFramebuffers(2, ping);
glGenTextures(2, ptex);

// 创建两个颜色附件：完整场景和亮部提取结果。
for (int i = 0; i < 2; ++i) {
    glBindFramebuffer(GL_FRAMEBUFFER, ping[i]);
    glBindTexture(GL_TEXTURE_2D, ptex[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, W, H, 0, GL_RGBA, GL_FLOAT,nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,ptex[i], 0);
}
 glBindFramebuffer(GL_FRAMEBUFFER, 0);
```



##### 亮度提取(objectShader.frag)

```glsl
#version 330 core
layout(location=0) out vec4 FragColor; 
layout(location=1) out vec4 BrightColor;

in vec3 FragPos; 
in vec3 Normal; 
in vec2 TexCoords; 
uniform sampler2D diffuseMap;
uniform vec3 lightPos; 
uniform vec3 viewPos;

void main(){
    
    //法线向量和入射向量
    vec3 N=normalize(Normal);
    vec3 L=normalize(lightPos-FragPos);

    //距离衰减因子
    float dist=length(lightPos-FragPos);
    float att=1.0 / (1.0 + 0.22*dist + 0.20*dist*dist);

    //漫反射贴图颜色
    vec3 diffuseColor=pow(texture(diffuseMap,TexCoords).rgb,vec3(2.2));

    //漫反射因子
    float d=max(dot(N,L),0.0);

    //最终颜色
    vec3 color=0.08*diffuseColor+diffuseColor*d*att;

    FragColor=vec4(color,1.0);

    //亮度计算公式
    float brightness=dot(color,vec3(0.2126,0.7152,0.0722));

    //亮度提取
    BrightColor=brightness>1.0?vec4(color,1.0):vec4(0.0);
}
```



##### 高斯模糊 (`blurShader.frag`)

```glsl
#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D image;
uniform bool horizontal;

// 一维高斯卷积核。中心权重最大，距离中心越远权重越小。
const float weight[5] = float[](
    0.227027,
    0.1945946,
    0.1216216,
    0.054054,
    0.016216
);

void main()
{
    // 将一个像素转换成 UV 距离，例如 800 像素宽时 x 为 1.0 / 800.0。
    vec2 texelSize = 1.0 / textureSize(image, 0);

    // 先采样卷积核中心，也就是当前像素。
    vec3 result = texture(image, TexCoords).rgb * weight[0];

    // 采样中心两侧各四个像素，共组成 9 个采样点。
    for (int i = 1; i < 5; ++i)
    {
        // 水平模糊只改变 UV.x，垂直模糊只改变 UV.y。
        vec2 offset = horizontal
            ? vec2(texelSize.x * float(i), 0.0)
            : vec2(0.0, texelSize.y * float(i));

        // 正负方向对称采样，并乘以相同的高斯权重。
        result += texture(image, TexCoords + offset).rgb * weight[i];
        result += texture(image, TexCoords - offset).rgb * weight[i];
    }

    // 输出当前方向的模糊结果，供下一轮或最终 Bloom 合成使用。
    FragColor = vec4(result, 1.0);
}
```



现在，我们手头上有两张图：

1. **原版未模糊的 HDR 场景图**（里面有正常的暗部、细节和未扩散的强光点）。
2. **经过高斯模糊后的 Bloom 光晕图**（一张周围带着耀眼光晕的柔和发光图）。

我们在最后的屏幕全屏渲染（Screen Quad）中，把它们**相加混合（Additive Blending）**，然后送入色调映射.

finalShader.frag 

```glsl
#version 330 core
out vec4 FragColor; 
in vec2 TexCoords; 
uniform sampler2D hdrScene; 
uniform sampler2D bloomBlur; 
uniform float exposure;

//ACES电影级别色调调色
vec3 ACESFilm(vec3 x)
{
    float a=2.51,b=0.03,c=2.43,d=0.59,e=0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e),0,1);
}

void main()
{
    vec3 hdr=texture(hdrScene,TexCoords).rgb+texture(bloomBlur,TexCoords).rgb;
    vec3 color=ACESFilm(hdr*exposure);
    
    //gamma校正
    FragColor=vec4(pow(color,vec3(1.0/2.2)),1);
}

```



渲染循环：

```c++
while (!glfwWindowShouldClose(window)) {
    float now = (float)glfwGetTime();
    dt = now - last;
    last = now;
    input(window);

    //观察矩阵和投影矩阵
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
```





## 第二十八章.延迟渲染（**Deferred Shading**）

​		在前面的所有章节中，我们使用的都是 **前向渲染 (Forward Rendering)**：画一个物体，就把它的材质、法线、纹理以及**场景里的所有光源（比如 100 个灯光）**全部塞进片段着色器里算一遍。

- **灾难：** 如果场景里有 10001000 个动态光源，前向渲染的复杂度直接飙升到 O(Objects×Lights)*O*(Objects×Lights)，显卡瞬间卡死成 PPT。

而 **延迟渲染 (Deferred Shading)** 的出现，彻底颠覆了传统的渲染逻辑：它把光照计算**“延迟”**到最后一步，通过 **G-Buffer（几何缓冲）** 让屏幕上的每个像素只计算一次光照，无论场景里有多少个光源，性能依然稳如泰山！

下面我们严格按照**硬核极客技术手册**的标准，把延迟渲染的**核心痛点、G-Buffer 结构、双 Pass 架构以及它为什么能秒杀成百上千个光源**彻底扒光！



#### 1.核心思想

在传统的前向渲染中：

- 渲染一个箱子时，如果场景里有 **100100 个点光源**：
- 每一个片元（Pixel）在执行片段着色器时，都要把这 100100 个光源的距离、衰减、漫反射、高光全部循环计算一遍。
- **最致命的是：** 如果场景里有大量互相遮挡的物体（比如箱子 A 挡住了箱子 B），那些被遮挡在后面的像素，明明最终不会显示在屏幕上，但显卡依然傻乎乎地为它们计算了 100 个光源的光照！这叫 **“过度绘制 (Overdraw) 的几何灾难”**。



延迟渲染的哲学是：

> **“在第一步，我们绝对不计算任何光照！我们只把物体的【基础材质、世界坐标、法线、颜色】像拍快照一样，整整齐齐地记录在几张大显存纹理里。到了最后一步，我们只针对屏幕上最终能看到的每一个像素，精确地计算一次光照！”**

复杂度直接从 O(Objects×Lights) 降级为 O(Pixels×Lights)，彻底摆脱了几何体遮挡带来的算力浪费。



##### 延迟渲染的技术载体叫 **G-Buffer（Geometry Buffer，几何缓冲）**。 我们在 C++ 端创建一个拥有**多个颜色附件（Multiple Render Targets, MRT）**的超级 FBO，在 Pass 1 把整个场景的几何信息“拍快照”存进 4 张纹理里：

```
				┌────────────────────────────────┐
                 │    G-Buffer (多渲染目标 FBO)    │
                 └───────────────┬────────────────┘
     ┌──────────────────┬────────┴─────────┬──────────────────┐
     ▼                  ▼                  ▼                  ▼
┌─────────┐        ┌─────────┐        ┌─────────┐        ┌─────────┐
│gPosition│        │ gNormal │        │ gAlbedo │        │gSpecular│
│(世界坐标)│        │(世界法线)│        │(固有颜色)│        │(高光强度)│
└─────────┘        └─────────┘        └─────────┘        └─────────┘
```

| G-Buffer 纹理槽位              | 数据格式     | 存储的内容                                    |
| :----------------------------- | :----------- | :-------------------------------------------- |
| **`gPosition` (Attachment 0)** | `GL_RGBA16F` | 当前像素在**世界空间下的 3D 坐标 (X,Y,Z)**    |
| **`gNormal` (Attachment 1)**   | `GL_RGBA16F` | 当前像素在**世界空间下的法线向量 (Nx,Ny,Nz)** |
| **`gAlbedo` (Attachment 2)**   | `GL_RGBA8`   | 当前像素的**固有漫反射颜色 (R,G,B)**          |
| **`gSpecular` (Attachment 3)** | `GL_RGBA8`   | 当前像素的**高光反射强度 (Spec)**             |



#### 2.实现代码

#####  C++ 端：配置 G-Buffer 帧缓冲 (MRT)

```c++
unsigned int gBuffer;
glGenFramebuffers(1, &gBuffer);
glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

unsigned int gPosition, gNormal, gAlbedo, gSpecular;

// 1. 位置纹理 (使用 16 位浮点数 GL_RGBA16F 保证 3D 坐标精度)
glGenTextures(1, &gPosition);
glBindTexture(GL_TEXTURE_2D, gPosition);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

// 2. 法线纹理 (同样使用 GL_RGBA16F 保存精度)
glGenTextures(1, &gNormal);
glBindTexture(GL_TEXTURE_2D, gNormal);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

// 3. 颜色纹理 (使用标准 GL_RGBA8 即可)
glGenTextures(1, &gAlbedo);
glBindTexture(GL_TEXTURE_2D, gAlbedo);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

// 4. 高光强度纹理
glGenTextures(1, &gSpecular);
glBindTexture(GL_TEXTURE_2D, gSpecular);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gSpecular, 0);

// 👈 极其关键：显式通知 OpenGL，这个 FBO 有 4 个颜色输出通道！
unsigned int attachments[4] = { 
    GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 
};
glDrawBuffers(4, attachments);

// 5. 绑定深度 RBO 供几何 Pass 进行深度测试 (Z-Testing)
unsigned int rboDepth;
glGenRenderbuffers(1, &rboDepth);
glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "G-Buffer Framebuffer not complete!" << std::endl;
glBindFramebuffer(GL_FRAMEBUFFER, 0);
```



##### 着色器端：

##### Pass 1: 几何着色器

在这步，**绝对不写任何光照公式**！只把 3D 物体的顶点和材质填进 4 张纹理里：

gBuffer.vert

```glsl
# version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    vs_out.FragPos = vec3(worldPos);
    
    // 计算世界空间法线
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vs_out.Normal = normalize(normalMatrix * aNormal);
    vs_out.TexCoords = aTexCoords;

    gl_Position = projection * view * worldPos;
}
```



gBuffer.frag    **- 多重渲染目标 MRT**

```glsl
# version 330 core
// 👈 核心：使用片段输出位置 layout(location = N)，同时吐出给 4 张纹理！
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gSpecular;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main() {
    // 1. 输出世界空间坐标到 Attachment 0
    gPosition = fs_in.FragPos;
    // 2. 输出世界空间法线到 Attachment 1
    gNormal = normalize(fs_in.Normal);
    // 3. 输出漫反射颜色到 Attachment 2
    gAlbedo = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    // 4. 输出高光强度到 Attachment 3
    gSpecular = texture(texture_specular1, fs_in.TexCoords).rrr;
}
```



##### Pass 2 光照着色器 （全屏后处理聚合光照）

我们画一个覆盖屏幕的 2D 矩形（Screen Quad）。 片段着色器**不再读取任何 3D 几何模型，而是直接去读刚才画好的 4 张 G-Buffer 纹理**，一口气算完所有光源：

deffered_lighting.frag

```glsl
# version 330 core
out vec4 FragColor;
in vec2 TexCoords;

// 👈 读取 Pass 1 拍下的 4 张快照纹理
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gSpecular;

struct Light {
    vec3 Position;
    vec3 Color;
    float Linear;
    float Quadratic;
};
const int NR_LIGHTS = 100; // 支持成百上千个动态光源！
uniform Light lights[NR_LIGHTS];
uniform vec3 viewPos;

void main() {
    // 1. 从 G-Buffer 中解包出当前像素的几何与材质属性
    vec3 FragPos   = texture(gPosition, TexCoords).rgb;
    vec3 Normal    = texture(gNormal, TexCoords).rgb;
    vec3 Albedo    = texture(gAlbedo, TexCoords).rgb;
    float Specular = texture(gSpecular, TexCoords).r;

    // 2. 基础环境光
    vec3 lighting = Albedo * 0.1;
    vec3 viewDir  = normalize(viewPos - FragPos);

    // 3. 遍历场景里的所有光源，累加光照 (Blinn-Phong)
    for(int i = 0; i < NR_LIGHTS; ++i) {
        // 光源衰减计算
        float distance = length(lights[i].Position - FragPos);
        float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * (distance * distance));
        
        // 漫反射 (Diffuse)
        vec3 lightDir = normalize(lights[i].Position - FragPos);
        float diff = max(dot(Normal, lightDir), 0.0);
        vec3 diffuse = lights[i].Color * diff * Albedo * attenuation;
        
        // 镜面高光 (Specular)
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
        vec3 specular = lights[i].Color * spec * Specular * attenuation;
        
        lighting += diffuse + specular;
    }

    FragColor = vec4(lighting, 1.0);
}
```



#### 3.工业级深度：延迟渲染的局限与三大缺点

延迟渲染虽然能轻松支持成百上千个光源，但它并非完美无缺，工业界为了解决它的缺陷付出了巨大的努力：

1. 无法原生支持半透明物体（Blending 灾难）：
   - **原因：** G-Buffer 的每个像素点只能保存**一个物体的几何快照**。如果有半透明玻璃遮挡在墙面前，G-Buffer 无法存下两层物体的位置和法线。
   - **工业级解决方案：** 先用延迟渲染画完所有不透明物体，**最后再用传统的前向渲染（Forward Rendering）把半透明物体叠加画在最上面**（混合架构）。
2. 巨大的显存带宽压力（Bandwidth Heavy）：
   - 在 1080P 下，同时读写 4 张 16 位浮点纹理，对显存带宽的吞吐量要求极高。
3. 不能直接使用 MSAA 硬件抗锯齿：
   - **原因：** 多重采样纹理在 G-Buffer 的多个 Attachment 间难以直接解包。
   - **工业级解决方案：** 配合后处理抗锯齿算法（如 **FXAA** 或 **TAA 时间序列抗锯齿**）。



#### 4.全屏 Quad（Screen Quad）

**从 HDR、Bloom、高斯模糊一直到刚才的延迟渲染光照 Pass，全场都在疯狂使用一个叫“全屏 Quad（Screen Quad / VAO）”的东西。**为什么每个高级特效的最后一步，都必须要搞一个覆盖全屏的“正方形纸片（Quad）”拍在屏幕上？



在计算机显卡眼里，它只认一种东西：**几何图元（顶点、三角形、线段）。**

- 当你给显卡发送一组 3D 顶点时，光栅化器会把它们变成一个个三角形，然后执行片段着色器。
- 但如果你不做任何几何体绑定，直接告诉显卡：“喂，请帮我把这张渲染好的 HDR 纹理拿来做高斯模糊 / 做色调映射 / 做延迟光照聚合”，显卡会一头雾水：
  - *“大哥，你想让我把这个特效画在 3D 世界的哪个位置？你想让我触发屏幕上的哪几个像素？”*
  - 显卡没有“自动对全屏每个像素执行一次片段着色器”的默认指令。



为了强行让显卡对屏幕上的每一个像素执行一次我们写好的后处理着色器（比如高斯模糊、色调映射、延迟光照），程序员们发明了一个极其聪明、甚至带点“物理外挂”性质的绝招——**全屏 Quad（全屏正方形）**。

我们在 C++ 端硬编码一个**刚好铺满整个屏幕裁剪空间（NDC 坐标 `[-1, 1]`）的 2D 正方形（由 2 个三角形、6 个顶点组成）**：

当我们在 C++ 里把这个全屏 Quad 发送给显卡时，发生了以下物理级奇迹：

1. **顶点着色器（VS）极其简单：** 它直接把这 4 个角（`(-1,-1)`, `(1,-1)`, `(1,1)`, `(-1,1)`）原封不动地输出给屏幕，并且把 UV 坐标完美对应设为 `(0,0)` 到 `(1,1)`。
2. **光栅化器（Rasterizer）被强制开工：** 因为这个正方形刚好覆盖了整个屏幕的每一个像素（从 `0,0` 到 `1920,1080`），光栅化器开始疯狂工作，把这个正方形切成无数个微小的片段（Fragments）。
3. **片段着色器（FS）大显身手：** **全屏里每一个像素，都会触发一次片段着色器！** 而在片段着色器内部，我们就可以拿着对应的 UV 坐标，去尽情地采样上一阶段生成的 FBO 纹理（如 `hdrBuffer`、`gPosition`、`gNormal`），在当前像素上做各种数学魔法（比如色调映射、高斯模糊、多光源光照累加）！



回过头来看我们学过的高级效果，**全屏 Quad（Screen Quad）** 几乎无处不在：

1. **在 HDR 中：** 我们在 FBO 里算好了高光浮点场景，然后**把全屏 Quad 贴在屏幕上**，在 FS 里用 ACES 公式把浮点数压缩成 LDR。
2. **在高斯模糊 / Bloom 中：** 我们把亮部图贴在全屏 Quad 上，在 FS 里做水平/垂直偏移采样。
3. **在延迟渲染的 Lighting Pass 中：** 最绝的一幕来了——**我们在世界空间里甚至不需要画任何 3D 模型！** 我们直接在屏幕正中央画一个巨大的全屏 Quad，在 FS 里同时读取 G-Buffer 的 4 张快照（位置、法线、颜色），把场景里 1000 个光源的循环全部写在这个全屏着色器里，一气呵成把整张画面算出来！

无论是 HDR 的色调映射、Bloom 的高斯模糊光晕，还是延迟渲染的百个光源聚合，它们的本质全都是： **“用一个永远贴在相机眼前的 2D 正方形纸片（Screen Quad）作为画布，把上一阶段渲染出来的离屏 FBO 纹理作为颜料，在片段着色器里对全屏像素进行重塑和再加工！”**





## 第二十九章.SSAO(屏幕空间环境光遮蔽)

在前面的光照模型（无论是前向渲染还是延迟渲染）中，我们都有一个巨大的痛点：**阴影虽然有了，但物体表面的“拐角处、缝隙处、两个物体紧密相交的阴暗死角”（比如墙角、桌子腿和地面的接缝处）依然显得光照过于均匀、不够真实。**

在现实中，这些狭窄的缝隙很难让光线射进去（环境光被严重遮挡），所以它们天然应该呈现出一团柔和、细腻的暗影。这就是**环境光遮蔽（Ambient Occlusion, AO）**。 而 **SSAO** 的天才之处在于：**它不需要复杂的射线追踪，直接在屏幕空间（Screen Space）里，利用深度缓冲和半球随机采样，实时模拟出这种极其逼真的拐角阴影！**



#### 1.核心思想

- **传统环境光的局限：** 在冯氏光照或 PBR 中，环境光（Ambient）通常是一个全局统一的常数（例如 `vec3(0.1) * color`）。这意味着，不管是开阔的墙面，还是极其狭窄的墙角缝隙，它们受到的环境光照射强度一模一样。
- **视觉上的假：** 现实生活中，光线在物体缝隙里会来回反弹、衰减，导致**所有凹陷、夹角、接触面都会产生一团天然的柔和阴影**。没有这层阴影，物体就会脱离环境，显得像浮在空中一样生硬。
- **传统离线 AO 的代价：** 美工在建模时把 AO 烘焙到纹理（Ambient Occlusion Map）里。但如果场景是动态的、或者物体在移动，烘焙纹理就彻底失效了。



**SSAO（Screen-Space Ambient Occlusion）** 是由大名鼎鼎的 Crytek 公司在开发《孤岛危机》时发明的神级实时算法。它的核心思想是：

> **“我不去管 3D 复杂的全局几何体。我只站在当前像素的视角，向它周围的半球空间发射好几条随机射线，看看周围有没有被其他几何物体挡住（即采样到的深度是不是比我的距离更近）。如果周围全被挡住了，说明这里是个深深的死角，环境光就应该减弱（变暗）！”**

因为这一切计算只发生在当前屏幕的像素范围内（利用延迟渲染的 G-Position 和 G-Normal 纹理），所以它叫**屏幕空间（Screen-Space）** AO。



#### 2.SSAO 实现代码

要在屏幕上实时算出生动的 SSAO 暗影，管线分为四个步骤：



##### 1.**C++ 端：生成半球采样核心（Kernel）与随机噪声纹理（Noise Texture）。**

在 CPU 端，我们在以原点 `(0,0,0)` 为中心的**半球（Hemisphere）**内部，随机生成64 个采样点向量（Sample Kernel）。这些向量主要集中在法线正方向，用来模拟从表面向四周发射的探测视线。



##### 半球采样向量（Sample Kernel）

```c++
# include <vector>
# include <random>
# include <glm/glm.hpp>

std::vector<glm::vec3> ssaoKernel;
std::uniform_real_distribution<GLfloat> randomFloats(0.05.0f, 1.0f); // 随机小数生成器
std::default_random_engine generator;

for (unsigned int i = 0; i < 64; ++i)
{
    // 在半球内随机生成向量 (Z 轴朝上，确保采样点都在表面外侧)
    glm::vec3 sample(
        randomFloats(generator) * 2.0f - 1.0f,
        randomFloats(generator) * 2.0f - 1.0f,
        randomFloats(generator) // 保证 Z 轴分量为正，朝向半球上方
    );
    sample = glm::normalize(sample);
    sample *= randomFloats(generator); // 赋予随机长度
    
    // 权重的非线性缩放：让采样点更多地聚集在原点附近（越近越精确）
    float scale = (float)i / 64.0f;
    scale = glm::mix(0.1f, 1.0f, scale * scale);
    sample *= scale;
    
    ssaoKernel.push_back(sample);
}
```



##### 生成 4×4 随机旋转噪声纹理

因为 64 个采样点太少了，直接渲染会有巨大的马赛克噪点。我们用一张极小的 4×4旋转向量纹理，在每个像素处把半球随机旋转：

```c++
std::vector<glm::vec3> ssaoNoise;
for (unsigned int i = 0; i < 16; i++)
{
    // 在切线空间的 XY 平面上生成随机旋转向量 (Z = 0)
    glm::vec3 noise(randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) * 2.0f - 1.0f, 0.0f);
    ssaoNoise.push_back(noise);
}

unsigned int noiseTexture;
glGenTextures(1, &noiseTexture);
glBindTexture(GL_TEXTURE_2D, noiseTexture);
// 注意：使用 GL_RGB16F，且环绕模式设为 GL_REPEAT 以便在屏幕上平铺
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
```



##### 2.SSAO 计算着色器（核心遮挡判定 `ssao.fs`）

这是全场数学计算最密集的地方。我们利用延迟渲染导出的 `gPosition` 和 `gNormal`，逐像素进行半球射线检测：

```glsl
# version 330 core
out float FragColor; // 输出单通道的 AO 遮挡因子 (0.0 = 全黑死角, 1.0 = 无遮挡)

in vec2 TexCoords;

uniform sampler2D gPosition; // 延迟渲染的世界坐标快照
uniform sampler2D gNormal;   // 延迟渲染的世界法线快照
uniform sampler2D texNoise;  // 4x4 随机旋转噪声纹理

uniform vec3 samples[64];    // CPU 传过来的 64 个半球采样点
uniform mat4 projection;     // 玩家摄像机投影矩阵

// 屏幕分辨率与 4x4 噪声纹理大小的比例 (用于在全屏上平铺噪声)
const vec2 noiseScale = vec2(800.0/4.0, 600.0/4.0); // 假设屏幕是 800x600

void main() {
    // 1. 从 G-Buffer 中读取当前像素的世界坐标与法线
    vec3 fragPos = texture(gPosition, TexCoords).xyz;
    vec3 normal  = normalize(texture(gNormal, TexCoords).xyz);
    
    // 如果法线为零（说明是天空盒背景），直接满分无遮挡返回 1.0
    if(length(normal) == 0.0) {
        FragColor = 1.0;
        return;
    }

    // 2. 采样噪声纹理，得到当前像素的随机旋转向量
    vec3 randomVec = texture(texNoise, TexCoords * noiseScale).xyz;
    
    // 3. 利用 Gram-Schmidt 正交化，构建以当前法线为基准的 TBN 旋转矩阵
    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN       = mat3(tangent, bitangent, normal);
    
    // 4. 遍历 64 个半球采样点，检测遮挡
    float occlusion = 0.0;
    float radius = 0.5;   // 采样半径 (控制阴影扩散的物理范围)
    float bias   = 0.025; // 深度偏置 (防止自阴影粉刺)

    for(int i = 0; i < 64; ++i) {
        // A. 将采样点从切线空间转换到世界空间，并放置在当前像素 fragPos 附近
        vec3 samplePos = TBN * samples[i]; 
        samplePos = fragPos + samplePos * radius;
        
        // B. 把这个 3D 采样点投影回屏幕空间，去采 gPosition 纹理，获取其实际几何深度！
        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset; // 投射到裁剪空间
        offset.xyz /= offset.w;       // 透视除法
        offset.xyz = offset.xyz * 0.5 + 0.5; // 映射到 [0, 1] 的屏幕 UV 空间
        
        // C. 获取该采样点正下方场景中实际几何体的深度值 (Z 坐标)
        float sampleDepth = texture(gPosition, offset.xy).z; 
        
        // D. 核心遮挡判定：
        // 如果场景里的实际深度 比 我们的半球采样点还要靠里（说明有物体挡在前面），
        // 并且距离差在合理范围内（避免远处毫不相干的物体产生错误遮挡）：
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        if (sampleDepth >= samplePos.z + bias) {
            occlusion += 1.0 * rangeCheck; // 产生遮挡！加重阴影
        }
    }
    
    // 5. 算出最终的遮挡系数并反转 (0.0 = 全遮挡变暗, 1.0 = 无遮挡)
    occlusion = 1.0 - (occlusion / 64.0);
    FragColor = occlusion;
}
```



##### 3.SSAO 模糊着色器（消除颗粒感 `ssao_blur.fs`）

由于 64 个采样点在屏幕上会留下非常明显的噪点，我们必须用一个简单的 2×22×2 模糊滤镜对 SSAO 纹理进行平滑：

```glsl
# version 330 core
out float FragColor;
in vec2 TexCoords;

uniform sampler2D ssaoInput; // 刚算出来的粗糙 SSAO 纹理

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
    float result = 0.0;
    
    // 采集周围 2x2 邻域求平均值
    for (int x = -2; x < 2; ++x) {
        for (int y = -2; y < 2; ++y) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssaoInput, TexCoords + offset).r;
        }
    }
    FragColor = result / (4.0 * 4.0);
}
```



##### 4.光照聚合阶段（将 AO 乘入环境光）

在最后的光照 Pass（或者延迟渲染的 Lighting Pass）中，我们将这张平滑后的 SSAO 遮挡图直接**乘以环境光（Ambient）**：

```glsl
vec3 Albedo = texture(gAlbedo, TexCoords).rgb;
float AmbientOcclusion = texture(ssaoBlur, TexCoords).r; // 👈 采样平滑后的 AO 图

// 核心融合：环境光被 AO 因子严重衰减，墙角和缝隙瞬间陷入柔和的暗影！
vec3 ambient = vec3(0.3) * Albedo * AmbientOcclusion; 
```



##### SSAO 全套工业级总结

1. **CPU 准备：** 生成 64 个半球采样点和 4×44×4 随机旋转噪声。
2. **SSAO 计算 Pass：** 逐像素利用 G-Buffer 的位置和法线，构建 TBN 矩阵，发射半球射线比对深度。
3. **模糊 Pass：** 用双边/邻域模糊抹平讨厌的噪点。
4. **光照融合：** `Ambient *= AO`，完美实现拐角与死角的物理级立体死角阴影。
