# Pytyliki

Pytyliki 是面向系统开发者和 QA 工程师的本地 C11 工作站，用于处理器信息检查、有界工作负载分析、性能剖析数据审阅和数值工具。

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="assets/readme/dashboard.svg">
  <img src="assets/readme/dashboard-light.svg" alt="Pytyliki 概览：本机处理器输入、有界 C11 处理与证据报告" width="100%">
</picture>

[![C11 CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/ci.yml?branch=main&style=flat-square&label=C11%20CI)](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml) [![最近提交](https://img.shields.io/github/last-commit/Ivan-VHS/Pytyliki?style=flat-square)](https://github.com/Ivan-VHS/Pytyliki/commits/main) [![MIT](https://img.shields.io/github/license/Ivan-VHS/Pytyliki?style=flat-square)](LICENSE)

[快速开始](#快速开始) · [截图](#截图) · [架构](#架构) · [路线图](#路线图) · [文档](docs/README.md) · [提交记录](https://github.com/Ivan-VHS/Pytyliki/commits/main)

> **状态：** 活跃开发。仓库包含 C11 核心、CLI 工具、测试和 Windows 桌面目标。GUI 仅面向 Windows；系统探测能力取决于平台。
>
> **平台：** CI 中包含 Windows 与 Linux 的构建/测试路径。当前不宣称 macOS 已通过验证。

## 快速开始

**要求：** CMake 3.16+、C11 编译器（MSVC、GCC 或 Clang）和 Git。Win32 GUI 在 Windows 上构建；可移植核心和 CLI 是其他构建环境的入口。

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

运行一个有界任务分区示例：

```sh
./build/pytyliki --partition 10000 8 256
./build/pytyliki --profile
./build/pytyliki-metrics --demo
```

Windows Release 可执行文件通常位于 `build\\Release\\`。可用参数见 `pytyliki --help`。

## 功能

| 领域 | 当前行为 | 边界 |
|---|---|---|
| 处理器清单 | 读取本机 CPU 型号、拓扑、内存和可用 ISA/XCR0 信息 | 只读本机探测 |
| CPU 与内存 | 运行有界本地测量和工作负载分区示例 | 结果取决于当前主机 |
| 基准统计 | 汇总调用方提供的样本，包括百分位数、变异度和 MAD | 不会启动或重复执行负载 |
| 性能剖析导入 | 读取 collapsed stacks 与 Speedscope；导出 folded stacks 和 SVG 火焰图 | 不会启动 profiler |
| Linux 传感器 | 读取可用频率、governor、热量和能耗计数器 | 只读 sysfs/procfs |
| 源码审阅与 QCS | 解析源码并提供语法诊断 | 不构建或执行被审阅源码 |
| Embedded C | 浏览 MCU 配置并估算 FLASH/SRAM 使用量 | 参考估算，不会部署固件 |
| C ABI 与 Atomic-Quantum | 构建版本化 C ABI v1 库，并运行有界经典状态向量子集 | 不加载任意库，也不访问量子硬件 |

Pytyliki 提供可检查的观测结果和建议性计划。它不会更改操作系统调度、CPU affinity、频率或电压，不会刷写固件，也不会声称计划分区带来已测得的性能提升。

## Pytyliki 的用途

- 一个有界的原生 C11 核心为系统工具和本地 CLI 提供能力。
- 本机观测、用户提供的计时样本和演示数据明确区分。
- 报告保留数据来源与未知值，不会虚构传感器读数。
- 源码审阅和嵌入式计算仅提供建议，不会执行或部署代码。
- Windows GUI 和可移植 CLI 共享项目边界与测试套件。

## 截图

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="assets/readme/workflow-dark.svg">
  <img src="assets/readme/workflow.svg" alt="Pytyliki 数据流：本地输入经过有界 C11 模块并生成结构化结果" width="100%">
</picture>

<p align="center">
  <a href="assets/screenshots/pytyliki-workstation.png"><img src="assets/screenshots/pytyliki-workstation.png" alt="Pytyliki Windows 处理器系统工作站的真实界面截图" width="100%"></a>
</p>
<sub>由桌面构建 workflow 捕获的真实 1920×1080 Windows 应用截图。上方 SVG 是架构图，不是截图。</sub>

## 架构

```mermaid
flowchart LR
    A[本机或文件输入] --> B[验证与边界检查]
    B --> C[C11 处理器核心]
    C --> D[检查与分析]
    D --> E[报告或显式文件导出]
```

| 路径 | 职责 |
|---|---|
| `src/` | C11 核心、Win32 GUI、CLI 入口、处理器探测、QCS 与互操作 |
| `tests/` | 原生、回归、边界和 CLI smoke 测试 |
| `tools/` 与 `scripts/` | 性能剖析/传感器工具和工程检查 |
| `docs/` | 功能、架构、安全与 UI 契约 |
| `assets/screenshots/` | 真实桌面应用截图 |
| `.github/workflows/` | CI、Windows 打包与发布自动化 |

## 兼容性

| 目标 | 工具链 | 当前范围 |
|---|---|---|
| Windows | CMake 3.16+、MSVC 或其他兼容 C11 编译器 | 核心、CLI 和 Win32 GUI |
| Linux | CMake 3.16+、GCC 或 Clang | 核心/CLI 与 Linux 主机探测 |
| macOS | 尚未纳入 CI 验证矩阵 | 不宣称已支持 |

在 GCC 或 Clang 中可通过 `-DPYTYLIKI_ENABLE_SANITIZERS=ON` 启用 sanitizer；MSVC 不支持该选项。

## README Revival 更新

- 首屏展示项目用途、成熟度、平台和可执行的构建命令。
- 真实 Windows 截图与可编辑 SVG 架构图分开呈现。
- 浅色和深色 SVG 版本尺寸一致，并保存在仓库中。
- 明确说明功能边界、本机数据来源以及项目不提供托管 Web 服务。

| 旧版阅读流程 | 更新后的阅读流程 |
|---|---|
| 先读长篇概述再找命令 | 构建和测试命令提前 |
| 只有固定主题的图表 | 浅色/深色 SVG 配对 |
| 重复描述功能范围 | 精简的功能与边界表 |
| 平台支持不够直观 | 明确列出目标、证据与限制 |

## 路线图

- [x] C11 核心、CLI 工具和原生测试目标。
- [x] Windows 桌面目标与 CI 打包路径。
- [x] 文档化只读探测和有界分析边界。
- [ ] 仅在具备平台专用测试夹具和 CI 证据后扩展平台探测；这是建议方向，不代表已实现。

## 生态

- [DuckOS](https://github.com/Ivan-VHS/DuckOS) — 操作系统项目与 Mora 桌面工作区。
- [Reaper](https://github.com/Ivan-VHS/Reaper) — 系统编程语言与开发工作台。
- [OPlague](https://github.com/Ivan-VHS/OPlague) — 只读设备与驱动证据工作台。
- [VentuSample](https://github.com/Ivan-VHS/VentuSample) — 程序化采样生成与离线 DSP。

参阅[文档索引](docs/README.md)、[架构说明](docs/ARCHITECTURE.md)和[贡献指南](CONTRIBUTING.md)。许可证：[MIT](LICENSE)。
