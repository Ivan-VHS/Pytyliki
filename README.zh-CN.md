# Pytyliki — 处理器系统工作站

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

> 基于受限 C11 核心的本地工作站，用于只读处理器发现、机器相关测量、内存/DSP 工具、静态诊断和显式原生互操作。

**规格**

| 字段 | 当前记录 |
|---|---|
| 仓库 | [Ivan-VHS/Pytyliki](https://github.com/Ivan-VHS/Pytyliki) |
| 项目状态 | 开发中；受限原生 C11 核心 |
| 产品版本 | 项目契约未声明统一版本 |
| 核心语言 | C11 / CMake 3.16+ |
| 运行平台 | Windows、Linux |
| 桌面平台 | Windows（Win32） |
| 工具 | Python 仅用于辅助工具，不是核心运行时 |
| 许可证 | [MIT](LICENSE) |
| 文档审核日期 | 2026-11-26 |

<p align="center"><img src="assets/pytyliki-hero.svg" alt="Pytyliki 处理器系统工作站" width="100%"></p>
<p align="center"><a href="assets/screenshots/pytyliki-workstation.png"><img src="assets/screenshots/pytyliki-workstation.png" alt="Pytyliki 工作站截图" width="100%"></a></p>

## 目录
- [用途与范围](#用途与范围)
- [架构与数据流](#架构与数据流)
- [构建与首次检查](#构建与首次检查)
- [接口与契约](#接口与契约)
- [限制与安全](#限制与安全)
- [文档与历史](#文档与历史)

## 用途与范围

Pytyliki 是一款本地处理器系统工作站。其 C11 核心提供只读硬件发现、受限的机器相关测量、内存和 DSP 操作、静态源码诊断以及版本化 C ABI。Win32 图形界面仅支持 Windows；CLI/core 声明支持 Windows 和 Linux。

应用输出观察结果和受限计算结果。它不会调整 CPU 时钟或电压、写入固件、安装驱动、访问物理 MMIO 寄存器，也不会改写被检查的源文件。

## 架构与数据流

~~~mermaid
flowchart LR
  U["CLI / Win32 界面"] --> V["参数与输入验证"]
  V --> C["C11 核心"]
  C --> D["只读 CPU / 内存发现"]
  C --> B["受限本地工作负载"]
  C --> S["静态源码 / QCS 检查"]
  C --> M["内存、DSP 与指标模块"]
  D --> R["带来源信息的结构化报告"]
  B --> R
  S --> R
  M --> R
  C --> A["版本化 C ABI v1"]
~~~

每个操作都经过输入验证、受限模块、结构化结果和可选的显式导出。硬件清单和源码检查均为只读。基准值取决于主机 CPU、操作系统、构建配置和热状态。

## 构建与首次检查

需要 CMake 3.16+ 和 C11 编译器（MSVC、GCC 或 Clang）。

~~~sh
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

命令仅提供本地、依赖机器环境的观察结果，不会调整系统。在 Windows 上，请使用当前 CMake 配置生成的可执行文件路径（多配置生成器通常位于 Release 配置目录）。

## 接口与契约

| 接口 | 契约 | 边界 |
|---|---|---|
| CLI | `pytyliki`、metrics 和 QCS 工具 | 本地执行；参阅命令帮助 |
| Native FFI | `pytyliki.c-abi.v1` | 显式可信库路径；缓冲区由调用方拥有 |
| 报告 | 带 provenance 的版本化 JSON/text | 未知值必须明确保留 |
| Embedded C | MCU 参考配置与 MMIO 掩码 | 仅模拟/参考 |
| OCTA-8 | 版本化 peer 拓扑契约 | 仓库联邦，不是集中式运行时 |

可选集成必须显式启用。Reaper 和 OPlague 可以通过调用方提供的可信库路径调用 C ABI v1。VentuSample 的直接 C ABI 集成仍属计划；当前 pack 元数据交换是独立机制。参阅 [Federated Integration](docs/FEDERATED_INTEGRATION.md)。

## 限制与安全

> 测量结果依赖主机环境；硬件参考模型不会执行真实设备操作。

处理器发现是只读的。基准工作负载有资源上限，不保证不同机器之间的结果可直接比较。静态源码诊断不会编译、执行或改写被检查文件。QCS 执行语法和结构验证，不负责通用程序执行。

Embedded C 工作台提供 MCU 内存参考配置和模拟 MMIO 掩码；它不是物理地址映射器、固件烧录器或实时调试器。Native FFI 会在进程内执行可信代码，只应加载显式选定且 ABI 兼容的库。不会写入 firmware、BIOS/UEFI、microcode、时钟、电压、驱动或设备寄存器。

清单数量和语言目录不是认证或性能证明。项目契约未声明统一产品版本，不应从旧 README 或目录中推断版本。

## 文档与贡献

从[文档索引](docs/README.md)和[信息架构图](docs/INFORMATION_ARCHITECTURE.md)开始。

- [功能矩阵](docs/FEATURES.md)
- [架构](docs/ARCHITECTURE.md)
- [基准统计](docs/BENCHMARK_STATISTICS.md)
- [内存 arena](docs/MEMORY_ARENA.md)
- [Embedded C 参考工作台](docs/EMBEDDED_C.md)
- [QCS 验证](docs/QCS_NATIVE.md)
- [报告封装](docs/REPORT_ENVELOPE.md)
- [Pytyliki/Reaper 集成](docs/REAPER_INTEGRATION.md)

修改 C ABI、报告格式或输入边界前，请阅读 [CONTRIBUTING.md](CONTRIBUTING.md)。安全问题报告请遵循 [SECURITY.md](SECURITY.md)。本仓库采用 [MIT](LICENSE)；请保留第三方许可证声明和归属信息。

## 文档与历史

当前项目契约定义了能力和集成边界，但未声明统一的产品 semantic version。仓库仍处于开发阶段。发布、分支支持和兼容性历史必须有真实 Git 标签、发行制品或迁移文档作为依据；没有证据时应明确标记为未声明。

基准结果应记录在 [Benchmark Statistics](docs/BENCHMARK_STATISTICS.md) 中，并提供主机、编译器、构建标志、工作负载、样本数和统计方法。不承诺通用延迟、内存占用或跨机器排名.