# 轮毂电机驱动 · 公开代码展示

基于 **HXS320F280025C / RISC-V / C / Haawking IDE** 的电机驱动项目展示，重点呈现嵌入式通信、模块拆分、数据封装和工程验证。

原工程包含电机控制、采样与 PWM 外设、编码器接口、Modbus 参数通信、波形上传和 Flash 参数管理。公开版将电机算法、参数与标定数据整体移除，选取通用通信逻辑并配套主机演示。

## 建议先看

| 入口 | 展示内容 |
| --- | --- |
| [Modbus 处理](src/comms/modbus_demo.c) | 03/06/10 功能码、CRC、广播、异常响应、长度与范围检查 |
| [提取的 CRC16](src/comms/crc16.c) | 项目串口模块中的标准 CRC 算法，增加空指针处理 |
| [遥测封装](src/telemetry/demo_telemetry.c) | 合成浮点数据的小端序封装、容量检查、标准帧尾 |
| [SCI 中断摘录](reference/haawking_sci_rx.c) | 接收字节、交给有界缓存、清除中断标志与 PIE 应答 |
| [脱敏驱动接口](src/drive/redacted_drive.c) | 固定返回已移除实现的状态，输出使能保持关闭 |
| [协议测试](tests/test_modbus.c) | 23 组场景，包含 512 组确定性异常输入 |

阅读顺序：本页 → [架构说明](docs/architecture.md) → [来源与改编记录](docs/source-provenance.md) → 源码与测试。

## 快速运行

环境：Python 3.10+，GCC。Windows 可使用 MinGW-w64 / MSYS2 GCC；Linux 可使用系统 GCC。

在项目根目录执行：

```sh
python tools/check_public_release.py
python tools/build_and_test.py
```

Linux 可使用 `python3`。构建脚本开启 `-Wall -Wextra -Wpedantic -Werror`，产物放入 `build/`。

```text
PASS: 23 Modbus scenarios, including 512 malformed inputs
PASS: synthetic telemetry serialization and redacted drive output checks
Modbus response: 01 03 02 00 2A 39 9B
Synthetic register 0 = 42
Synthetic telemetry packet: 20 bytes
Drive implementation: REDACTED; output_enabled=0
PASS: all host targets compiled with warnings treated as errors
```

演示采用内存中的示例寄存器和合成遥测值。电机接口固定保持 `output_enabled=0`。

## 目录

```text
include/          通信、遥测与脱敏驱动接口
src/comms/        提取的 CRC 与改编的通用协议处理
src/telemetry/    使用合成数据的遥测封装
src/drive/        已移除算法实现的公开接口
reference/        SCI 接收中断阅读摘录
examples/         PC 主机演示
tests/            协议与公开模块行为检查
tools/            编译、文件清单与源码摘要检查
docs/             架构、来源、脱敏范围、验证记录
```

## 展示版本说明

公开模块来源于同名项目的可读注释副本；目标工程目录用于结构与接口核对。标准 CRC 函数直接提取，协议与遥测模块经过拆分和改编，演示、测试及驱动占位接口为本次公开版新增内容。详细记录见[来源说明](docs/source-provenance.md)。

`reference/` 供硬件集成时阅读，需要另行准备厂商 SDK、设备配置和工具链。默认构建运行 PC 主机样例。

文档可直接在 GitHub 编辑；源码、脚本和 CI 配置修改后，先审阅变更，再运行 `python tools/check_public_release.py --refresh-manifest`，将更新的 `public-files.json` 一起提交。

[脱敏范围](docs/sanitized-scope.md) · [验证记录](docs/verification.md)
