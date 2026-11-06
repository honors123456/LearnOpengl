# 网格生成与 OBJ 读取

两个工具均为 C++17 仅头文件实现，命名空间为 `marmoset`，复用现有 GLM。不调用 Qt、OpenGL，不上传 GPU，也不加载纹理。

## 公共数据

`Geometry/MeshGenerator.h` 定义：

- `Vertex`：`glm::vec3 position`、`glm::vec3 normal`、`glm::vec2 uv`。
- `Material`：线性空间 `albedo`、`metallic`、`roughness`、`ao` 和 HDR `emissive`，均为纯数值。
- `MeshData`：顶点数组、`std::uint32_t` 三角形索引数组与一个默认材质。
- `MeshGenerator`：静态生成函数和经过范围检查的 `material()`。

材质在每个 Mesh 上保存一份，不逐顶点复制，不包含纹理 ID。已有 `Material` 结构可直接赋值；使用 `MeshGenerator::material()` 或传入生成函数时会验证数值，颜色和三个标量范围为 [0,1]，发光颜色非负且可超过 1。粗糙度允许 0，实际 BRDF 在除法处应自行处理数值边界。

顶点 stride 必须使用 `sizeof(Vertex)`，成员偏移使用 `offsetof(Vertex, position)` 等，不能假设 GLM 的对齐模式。位置、法线、UV 可分别对应 attribute location 0、1、2；索引使用 `GL_UNSIGNED_INT`。没有预生成切线，法线贴图接入时按需要计算。

## 程序化网格

```cpp
#include "Geometry/MeshGenerator.h"

using namespace marmoset;

const auto copper = MeshGenerator::material({0.95f, 0.64f, 0.54f}, 1.0f, 0.25f);
auto sphere = MeshGenerator::sphere(1.0f, 64, 32, copper);
auto disk = MeshGenerator::circle(1.0f, 64);
auto cube = MeshGenerator::cube(1.0f);
auto box = MeshGenerator::box({1.0f, 2.0f, 3.0f});
auto torus = MeshGenerator::torus(1.0f, 0.25f, 64, 24);
auto plane = MeshGenerator::plane(10.0f, 10.0f, 32, 32);
auto cylinder = MeshGenerator::cylinder(1.0f, 2.0f, 64, true);
auto cone = MeshGenerator::cone(1.0f, 2.0f, 64, true);
auto terrain = MeshGenerator::terrain(20.0f, 20.0f, 128, 128,
    [](float x, float z) { return 0.5f * std::sin(x) * std::cos(z); });

// 也可以在生成或导入后赋予一个数值材质。
terrain.material = MeshGenerator::material({0.18f, 0.32f, 0.12f}, 0.0f, 0.9f);
```

| 函数 | 空间与几何约定 |
|---|---|
| `sphere(radius, segments, rings, material)` | 原点居中，Y 轴为极轴；经向 UV 接缝重复顶点，极点不输出退化三角形 |
| `circle(radius, segments, material)` | XY 平面上的实心圆盘，法线 +Z；不是线框圆 |
| `cube(size, material)`、`box(size, material)` | 原点居中；六个面独立顶点，保留硬边法线与各面 UV |
| `torus(majorRadius, minorRadius, majorSegments, minorSegments, material)` | 围绕 Y 轴；要求主半径大于管半径，不生成自交圆环 |
| `plane(width, depth, xSegments, zSegments, material)` | XZ 平面，Y=0，法线 +Y |
| `terrain(width, depth, xSegments, zSegments, height, material)` | XZ 高度场；回调使用实际 X/Z 坐标且每顶点调用一次，法线由网格面积加权计算 |
| `cylinder(radius, height, segments, capped, material)` | 沿 Y 轴，原点居中，可选顶底盖，盖面与侧面法线分离 |
| `cone(radius, height, segments, capped, material)` | 底面 Y=-height/2，顶点 Y=+height/2；可选底盖 |

所有索引按外向 CCW 三角形组织，UV 通常为 [0,1]。尺寸必须为有限正值；圆周细分至少 3、球体纬向至少 2、网格各轴细分至少 1。非法参数抛 `std::invalid_argument`，超过 32 位顶点/索引计数抛 `std::length_error`。调用方仍需根据内存和 GPU 能力限制实际细分数，内存分配失败遵循标准容器异常。

## OBJ

```cpp
#include "Geometry/ObjLoader.h"

auto model = marmoset::ObjLoader::load("assets/models/example.obj");
auto &mesh = model.mesh;
mesh.material = marmoset::MeshGenerator::material({0.7f, 0.7f, 0.7f}, 0.0f, 0.5f);

marmoset::ObjLoader::Options options;
options.flipV = true; // 默认 false，不自动翻转纹理 V。
auto imported = marmoset::ObjLoader::load("assets/models/example.obj", options);

// 也可从已有输入流或字符串读取。
auto triangle = marmoset::ObjLoader::fromString(
    "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n");
```

文件入口使用 `std::filesystem::path`，Windows 中文路径可传宽字符串路径或 `std::filesystem::u8path()`。读取和解析失败抛 `std::runtime_error`，解析错误带 OBJ 源行号。调用不改变当前工作目录、不写文件、不加载外部材质文件。

支持范围：

- `v`、`vt`、`vn` 和 `f`，包括 `v`、`v/vt`、`v//vn`、`v/vt/vn` 四种面引用；索引从 1 开始，负索引相对当前已读数据尾部，0 和越界引用报错。
- 三角形、四边形和简单平面凹/凸多边形；使用耳切法三角化，保持源面朝向。自交、退化、重复角点及明显非平面多边形报错，不静默生成错误三角形。大 n-gon 的耳切和交点检查开销较高，复杂模型建议导出时预先三角化。
- `v x y z [w]` 会按 w 转成三维坐标；`vt u [v [w]]` 保留前两个分量，第三纹理分量不使用。缺失 UV 填 0。顶点颜色扩展不支持，避免误将颜色解释为 w。
- `s off` / `s 0` 保留平面硬边，正数平滑组对缺失法线做按原始多边形面积加权的共享法线；UV 接缝拆分顶点但不打断同组法线平滑。源 `vn` 优先且会单位化。
- `Options::generateMissingNormals` 默认为 true，设 false 后缺失法线保留零向量，由调用方处理。
- `o`、`g`、`usemtl` 保存在 `Result::parts` 的连续索引范围中；`firstIndex`、`indexCount` 单位为索引元素，不是字节。多个组名保留为完整字符串。MTL 文件名保存在 `materialLibraries`，不会解析 MTL 或自动填写贴图、材质数值。
- 空行、注释、UTF-8 BOM、CRLF 和反斜线续行。其他语句（包括点、线、曲线/曲面）明确报不支持。

格式语义参考：[Wavefront 原始 OBJ 格式说明镜像](https://paulbourke.org/dataformats/obj/) 。此工具只实现上述多边形子集，不宣称支持整个 OBJ 规范。

## CMake 与测试

四个项目已链接仅头文件的 `MarmosetGeometry` INTERFACE 目标，可直接使用上面的 include。其他 CMake 目标用 `target_link_libraries(YourTarget PRIVATE MarmosetGeometry)`。GLM 默认使用现有 `../External/include`；其他位置可通过 `-DMARMOSET_GLM_INCLUDE_DIR=<包含 glm 目录的路径>` 指定。

```powershell
cmake -S . -B .tmp/geometry-tools/build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/Qt5.12.12/5.12.12/msvc2017_64 -DBUILD_GEOMETRY_TESTS=ON
cmake --build .tmp/geometry-tools/build --config Debug --parallel 4
ctest --test-dir .tmp/geometry-tools/build -C Debug --output-on-failure
```

测试不创建 OpenGL 上下文，覆盖几何属性、索引、朝向、法线、UV 接缝、地形斜率、材质与参数错误、OBJ 解析和平滑边界、文件错误及多翻译单元链接。临时 OBJ fixture 只写入测试构建目录。
