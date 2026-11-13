# 当前目录调整

- 2026-11-11 18:00：进行中：PBR 接入与 27-PBR 一致的左键环绕和滚轮缩放，拖动灵敏度 0.2，俯仰限制 ±65°，距离限制 7～17；释放或丢失鼠标抓取后停止旋转。GUI 参数通过 synchronize 复制，render 计算相机位置及 view，以实际 FBO 尺寸计算 projection，并防止零尺寸除法。源码编码和静态调用链检查通过；未编译、未运行，实际鼠标交互和 GPU 绘制待验证。

- 2026-11-10 16:42：进行中：PBR 改为通过 Qt 函数入口显式创建、绑定和删除 VAO／VBO／EBO，首次渲染上传现有球体数据，以 `sizeof`／`offsetof` 配置位置、法线和 UV；空数据不创建资源，后续帧不重复上传。保留 EBO 与 VAO 的绑定关系，渲染器销毁时释放资源；未接入 Shader 和绘制，当前仍只清屏。未编译、未运行，GPU 上传和释放待验证。

- 2026-11-10 16:32：验证：Shader 使用的 21 个 GL 接口均存在于本机 Qt 5.12.12 头文件；源码 GB2312 往返检查、`git diff --check`、加载 MSVC 环境后的 qmake 配置生成通过。未编译、未运行；qmake 产物位于 `.tmp/qt-opengl-unify/`。

- 2026-11-10 16:32：进行中：将 Shader 的 GLAD 调用改为 Qt `QOpenGLExtraFunctions`，移除 qmake 的 GLAD 编译和 GLFW 链接，去除 Shader.pri 大小写重复条目，补齐 CMake 的 Shader 源文件入口。静态检查通过，未编译、未运行；实际上下文能力及渲染集成待验证。

- 2026-11-10 14:35：进行中：完成六项 UI 优化的 QML 实现，包括实验目标与空状态、参数范围和无效输入反馈、可调侧栏与诊断高度、专注模式、选中状态语义、GPU 实例档位与整数种子输入；修正单应用文案，同步 README 和 DESIGN。真实 C++／OpenGL 运行集成仍待验证。
- 2026-11-10 14:35：验证：Qt 5.12.12 参数控件回归 6 passed／0 failed；隔离视口替身工作台测试 3 passed／0 failed，覆盖模块参数保留、专注切换、侧栏键盘与拖动调整。已检查 1440 × 900、960 × 640 及 GPU 面板的替身运行截图，产物位于 `.tmp/ui-optimization/`。替身仅验证 QML，不证明真实 FBO、屏幕阅读器或高 DPI 行为；CMake 所需两个模块入口仍缺失，未编译应用。

- 2026-11-10 11:31：新增深色薄荷绿 M 字母应用图标，包含 SVG、256 像素 PNG 和八尺寸 ICO；`main.cpp` 设置内嵌窗口图标，qmake／CMake 配置 Windows 可执行文件图标。资源路径、SVG/XML、PNG 尺寸、ICO 全部尺寸、Qt 5.12.12 `rcc --list`、GB2312 往返和差异检查通过，已查看 PNG 图案；按用户既有要求未编译、未启动，EXE 嵌入与运行窗口显示待验证。

- 2026-11-10 09:39：进行中：四个 lab 类改为独立 `QQuickFramebufferObject`，注册 `Marmoset.Labs 1.0` 并绑定各模块的 `viewportComponent`；参数和实验索引经 `synchronize()` 复制，当前只清屏，算法仍待接入。保留默认空视口契约及现有占位组件文件，不再实例化占位组件。同步四个 lab 的 CMake 源文件入口和接入文档。
- 2026-11-10 09:39：静态验证：五个相关 QML 的 Qt 5.12.12 `qmllint`、四个视口头文件的 `moc` 解析、注册顺序与 QML／qmake／CMake 接线核对、GB2312 严格往返检查及 `git diff --check` 通过；按用户要求未编译、未运行，类型加载、实际 GL 上下文和切换行为待验证。当前 CMake 入口引用的 `Logging/CMakeLists.txt` 和 `Geometry/CMakeLists.txt` 已缺失，未修复该既有问题。

- 2026-11-04 10:18：移除 Clang 不支持的 /source-charset:gb2312 参数，MSVC 依赖已核实的系统代码页 936 读取源码，保留 /execution-charset:utf-8。13 个源码 GB2312 解码检查、qmake 强制重编译与 CMake/MSVC Debug 构建通过；Qt Creator 代码模型刷新尚未验证。

- 2026-11-04 10:16：按用户要求将全部 13 个项目 .h/.cpp 无损转为 GB2312，其他项目文本为 UTF-8；同步 AGENTS、README 和 qmake/CMake 的 MSVC 源码/执行字符集。修复 ARCHITECTURE、ROADMAP 已存在的截断 UTF-8 片段，转换前原始文件保存在 .tmp/encoding-fix/original/。57 个文本文件严格解码及往返检查通过，源码解码后文字与转换前完全一致，qmake/nmake、CMake/MSVC Debug 构建通过；未运行界面。

- 2026-11-04 09:46：补充其余 22 个 QML/C++ 源文件的中文注释，覆盖四个实验类骨架、日志接口与生命周期、公共控件、主界面状态与布局，以及几何生成/OBJ 解析算法。与修改前快照逐文件核对，除整行注释和空白外完全一致；10 个 QML 的 qmllint 及 qmake/nmake Debug 构建通过，未验证图形界面。

- 2026-11-04 09:42：main.cpp 按启动流程补充中文分段注释，说明 OpenGL 后端、应用与日志初始化、命令行校验、QML 加载、定时截图及退出处理；仅新增注释和空行，未改变执行逻辑，未重新编译或运行。

- 2026-11-04 09:40：启动逻辑合并至根 main.cpp，删除 WorkbenchApplication.h/.cpp 及未使用的注册回调参数，同步构建配置和接入文档。qmake/nmake 与 CMake/MSVC Debug 构建通过；离屏非法模块参数返回 2，未验证图形界面。

- 2026-11-04 09:37：全部模块目录及同名 .pri 完成首字母大写：Logging、Geometry、Pbr_lab、Deferred_lab、Ssao_lab、Gpu_lab。退出 Typora 后四个 lab 目录重命名成功；实际名称核对及 qmake/nmake、CMake/MSVC Debug 构建通过，未运行界面。


- 2026-11-04 09:27：移除 common 模块，workbench.qrc 移至 ui/ 并由根工程直接引用，应用 CMake 装配展开至根 CMakeLists.txt。资源文件路径检查通过，qmake/nmake 与 CMake/MSVC Debug 构建通过；未运行界面。

- 2026-11-04 09:24：日志与几何工具迁至根 logging/、geometry/，分别提供独立 .pri 和 CMakeLists.txt；common/app 下全部文件上移根目录，common 仅保留共享资源与构建装配。qmake/nmake 和 CMake/MSVC Debug 构建通过，活动构建文件中旧路径检查无残留；未运行界面。

- 2026-11-04 09:21：qmake/CMake 可执行文件统一输出到 bin/，日志默认写入 bin/logs/，对象文件、生成源码、PDB 等中间产物放入 build/。两个 Debug 构建通过，检查 bin/ 仅含可执行文件和 logs/；离屏运行非法模块参数返回 2，日志路径及错误内容验证通过。既有 .tmp 历史产物未清理。

- 2026-11-04 09:19：新增 common/Logger，使用 D:/spdlog 头文件模式，接入应用启动与 Qt/QML 日志，支持分级、多线程同步写入、5 MB 滚动和 3 个备份。qmake/nmake 与 CMake/MSVC Debug 构建通过；独立测试验证中文路径/内容、并发写入、Qt 接管、滚动及无效目录失败，退出码 0；未运行图形界面。

- 2026-11-04 09:13：删除各 lab 的 main.cpp，新增 PbrLab、DeferredLab、SsaoLab、GpuLab 的 .h/.cpp 类骨架；模块 .pri 和 CMakeLists.txt 接入同一个 Mini-Marmoset 应用，仅保留根程序入口。qmake/nmake 与 CMake/MSVC Debug 全新构建均通过，四个类参与编译；未运行界面，尚未实现算法。

- 2026-11-04 09:10：移除四个 lab .pri 中重复的 QML DISTFILES 引用，QML 统一由根工程和共享资源管理；静态核对 lab 目录无 QML，模块 .pri 无 QML 引用。

- 2026-11-04 09:09：新增根 main.cpp，主界面重命名为 ui/main.qml；同步 qmake、资源加载及 CMake Mini-Marmoset 目标。qmake/nmake 与 CMake/MSVC Debug 构建均通过，未运行界面。

- 2026-11-04 09:07：新增 common/common.pri，集中管理共享源码、头文件、资源与包含路径，根 .pro 统一引用 common 和四个 lab 模块；qmake + MSVC nmake Debug 全新构建通过，未运行界面。

- 2026-11-04 09:06：四个 lab 新增同名 .pri，由根 Mini-Marmoset.pro 统一引用，模块独立管理 qmake 文件和依赖；qmake + MSVC nmake Debug 全新构建通过，仅编译一个 main()，未运行界面。

- 2026-11-04 09:03：根 Mini-Marmoset.pro 生成单个工作台应用，ui/ 集中全部 QML，四个实验目录与根工程同级；已同步资源路径和构建文档。
- 2026-11-04 09:03：Qt 5.12.12 qmake + MSVC nmake Debug 构建通过，RCC 编译通过，目录与资源路径检查通过；未运行界面。

以下为调整前的记录。

# 项目状态
## 当前范围

四个 OpenGL 学习项目的Qt/QML UI、目录结构与接入架构，以及共享的 CPU 程序化网格、数值材质和 OBJ 读取工具。具体渲染项目由用户继续补充。
## 最近完成
- 2026-11-03 17:19：根 Mini-Marmoset.pro 改为四模块SUBDIRS 总工程，新增四个独立 .pro 与common/workbench.pri，共享启动代码、QML 资源和几何工具；旧示例保留在 legacy/LegacyPbr.pro，支持按需包含。同步qmake 使用文档。
- 2026-11-03 17:13：新增仅头文件的 MeshGenerator 与ObjLoader；共享位置法线/UV/索引及数值PBR 材质，支持八类网格、地形高度回调、OBJ 多边形三角化、平滑组和材质分段元数据。已接入 MarmosetGeometry INTERFACE 目标，用法见 `GEOMETRY.md`。
- 2026-11-03 17:06：建立四个独立CMake 目标及各自程序入口、QML 模块、render/ 和shaders/ 扩展目录；保留旧 PBR 示例入口。架构见 `ARCHITECTURE.md`。
- 2026-11-03 17:06：实现共享工作台，包括模块实验选择、可折叠侧栏、参数编辑与重置、会话状态保留、管线阶段说明和诊断面板。
- 2026-11-03 17:06：建立AGENTS、README、PRODUCT、DESIGN 与模块接入文档；临时产物隔离到`.tmp/ui-scaffold/`。
## 最近验证
- 2026-11-03 17:19：Qt 5.12.12 qmake + MSVC nmake 构建四个 Debug 目标通过；独立GpuLab.pro Release 构建通过；PbrLab Debug 运行并检查真实工作台截图。LegacyPbr.pro 仅完成qmake Makefile 生成，未编译或运行。
- 2026-11-03 17:19：运行对照中，qmake 和CMake 的GpuLab Release 均退出为 0xC0000005，未完成 Release 运行验证，根因待定位；显式指定匹配Qt 插件路径后可加载 qwindows.dll，仍发生该异常。
- 2026-11-03 17:13：MSVC 2022 Debug 构建四个工作台目标与 GeometryTests 通过；CTest 1/1 通过，包含五组CPU 回归及多翻译单元头文件链接，覆盖朝向、非退化三角形、UV/法线接缝、坡地法线、材质尺寸错误、OBJ 四种索引、负索引、凹多边形、平滑组、文件读取与失败路径。未接入 GPU 显示。
- 2026-11-03 17:06：独立UI 审查结论 ship；输入失焦恢复的补充审查判定 resolved。DESIGN.md 与schemaVersion 2 设计 sidecar 已按实际组件核对，JSON 解析通过。
- 2026-11-03 17:06：全部QML 的qmllint 通过；样式检测未报告问题；非法初始模块索引返回退出码 2。
- 2026-11-03 17:06：MSVC 2022 + Qt 5.12.12 Debug 构建，PbrLab、DeferredLab、SsaoLab、GpuLab 四个目标通过。
- 2026-11-03 17:06：Qt Quick Test 回归，模块实验切换、参数会话保留、数值输入与滑块同步、边界NaN/未知键处理、参数重置及面板交互通过： passed / 0 failed（含初始化与清理）。
- 2026-11-03 17:06：运行四个目标并检查实际窗口截图，覆盖 1440 × 900 与1080 × 720；截图为真实 UI 占位状态，不代表算法或硬件性能验证。
## 用户后续实现

- PBR / IBL：接入实际材质、贴图、环境光照和显示处理。
- Deferred：接入G-Buffer、多光源计算及与前向路径的对比。
- SSAO：接入采样、降噪和环境光合成。
- GPU Simulation：接入粒子实例数据、并行更新和批量绘制。
- 按实际复用需要提取公共渲染工具，并接入真实GPU 计时。
## 未验证范围
原有 PBR 示例及可选LegacyPbr 构建未在本次任务中运行验证；新工作台没有接入渲染算法、外部模型、GPU 计时或独立发布打包。Release 运行异常见最近验证，尚未定位或修复。
