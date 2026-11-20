# Pytyliki — 处理器系统工作站

<p align="center">
  <a href="https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml"><img alt="CI" src="https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml/badge.svg"></a>
  <img alt="C11" src="https://img.shields.io/badge/core-ISO%20C11-2563EB?style=flat-square">
  <img alt="Windows 和 Linux" src="https://img.shields.io/badge/platforms-Windows%20%7C%20Linux-DC2626?style=flat-square">
  <img alt="MIT" src="https://img.shields.io/badge/license-MIT-334155?style=flat-square">
</p>

**基于模块化 C11 的处理器工作站，用于只读硬件发现、受限基准测试、源码诊断和本地 DSP。** 项目提供原生 Windows 桌面界面以及可移植的命令行核心。默认原则是检查、测量和报告，不静默修改硬件。

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

## 界面预览

<p align="center"><img src="assets/pytyliki-workstation-preview.svg" alt="Pytyliki Win32 工作站示意界面" width="100%"></p>
<sub>根据仓库中的 Win32 控件重构的示意图，并非运行中的 Windows 应用截图。当前仓库尚未提交真实运行截图。</sub>

## 工作模块

- **处理器清单：** CPU 型号、架构、逻辑拓扑、内存与 ISA 能力；AVX 可用性会检查由操作系统管理的 XCR0 状态。
- **受限基准套件：** CPU、内存和矩阵工作负载，提供耗时与完整性检查；结果仅供本机比较，不构成硬件认证。
- **源码静态检查：** 检查受支持的源码并导出发现，不编译、不执行、不改写输入文件。
- **音频 / DSP：** PCM16 WAV 导入导出、测试音生成、低通滤波、压缩、饱和、增益与电平测量。
- **内存内核：** 类型化视图、溢出检查切片、有界 arena、回退检查点、遥测和固定块内存池。
- **度量引擎：** 向量范数与距离、正交性、加权二次型、几何和方差运算。
- **Embedded C：** MCU 参考配置、FLASH/SRAM 预算、带掩码的模拟 MMIO 和模板生成；并非真实寄存器映射器或固件烧录器。
- **工程目录：** 66 项系统语言能力目录、14 个 CPU/MCU/虚拟 ISA 配置、APSIA 路由器及 33 项处理器/区块链系统实验室接口。
- **互操作：** 面向显式配置原生调用方的 Pytyliki C ABI v1。
- **QCS 前端：** 原生 C11 词法分析器、帧解析器、紧凑 AST 和 CLI，用于检查域与帧边界；仅做语法检查，不是编译器或硬件控制器（[规范](docs/QCS_NATIVE.md)）。

## 构建与测试

需要 CMake 3.16+、C11 编译器和 Git。CI 覆盖 Windows 与 Linux；Win32 图形界面仅支持 Windows。

```bash
git clone https://github.com/Ivan-VHS/Pytyliki.git
cd Pytyliki
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

常用命令：

```bash
./build/pytyliki --profile
./build/pytyliki --suite
./build/pytyliki --systems
./build/pytyliki --embedded profiles
./build/pytyliki-metrics --demo
./build/pytyliki --json report.json
```

Windows 多配置构建中，示例路径为 `build/Release/pytyliki.exe`。

## 项目演进

0.5.0 工作站基线（2026 年 11 月 16 日）以处理器画像、源码检查和 DSP 为核心。随后 main 分支持续扩展 C11 模块与测试，加入内存和度量内核、原生 ABI、Embedded C 参考工具、架构与语言目录、系统实验室以及版本化项目契约。当前源码和测试是已实现功能的依据。

## 安全与边界

CPU 发现为只读。基准任务有明确上限且依赖硬件环境。源码检查不会执行被检查程序。DSP 仅处理本地缓冲区。硬件规则评分和 APSIA 路由仅供参考。不会写入 BIOS/UEFI、固件、微码、电压、频率、电源限制、驱动或设备寄存器。工具链发现只检查 `PATH` 中的可执行文件名，不安装或运行编译器。

## 文档

[架构](docs/ARCHITECTURE.md) · [内存内核](docs/MEMORY_ARENA.md) · [Embedded C](docs/EMBEDDED_C.md) · [系统语言与架构目录](docs/SYSTEMS_LANGUAGE_ARCHITECTURE_ATLAS.md) · [统一工程标准](docs/UNIFIED_ENGINEERING_STANDARD.md)


[治理准则](docs/HUMAN_FIRST_GOVERNANCE.md) · [Anatidae 设计系统](docs/ANATIDAE_DESKTOP_DESIGN_SYSTEM.md) · [UI/UX 架构](docs/UI_UX_ARCHITECTURE.md) · [12 项动效原则](docs/DISNEY_MOTION_SYSTEM.md) · [临床安全架构](docs/clinical-safety-architecture.md) · [项目契约](docs/UNIFIED_PROJECT_CONTRACT.md)
[Windows 发布版](https://github.com/Ivan-VHS/Pytyliki/releases) · [桌面构建工作流](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/desktop-build.yml)

## Anatidae 工程准则

- **以人为本并尊重明确授权：** 硬件发现、分析和导出保持分离；报告导出、共享和遥测分别受知情同意及角色权限控制。
- **有界且安全默认：** 校验输入、溢出、切片边界和工作负载上限。处理器发现和源码审查均为只读；不会写入 BIOS/UEFI、固件、微码、驱动或设备寄存器；模拟操作必须明确标注。
- **无障碍工作台体验：** 采用一致的设计系统和清晰层级，支持键盘操作、可见焦点、足够对比度、本地化及减少动态效果。
- **有目的的动效：** 动画用于表达状态和反馈，不替代可读状态，也不应成为完成任务的前提。
- **以证据为准：** 明确区分已实现模块、参考配置、模拟功能和建议性评分；实际能力以源码和测试为准。

本节定义项目设计基线，并不表示所有建议中的功能都已实现。

## 许可证

[MIT](LICENSE)

UI motion and accessibility: [12 animation principles](docs/DISNEY_MOTION_SYSTEM.md).
