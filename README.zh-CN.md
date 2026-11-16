# Pytyliki — C11 处理器系统工作站

**TL;DR：** Pytyliki 是面向 Windows 和 Linux 的免费 MIT 许可 C11 处理器工作站，提供 CPU/ISA 探测、受限基准测试、源代码分析、本地 PCM16 音频/DSP 工具以及原生 Windows 图形界面。

**语言：** [English](README.md) · 简体中文 · [Русский](README.ru.md) · [Українська](README.uk.md)

<p align="center">

**探测 · 分析 · 优化 · 验证**

![C11](https://img.shields.io/badge/C-ISO_C11-2563EB?style=for-the-badge)
![平台](https://img.shields.io/badge/OS-Windows%20%7C%20Linux-DC2626?style=for-the-badge)
![许可证](https://img.shields.io/badge/License-MIT-2563EB?style=for-the-badge)

</p>

Pytyliki 面向系统程序员、C 语言开发者和硬件爱好者，用于检查处理器能力并执行有边界的本地实验。项目不宣称能够重新编程物理芯片。

## 为什么选择 Pytyliki？

处理器信息、源代码检查、性能测试和 DSP 工具通常分散在不同软件中。Pytyliki 将这些工作流整合到原生 C11 代码库中，并明确安全边界，确保结果可验证。

**核心价值：** 将只读处理器探测、受限实验、源代码诊断和证据驱动的工程流程整合到一个可审计工作站。

## 功能

- **CPU 与 ISA 探测：** 收集处理器和架构信息；ISA 标志只是提示，不能证明操作系统已启用相应指令状态。
- **受限基准测试：** 使用可控工作负载执行本地处理器测试；结果仅供参考，并受硬件条件影响。
- **源代码分析：** 扫描源文件并生成 JSON 报告，不执行或改写被扫描的代码。
- **音频与 DSP：** 在边界限制内读写 PCM16 WAV、生成音调并执行本地数字信号处理。
- **桌面与命令行：** 提供原生 Win32 图形界面，以及适用于 Windows 和 Linux 的 CLI 工作流。
- **改进规划：** 查看包含 20 项能力的工程/优化/自动化目录，以及基于证据的迭代计划。

## APSIA 2.0 多语言自进化

APSIA 在 C11 中整合了工作负载路由与证据晋级策略。CPU 密集型任务映射到 Rust；检测到可用 GPU 时，GPU 任务映射到 CUDA；并发 I/O 映射到 Go；分析任务映射到 DuckDB；嵌入式脚本映射到 Lua；不可信插件映射到 WebAssembly。规划器会比较宿主语言优化收益与 FFI 开销，不会自行授予任何能力，并始终要求人工审查。

`pytyliki-apsia` CLI 可查看路由决策，但不会编译或执行生成的代码。详见 [APSIA 2.0](docs/APSIA_2.0.md) 和 [evolution-lock](apsia/evolution-lock.schema.json)。

## 当前架构

| 模块 | 实现内容 |
|---|---|
| Core | CPU 配置、受限基准测试、源代码扫描器、JSON 报告 |
| Processor | 架构/ISA 清单，以及受限计算、内存和矩阵测试 |
| DSP | PCM16 WAV 输入输出、音调生成、处理和测量 |
| Improvement | 20×3 能力目录和证据门控生命周期 |
| Hardware rules | 已实现 12 条建议性规则；其他目录 ID 仅作预留 |
| HyperFusion | 六个受限分析模块，每轮包含 36 个有向模块交互 |
| Pipeline | C11/C23 参考流水线方案，并非生产级转译器 |
| Policy | 默认拒绝的本地授权策略 |
| Interfaces | 跨平台 CLI 和原生 Windows GUI |

HyperFusion 的就绪度和协同指标属于启发式规划指标，不代表实测加速，也不构成独立 CI 认证。

## 快速开始

### 需要什么环境？

- CMake 和支持 C11 的 C 编译器。
- Windows 或 Linux。
- 用于克隆仓库的 Git。

### 如何构建和测试 Pytyliki？

```bash
git clone https://github.com/Ivan-VHS/Pytyliki.git
cd Pytyliki
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Windows 构建还会生成 `pytyliki-gui.exe`。

## CLI 工作流

示例：`pytyliki-apsia cpu 2.0 0.1 0.2` — 仅规划路由，不会编译或加载模块。

```bash
pytyliki --improvement-plan
pytyliki --evolve --evolution-rounds 8
```

请使用可执行文件的帮助信息查看当前版本支持的完整命令。不同版本的选项可能有所变化。

## 使用前后对比

| 分散式工作流 | Pytyliki 工作流 |
|---|---|
| 在 CPU 工具、源代码扫描器和 DSP 工具之间切换 | 使用统一的 C11 工作站和构建/测试流程 |
| 将 ISA 标志视为最终执行许可 | 明确考虑操作系统状态和指令可用条件 |
| 依赖未经验证的优化声明 | 记录输入、输出和验证证据 |

## 质量与自动化

仓库使用 CMake 和 CTest，并通过 CI 检查 Windows 与 Linux。改进目录描述目标工作流，并不意味着所有转换都已实现自动执行。

受控集成流程：

1. 创建独立功能分支。
2. 提交范围明确、可追踪的更改。
3. 在受支持的 CI 平台上构建并运行测试。
4. 获取独立审查，并核对已测试提交的 SHA。
5. 仅通过仓库授权的机制合并。

自动提交、自动推送和自动合并是权限不同的独立操作。检查失败必须停止晋级。请在 GitHub 中配置分支保护，要求相关 CI 检查和独立审查；单靠 workflow 无法阻止有权限的维护者手动绕过仓库策略。

## 安全与实现边界

- CPU 探测为只读操作。
- 扫描器不会执行或改写被扫描的源代码。
- Pytyliki 不会写入固件、BIOS/UEFI、微码、电压、时钟、功率限制、操作系统内核、驱动程序或设备寄存器。
- 硬件规则仅提供建议；目前实际实现了 12 条规则。
- C11/C23 流水线是参考方案，不是生产级转译器。
- 基准测试结果取决于硬件、操作系统、编译器和工作负载，不保证带来性能提升。

## 项目结构

- `src/core.c/.h` — CPU 配置、基准测试、扫描器和 JSON 报告。
- `src/processor.c/.h` — 处理器/ISA 清单和测试套件。
- `src/dsp.c/.h` — WAV 与 DSP 操作。
- `src/improvement.c/.h` — 改进目录和生命周期。
- `src/hw_rules.c/.h` — 建议性硬件规则。
- `src/evolution.c/.h` — HyperFusion 分析。
- `src/pipeline.c/.h` — 参考流水线方案。
- `src/policy.c/.h` — 本地授权策略。
- `src/apsia.c/.h`、`src/apsia_cli.c` — APSIA 路由器与 CLI。
- `docs/APSIA_2.0.md`、`apsia/evolution-lock.schema.json` — 架构与证据模式。
- `src/cli.c`、`src/win_gui.c` — 命令行和 Windows 桌面界面。
- `.github/workflows/` — CI 和受控仓库自动化。

## 兼容性与数据格式

- **操作系统：** 项目 CI 矩阵包含 Windows 和 Linux。
- **语言标准：** 实际实现采用 ISO C11；C23 出现在参考流水线方案中。
- **数据格式：** JSON 报告和 PCM16 WAV 音频。
- **未作出的兼容性声明：** macOS、ARM64/x86_64 性能保证、Protobuf、Parquet、Arrow，以及未经单独验证的生产级 RTL/FPGA 综合。

## 许可证

Pytyliki 使用 [MIT 许可证](LICENSE) 发布。

---

<p align="center">

**精准工程 · 验证每一项更改**

</p>
