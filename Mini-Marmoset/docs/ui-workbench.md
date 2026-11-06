# 工作台实现约定

本界面属于 Operate 模式，直接沿用已经讨论的 Qt/QML 实验工作台方向。界面直接以代码实现，不生成效果图；本次选择不写入全局工作流默认值。

## Direction contract

THESIS：以实验视口为中心，参数、管线阶段和接入说明围绕当前实验组织。

OWN-WORLD：保留原有暗色界面的连续性，采用石墨灰视口、略带蓝色的工具栏、暖白正文和青绿色选中态；紧凑矩形控件、不使用装饰性光晕。

STORY：选择模块和实验，调整参数，查看管线位置，然后在模块自己的视口组件中接入 OpenGL。

FIRST VIEWPORT：顶部四个模块入口；左侧实验列表；中央大视口与渲染接入提示；右侧参数；底部可折叠诊断区及状态栏。签名交互为选中管线阶段后显示该阶段的接入职责。

FORM：桌面实验工具工作台；用户已确认的上一轮布局，直接实现。

seed: none (unavailable)

FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

## 交付审查

- 2026-11-03 17:06：真实运行截图覆盖四个模块、1440 × 900 和 1080 × 720，独立 UI 审查结论为 `ship`，无待处理的实质修复。
- 输入失焦恢复为补充审查范围，修复判定 `resolved`；Qt Quick Test 结果为 5 passed / 0 failed，其中包含 3 个交互测试及初始化、清理。
- 已按实际 QML 完成 DESIGN.md 与 `.impeccable/design.json`；sidecar 中 HTML/CSS 仅为文档预览，不属于 Qt 程序运行实现。
- 无交付用生成位图；`.tmp/ui-scaffold/` 的窗口截图仅作为验证证据。
