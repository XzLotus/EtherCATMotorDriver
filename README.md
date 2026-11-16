# 嵌入式电机驱动系统 · 脱敏代码展示

面向招聘与技术交流的项目代码样例，展示工业通信、外设驱动、模块拆分和异常处理能力。

原工程采用 **N32G455 / ARM Cortex-M4 / C / Keil MDK**，集成 EtherCAT 从站应用、CiA402 状态与模式处理、RS485 Modbus RTU、CAN 和参数存储。本公开版选取通信与外设模块，电机驱动算法、运动规划、整定参数及固件产物已移除。

## 先看这几处

| 阅读入口 | 展示内容 |
| --- | --- |
| [Modbus RTU](src/protocol/modbus.c) | 报文累积、CRC16、03/06/10 功能码、广播与异常响应、寄存器范围检查 |
| [RS485 DMA](ports/n32/rs485.c) | RX 循环 DMA、空闲事件、TX DMA 与 UART 发送完成事件配合、收发方向控制 |
| [EtherCAT PDI / QSPI](ports/n32/qspi.c) | 从站控制器读写、超时退出、错误锁存与诊断计数 |
| [EEPROM](ports/n32/eeprom.c) | I2C 访问、页边界拆分、ACK 轮询、超时和错误恢复 |
| [CAN](ports/n32/can.c) | 标准帧与扩展帧封装、硬件 FIFO 读取、中断回调 |
| [行为测试](tests/test_protocol.c) | 24 组协议场景，含 1024 组确定性异常输入 |

建议阅读顺序：**本页 → [架构说明](docs/architecture.md) → [源码导读](docs/code-tour.md) → 测试代码**。

## 快速运行

环境：Python 3.10+，GCC，可用于 Windows / Linux。Windows 中可使用已有的 MinGW-w64 或 MSYS2 GCC，并将编译器目录加入 `PATH`。

在仓库根目录执行：

```sh
python tools/check_public_release.py
python tools/build_and_test.py
```

Linux 环境可将 `python` 换为 `python3`。构建脚本使用 C99、`-Wall -Wextra -Wpedantic -Werror`，生成文件位于 `build/`。

预期输出：

```text
PASS: 24 protocol scenarios, including 1024 malformed inputs
PASS: redacted backend keeps all outputs disabled
Modbus response: 01 03 02 00 2A 39 9B
Register 0 = 42 (synthetic demo data)
Motor backend: REDACTED, output_enabled=0
PASS: all portable targets compiled with warnings treated as errors
```

演示使用内存模拟 RS485，寄存器 0 的数值 42 为合成数据。电机后端固定返回 `REDACTED`，输出使能保持 0。

## 工程组织

```text
include/            可移植协议、传输接口、电机脱敏接口
src/protocol/       从原工程提取的 Modbus RTU 模块
src/motor/          新增的脱敏电机后端
ports/host/         主机测试用内存传输适配器
ports/n32/          从原工程提取的 N32 通信与外设源码
examples/           可直接运行的主机演示
tests/              协议行为与脱敏后端测试
tools/              编译验证、公开文件清单与内容检查
docs/               架构、源码导读、脱敏说明与验证记录
```

## 版本说明

- **源码提取部分**：Modbus、RS485、CAN、EEPROM、QSPI 共 10 个 `.c/.h` 文件。调整聚合头依赖，移除产品标识和本地作者占位信息，统一为 UTF-8。
- **展示辅助部分**：主机传输适配器、电机脱敏接口、演示、测试、发布检查和文档为公开版新增。
- **硬件阅读部分**：`ports/n32/` 使用厂商 SDK 接口，需要另行准备 SDK、启动文件和目标板工程；当前默认构建覆盖可移植模块与主机演示。
- **第三方依赖**：厂商 SDK、EtherCAT 从站协议栈及其生成文件由各自渠道获取，公开包仅保留相关集成结构说明。

[脱敏说明与公开清单](docs/sanitization.md) · [验证记录与待验证项目](docs/verification.md)
