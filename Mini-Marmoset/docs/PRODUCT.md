# Mini-Marmoset

<!-- impeccable:product-schema 1 -->

## Platform

Windows 桌面 Qt Quick 应用。不是网页或移动应用。

## Users

lxx，使用 C++ / Qt 学习 OpenGL，需要独立阅读、运行和扩展四个渲染实验项目。

## Product Purpose

把 PBR / IBL、Deferred、SSAO 和 GPU 模拟组织成四个可独立构建的项目，提供一致的实验选择、参数编辑、视口和调试入口。

## Capabilities and Constraints

已确认使用 Qt/QML。本阶段交付工作台 UI、目录和接入骨架；具体渲染算法由用户补充。现有 PBR 示例保留为独立入口。未实现的渲染、输出纹理和 GPU 计时必须明确说明，不伪造结果。

## Operating Context

桌面长时间编程和观察图像；参数修改后需要追踪其对画面的作用。四个独立程序使用同一套工作台，也允许在工作台中切换模块。模块参数在当前程序会话中保留，不自动写磁盘。
