# 架构与模块接入
## 构建边界

qmake 与 CMake 均构建单个 `Mini-Marmoset` 应用，唯一入口为根 `main.cpp`，加载 `ui/main.qml`。四个 lab 分别提供 `PbrLab`、`DeferredLab`、`SsaoLab`、`GpuLab` 类，目前为待实现的类骨架，不包含渲染算法。模块源码由各自同名 `.pri` 和 CMakeLists.txt 管理，日志与几何工具使用独立模块，QML 资源由根构建直接引用 `ui/workbench.qrc`。

## UI 与状态
`main.qml` 负责模块选择、实验列表、视口Loader、通用参数检查器和诊断区。四个模块的 Loader 常驻，因此参数和实验选择在模块切换时保留。只有当前模块的 `viewportComponent` 被实例化，切换时旧视口销毁。
`LabModule.qml` 是轻量接入契约：

| 属性| 职责 |
|---|---|
| `moduleId`、`title`、`subtitle` | 模块身份和展示文案|
| `experiments`、`experimentIndex` | 实验描述与当前选择 |
| `parameterSpecs` | 参数标签、上下限、步长、小数位 |
| `defaults`、`parameters` | 默认值与当前数值快照|
| `stages` | 管线阶段与接入职责说明，不代表已执行的Pass |
| `viewportComponent` | 当前模块自己的可视化 Item |
| `rendererReady` | 默认 false；算法真正接入后设为 true，更新工作台状态文案|

通过 `setParameter(key, value)` 修改参数：未知键、非数值和非有限值被拒绝，超出范围的值被限制在边界。`resetParameters()` 恢复当前模块默认值。当前版本参数仅保存在内存，不写配置文件。
## 接入 OpenGL

1. 在模块 render/ 中实现自己的 QQuickFramebufferObject 和 Renderer。
2. 在模块 .pri 与 CMakeLists.txt 中添加源文件及实际所需的依赖。需要特定 OpenGL 格式时，在根 main.cpp 创建 QGuiApplication 之前设置，并在运行时核对上下文能力。
3. 在根 main.cpp 创建应用之后、加载 QML 引擎之前调用 qmlRegisterType()；GL 资源初始化放在渲染阶段。
4. 在 ui/ 对应模块的 QML 中设置 viewportComponent，真正接入渲染后将 rendererReady 设为 true，例如：

```qml
LabModule {
    id: moduleRoot
    rendererReady: true
    // 保留本模块已有元数据、defaults、parameterSpecs 等。    viewportComponent: Component {
        MyRenderView {
            parameters: moduleRoot.parameters
            experimentIndex: moduleRoot.experimentIndex
        }
    }
}
```

`MyRenderView` 是接入示意，当前工程不注册此类型。可选`showGuides` 属性仅控制 UI 辅助层，没有该属性的自定义视口不会收到赋值。
QML 和UI 线程只修改属性。Renderer 在`synchronize()` 复制参数，在有当前OpenGL 上下文的渲染阶段创建、使用和释放 GL 资源。视口切换时必须允许资源安全释放；按实验切换时自行重建相关资源。Qt Quick 与自定义渲染之间必须恢复相应 OpenGL 状态。
## 后续公共能力

`Geometry/` 已提供共享CPU 数据 `Vertex`、`Material`、`MeshData`，以及仅头文件的 `MeshGenerator` 和`ObjLoader`；通过 `MarmosetGeometry` INTERFACE 目标复用现有 GLM。接口与支持范围见`docs/GEOMETRY.md`。
Shader、Texture、Camera、Framebuffer、GPU Mesh 和GPU Timer 等能力在出现真实复用时再提取到`common/render/` 或`common/scene/`。当前没有空实现的渲染接口、通用 Render Graph 或未使用的抽象基类。
## 验证

四个目标应分别构建；运行时检查四个模块、参数数值输入和滑块同步、模块间状态保留、当前模块重置、阶段选择、诊断区收起，以及1080 × 720 布局。`--capture <absolute-file>` 在窗口渲染后保存截图并退出；`--module 0..3` 选择模块，非法索引退出码与2；`--compact` 使用窄窗口。此截图入口只抓取UI，不提供渲染算法成GPU 性能验证。