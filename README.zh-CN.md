
<p align="center"><picture><source media="(prefers-color-scheme: dark)" srcset="assets/readme/hero-banner-dark.svg"><source media="(prefers-color-scheme: light)" srcset="assets/readme/hero-banner-light.svg"><img src="assets/readme/hero-banner-light.svg" alt="Pytyliki 处理器系统工作站" width="880"></picture></p>
<h1 align="center">Pytyliki — 处理器系统工作站</h1>
<p align="center"><strong>基于原生 C11 的工作站，用于只读处理器发现、受限本地测试、数值工具和源代码静态诊断。</strong></p>
<p align="center"><a href="https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml"><img alt="CI" src="https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/ci.yml?branch=main&style=flat-square&label=CI"></a> <a href="https://github.com/Ivan-VHS/Pytyliki/actions/workflows/desktop-build.yml"><img alt="Windows build" src="https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/desktop-build.yml?branch=main&style=flat-square&label=Windows%20build"></a> <a href="LICENSE"><img alt="MIT" src="https://img.shields.io/github/license/Ivan-VHS/Pytyliki?style=flat-square"></a></p>

[快速开始](#快速开始) · [Revival 更新](#文档-revival-更新) · [视觉概览](#视觉概览) · [架构](#架构) · [文档索引](docs/README.md)

> **项目边界：**处理器发现与源代码检查均为只读操作。Pytyliki 不更改 CPU affinity、操作系统调度、频率或电压，不刷写固件，也不部署嵌入式代码。Win32 GUI 仅面向 Windows；在平台 API 可用时，C11 核心和 CLI 可移植构建。

## 快速开始

**环境要求：**CMake 3.16+、C11 编译器（MSVC、GCC 或 Clang）和 Git。

~~~sh
git clone https://github.com/Ivan-VHS/Pytyliki.git
cd Pytyliki
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
~~~

首次本地检查：

~~~sh
./build/pytyliki --profile
./build/pytyliki --suite
./build/pytyliki --partition 10000 8 256
./build/pytyliki-metrics --demo
~~~

Windows Release 可执行文件位于 build/Release/。Win32 GUI 目标仅在 Windows 上构建。

## 文档 Revival 更新

| 之前 | Revival 文档 |
|---|---|
| 单一静态横幅 | 根据浅色/深色主题切换的 SVG Hero |
| 项目介绍与实现细节混杂 | 清晰用途、显式边界和可执行快速开始 |
| 截图与流程图未区分 | 真实桌面截图和单独标注的架构示意图 |
| 功能成熟度分散在多个页面 | 带边界说明的摘要，并链接功能矩阵与架构地图 |
| 导航较长 | 独立文档索引、视觉规范和贡献指南 |

这是文档更新，**不代表软件版本升级或新 Release**。实现状态以源代码、Schema、测试和 CI 为准。

## 已实现范围

| 领域 | 当前能力 | 边界 |
|---|---|---|
| 处理器清单 | CPU 标识、拓扑、内存和可用 ISA/XCR0 | 本地只读发现 |
| 基准测试 | 受限 CPU/内存测试与负载分区示例 | 结果依赖主机 |
| Profile 工具 | 导入 collapsed stacks 和 Speedscope；导出 folded stacks 与 SVG flamegraph | 不启动 profiler |
| 源代码检查 | 静态诊断和结构化报告 | 不编译、不运行、不重写源代码 |
| Embedded C / QCS | MCU 资源估算与语法/AST 检查 | 参考/模拟模型与语法分析 |
| C ABI | 版本化 ABI v1 | 仅显式加载可信库 |
| Atomic-Quantum | 受限经典 state-vector 子集 | 仅模拟，不访问量子硬件 |

## 视觉概览

<table><tr><td width="50%"><a href="assets/screenshots/pytyliki-workstation.png"><img src="assets/screenshots/pytyliki-workstation.png" alt="Pytyliki Windows 桌面工作站实际截屏" width="100%"></a><p><sub><strong>真实应用截屏</strong> — Windows desktop，1920×1080。并非浏览器演示。</sub></p></td><td width="50%"><img src="assets/readme/workflow.svg" alt="Pytyliki 输入、验证、C11 模块和结构化结果流程图" width="100%"><p><sub><strong>架构示意图</strong> — 已实现数据路径的说明图，不是 UI 截图。</sub></p></td></tr></table>

## 架构

~~~mermaid
flowchart LR
  A[本地输入 / 处理器探测] --> B[验证与边界检查]
  B --> C[原生 C11 核心]
  C --> D[结构化结果]
  D --> E[CLI 或 Win32 GUI]
  D --> F[可选报告导出]
  G[测试与平台检查] --> C
~~~

## 配置

常规工作站不需要 .env、账户、API Token 或托管服务。实际 CMake 选项：

| 选项 | 默认值 | 用途 |
|---|---|---|
| BUILD_TESTING | CMake 默认值 | 启用原生测试与可选契约检查 |
| PYTYLIKI_ENABLE_SANITIZERS | OFF | 在受支持的 GCC/Clang 构建中启用 ASan/UBSan |

## 仓库结构

| 路径 | 内容 |
|---|---|
| src/ | C11 核心、处理器探测、CLI、Win32 GUI、QCS 和互操作 |
| tests/ | 单元、回归、边界与 CLI smoke 测试 |
| tools/ / scripts/ | 校验、Profile 导入和工程检查 |
| docs/ | 架构、功能契约、UI 与工程规范 |
| assets/readme/ | 主题适配 Hero 和流程示意图 |
| assets/screenshots/ | 应用实际截屏 |
| .github/workflows/ | CI、Windows 打包和仓库质量门禁 |

## 验证与项目状态

~~~sh
ctest --test-dir build -C Release --output-on-failure
python scripts/validate_errors.py
git diff --check
~~~

当前范围与计划见 [功能矩阵](docs/FEATURES.md)、[架构](docs/ARCHITECTURE.md)、[信息架构地图](docs/INFORMATION_ARCHITECTURE.md) 和 [自动化路线图](docs/AUTOMATION_ROADMAP.md)。目录计数和参考模型不等于硬件认证。

## 参与开发

修改核心、ABI 或平台探测前，请阅读 [CONTRIBUTING.md](CONTRIBUTING.md)。文档发布规则见 [README Visual Standard](docs/README_VISUAL_STANDARD.md)，安全问题请参阅 [SECURITY.md](SECURITY.md)。

## 许可证

[MIT](LICENSE)。
