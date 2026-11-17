# Pytyliki — 基于 C11 的处理器系统工作站

<p align="center">
<strong>发现 · 分析 · 优化 · 验证</strong><br>
<img alt="C11" src="https://img.shields.io/badge/C-ISO_C11-2563EB?style=flat-square">
<img alt="Windows 和 Linux" src="https://img.shields.io/badge/平台-Windows%20%7C%20Linux-DC2626?style=flat-square">
<img alt="MIT" src="https://img.shields.io/badge/许可证-MIT-FFFFFF?style=flat-square&labelColor=1F2937">
</p>

**Pytyliki 是一个模块化 C11 工作站，用于只读处理器信息发现、受限本地基准测试、源代码诊断以及 PCM16 WAV/DSP 工具。** 项目提供适用于 Windows 和 Linux 的 CLI，以及原生 Win32 图形界面。它不会重新编程物理芯片。

**文档语言：** [English](README.md) · [Русский](README.ru.md) · [Українська](README.uk.md) · 简体中文

---

## 功能概览

| 领域 | 功能 |
|---|---|
| 处理器 | CPU/ISA 信息清单；受限处理器、内存和矩阵测试 |
| 分析 | 只读源代码扫描器和 JSON 报告 |
| 音频/DSP | PCM16 WAV 读写、音调生成与处理 |
| 界面 | Windows/Linux CLI 和原生 Windows GUI |
| 改进 | 20 项能力目录及基于证据的规划 |
| 协调 | 六个受限 HyperFusion 分析模块 |
| 策略 | 默认拒绝的本地授权模型 |
| CI | Windows 和 Linux 上的 CMake/CTest 验证 |

> [!NOTE]
> 能力目录描述分类体系和预期工作流程，并不表示所有转换都能自主执行、自动修改源代码或保证性能提升。

## 快速开始

**环境要求：** CMake、支持 C11 的编译器、Windows 或 Linux、Git。

```bash
git clone https://github.com/Ivan-VHS/Pytyliki.git
cd Pytyliki
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Windows 构建还会提供 `pytyliki-gui.exe`。

```bash
pytyliki --improvement-plan
pytyliki --evolve --evolution-rounds 8
pytyliki --syntax-pie
pytyliki --mtte
```

## 架构

| 模块 | 职责 |
|---|---|
| `src/core.c/.h` | CPU 配置、受限基准测试、扫描器、JSON 报告 |
| `src/processor.c/.h` | 处理器/ISA 清单与测试套件 |
| `src/dsp.c/.h` | 受限 WAV 和 DSP 操作 |
| `src/improvement.c/.h` | 改进目录与生命周期 |
| `src/hw_rules.c/.h` | 建议性硬件规则 |
| `src/evolution.c/.h` | HyperFusion 分析 |
| `src/pipeline.c/.h` | C11/C23 参考流水线方案 |
| `src/policy.c/.h` | 本地授权策略 |
| `src/cli.c`、`src/win_gui.c` | CLI 与 Windows 桌面界面 |
| `.github/workflows/` | CI 和受控仓库自动化 |

## APSIA 2.0 — 受控多语言路由

APSIA 是一个 C11 建议性工作负载路由器。只有当预期收益超过 FFI 开销、且宿主语言优化不再是更优选择时，它才会选择候选实现。APSIA 不会编译、加载或执行生成的模块，不会授予任何 capability，并始终要求人工审查。

| 工作负载 | 候选方案 | 门控条件 |
|---|---|---|
| CPU 密集型 | Rust | 预期加速必须高于 FFI 开销 |
| GPU 张量 | CUDA | 显式声明 `--gpu` 并请求 GPU capability；确定性任务保留在宿主端 |
| 并发 I/O | Go | 通过成本与宿主优化门控 |
| 分析任务 | DuckDB | 通过成本与宿主优化门控 |
| 嵌入式脚本 | Lua | 通过成本与宿主优化门控 |
| 不可信插件 / 沙箱 | WebAssembly | 显式声明 `--wasm`；不可用时拒绝路由 |

`pytyliki-apsia WORKLOAD EXPECTED_SPEEDUP FFI_OVERHEAD_FRACTION HOST_OPTIMIZATION_FRACTION [options]`

`--gpu` 和 `--wasm` 仅显式声明候选后端可用性，不会探测硬件或运行时。`--capabilities LIST` 会追加能力标志，不会清除先前选项。路由器本身不会授予这些能力。

详见 [APSIA 2.0 架构](docs/APSIA_2.0.md) 和 [evolution-lock 模式](apsia/evolution-lock.schema.json)。

### 能力目录与实现状态

- **Syntax-pie：** 确定性的 20 层函数与语法抽象映射；这是目录和查询 API，不是通用解析器。
- **MTTE：** 20 个语义/函数/机械关联层及逻辑节拍计数器。它只模拟软件调度，不会更改硬件计时器、CPU 频率、寄存器、固件或电压。
- **改进矩阵：** 20 项工程分类、20 项优化分类和 20 项自动化分类。
- **HyperFusion：** 六个受限分析模块，每轮包含 36 个有向配对交互。就绪度和协同值是规划启发式指标，不是实测加速结果。
- **APSIA 2.0：** 带有目标可用性和 FFI 成本门控的路由规划器。它提出路由方案，但不会编译或执行所选语言模块。
- 目前已实现 **12 条建议性硬件规则**；其他目录标识符为预留项。C11/C23 流水线是参考方案，并非生产级转译器。

## 已验证的 GitHub 自动合并

仓库使用 `.github/workflows/verified-auto-merge.yml` 和 GitHub 原生 Auto-Merge（squash）。

1. 目标为 `main` 的 PR 会在 `ubuntu-latest` 和 `windows-latest` 上构建与测试。
2. CMake 配置并构建项目；CTest 运行已注册的测试。
3. 工作流验证当前 PR HEAD SHA 与通过验证的提交完全一致。
4. 必须有针对该 HEAD 提交的独立 `APPROVED` 审查；旧提交的批准不符合要求。
5. 随后请求执行 `gh pr merge --auto --squash --delete-branch`。只有满足仓库全部规则后，GitHub 才会完成合并。

配置要求：启用 **Allow auto-merge**；保护 `main` 并要求适用的 CI 检查和独立审查；仅向合并任务授予写权限。缺少批准、HEAD SHA 变化或 API 出错时必须停止，不得绕过分支保护。

> [!IMPORTANT]
> 启用 Auto-Merge 不代表 PR 已经合并。缺少批准或必需检查时，PR 会保持打开。工作流不会伪造批准，也不会覆盖仓库保护规则。

## 文档设计系统

项目统一采用蓝 / 白 / 红配色：

| 令牌 | 数值 | 用途 |
|---|---|---|
| 蓝色 | `#2563EB` | 链接、结构和中性技术强调 |
| 白色 | `#FFFFFF` | 页面背景和高对比表面 |
| 红色 | `#DC2626` | 错误、阻塞和关键状态 |
| 正文 | `#111827` | 标题与正文 |
| 次要文本 | `#6B7280` | 元数据 |

规范：

- 只使用一个 H1；后续标题层级连续，不跳级。
- 在深入架构细节前，先说明用途、平台范围和第一个可执行步骤。
- 段落简短，每个章节都应具备独立的信息完整性。
- 以 8 px 为间距基准：8 / 16 / 24 / 32 px。这是内容布局规范，不是 CSS 保证；Markdown 页面由 GitHub 渲染。
- 可比较的信息使用表格；有顺序的操作使用列表。
- 代码示例应可执行、标注语言，并适合窄屏阅读。
- 使用语义明确的链接锚点、GitHub Alerts，以及能解释关系的文本图表。
- 避免徽章堆叠、过量 emoji、重复内容和未经验证的性能承诺。
- 对比度和语义不能只依赖颜色。GitHub 控制响应式布局与字体；README 无法强制指定字体、精确行宽或像素级网格。

## CI 与质量验证

Pull Request 使用 Windows/Linux CMake/CTest 矩阵和 Recursive Evolution Gate。请参阅[质量审计](docs/QUALITY_AUDIT_0.7.1.md)，了解已记录的审查发现和验证证据。提交状态以 workflow 结果为准。

## 兼容性与限制

- **CI 验证平台：** Windows 和 Linux。
- **语言：** ISO C11 实现；C23 出现在参考流水线方案中。
- **数据格式：** JSON 报告和 PCM16 WAV 音频。
- **未作出以下保证：** macOS、ARM64 或 x86_64 专项性能保证、Protobuf、Parquet、Arrow，以及未经单独验证的生产级 RTL/FPGA 综合。

## 安全边界

- 处理器信息发现为只读操作。
- 源代码扫描器不会执行或改写被扫描文件。
- 不写入 firmware、BIOS/UEFI、microcode、电压、时钟、功率限制、操作系统内核、驱动或设备寄存器。
- 基准测试结果取决于硬件、操作系统、编译器和工作负载；不保证性能提升。
- 建议性规则和规划评分不构成硬件认证。

## 许可证

本项目采用 [MIT 许可证](LICENSE)。
