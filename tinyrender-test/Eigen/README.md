# Eigen 3.4.0

从本机 `D:/eigen-3.4.0/eigen-3.4.0/` 复制的原版头文件，保持上游内容不变。

- `include/Eigen/`：正式模块的入口头文件及完整内部实现，可使用 `<Eigen/Dense>`、`<Eigen/Geometry>` 等。
- `bin/`：预留目录；Eigen 是头文件库，无需 EXE 或 DLL。
- `lib/`：预留目录；无需静态库或导入库。
- `COPYING*`：上游许可证及说明，具体适用范围见 `COPYING.README` 和各文件声明。

未复制上游测试、示例、构建脚本和 `unsupported` 扩展。项目通过 CMake 添加本目录下的 `include` 路径，无需安装或链接 Eigen 二进制文件。
