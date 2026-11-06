---
name: Mini-Marmoset Workbench
description: 以渲染视口为中心的 Qt Quick 实验工作台
colors:
  primary: "#80cbb7"
  background: "#16191d"
  surface: "#20252b"
  elevated: "#292f37"
  header: "#232e38"
  border: "#38414b"
  text: "#edf0f2"
  muted: "#adb8c3"
  selected: "#2b4945"
  warning: "#e5bd7b"
typography:
  experiment:
    fontFamily: "Microsoft YaHei UI"
    fontSize: "22px"
    fontWeight: 600
  product:
    fontFamily: "Microsoft YaHei UI"
    fontSize: "19px"
    fontWeight: 600
  title:
    fontFamily: "Microsoft YaHei UI"
    fontSize: "15px"
    fontWeight: 600
  body:
    fontFamily: "Microsoft YaHei UI"
    fontSize: "13px"
  parameter:
    fontFamily: "Microsoft YaHei UI"
    fontSize: "12px"
  label:
    fontFamily: "Microsoft YaHei UI"
    fontSize: "11px"
rounded:
  control: "4px"
  input: "3px"
spacing:
  small: "8px"
  medium: "16px"
  panel: "20px"
components:
  button-default:
    textColor: "{colors.text}"
    typography: "{typography.body}"
    rounded: "{rounded.control}"
    height: "34px"
  button-selected:
    backgroundColor: "{colors.selected}"
    textColor: "{colors.primary}"
    typography: "{typography.body}"
    rounded: "{rounded.control}"
    height: "34px"
  parameter-input:
    backgroundColor: "{colors.background}"
    textColor: "{colors.text}"
    typography: "{typography.parameter}"
    rounded: "{rounded.input}"
    width: "82px"
    height: "29px"
---

## Overview

**Creative North Star: "以渲染视口为中心的实验工作台"**

供桌面长时间观察渲染图像的实验工具。沿用项目暗色工作环境，以低饱和工具栏和石墨色视口降低周边干扰。公共运行时颜色定义位于 `ui/Theme.qml`；本文件记录实际 QML 组件使用的设计。

界面以紧凑控件承载参数编辑，以明确的选中状态表达当前模块和管线阶段。空视口与未采样性能保持可见状态说明，辅助网格只承担布局参考。

**Key Characteristics:**

- 视口优先的桌面工作台。
- 低饱和深色表面与青绿色选中态。
- 数值输入与滑块成组呈现。
- 实色分层和细线分隔，不使用装饰阴影。

## Colors

青绿色用于交互选择与参数位置，石墨灰和带蓝色的工具栏构成中性工作环境。

### Primary

主色标记当前模块、选中阶段、参数滑块和交互反馈；选中底色提供独立的面背景，避免仅靠文字颜色区分状态。

### Neutral

背景用于视口和数值输入，surface 用于侧栏和诊断区，elevated 用于按钮悬停，header 用于顶部与状态栏。正文和次要说明使用不同亮度的暖白/灰蓝色，border 负责控件边界与分隔线。warning 仅用于工作台骨架状态。

**The Selection Rule.** 选中控件使用选中底色与主色文字，键盘焦点另外以主色边框标明。

## Typography

中文界面采用 Windows 常用 UI 字体。字号依次承担实验标题、产品名、面板标题、常规控件、参数标签和次要说明；参数数值按配置的小数位显示，右对齐。

**The Reading Rule.** 技术文案继续使用 UI 正文字体，文件路径和数值不额外引入装饰性字体。

## Layout

默认 1440 × 900，最小 960 × 640；1080 × 720 为已检查的紧凑尺寸。顶部为模块导航，左侧为实验列表，中央为视口和诊断区，右侧为参数检查器。宽度小于 1200 时实验列表默认收起，可手动展开；右侧宽度从 292px 调整为 264px。高度小于 780 时诊断区从 190px 调整为 150px。

工具栏与实验列表使用紧密间距，面板内容使用中等留白，参数组之间保留 22px。侧栏与诊断区都可通过明确按钮切换，长参数列表滚动。

## Elevation & Depth

面板通过实色背景和 1px 分隔线区分层次，不使用装饰阴影、光晕或毛玻璃。控件状态直接切换颜色，没有自定义入场或过渡动画。

## Shapes

控件以小圆角矩形为主；参数输入使用更小的圆角。立方体线框为 QML Canvas 绘制的小型一致线条图标，辅助网格明确标注为 UI 参考，不冒充渲染场景。

## Components

### Buttons and Navigation

`LabButton` 统一模块导航、实验选择、阶段选择和面板操作。普通状态透明，悬停使用抬高表面色，按下和选中使用选中底色；键盘焦点使用主色边框。禁用文本使用次要色并降低透明度。长标签省略显示。

### Parameter Input

`ParameterControl` 将标签、右对齐数值输入和滑块组织为一组。输入可选择文本，验证范围与小数位；焦点边框使用主色，编辑结束或失焦后提交有效输入，空值和无效输入恢复当前值及数据绑定。滑块轨道为细线，已选部分和圆形手柄使用主色，键盘焦点为手柄添加正文色边框。

### Pipeline Stages

阶段按钮共享 `LabButton`，选中后显示对应的职责说明。未执行 GPU Pass 时保留文字状态，不提供虚构输出纹理或计时。

### Viewport State

`ViewportPlaceholder` 使用居中的小型线框图标、实验标题和接入说明。角落状态标签与辅助网格标注说明当前能力边界。此状态组件可被模块自己的真实视口替换。

### Documentation Previews

`.impeccable/design.json` 中的 HTML/CSS 片段仅为上述 QML 样式的文档镜像，不是应用运行代码，也不替代 Qt 实际渲染验证。

## Do's and Don'ts

### Do:

- **Do** 新 UI 复用 Theme 与现有组件，具体算法的参数放在各自模块。
- **Do** 为实验参数声明数值范围，并保留确定的编辑和重置结果。
- **Do** 让选中状态和键盘焦点具有不同的可见提示。

### Don't:

- **Don't** 未接入 GPU 计时时显示伪造性能；没有真实纹理时显示虚构缩略图。
- **Don't** 用图标字符或 emoji 代替操作名称。
- **Don't** 将文档 HTML/CSS 预览描述为实际 Qt 运行组件。
