# Pytyliki — 处理器系统工作站

C11 · Windows/Linux · MIT · [CI](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml)
**开发者：** [Ivan-VHS](https://github.com/Ivan-VHS)

用于 CPU 检测、有界工作负载、静态源码诊断、数值工具和本地 DSP 的模块化原生工作站。Windows GUI 与 CLI 共用 C11 核心。

| 模块 | 功能 | 安全边界 |
|---|---|---|
| CPU | 型号、拓扑、内存、ISA/XCR0 | 只读 |
| 测试 | CPU、内存、矩阵、任务分区 | 有界测量 |
| 源码 | 静态诊断、报告 | 不编译/执行 |
| DSP | PCM16 WAV、滤波、压缩 | 本地缓冲区 |
| 内存 | views、slices、arena、pool | 固定限制 |
| Embedded C | MCU、FLASH/SRAM、MMIO | 仅模拟 |
| Rules/TRACE | 建议和测量溯源 | 非认证 |
| QCS | lexer、parser、AST | 仅语法 |
| ABI | 版本化 C ABI v1 | 显式选择库 |

构建：CMake 3.16+、C11 编译器、Git。

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

| 目录 | 条目 |
|---|---:|
| 系统语言能力 | 66 |
| CPU/MCU/虚拟 ISA 配置 | 14 |
| 系统实验室功能 | 33 |

分区 CLI：`--divide A B`、`--partition ITEMS WORKERS GRAIN`、`--partition-auto ITEMS GRAIN`。计划有界，不创建线程、不修改 CPU affinity。

[功能](docs/FEATURES.md) · [架构](docs/ARCHITECTURE.md) · [分区](docs/DIVISION_ENGINE.md) · [TRACE](docs/TRACE_MEASUREMENT_PROVENANCE.md) · [文档索引](docs/README.md)

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [Releases](https://github.com/Ivan-VHS/Pytyliki/releases) · [MIT](LICENSE)
