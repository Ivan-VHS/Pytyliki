# Pytyliki — 处理器系统工作站

<p align="center">
  <a href="https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml"><img alt="Windows 和 Linux CI" src="https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml/badge.svg"></a>
  <img alt="ISO C11" src="https://img.shields.io/badge/C-ISO%20C11-2563EB?style=flat-square">
  <img alt="Windows 和 Linux" src="https://img.shields.io/badge/platforms-Windows%20%7C%20Linux-DC2626?style=flat-square">
  <img alt="MIT 许可证" src="https://img.shields.io/badge/license-MIT-111827?style=flat-square">
</p>

<p align="center"><strong>发现 · 分析 · 处理 · 验证</strong></p>

<p align="center"><img src="assets/pytyliki-hero.svg" alt="Pytyliki 处理器系统工作站" width="100%"></p>

Pytyliki 是一个模块化 C11 工作站，用于本地处理器检查、有界基准测试、源代码静态诊断和 PCM16 音频处理。项目包含原生 Win32 桌面界面、命令行工具和可测试的工程目录。

**语言：** [English](README.md) · [Русский](README.ru.md) · [Українська](README.uk.md) · 简体中文

## 视觉预览

<p align="center"><img src="assets/pytyliki-workstation-preview.svg" alt="根据 Pytyliki 源码制作的 Win32 界面预览" width="100%"></p>
<p align="center"><img src="assets/pytyliki-audio-meters-preview.svg" alt="独立立体声音频测量预览" width="100%"></p>
<p align="center"><img src="assets/pytyliki-architecture.svg" alt="Pytyliki 架构概览" width="100%"></p>

<sub>仓库中的图片是依据源码制作的 SVG 界面预览和架构图，并非真实 Windows 会话中的照片或屏幕截图。真实 Win32 截图需要在 Windows 桌面会话中运行应用后采集。</sub>

## 工作站功能

| 领域 | 当前功能 |
|---|---|
| 处理器信息 | CPU 标识、架构、逻辑处理器、内存和可用 ISA 能力 |
| 基准测试 | 有界 CPU、内存和矩阵负载，用于本机比较 |
| 源码检查 | 对支持的文件进行静态诊断；不会执行或改写被检查的源文件 |
| 音频 / DSP | PCM16 WAV 导入/导出、测试音生成、滤波、压缩、饱和和增益 |
| 通道测量 | 每个声道独立计算峰值、RMS、DC 偏移、峰值因数和削波次数 |
| Windows 桌面 | 原生 Win32 工作站，采用统一的白、红、蓝 UI 主题 |
| 工程目录 | Syntax-pie、CPU Suite Pro、Forge66、TriMatrix、HyperFusion、APSIA 和建议型硬件规则 |
| 验证 | CMake/CTest、Windows 与 Linux CI、Linux ASan/UBSan 和只读 HMGA 审计 |

## 快速开始

### 环境要求

- Windows 或 Linux
- CMake 3.16+
- 支持 C11 的编译器
- Git

### 构建与测试

    git clone https://github.com/Ivan-VHS/Pytyliki.git
    cd Pytyliki
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
    cmake --build build --config Release --parallel
    ctest --test-dir build -C Release --output-on-failure

Windows 构建还会生成原生 pytyliki-gui.exe 目标。

### 常用命令

    ./build/pytyliki --profile
    ./build/pytyliki --suite
    ./build/pytyliki --improvement-plan
    ./build/pytyliki --systems
    ./build/pytyliki --tone-out test-tone.wav --frequency 440 --seconds 3
    ./build/pytyliki --json report.json

Windows 下请使用对应配置的可执行文件路径，例如 build/Release/pytyliki.exe。

## 集成更新

集成分支整合了现有开发中兼容且尚缺失的改动：

- **音频测量：** 每个声道独立计算 peak/RMS/DC/crest/clipping，并通过 CLI/Win32 输出。
- **WAV 安全：** 处理前检查 RIFF 范围、数据块头、负载和对齐字节边界。
- **统一计时：** 处理器测量和基准测试共用单调毫秒时钟，并新增独立契约测试。
- **HMGA v2：** 只读仓库审计器、测试和有界工程策略。
- **HMGA v3：** 确定性的 C11 目录，包含 20 条公理、400 个描述符，最多 64 轮规划。
- **Win32 主题：** 集中管理白、红、蓝颜色令牌和自绘控件。
- **CPU Suite：** 简化按 ID 和名称查找模块的控制流程，不改变查找行为。

历史分支经过逐项审查，而不是整批复制。main 中已有的功能会保留而不重复实现；重叠原型不会形成并行 API。详情请参阅[集成审计](docs/INTEGRATION_AUDIT_2026-11-19.md)。

## CI、安全与项目边界

GitHub Actions 在 Ubuntu 和 Windows 上构建并测试项目。Linux 测试还使用 AddressSanitizer 和 UndefinedBehaviorSanitizer。HMGA 审计检查工程证据和仓库策略，不会改写源代码。

- 处理器检查为只读操作，不写入固件、BIOS/UEFI、频率、电压、功率限制、内核、驱动或设备寄存器。
- 源码检查不会执行或修改被检查文件。
- 基准结果取决于硬件、编译器、操作系统和负载，不构成认证或加速保证。
- HMGA v3 只枚举有界规划描述符，不生成或执行源码。
- 桌面界面仅声明为 Windows 原生 Win32。未声明 macOS 支持或 ARM64/x86_64 的固定性能保证。

## 文档

- [架构](docs/ARCHITECTURE.md)
- [工程标准](docs/ENGINEERING_STANDARD.md)
- [HMGA v2.0 契约](docs/HMGA_V2.0.md)
- [HMGA v3.0 操作矩阵](docs/HMGA_V3.md)
- [集成审计](docs/INTEGRATION_AUDIT_2026-11-19.md)
- [APSIA 2.0 路由设计](docs/APSIA_2.0.md)
- [硬件规则引擎](docs/HARDWARE_RULE_ENGINE.md)
- [处理器分类](docs/PROCESSOR_TAXONOMY.md)
- [平台验证策略](docs/PLATFORM_VALIDATION_POLICY.md)


- [AETHER-ANAS 工程配置](docs/AETHER_ANAS_INTEGRATION.md)
- [Architecture 66 架构契约](docs/ARCHITECTURE_66.md)
- [666 条规则符合性配置](docs/CONSTITUTION_666_PROFILE.md)
- [100 条中国谚语工程交叉表](docs/CHINESE_PROVERBS_100_ENGINEERING_CROSSWALK.md)
- [MCST Elbrus E2K 目标配置](docs/ELBRUS_E2K.md)

## 许可证

项目采用 [MIT 许可证](LICENSE)。
