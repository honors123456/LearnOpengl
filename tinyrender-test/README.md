# TinyRender ImGui 窗口

C++17 图片显示窗口，使用 Dear ImGui v1.91.9b、GLFW 3.4 和 OpenGL 3。初始窗口为 800 × 800，窗口内容仅显示居中的图片，没有文字、按钮或 ImGui 面板。图片保持原始比例，随窗口大小放大或缩小适配，使用最近邻采样保持像素边界清晰；未设置图片时显示空白背景。当前主程序加载 African Head OBJ，在 CPU 上生成 1024 × 1024 纹理渲染图片，再通过 `SetImage(tgaImage)` 显示。

## CPU 纹理渲染

主程序分别使用 `ParseAssimp::loadData()` 和 `TGAImage::read_tga_file()` 加载模型与贴图。调用方创建一次画布和初始化为正无穷的深度缓冲，然后调用 `SoftwareRenderer::drawModel(model, shader, framebuffer, zbuffer)` 绘制；连续绘制多个模型可以共用缓冲，函数不清空已有内容。当前入口仍只绘制头部，未加入眼球。

`TexShader` 引用已加载的模型和两张贴图，构造时不执行文件读取，调用方须保证引用对象的生命周期。顶点使用 `projection * view * model` 变换，并保存当前面的 UV；drawModel 保证三个顶点准备完毕后立即绘制当前面。片元按 UV 最近邻采样、翻转 V 并限制边界，解包模型空间法线计算漫反射。当前模型矩阵为单位矩阵；尚未加入模型法线逆转置变换或切线空间 TBN。

Shader 三角形重载接收齐次裁剪坐标，执行透视除法、屏幕映射和深度测试。NDC 深度按屏幕重心坐标插值并限制到 [-1, 1]；UV 使用 `1/w` 校正权重。fragment 返回 true 后写入颜色和深度，false 丢弃。包含非正或过小 w 的三角形整体跳过，尚未实现完整裁剪；原有固定颜色重载仍保留在 SoftwareRenderer。

## Eigen 数学库

项目内置 Eigen 3.4.0 的正式模块头文件，CMake 已为应用添加 `Eigen/include` 包含路径，不依赖原始的 D 盘源码目录。目录结构和许可证见 `Eigen/README.md`；未包含 `unsupported` 扩展模块。模型顶点、法线、屏幕坐标和重心权重使用 `Eigen::Vector3f`，纹理坐标和二维像素采样点使用 `Eigen::Vector2f`；叉积、长度及点积使用 Eigen 运算，分量通过 `x()`、`y()`、`z()` 访问。

在项目源文件中可以直接使用：

```cpp
#include <Eigen/Dense>

Eigen::Vector3f a(1.0f, 0.0f, 0.0f);
Eigen::Vector3f b(0.0f, 1.0f, 0.0f);
Eigen::Vector3f normal = a.cross(b).normalized();
Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();
```

添加依赖后需重新执行下文的 CMake 配置命令，以更新 IDE 工程的包含路径。

## 图片接口

`SoftwareRenderer::lookAt(camera, center, up)` 返回右手坐标系的世界到相机视图矩阵，使用列向量，相机朝负 Z 方向观察。用 `view * Eigen::Vector4f(x, y, z, 1)` 转换位置；camera 为相机位置，center 为观察目标，up 为参考向上方向。当前主程序通过构造函数调用此函数；调用方须提供有效相机参数。

`SoftwareRenderer::perspective(fovYRadians, aspect, nearPlane, farPlane)` 返回 OpenGL 右手透视投影矩阵，把相机空间坐标转换为裁剪空间坐标。视野角用弧度表示，aspect 为宽除以高；当前 CPU 光栅化器在裁剪后执行透视除法。有效范围为 `0 < fovYRadians < pi`、`aspect > 0`、`0 < nearPlane < farPlane`，无效参数抛出 `std::invalid_argument`。例如 60 度需传入约 `1.0472f`。

`src/ImageView.h` 提供 `ImageView::SetImage(rgba, byte_count, width, height)`、`Draw()` 和 `Clear()`。在 `src/main.cpp` 的 `ImageView image;` 后调用 `SetImage`，也可在循环中更新图片：

```cpp
// rgba points to width * height * 4 bytes supplied by your renderer.
// Channels: R, G, B, A; rows: top to bottom; no row padding.
if (!image.SetImage(rgba, byte_count, width, height)) {
    // Handle invalid input or texture allocation failure.
}
```

RGBA 接口接收连续 RGBA8 像素数据；上传后可以释放原像素缓冲区。无效指针、非正尺寸、缓冲区不足或超过 GPU 尺寸上限返回 `false`，保留已有图片。所有接口及析构在渲染线程使用，上传和清理时必须有当前 OpenGL 上下文；主程序已在关闭上下文前清理纹理。

通过 `SetImage(const TGAImage&)` 显示渲染器生成的 `TGAImage`：

```cpp
// After including "tgaimage.h":
TGAImage rendered(800, 600, TGAImage::RGB);
// Write rendered pixels with rendered.set(x, y, color).
image.SetImage(rendered);
```

图片文件直接使用 `TGAImage::read_tga_file(path)` 解码，再交给 ImageView 显示。

支持 8 位灰度、24 位 BGR 和 32 位 BGRA 的普通及 RLE TGA，不支持调色板格式。显示接口自动转换通道并保留 32 位透明度；文件读取器将各原点方向统一为左上角。

## 画线接口

SoftwareRenderer::line() 复用 Bresenham 整数算法，在传入的 TGAImage 上绘制线条。renderer.creator() 返回 256 × 256 空图片。

```cpp
SoftwareRenderer renderer;
TGAImage framebuffer(800, 800, TGAImage::RGB);
const TGAColor color{{255, 255, 255, 255}, 4};
renderer.line(10, 10, 200, 100, framebuffer, color);
image.SetImage(framebuffer);
```

## Windows 构建与运行

需要 CMake 3.24 或更高版本、Visual Studio 2022 C++ 桌面开发工具及 Windows SDK。首次配置需要访问 GitHub 下载依赖；后续依赖缓存在 `build/_deps/`。

在项目根目录执行：

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug --parallel
Push-Location bin
.\tinyrender_test.exe
Pop-Location
```

当前 main 中的 OBJ 相对路径以 `bin/` 为工作目录；IDE 调试也需要设置此工作目录。窗口需要支持 OpenGL 3.0 的图形驱动。使用系统窗口关闭按钮退出。调用显示接口传入图片前，窗口显示空白背景。

最终 EXE 和项目 DLL 统一输出到根目录 `bin/`，链接目标声明的依赖 DLL 会在构建后复制到同一目录；当前 GLFW 和 ImGui 均为静态链接，无第三方 DLL 需要复制。中间文件、依赖源码、静态库及导入库、PDB 保存在 `build/`。Debug / Release 共用 `bin/`，同名产物由最后构建的配置覆盖。

如果环境中的 CMake 下载器出现 TLS 错误，可以自行下载并解压上述固定版本源码，在配置命令中追加 `-DFETCHCONTENT_SOURCE_DIR_GLFW=<GLFW源码绝对路径>` 和 `-DFETCHCONTENT_SOURCE_DIR_IMGUI=<ImGui源码绝对路径>`，使用本地依赖。

项目规范见 [AGENTS.md](AGENTS.md)，开发状态见 [ROADMAP.md](ROADMAP.md)。
