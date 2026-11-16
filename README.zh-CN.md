# Pytyliki — C11 处理器系统工作站

**简介：** Pytyliki 是面向 Windows 和 Linux 的原生 C11 工作站，支持只读 CPU/ISA 检查、受限处理器基准测试、源代码诊断、本地 PCM16 WAV/DSP 处理，以及证据门控的工程工作流。提供 CLI 和原生 Win32 桌面 GUI。项目不会重新编程物理处理器。

**语言：** [English](README.md) · 简体中文 · [Русский](README.ru.md) · [Українська](README.uk.md)

<p align="center">

**探测 · 分析 · 规划 · 验证**

![C11](https://img.shields.io/badge/C-ISO_C11-2563EB?style=for-the-badge)
![平台](https://img.shields.io/badge/OS-Windows%20%7C%20Linux-DC2626?style=for-the-badge)
![许可证](https://img.shields.io/badge/License-MIT-2563EB?style=for-the-badge)

</p>

## 功能

- **CPU 与 ISA：** 读取处理器、架构、内存和指令集信息。ISA 标志本身不能证明操作系统已启用相应执行状态。
- **基准测试：** 执行受限的本地计算、内存和矩阵测试；结果取决于硬件、编译器、操作系统和工作负载。
- **源代码诊断：** 检查处理器相关代码模式并生成报告，不执行或改写被检查的源代码。
- **音频/DSP：** 在边界限制内读写 PCM16 WAV、生成音调、处理并测量信号。
- **桌面与 CLI：** Windows/Linux 命令行工作流和原生 Win32 图形界面。
- **改进规划：** 20 组三元工程/优化/自动化目录，以及证据门控的递归改进流程。
- **硬件建议：** 已实现 12 条建议性规则；其他目录 ID 保留，不能视为已实现。
- **APSIA 2.0：** 确定性工作负载语言路由，以及默认拒绝的晋级证据门。

## 架构

| 层 | 当前范围 |
|---|---|
| Core | CPU 配置、受限基准测试、源代码扫描器、JSON 报告 |
| Processor | 架构/ISA 清单和受限处理器测试 |
| DSP | PCM16 WAV 输入输出、音调生成、处理和测量 |
| Improvement | 20×3 能力目录和证据门控迭代规划 |
| HyperFusion | 六个受限分析模块；每轮 36 次有向配对交互 |
| Hardware rules | 12 条建议性规则，并验证指标范围 |
| APSIA 2.0 | 工作负载路由、FFI 成本评估、能力声明和晋级门 |
| Pipeline | C11/C23 参考流水线方案，并非生产级转译器 |
| Policy | 默认拒绝的本地授权策略 |
| UI | Windows/Linux CLI 与原生 Windows GUI |

HyperFusion 的就绪度和协同指标属于启发式规划数据，不代表实测加速或独立 CI 认证。APSIA 路由器不会编译、加载、热替换或执行生成的模块，也不会授予运行时能力。

## 递归改进模型

工程生命周期：

`观察 → 诊断 → 提议 → 实现 → 验证 → 评估 → 审查 → 集成 → 学习 → 改进引擎`

迭代目标和计数器用于约束规划，并不意味着系统会自动执行同等数量的构建或源代码重写。66,666,666× 是愿景目标，不是实测倍率。

20×3 目录定义了 60 项工程、工作流和质量操作。它是术语体系与策略矩阵，并不表示每项操作都已自主执行。

| # | 工程 | 优化 | 自动化 |
|---:|---|---|---|
| 01 | 现代化 | 现代化 | 自动优化 |
| 02 | 优化 | 优化 | 自动现代化 |
| 03 | 改进 | 转译 | 自动适配 |
| 04 | 重构 | 编译 | 自动纠错 |
| 05 | 转换 | 重新配置 | 自动编译 |
| 06 | 加速 | 扩展 | 自动转译 |
| 07 | 适配 | 加速 | 自动诊断 |
| 08 | 扩展 | 综合 | 自动验证 |
| 09 | 自动化 | 分解 | 自动校准 |
| 10 | 集成 | 集成 | 自动调优 |
| 11 | 稳定化 | 适配 | 自动配置 |
| 12 | 统一 | 自动化 | 自动自动化 |
| 13 | 合理化 | 并行化 | 自动集成 |
| 14 | 重建 | 验证 | 自动扩展 |
| 15 | 重新设计 | 诊断 | 自动加速 |
| 16 | 自学习 | 稳定化 | 自动恢复 |
| 17 | 自调优 | 重构 | 自动综合 |
| 18 | 自恢复 | 初始化 | 自动重新配置 |
| 19 | 自优化 | 晶化 | 自动学习 |
| 20 | 演进 | 完善 | 自动完善 |

## APSIA 2.0：受控多语言演进

已实现的 C11 基础模块会评估工作负载类型、预期收益、FFI 开销、宿主语言优化空间和平台可用性。候选语言/后端包括 Rust、C++、Zig、CUDA、WebAssembly、Go、Elixir、SQL/DuckDB、Lua 和 Python。

证据不完整时禁止晋级。检查项包括接口契约、差分测试、模糊测试、消毒器、静态分析、实测收益、影子等价性、独立审查、制品签名、回滚测试，以及已验证与当前版本令牌的一致性。

**已实现：** 路由策略和晋级证据门。

**尚未实现：** LLM 源码合成、多语言编译器集群、自动安装依赖、生产级 eBPF/APM、FFI 代码生成、运行时 hot-swap/RCU、无人值守部署和跨仓库执行器。详见 [APSIA 2.0 设计](docs/APSIA_2.0.md)。

## 开发中的目录

近期设计讨论还定义了三个处理器方向的目录：

- **Syntax Pie：** 20 层“函数 / 房间 / 语法”映射，从词素、词元到 AST、语法规则、图和层级。
- **MTTE：** 20 层“语义概念 / 函数 / 机械操作”映射，以及确定性软件 tick 计数器模型。
- **Berry Core：** 面向逻辑处理器核心的 20 条命令目录，采用 dry-run 规划语义。

这些是分阶段集成的目录，不是完整解析器、硬件计时器控制器、IPI/亲和性控制器或生产级处理器控制接口。Concreo、Pyemont、Bollo、JJ-HH、PenWhite 和 Serious Serious 是实验性功能套件概念，不是稳定的公共 API。

## 构建与测试

需要 CMake 3.16+、支持 C11 的编译器、Git，以及 Windows 或 Linux。

```bash
git clone https://github.com/Ivan-VHS/Pytyliki.git
cd Pytyliki
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Windows 构建还包含原生 `pytyliki-gui.exe` 目标。

## CLI 示例

```bash
pytyliki --improvement-plan
pytyliki --evolve --evolution-rounds 8
pytyliki --review-processor source.c
pytyliki --json report.json
```

请使用 `pytyliki --help` 查看当前构建支持的选项。音频工作流包括 `--tone-out`、`--audio-info` 和 `--process-wav`。

## 质量门与仓库自动化

1. 创建隔离的功能分支。
2. 提交范围明确、可追踪的更改。
3. 在受支持的 CI 平台上构建并运行 CTest。
4. 检查回归、证据以及已测试提交的 SHA。
5. 获取独立人工批准。
6. 通过仓库授权机制按顺序集成。

自动提交、自动推送、自动合并和部署是权限不同的独立操作。检查失败、过期或证据不完整时必须阻止晋级。应为 `main` 配置分支保护、必需 CI 检查和独立审查。

未来的 README 生成应从仓库状态确定性地产生，包括 API/AST、指标、CI、依赖和历史记录。在生成器实现并验证之前，README 仍由人工维护。

## 安全与边界

- CPU 探测只读；扫描器不会执行或改写被检查的代码。
- 不写入固件、BIOS/UEFI、微码、电压、时钟、功率限制、内核、驱动或设备寄存器。
- 不任意执行生成代码，也不自动授予插件权限。
- 硬件规则仅提供建议；基准测试和 HyperFusion 估算不保证性能提升。
- 实现基线为 C11。C23 仅作为参考流水线目标，不代表完整转译器。
- 文档声明的 CI 目标为 Windows/Linux；未经单独验证，不宣称支持 macOS、ARM64 或特定 ISA 性能保证。

## 项目结构

- `src/core.c/.h`、`src/processor.c/.h` — 核心和处理器检查。
- `src/dsp.c/.h` — WAV 与 DSP 操作。
- `src/improvement.c/.h`、`src/evolution.c/.h` — 改进目录和演进分析。
- `src/hw_rules.c/.h` — 处理器建议规则。
- `src/apsia.c/.h`、`src/apsia_cli.c` — APSIA 路由与晋级策略。
- `src/pipeline.c/.h`、`src/policy.c/.h` — 参考流水线和授权策略。
- `src/cli.c`、`src/win_gui.c` — CLI 和 Windows GUI。
- `.github/workflows/` — CI 和受控仓库自动化。

## 许可证

MIT — 参见 [LICENSE](LICENSE)。

---

<p align="center">

**精确工程 · 验证每一项更改**

</p>

爱与耐心，让每一次改进都有意义。