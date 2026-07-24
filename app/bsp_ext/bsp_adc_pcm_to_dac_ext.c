#include "include.h"
#include "bsp_dac_ext.h"

//A2 任务: 在应用层重写 auxadc_pcm_to_dac()
//
//使用 GCC --wrap 链接选项: 库代码里所有 auxadc_pcm_to_dac 调用
//会自动改写到 __wrap_auxadc_pcm_to_dac (本函数)。
//
//DAC FIFO 状态:
//   AUBUFCON BIT(8) = 1 表示 FIFO 已满, BIT(8) = 0 表示未满可写
//   AUBUFCON BIT(0) = 1 表示 FIFO reset (冻结, 不消费)
//   dac_obuf_init() 写了 BIT(0)=1, 必须写 0 退出 reset 才能消费
//
//SDADC DMA 双缓冲布局 (双声道, 256 samples per half):
//   flag=1 (Half Done): buf[0..(256*2*2-1)] = L0 R0 L1 R1 ... L255 R255
//   flag=0 (Done):     buf[0..(256*2*2-1)] = L0 R0 L1 R1 ... L255 R255
//   每个 sample 是 16-bit, 一对 L+R = 4 bytes
//
//输出到 DAC FIFO (AUBUFDATA = 32-bit 容器, 左 16 + 右 16):
//   每个 AUX 立体声对打包成 (right << 16) | left。

AT(.com_text.isr.sdadc)
void __wrap_auxadc_pcm_to_dac(u8 flag, u8 *adc_buf, u16 adc_samples)
{
    u16 *buf16 = (u16 *)adc_buf;
    u16 samples = adc_samples;

    //关键: 退出 DAC FIFO reset 状态, 否则 FIFO 冻结不会消费
    AUBUFCON &= ~BIT(0);

    for (u16 i = 0; i < samples; i++) {
        if (AUBUFCON & BIT(8)) {
            //FIFO 已满, 跳过剩余
            break;
        }
        u16 left  = buf16[i * 2 + 0];
        u16 right = buf16[i * 2 + 1];
        AUBUFDATA = ((u32)right << 16) | (u32)left;
    }
}