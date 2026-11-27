# Pytyliki — 处理器系统工作站

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

<p align="center"><img src="assets/readme/dashboard.svg" alt="Pytyliki 项目概览" width="100%"></p>

[源代码仓库](https://github.com/Ivan-VHS/Pytyliki) · [C11 CI](.github/workflows/ci.yml) · [Windows 桌面构建](.github/workflows/desktop-build.yml) · [文档索引](docs/README.md) · [MIT](LICENSE)

## 实际应用截屏

<p align="center"><a href="assets/screenshots/pytyliki-workstation.png"><img src="assets/screenshots/pytyliki-workstation.png" alt="Pytyliki 在 Windows 上运行的实际截屏" width="100%"></a></p>
<p align="center"><sub>1920×1080 · 点击图片查看原始 PNG 文件。</sub></p>

这张图片是在本仓库的桌面构建工作流中，从真实运行的 Windows 桌面应用截取的。上方横幅和流程图是原创 SVG 矢量插图，并非应用截图。

### 架构一览

<p align="center"><img src="assets/readme/workflow.svg" alt="Pytyliki implemented data flow" width="100%"></p>

实际流程：本地主机数据与文件 → 有界 C11 核心 → 检查与分析 → 证据和本地输出。


[![C11 CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/ci.yml?branch=main&label=C11%20CI)](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml)  [![Windows build](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/desktop-build.yml?branch=main&label=Windows%20build)](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/desktop-build.yml)  [![License](https://img.shields.io/github/license/Ivan-VHS/Pytyliki)](LICENSE)


> **项目范围：** 基于 C11 的本地处理器系统工作站，包含 Windows 图形界面和可移植命令行工具。本仓库**没有**托管的 Web Demo、浏览器 Sandbox、API 服务或生产控制平面。文档已于 **2026-11-22** 对照 `main` 分支核对。

## 项目用途与边界

Pytyliki 将处理器信息发现、有界工作负载规划、源代码诊断、数值工具整合到一个原生 C11 核心中。目标读者包括系统开发者、硬件研究人员、QA 工程师以及需要可检查本地结果的代码审查者。

应用展示能够在本机读取到的信息，并生成建议性计划。它不会更改操作系统调度、CPU affinity、处理器频率或电压；不会刷写固件，也不会声称未经测量的性能提升。MCU/MMIO 内容属于参考或模拟数据。QCS 只执行语法分析，不会任意运行被检查的源代码。

### 已实现的功能

| 模块 | 可执行的操作 | 明确边界 |
|---|---|---|
| 处理器清单 | 查看型号、拓扑、内存以及可用的 ISA/XCR0 信息 | 本地只读检测 |
| CPU 与内存测试 | 运行有界的本地测量和工作负载分区示例 | 结果取决于当前主机，并非通用排行榜 |
| 基准统计 | 分析调用方提供的计时样本：百分位数、波动性和 MAD | 仅分析；不会启动或重复工作负载 |
| 性能分析导入 | 导入 collapsed stacks 和 Speedscope；导出 folded stacks 与 SVG 火焰图 | 将数据作为文本/JSON 解析；不会启动分析器 |
| Linux 传感器快照 | 读取 CPU 频率、governor、温度和能耗计数器并记录来源 | 只读 sysfs/procfs；缺失传感器保持未知状态 |
| 源码检查 | 检查源文本并生成诊断/报告 | 不构建或执行被检查的源代码 |
| Embedded C | 浏览 MCU 配置并估算 FLASH/SRAM | 参考估算，不部署固件 |
| QCS | 词法分析、解析、查看 AST 和诊断 | 仅语法分析 |
| Interop | 构建版本化 C ABI v1 库 | 调用方必须显式选择可信库 |
| Atomic-Quantum | 运行有界的经典 state-vector 子集 | 仅模拟，不访问量子硬件 |

下方图片是桌面应用的实际截屏，并非交互式浏览器预览。


## 相关操作系统产品：DuckOS

七款 Mora 应用（Cockos、Kavun、Ka-bon、Gloverty、Po、Illidia 和 Unify）在独立的 [DuckOS 项目](https://github.com/Ivan-VHS/DuckOS/blob/main/docs/PREINSTALLED_APPLICATIONS.md)中定义。它们属于操作系统应用层，并不是 Pytyliki 的组成部分。本仓库不声称随附这些应用或提供其操作系统服务。Pytyliki 自身的能力和接口仍以本仓库的源代码、测试和版本化契约为准。

## 快速开始

### 环境要求

- CMake **3.16 或更高版本**。
- C11 编译器：MSVC、GCC 或 Clang。
- Git。
- Python 3 是可选的，仅用于额外的 plugin-federation 合同检查；C/C++ 运行时不依赖 Python。

Win32 GUI 目标仅在 Windows 上构建。共享 C11 核心和 CLI 是可移植构建目标；实际能力仍取决于当前操作系统 API。

### 构建与测试

在仓库根目录执行：

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Windows 下，CMake 会将 Release 可执行文件放入 `build/Release/`。常见的单配置 Linux 构建则放在 `build/`。

### 首次冒烟检查

```sh
./build/pytyliki --profile
./build/pytyliki --suite
./build/pytyliki --partition 10000 8 256
./build/pytyliki-metrics --demo
```

Windows 下使用 `build\Release\pytyliki.exe` 和 `build\Release\pytyliki-metrics.exe`。图形界面程序为 `build\Release\pytyliki-gui.exe`。

其他本地示例：

```sh
pytyliki --systems
pytyliki --embedded profiles
pytyliki --divide -17 5
```

完整参数列表可通过 `pytyliki --help` 查看。以上命令均在本地运行；项目没有默认 HTTP 端口或 localhost URL。

### 可选的 Sanitizer 构建

GCC 和 Clang 可以启用 AddressSanitizer 与 UndefinedBehaviorSanitizer：

```sh
cmake -S . -B build-sanitized -DBUILD_TESTING=ON -DPYTYLIKI_ENABLE_SANITIZERS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-sanitized --parallel
ctest --test-dir build-sanitized --output-on-failure
```

MSVC 不支持此选项。

## 数据、测试场景与状态

工作站不会连接远程数据库，也没有登录、演示账号、Mock API 或浏览器存储。处理器信息来自当前本地主机，前提是相应平台探测器已实现。`--demo` 指标模式使用示例数据，不能将其描述为真实主机遥测。

可以按以下场景验证：

1. **主机配置：**运行 `pytyliki --profile`，查看当前机器的处理器信息。
2. **有界规划：**运行 `pytyliki --partition 10000 8 256`，检查确定性的工作区间。计划不会启动线程。
3. **除法：**运行 `pytyliki --divide -17 5`，并与 Euclidean division 测试对照。
4. **指标示例：**运行 `pytyliki-metrics --demo`；这些是演示数据，不是主机测量值。
5. **源码检查：**运行 `pytyliki --review-processor path/to/source.c`；诊断过程不会执行源文件。

工作数据存在于当前进程以及操作者选择读取或保存的文件中。关闭应用会结束当前内存会话；生成的报告不会自动删除。项目没有“重置演示”按钮，也没有共享服务器端状态。界面外观和动态效果偏好由桌面 UI 管理，不使用 cookies、localStorage、sessionStorage 或 IndexedDB。

## 配置与访问

工作站常规运行不需要 `.env.example`、Docker Compose、Web Proxy、CORS、HTTPS 证书、身份验证或 API Token。不需要测试账号，也不应将秘密信息提交到仓库。

`CMakeLists.txt` 中实际存在的构建选项：

| 选项 | 默认值 | 用途 |
|---|---:|---|
| `BUILD_TESTING` | CMake/CTest 默认值 | 包含原生测试和可选 Python 合同检查 |
| `PYTYLIKI_ENABLE_SANITIZERS` | `OFF` | 在 GCC 或 Clang 上启用 ASan/UBSan |

缺少 C 编译器或 Sanitizer 工具链不兼容属于构建配置错误，而不是遗漏了 Web 环境变量。请查看第一条 CMake 错误，并确认所选编译器与平台相匹配。

## 架构与仓库目录

本地数据流为：**输入 → 验证 → 有界模块 → 结构化结果 → 可选文件导出**。

| 路径 | 内容 |
|---|---|
| `src/` | C11 核心、Win32 GUI、CLI 入口、处理器探测、QCS 和 interop |
| `tests/` | CTest 单元、回归、边界和 CLI 冒烟测试 |
| `tools/` | 验证工具和仓库工具 |
| `scripts/` | 架构与工程合同检查 |
| `docs/` | 架构、功能边界、UI 行为和模块合同 |
| `assets/screenshots/` | 桌面程序截图 |
| `security/` | Beelzebul 机器可读安全策略 |
| `.github/workflows/` | CI、Windows 打包和发布自动化 |

核心使用 C11。GUI 使用 Windows Win32 API，只在 Windows 上构建。项目没有 frontend/backend 分层、Web Server、Mock Server、Browser Bundle、端口监听或部署预览。浏览器兼容性、Cookie、CORS、Viewport Breakpoint 和 HTTPS 均不适用。

## 构建产物、CI 与发布

- [C11 CI](.github/workflows/ci.yml) 在 Ubuntu 和 Windows 上构建、测试，并包含 Linux Sanitizer Job。
- [Desktop build](.github/workflows/desktop-build.yml) 构建 Windows GUI 和 CLI，运行 CTest，截取应用画面并生成 `Pytyliki-windows-x64.zip`。
- Windows 压缩包作为 Workflow Artifact 上传，保留 30 天。
- 名称匹配 `v*` 的 Git Tag 会触发 Release 发布。向 `main` 推送本身并不表示 Web 部署。

构建产物位于 Workflow 的 build/dist 路径中。回滚已分发的桌面版本时，使用上一个已知正常的 GitHub Release/压缩包。项目没有需要回滚的在线服务。

## 故障排查

| 现象 | 检查方法 |
|---|---|
| CMake 无法识别 C 编译器 | 安装 C11 工具链，并在对应的开发者环境中运行配置 |
| 找不到 `pytyliki-gui` | GUI 目标仅支持 Windows |
| CTest 在某台主机上失败 | 保存失败测试名称与完整输出；不同 OS/CPU 的探测结果可能不同 |
| Sanitizer 选项失败 | `PYTYLIKI_ENABLE_SANITIZERS` 需要 GCC 或 Clang |
| Embedded 估算被误解为部署 | 它只是参考计算；本项目不会刷写或编程硬件 |

CI 日志可在对应的 GitHub Actions Workflow 中查看。README 和测试套件是受支持命令的依据；CLI 选项改变时，应同步更新示例及所有语言版本。

## 已知边界、贡献与许可证

已知限制明确列出：GUI 仅支持 Windows，源码检查为静态分析，Embedded 数值是建议性参考，量子功能是经典模拟。仓库的贡献流程、变更验证和 Pull Request 要求见 [CONTRIBUTING.md](CONTRIBUTING.md)。提交改进时，请创建 GitHub Issue 或 Pull Request，说明平台、确切命令、预期结果和实际输出；提交前运行构建与 CTest。

支持和缺陷反馈：[GitHub Issues](https://github.com/Ivan-VHS/Pytyliki/issues)。许可证：[MIT](LICENSE)。

## README 对 66 条规则的映射

66 条 Web Stand 文档规则在不虚构 Web 应用的前提下应用于本 README：

| 规则范围 | README 的处理方式 |
|---|---|
| 1–11 · 用途与边界 | 说明本地工作站、目标读者、可用流程、限制、数据来源、`main` 核对日期以及没有托管 Demo |
| 12–22 · 快速开始 | 给出准确工具链、构建、测试、Smoke Check 和 GUI 命令；没有隐藏的 Web Server 或端口步骤 |
| 23–33 · 配置与访问 | 记录真实 CMake 选项；ENV、账号、CORS、Proxy 和 HTTPS 标记为不适用 |
| 34–44 · 数据与行为 | 区分 Host Data 与 Demo Metrics，说明文件边界、可重复测试、会话生命周期和错误情况 |
| 45–55 · 架构与构建 | 列出真实目录、C11/Win32 技术栈、CTest、构建产物和 CI 边界 |
| 56–66 · 运维与支持 | 链接真实 Workflow/Release，说明桌面压缩包回退、故障排查、已知限制、贡献方式与许可证 |

**本项目没有公开的 Web Stand URL。** 桌面截图和 GitHub Actions Artifact 并不是交互式 Web Stand。

## 统一报告封装格式

可以将 JSON 输出包装为通用的 ivan-vhs.report.v1 元数据格式，而不改变工具自身的数据：

    python tools/report_envelope.py --tool TOOL_NAME --version VERSION < result.json

请参阅 docs/REPORT_ENVELOPE.md 和 schemas/ivan-vhs.report.v1.schema.json。该封装记录结果来源和限制；它不是密码学签名，也不代表兼容性认证。


## 文档导航图

请从[文档索引](docs/README.md)进入，并使用[信息架构图](docs/INFORMATION_ARCHITECTURE.md)查看主题集群、相关页面及跨仓库边界。主题入口链接到详细契约；实现状态仍以对应的源代码、测试和 Schema 为准。


## Mora 桌面集成

Pytyliki 已加入统一的 Mora 应用目录。只有在 Windows 上构建现有 Win32 GUI 目标后，才能从 Mora 启动原生窗口；可移植 C11 核心和 CLI 仍保持独立。请参阅 [Mora 集成说明](docs/MORA_DESKTOP_INTEGRATION.md) 和 [DuckOS 工作空间契约](https://github.com/Ivan-VHS/DuckOS/blob/main/docs/MORA_DESKTOP.md)。
