# Pytyliki — 处理器系统工作站

[CI](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml) · [Windows 桌面构建](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/desktop-build.yml) · C11 · Windows/Linux · MIT
**开发者：** [Ivan-VHS](https://github.com/Ivan-VHS)

Pytyliki 是一个模块化原生工作站，用于处理器信息发现、有界工作负载规划、源代码静态诊断、数值工具和本地 DSP。Windows GUI 与命令行工具共享同一个 C11 核心。

## Windows 实际运行截图

<p align="center"><img src="assets/screenshots/pytyliki-workstation.png" alt="Windows 中运行的 Pytyliki 工作站" width="100%"></p>
<sub>由 desktop-build 工作流捕获的真实桌面画面，1920×1080。</sub>

## 功能范围

| 模块 | 当前功能 | 安全边界 |
|---|---|---|
| CPU | 型号、拓扑、内存、ISA/XCR0 与处理器配置 | 只读检测 |
| 测试 | CPU、内存、矩阵及经过检查的工作负载分区 | 有界本地测量 |
| 源码 | 静态诊断和结构化报告 | 不构建或执行被检查的源码 |
| DSP | PCM16 WAV、滤波、压缩、饱和和电平测量 | 本地缓冲区；验证 RIFF 分块边界 |
| 内存 | 类型化 views、checked slices、arena 和 pool | 固定限制并检查范围 |
| Embedded C | MCU 配置、FLASH/SRAM 估算和 MMIO 模型 | 仅模拟与参考数据 |
| Rules / APSIA / TRACE | 负载建议、任务规划和测量溯源 | 证据记录，不是认证 |
| QCS | lexer、parser、AST 和诊断 | 仅语法分析 |
| Interop | 版本化 C ABI v1 | 显式选择可信库 |

## 界面动效与无障碍

Windows 工作台提供 **Motion Settings**。**Reduce Motion** 会关闭启动窗口过渡；**Eco / Low-CPU Visuals** 会抑制非必要视觉效果。由于 Pytyliki 没有项目传输或主 BPM 来源，**Sync to Transport / BPM** 暂不可用。设置按当前用户保存，并遵循 Windows 客户区动画偏好及 `UI_REDUCED_MOTION` 环境变量。详见 [界面动效规范](docs/UI_MOTION.md)。

## 构建与测试

需要 CMake 3.16+、C11 编译器和 Git。

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Windows 工作流会构建 GUI 和 CLI 工具、运行 CTest，并发布 Windows x64 压缩包。当前归档包含 `pytyliki-gui.exe`、`pytyliki.exe`、`pytyliki-apsia.exe` 和 `pytyliki-metrics.exe`。

## CLI 示例

```sh
pytyliki --profile
pytyliki --suite
pytyliki --systems
pytyliki --embedded profiles
pytyliki-metrics --demo

pytyliki --divide -17 5
pytyliki --partition 10000 8 256
pytyliki --partition-auto 10000 256
```

分区计划是确定性的、有界的建议。它描述连续工作区间，但不会创建线程、修改 CPU affinity 或声称未经测量的性能提升。

## 目录规模

| 目录 | 条目 |
|---|---:|
| 系统语言能力 | 66 |
| CPU / MCU / 虚拟 ISA 配置 | 14 |
| 系统实验室功能 | 33 |

[功能](docs/FEATURES.md) · [UI motion](docs/UI_MOTION.md) · [架构](docs/ARCHITECTURE.md) · [除法与分区](docs/DIVISION_ENGINE.md) · [TRACE 溯源](docs/TRACE_MEASUREMENT_PROVENANCE.md) · [文档索引](docs/README.md)

[English](README.md) · [Русский](README.ru.md) · [Українська](README.uk.md) · [Releases](https://github.com/Ivan-VHS/Pytyliki/releases) · [MIT](LICENSE)
