#include "include.h"
#include "bsp_dac_ext.h"
#include "bsp_iis_ext.h"
#include "bsp_adc_aux_ext.h"

//A3 任务: 在应用层重写 iis_master_srctx_init()
//
//使用 GCC --wrap 链接选项: 库代码里所有 iis_master_srctx_init() 调用
//会自动改写到 __wrap_iis_master_srctx_init() (本函数)。
//
//A3 整体架构:
//   test_aux_adc2dac_for_a3()  ->  AUX (PB1/PB2) -> SDADC 48K -> DMA -> DAC FIFO
//   __wrap_iis_master_srctx_init() ->  把 DAC FIFO 中的数据通过 IIS SRCTX 同步推出
//   用户在 IIS PE5/PE6/PE7 (BCLK/LRC/DO) 三个 IO 上能看到与 AUX 输入同步的音频数据
//
//IO 映射 (8920A2 开发板可用引脚: PB1, PB2, PE4, PE5, PE6, PE7; PA3-PA7 未引出):
//   PB1 = AUX-L 模拟输入
//   PB2 = AUX-R 模拟输入
//   PE5 = IIS BCLK (master 输出)
//   PE6 = IIS LRC  (master 输出)
//   PE7 = IIS DO   (master 输出)
//   MCLK 必须禁用 (MCLK_OUT_DIS), 避免和 PB1 (AUX-L 模拟输入) 冲突
//
//================================================================================
// !! 重要提示 !!!
//================================================================================
// 8920A2 的 IIS SFR 地址在 sfr.h 里没有暴露, 上面加的 IISCON0/IISBAUD/IISDMA*
// 是 EDUCATED GUESSES (530X 文档 + DAC cluster 间距推测), 实际地址可能不同。
//
// 验证方法: 在你的 Windows Code::Blocks 工程里跑:
//   cd <sdk>\app\platform\libs
//   riscv32-elf-objdump -d libdrivers.a > iis_disasm.txt
//   grep -A 80 "iis_master_srctx_init>:" iis_disasm.txt
// 反汇编结果里所有 sw/lw 指令的立即数就是 SFR 偏移 (相对 SFR_BASE=0)。
// 找到真实地址后, 同步修改 sfr.h 里 IISCON0/IISBAUD 等宏, 重新编译测试。
//================================================================================

AT(.com_text.iis_ext)
void __wrap_iis_master_srctx_init(void)
{
    printf("\n--->%s (A3 PC3+PC4)\n", __func__);

    //-----------------------------------------------------------------------
    // 1) GPIO 配置 (PC3)
    //-----------------------------------------------------------------------
    //PE5 = BCLK 输出, PE6 = LRC 输出, PE7 = DO 输出
    GPIOEDE |= BIT(5) | BIT(6) | BIT(7);          // 数字使能
    GPIOEFEN |= BIT(5) | BIT(6) | BIT(7);         // 功能使能 (走 IIS 模块)
    GPIOEDIR &= ~(BIT(5) | BIT(6) | BIT(7));      // 输出
    GPIOEDRV |= BIT(5) | BIT(6) | BIT(7);         // 高速驱动 (IIS 时钟高速)
    GPIOEPU &= ~(BIT(5) | BIT(6) | BIT(7));       // 关闭上拉

    //PB1 = MCLK 输出位置, A3 关闭 MCLK -> 配 input+pull-down 防止浮空
    GPIOBDE |= BIT(1);                            // 数字使能 (留作 fallback)
    GPIOBDIR |= BIT(1);                           // input
    GPIOBPU &= ~BIT(1);                           // 不要上拉, 用下拉避免漏电流
    GPIOBFEN &= ~BIT(1);                          // 关闭功能映射, PB1 完全归 AUX-L 模拟

    //-----------------------------------------------------------------------
    // 2) FUNCMCON2: IIS IO 复用选择
    //-----------------------------------------------------------------------
    //530X 模板里 0x02 表示 GPIOE 路径, 8920A2 实际值可能不同
    //先按 0x02 试, 如果 BCLK 没出来再扫描 {0x01, 0x02, 0x04, 0x08}
    FUNCMCON2 = 0x02;

    //-----------------------------------------------------------------------
    // 3) IIS 时钟门控 + 时钟源选择 (PC3)
    //-----------------------------------------------------------------------
    CLKGAT1 |= BIT(4);                            // IIS 模块时钟使能
    CLKGAT0 |= BIT(12);                           // IIS 时钟源使能
    CLKCON3 = (CLKCON3 & ~(0x0F << 8));           // iis_clk_div = 0
    CLKCON1 = (CLKCON1 & ~(0x03 << 8));           // iis_clk_ch = 0 (dac_clk)

    printf("  GPIO+CLOCK ok, PE5=BCLK PE6=LRC PE7=DO, MCLK_DIS, FUNCMCON2=0x%X\n", FUNCMCON2);
    printf("  CLKGAT1=0x%X CLKGAT0=0x%X CLKCON3=0x%X CLKCON1=0x%X\n",
           CLKGAT1, CLKGAT0, CLKCON3, CLKCON1);

    //-----------------------------------------------------------------------
    // 4) PC4: IIS 内部寄存器配置 (SRCTX 模式: 不需要 DMA)
    //-----------------------------------------------------------------------
    // 4.1 清 pending bits (按 530X 模板, 写 1 清)
    IISCON0 = BIT(16) | BIT(17);
    IISCON0 = 0;

    // 4.2 配置 IISCON0
    //   bit(0)  = 1   enable
    //   bit(1)  = 0   master mode (BIT(1)=0)
    //   bit(2)  = 0   16-bit mode
    //   bit(3)  = 1   normal mode (data delay 1 clock after WS change)
    //   bit(4)  = 0   data source: 不设 (SRCTX 模式从 DAC SRC 取, 不走 RAM)
    //   bit(5)  = 0   DMA output disabled (SRCTX 不需要 DMA)
    //   bit(6)  = 0   DMA input disabled
    //   bit(7)  = 0   TX DMA interrupt disabled
    //   bit(8)  = 0   RX DMA interrupt disabled
    //   bit(10) = 1   DMA output request mask delay enable (530X 推荐)
    //   bit(16) = 0   TX pending (just cleared)
    //   bit(17) = 0   RX pending (just cleared)
    u32 iiscon = BIT(0) | BIT(3) | BIT(10);    // 0x0409

    // 4.3 配置 IISBAUD (48k LRC, 16-bit, MCLK 256fs, DAC clock ≈ 24.576 MHz)
    //   MCLK = 48k * 256 = 12.288 MHz
    //   BCLK = 48k * 32  = 1.536 MHz
    //   mclk_div = DAC_CLK / MCLK = 24.576 / 12.288 = 2  -> reg = mdiv-1 = 1
    //   bclk_div = MCLK / BCLK    = 12.288 / 1.536  = 8  -> reg = bdiv-1 = 7
    //   IISBAUD[6:0]  = mdiv-1 = 1
    //   IISBAUD[11:7] = bdiv-1 = 7
    u32 iisbaud = (7 << 7) | 1;                 // 0x0381

    // 4.4 启用 DAC -> IIS SRCTX 路径 (DACDIGCON0 BIT(23))
    DACDIGCON0 |= BIT(23);

    // 4.5 先写 IISBAUD, 再写 IISCON0 (bit(0)=EN 必须最后)
    IISBAUD = iisbaud;
    IISCON0 = iiscon;

    printf("  IISCON0=0x%X IISBAUD=0x%X DACDIGCON0=0x%X\n",
           IISCON0, IISBAUD, DACDIGCON0);
    printf("  -> 预期: 逻辑分析仪 PE5 看到 1.536MHz BCLK, PE6 看到 48kHz LRC, PE7 看到 DO 数据\n");
    printf("  -> 如果 PE5/PE6 没信号: 大概率 IISCON0/IISBAUD 地址错, 用 objdump 反汇编库验证\n");
}