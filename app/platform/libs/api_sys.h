#ifndef _API_SYS_H_
#define _API_SYS_H_

#define IRQ_BT_VECTOR                   1
#define IRQ_SW_VECTOR                   2
#define IRQ_TMR0_VECTOR                 3
#define IRQ_TMR1_VECTOR                 4
#define IRQ_TMR2_VECTOR                 5
#define IRQ_IRRX_VECTOR                 6
#define IRQ_USB_VECTOR                  7
#define IRQ_SD_VECTOR                   8
#define IRQ_AUBUF_VECTOR                9
#define IRQ_SDADC_VECTOR                10
#define IRQ_AUDEC_VECTOR                11      //mp3, sbc interrupt
#define IRQ_CVSD_VECTOR                 12
#define IRQ_PIANO_VECTOR                13      //piano, sdadc sample interrupt
#define IRQ_UART_VECTOR                 14
#define IRQ_HUART_VECTOR                15
#define IRQ_TMR3_VECTOR                 16
#define IRQ_TMR4_VECTOR                 17
#define IRQ_TMR5_VECTOR                 18
#define IRQ_GPDMA_VECTOR                19
#define IRQ_SPI_VECTOR                  20
#define IRQ_KEY_VECTOR                  21
#define IRQ_BTMDM_VECTOR                22
#define IRQ_TKEY_VECTOR                 23      //FMDET, TK
#define IRQ_AUDMA_VECTOR                24
#define IRQ_SRC_VECTOR                  25
#define IRQ_PORT_VECTOR                 26
#define IRQ_I2S_VECTOR                  27
#define IRQ_SARADC_VECTOR               28
#define IRQ_RTC_VECTOR                  29
#define IRQ_I2C_VECTOR                  30
#define IRQ_TICK_VECTOR                 31
#define IRQ_TOTAL_NUM                   32      //max irq

#define SW_RST_DC_IN                    1
#define SW_RST_QTEST_NORMAL             2
#define SW_RST_QTEST_ONLY_PAIR          3
#define SW_RST_FLAG                     4 //不管什么情况，直接复位

typedef void (*isr_t)(void);
isr_t register_isr(int vector, isr_t isr);

extern u16 tmr5ms_cnt;

void sys_set_tmr_enable(bool tmr5ms_en, bool tmr1ms_en);
u32 sys_get_rand_key(void);

void xosc_init(void);

bool sys_irq_init(int vector, int pr, isr_t isr);           //初始化一个中断, pr为优先级

u16 get_random(u16 num);                                    //获取[0, num-1]之间的随机数
void sys_clk_output(u32 type, u32 mapping);                 //输出时钟   type: 0->xosc26m, 1->x26m_13m, 2->12m, 3->24m

u16 convert_uni2gbk(u16 code, u32 addr, u32 len);           //输入utf16, 返回gbk编码, 找不到编码，使用'□'
u8 utf8_char_size(u8 code);                                 //返回UTF8占用几字节
u16 utf8_convert_to_unicode(u8 *in, u8 char_size);          //UTF-8转成UTF-16 LE编码

void sw_reset_kick(u8 source);
u8 sw_reset_source_get(void);
void set_uart0_baud(u32 baud);
void set_uart1_baud(u32 baud);
void set_uart1_baud_rc2m_clk(u32 baud);
int s_abs(int x);

u32 timer_get_tick(void);                   //获取时钟Tick

void sys_pe4_power_gate_set(u8 on);         //PE4供电开关

bool bt_get_ft_trim_value(void *rf_param);
#endif // _API_SYS_H_

