# Pytyliki — 处理器系统工作站

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

**基于 C11 的模块化工作站，用于本地处理器检查、受限基准测试、源代码诊断以及 PCM16 音频/DSP。** 项目包含原生 Win32 桌面界面和命令行工具。

## 当前功能

- **处理器信息：** CPU 型号与厂商、逻辑处理器、内存和可用 ISA 能力，并检查操作系统的向量状态支持。
- **基准测试：** 有界 CPU、内存和矩阵工作负载，用于本机比较；结果取决于硬件、编译器和操作系统。
- **源代码诊断：** 检查受支持的源文件，不执行或改写文件；可导出 JSON 报告。
- **音频/DSP：** PCM16 WAV 导入/导出、测试音生成、低通滤波、压缩、饱和、增益和电平计量。
- **规划目录：** Syntax-pie、改进矩阵、HyperFusion 和 APSIA 路由方案。这些是辅助规划模型，并非自主优化器或编译器。
- **桌面界面：** Windows 原生 GUI，支持处理器扫描、源代码审查和音频控制。

## 截图

当前仓库尚无经过验证的运行中应用截图。此前的界面示意图不再冒充真实截图。待实际构建并运行 Win32 应用后，可在此添加现场截屏。

## 构建与测试

要求：CMake 3.16+、C11 编译器、Git，以及 Windows 或 Linux。

```bash
git clone https://github.com/Ivan-VHS/Pytyliki.git
cd Pytyliki
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Windows 构建还会生成 `pytyliki-gui.exe`。CLI 示例：

```bash
./build/pytyliki --profile
./build/pytyliki --suite
./build/pytyliki --improvement-plan
./build/pytyliki --tone-out test-tone.wav --frequency 440 --seconds 3
./build/pytyliki --json report.json
```

典型 Windows 多配置构建的可执行文件位于 `build/Release/pytyliki.exe`。

## 架构与安全

项目使用 ISO C11、CMake 和 CTest。GitHub Actions 验证 Windows 与 Linux 构建；独立 Linux 任务运行 AddressSanitizer 和 UndefinedBehaviorSanitizer。

处理器发现仅执行只读操作。应用不会写入固件、BIOS/UEFI、微码、频率、电压、电源限制、内核设置、驱动或设备寄存器。源代码检查不会执行被扫描文件。基准分数不代表认证，也不保证性能提升。规划目录仅用于辅助决策，并不证明已经完成优化。

## 文档

- [架构](docs/ARCHITECTURE.md)
- [APSIA 路由设计](docs/APSIA_2.0.md)
- [硬件规则引擎](docs/HARDWARE_RULE_ENGINE.md)
- [处理器分类](docs/PROCESSOR_TAXONOMY.md)
- [平台验证政策](docs/PLATFORM_VALIDATION_POLICY.md)

## 许可证

[MIT](LICENSE)。

> 千里之行，始于足下。
