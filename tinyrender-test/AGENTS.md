# 项目规范

本项目是使用 Dear ImGui、GLFW 和 OpenGL 3 的独立 C++ 窗口示例，使用 CMake 构建。

- `src/main.cpp`：窗口生命周期、事件循环和界面。
- `src/ImageView.h` / `src/ImageView.cpp`：RGBA 图片上传、纹理生命周期和居中显示；GPU 接口在持有当前 OpenGL 上下文的渲染线程调用。
- `src/SoftwareRenderer.h` / `src/SoftwareRenderer.cpp`：CPU 相机矩阵、画线和三角形光栅化；drawModel 接收调用方提供的模型、Shader、画布和深度缓冲，不负责文件加载或清空缓冲。
- `src/tgaimage.h` / `src/tgaimage.cpp`：TGA 图片读写及像素访问；解码后统一为左上角原点。
- `src/Shader.h`：CPU 顶点与片元着色接口；fragment 接收透视校正后的属性权重，返回 true 绘制、false 丢弃。TexShader 引用调用方加载的模型和贴图，引用对象必须在 Shader 使用期间有效；每个面依次调用三个 vertex 后立即光栅化，drawModel 统一保证此顺序。
- `CMakeLists.txt`：依赖获取、库目标和可执行程序。
- `README.md`：当前功能及构建运行方法。
- `ROADMAP.md`：进度和验证结果。
- 使用 C++17，项目文件使用 UTF-8；GLFW / ImGui 通过 FetchContent 固定版本，Eigen 3.4.0 头文件随项目保存在 `Eigen/include/Eigen/`，不修改依赖源码。`Eigen/bin/` 和 `Eigen/lib/` 为预留目录，Eigen 不需要二进制库；目录说明见 `Eigen/README.md`。
- 构建目录为 `build/`，最终 EXE / DLL 统一放在 `bin/`；静态库、导入库和调试符号留在 `build/`。Debug / Release 共用 bin，同名产物由最后构建的配置覆盖。任务临时文件为 `.tmp/<task-slug>/`。
- 验证入口：README 中的 CMake 配置、构建命令；构建成功和实际窗口显示分别记录。
