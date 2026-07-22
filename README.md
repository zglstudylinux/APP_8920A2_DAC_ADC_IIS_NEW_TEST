APP_8920A2_DAC_ADC_IIS_NEW_TEST

> 面向 **BT8920A2 RISC-V 音频 SoC** 的 Code::Blocks 工程，主要用于：
>
> - 厂商 SDK（[app/platform](../app/platform)）的初学者上手；
> - 任务 A/B/C/D 的应用层实现（[docs/待完成任务.md](docs/待完成任务.md)）；
> - 验证 DAC / SDADC / IIS 通路在真实开发板上的工作行为。

## 仓库结构

```text
.
├─ app/
│  ├─ bsp_ext/                 应用层扩展 BSP：重写库函数的位置
│  ├─ platform/                厂商 SDK（头文件 + 静态库）
│  └─ projects/standard/       Code::Blocks 工程
│     ├─ main.c                入口与 4 种测试模式分派
│     ├─ config.h              编译期配置
│     ├─ config.c              配置读取包装
│     ├─ xcfg.h                配置工具生成的运行时配置
│     ├─ ram.ld                链接脚本
│     └─ app.cbp               Code::Blocks 工程
└─ docs/
   ├─ I2S驱动资料.md            530X IIS 示例驱动
   ├─ 待完成任务.md              任务原文
   └─ SDK入门与任务实施指南.md     详细入门与任务指南（含首次日志分析）
```

## 工具链

- **IDE**：Code::Blocks（工程已包含 `.cbp`）；
- **编译器**：厂商 `riscv32`（RISC-V 32-bit，`-march=rv32imacxbs1 -Os`）；
- **下载器**：厂商 `BT8920A2` Downloader（烧录 `Output/bin/app.dcf`）。

## 入门顺序

1. 阅读 [docs/SDK入门与任务实施指南.md](docs/SDK入门与任务实施指南.md) 第 0～7 章；
2. 在 Code::Blocks 打开 `app/projects/standard/app.cbp`；
3. 第一次 Build 成功后烧录到开发板；
4. 验证 `TEST_PCM2DAC` 模式能输出 1 kHz 正弦波（基线，详见指南第 21 章）；
5. 按指南第 14～15 章顺序逐项完成任务 A/B/C/D。

## 任务状态

| 任务 | 状态 | 提交/文档 |
|---|---|---|
| 工程基线（PC1：Code::Blocks Build + 烧录 + 1 kHz 声音） | ✅ | [首次日志分析](docs/SDK入门与任务实施指南.md#21-第一次运行日志逐行分析) |
| 任务 A1 第 1 步：16 kHz 采样率（sine8k1k 数组）| ✅ | [A1 步骤 1 验证记录](docs/SDK入门与任务实施指南.md#22-任务-a1-第-1-步验证采样率-8k--16k) |
| 任务 A1 第 2 步：32 点/500 Hz 正弦表 | ⏳ | 待完成 |
| 任务 A1 第 3 步：16 点/1 kHz + 8 点/2 kHz 正弦表 | ⏳ | 待完成 |
| 任务 A2：AUX ADC → DAC（重写 `auxadc_pcm_to_dac`） | ⏳ | 待完成 |
| 任务 A3：IIS Master SRCTX（重写 `iis_master_srctx_init`） | ⏳ | 待完成 |
| 任务 A4：IIS Slave RAMRX → DAC（重写 `iis_slave_ram_rx_2_dac`） | ⏳ | 待完成 |
| 任务 B：SDDAC 实战 | ⏳ | 待完成 |
| 任务 C：SDADC 实战 | ⏳ | 待完成 |
| 任务 D：IIS 端到端链路 | ⏳ | 待完成 |

## 仓库地址

```text
git@github.com:zglstudylinux/APP_8920A2_DAC_ADC_IIS_NEW_TEST.git
```

## 维护约定

- 每次完成一个子任务：先改代码 → Build → 烧录验证 → 把日志、示波器/Audacity 记录与说明追加到 `docs/SDK入门与任务实施指南.md` 或新文档 → `git commit` → `git push`。
- 不要提交 `app/projects/standard/Output/` 下的生成产物（已在 `.gitignore` 中忽略）。
- 不要手改 `Output/bin/xcfg.*` 和 `Output/bin/res.*`，它们由 prebuild 与配置工具重新生成。
