# Pytyliki — 处理器系统工作站

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

[![C11 CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/ci.yml?branch=main&label=C11%20CI)](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml) [![Windows build](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/desktop-build.yml?branch=main&label=Windows%20build)](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/desktop-build.yml) [![MIT](https://img.shields.io/github/license/Ivan-VHS/Pytyliki)](LICENSE)

<p align="center"><img src="assets/readme/dashboard.svg" alt="Pytyliki 项目概览" width="100%"></p>
<p align="center"><a href="assets/screenshots/pytyliki-workstation.png"><img src="assets/screenshots/pytyliki-workstation.png" alt="Pytyliki Windows 实际运行画面" width="100%"></a></p>

Pytyliki 是一款面向处理器系统分析的本地工作站，采用受限 C11 核心、Windows 桌面界面和可移植命令行工具。

**范围：**处理器与文件盘点、受限本地分析和诊断报告。硬件发现仅执行只读操作；应用不会更改操作系统调度、CPU 亲和性、频率、电压或固件。

## 功能

- CPU 拓扑、ISA 与内存信息；受限 CPU/内存测量及工作负载分区示例。
- 面向应用自主管理工作线程的周期性故障分析与受限恢复计划。
- 性能分析数据导入、分析、折叠堆栈和 SVG 火焰图。
- 源码诊断、QCS 语法检查以及 MCU FLASH/SRAM 参考估算。
- 版本化 C ABI，以及包含来源和限制信息的结构化报告。

详细资料：[功能](docs/FEATURES.md) · [架构](docs/ARCHITECTURE.md) · [基准统计](docs/BENCHMARK_STATISTICS.md) · [Embedded C](docs/EMBEDDED_C.md) · [QCS](docs/QCS_NATIVE.md) · [处理器线程故障恢复](docs/PROCESSOR_THREAD_RECOVERY.md)。

## 构建与测试

需要 CMake 3.16+、C11 编译器（MSVC、GCC 或 Clang）和 Git。Win32 GUI 仅支持 Windows；核心和 CLI 可移植。Python 仅供可选的仓库契约检查使用，不属于 C11 运行时。

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

首次检查：

```sh
./build/pytyliki --profile
./build/pytyliki --suite
./build/pytyliki --partition 10000 8 256
./build/pytyliki-metrics --demo
```


Windows 事件日志查看与有界故障历史分析：

```sh
./build/pytyliki --thread-recovery tests/fixtures/processor_thread_recovery.csv
./build/pytyliki --thread-recovery-demo
# 仅 Windows：只读读取 WHEA 元数据
./build/pytyliki --ras-whea
```

PTFR 仅为应用程序管理的 worker 提供建议性恢复决策。WHEA 记录仍属于系统级证据；不会自动推断错误严重性或 worker 归属。
\n## 规格与集成

- [文档索引](docs/README.md) · [发布流程](docs/RELEASE.md) · [报告格式](docs/REPORT_ENVELOPE.md)
- [Reaper 集成](docs/REAPER_INTEGRATION.md) — 可选的 CI 策略检查，不是应用运行时依赖。
- [生态共享标准](https://github.com/Ivan-VHS/DAK/blob/main/docs/SHARED_STANDARDS.md)

[贡献指南](CONTRIBUTING.md) · [安全策略](SECURITY.md) · [MIT 许可证](LICENSE)
