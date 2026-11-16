# 🔵🔴 Pytyliki — C11 处理器系统工作站

<p align="center">

**探测 · 分析 · 优化 · 验证**

![C11](https://img.shields.io/badge/C-ISO_C11-2563EB?style=for-the-badge)
![操作系统](https://img.shields.io/badge/OS-Windows%20%7C%20Linux-DC2626?style=for-the-badge)
![许可证](https://img.shields.io/badge/License-MIT-2563EB?style=for-the-badge)

**一套精简的 C11 工作站。可验证的数据。边界明确的实验。**

</p>

**语言：** [English](README.md) · 简体中文 · [Русский](README.ru.md) · [Українська](README.uk.md)

Pytyliki 是一套模块化 C11 工作站，用于处理器能力探测、受限性能实验、源代码检查和本地 DSP 工作流。仓库整合命令行工具、原生 Windows GUI 及可测试的工程组件。

## 功能概览

| 领域 | 功能 |
|---|---|
| CPU 与 ISA | 处理器和指令集探测；同时验证操作系统启用的执行状态 |
| 基准测试 | 有边界的本地负载；结果仅供参考且取决于硬件 |
| 源代码分析 | 只读扫描，不执行或改写文件，生成 JSON 报告 |
| DSP | PCM16 WAV、音调生成和本地数字信号处理 |
| 界面 | Windows/Linux CLI 与原生 Win32 GUI |
| 工程 | CMake/CTest、证据门控规划、默认拒绝授权策略 |
| CPU Suite Pro 78X | 78 项模块目录、稳定 ID、六个领域、查询 API 和建议计划 |

## CPU Suite Pro 78X

借鉴经典集成软件套件的理念，CPU Suite Pro 以统一 CPU Hub 为中心组织 78 个专业工具。

| 领域 | 模块 ID |
|---|---:|
| Hub 与基础工具 | 1–14 |
| CPU 架构分析 | 15–26 |
| 温度与功耗 | 27–38 |
| 内存与缓存 | 39–50 |
| 调度与并发 | 51–60 |
| 诊断与基准实验室 | 61–78 |

**已实现：** 确定性的 78 项目录、唯一数字 ID、六个领域、无动态内存分配的枚举、按索引/ID/名称查询、建议计划生成和 CTest 覆盖。

**范围说明：** 78 个目录条目不代表 78 个完整硬件控制应用。实时传感器适配器、厂商专属调节、操作系统性能计数器和独立 GUI 面板仍需单独开发并验证。当前规划器不会写入 CPU 寄存器，也不会修改电压、频率、固件、微码、风扇、中断或内核设置。

参阅 [CPU Suite Pro 78X 技术规格](docs/CPU_SUITE_PRO_78.md)。

## 架构与构建

- **语言：** ISO C11；C23 仅出现在参考流水线方案中。
- **构建与测试：** CMake 和 CTest。
- **组件：** CPU 清单、受限基准测试、源代码扫描器、JSON 报告、PCM16/DSP、CLI 和 GUI。
- **变更治理：** 证据门控、可审查集成和默认拒绝授权策略。

需要 CMake、支持 C11 的编译器，以及 Windows 或 Linux。

```bash
git clone https://github.com/Ivan-VHS/Pytyliki.git
cd Pytyliki
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Windows 构建还定义原生应用目标 `pytyliki-gui`。

## 质量、兼容性与限制

- CI 状态以当前 workflow 运行为准；未经确认，不宣称测试通过。
- 结果取决于 CPU、固件、操作系统、编译器、散热和负载；不保证性能提升。
- ISA 标志不能单独证明指令可安全执行；必须运行时检查 CPU 能力和操作系统状态。
- 压力负载必须设置明确上限和停止条件。
- 不宣称 macOS、ARM64 或特定 x86_64 平台的性能保证。
- Protobuf、Parquet、Arrow 和生产级 RTL/FPGA 综合需另行验证。
- CPU Suite Pro 不会自动调压/调频、刷写固件、关闭微码安全缓解或执行特权寄存器写入。

## 项目结构

- `src/cpu_suite.h/.c` — 78 模块目录和建议规划 API。
- `tests/test_cpu_suite.c` — 目录、唯一性、查询和非法输入测试。
- `docs/CPU_SUITE_PRO_78.md` — 模块领域、API 和安全边界。
- `src/` — 核心、处理器、DSP、策略、改进、CLI/GUI。
- `CMakeLists.txt` — 库、应用程序和 CTest 目标。

## 许可证

MIT — 参阅 [LICENSE](LICENSE)。

<p align="center">

**愿你所爱，皆有回响。**

🔵 **清晰构建，以证据改进。** 🔴

</p>
