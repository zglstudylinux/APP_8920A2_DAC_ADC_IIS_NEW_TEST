#include "include.h"
#include "bsp_dac_ext.h"
#include "bsp_iis_ext.h"

xcfg_cb_t xcfg_cb;
sys_cb_t sys_cb AT(.buf.bsp.sys_cb);

//timer tick interrupt(1ms)
AT(.com_text.timer)
void usr_tmr1ms_isr(void)
{
}

//timer tick interrupt(5ms)
AT(.com_text.timer)
void usr_tmr5ms_thread(void)
{

}

//UART0打印信息输出GPIO选择，UART0默认G1(PA7)
void uart0_mapping_sel(void)
{
    //等待uart0发送完成
    if(UART0CON & BIT(0)) {
        while (!(UART0CON & BIT(8)));
    }

    GPIOADE  &= ~BIT(7);
    GPIOAFEN &= ~BIT(7);
    GPIOAPU  &= ~BIT(7);
    GPIOBFEN &= ~(BIT(2) | BIT(3));
    GPIOBPU  &= ~(BIT(2) | BIT(3));
    FUNCMCON0 = (0xf << 12) | (0xf << 8);           //clear uart0 mapping

#if (UART0_PRINTF_SEL == PRINTF_PA7)
    GPIOADE  |= BIT(7);
    GPIOAPU  |= BIT(7);
    GPIOADIR |= BIT(7);
    GPIOAFEN |= BIT(7);
    GPIOADRV |= BIT(7);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PA7;           //RX0 Map To TX0, TX0 Map to G1
#elif (UART0_PRINTF_SEL == PRINTF_PB2)
    GPIOBDE  |= BIT(2);
    GPIOBPU  |= BIT(2);
    GPIOBDIR |= BIT(2);
    GPIOBFEN |= BIT(2);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PB2;           //RX0 Map To TX0, TX0 Map to G2
#elif (UART0_PRINTF_SEL == PRINTF_PB3)
    GPIOBDE  |= BIT(3);
    GPIOBPU  |= BIT(3);
    GPIOBDIR |= BIT(3);
    GPIOBFEN |= BIT(3);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PB3;           //RX0 Map To TX0, TX0 Map to G3
#elif (UART0_PRINTF_SEL == PRINTF_PE7)
    GPIOEDE  |= BIT(7);
    GPIOEPU  |= BIT(7);
    GPIOEDIR |= BIT(7);
    GPIOEFEN |= BIT(7);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PE7;           //RX0 Map To TX0, TX0 Map to G4
#elif (UART0_PRINTF_SEL == PRINTF_PA1)
    GPIOADE  |= BIT(1);
    GPIOAPU  |= BIT(1);
    GPIOADIR |= BIT(1);
    GPIOAFEN |= BIT(1);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PA1;           //RX0 Map To TX0, TX0 Map to G5
#elif (UART0_PRINTF_SEL == PRINTF_PE0)
    GPIOEDE  |= BIT(0);
    GPIOEPU  |= BIT(0);
    GPIOEDIR |= BIT(0);
    GPIOEFEN |= BIT(0);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PE0;           //RX0 Map To TX0, TX0 Map to G6
#elif (UART0_PRINTF_SEL == PRINTF_PF2)
    GPIOFDE  |= BIT(0);
    GPIOFPU  |= BIT(0);
    GPIOFDIR |= BIT(0);
    GPIOFFEN |= BIT(0);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PF2;           //RX0 Map To TX0, TX0 Map to G7
#elif (UART0_PRINTF_SEL == PRINTF_VUSB)
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_VUSB;          //RX0 Map To TX0, TX0 Map to G8
#endif
}

//开user timer前初始化的内容
//AT(.text.bsp.sys.init)
//static void bsp_var_init(void)
//{
//    memset(&sys_cb, 0, sizeof(sys_cb));
//    sys_cb.ms_ticks = tick_get();
//}

AT(.text.bsp.sys.init)
static void bsp_io_init(void)
{
    GPIOADE = BIT(7);           //UART
    GPIOBDE = 0;
    GPIOEDE = 0;
    GPIOFDE = 0;
    GPIOGDE = 0x3F;             //MCP FLASH
    uart0_mapping_sel();        //调试UART IO选择或关闭
}

void bsp_get_xosc_xcfg(u8 *osci_cap, u8 *osco_cap, u8 *both_cap)
{
    *osci_cap = xcfg_cb.osci_cap;
    *osco_cap = xcfg_cb.osco_cap;
    *both_cap = xcfg_cb.osc_both_cap;
}

AT(.text.bsp.sys.init)
void bsp_update_init(void)
{
}

AT(.com_text.sys)
bool spiflash_speed_up_en(void)   //SPI FLASH提速。部份FLASH不支持提速
{
#if SPIFLASH_SPEED_UP_EN
    return true;
#else
    return false;
#endif
}

void dac_init_org(void);
void auxadc_init(void);

void bsp_sys_init(void)
{
    if (!xcfg_init(&xcfg_cb, sizeof(xcfg_cb))) {           //获取配置参数
        printf("xcfg init error\n");
    }
    set_sys_clk(SYS_CLK_SEL);
    bsp_io_init();
    sys_set_tmr_enable(1, 1);  //开了这里后，delay_5ms才有效
}


static u32 pcm_cnt = 0;
void print_audio_sfr_info(void)//打印AUDIO查看相关寄存器查看
{
    static u32 ticks = 0;
    if (tick_check_expire(ticks,1000)) {  //1s打印一次
        ticks = tick_get();
        printf("\n==>SampleRate = %d\n",pcm_cnt);
        printf("==>Dacbuf Sta[%d/%d], dvol=0x%X,avol=0x%X, PHSC[%d]= 0x%X,DACDIGCON0= 0x%X\n",AUBUFFIFOCNT&0xFFFF,(u16)AUBUFSIZE,DACVOLCON&0x7FFF,AUANGCON3 &0x7F,((DACDIGCON0&BIT(6))>>6),PHASECOMP,DACDIGCON0);
        printf("CLKCON[0~2]: 0x%X,0x%X,0x%X\n",CLKCON0,CLKCON1,CLKCON2);
        my_printf("PWRCON[0~2] = 0x%08X,0x%08X,0x%X\n",PWRCON0,PWRCON1,PWRCON2);
        my_printf("RTCCON[0~15] = 0x%08X,0x%08X,0x%08X,0x%08X,0x%08X, [5] = 0x%08X,0x%08X,0x%08X,0x%08X,0x%08X, [10] = 0x%08X,0x%08X,0x%08X,0x%08X,0x%08X\n",RTCCON0,RTCCON1,RTCCON2,RTCCON3,RTCCON4,RTCCON5,RTCCON6,RTCCON7,RTCCON8,RTCCON9,RTCCON10,RTCCON11,RTCCON12,RTCCON13,RTCCON14,RTCCON15);
        my_printf("AUANGCON[0~4] = 0x%08X,0x%08X,0x%08X,0x%08X,0x%08X\nAUANGCON[5~10] = 0x%08X,0x%08X,0x%08X,0x%08X,0x%08X,0x%08X\n",AUANGCON0,AUANGCON1,AUANGCON2,AUANGCON3,AUANGCON4,AUANGCON5,AUANGCON6,AUANGCON7,AUANGCON8,AUANGCON9,AUANGCON10);
        pcm_cnt = 0;
    }
}

void clk_test_out(void)  //系统内部时钟输出测试
{
    CLKCON0 &= ~(0x0F << 13);
    CLKCON2 &= ~(0x1f << 17);
    CLKCON0 |= (12 << 13);     //test clk sel  4 :pll0div2    //12: dac_clkpp
    CLKCON2 |= (9 << 17);      //测试时钟10分频输出
    GPIOADE |= BIT(5);
    GPIOADIR &= ~BIT(5);
    FUNCMCON1 = 0x0F;    //clear all mapping
    FUNCMCON1 = 0x01;    //map to G1, PA5
    printf("clk_test_out init finish\n");
    WDT_DIS(); while(1);
}


unsigned char sine8k1k[32] = {
        0x00, 0x00, 0x00, 0x00, 0x5C, 0x2D, 0x5C, 0x2D, 0x27, 0x40, 0x27, 0x40, 0x5C, 0x2D, 0x5C, 0x2D,
        0x00, 0x00, 0x00, 0x00, 0xA4, 0xD2, 0xA4, 0xD2, 0xD9, 0xBF, 0xD9, 0xBF, 0xA4, 0xD2, 0xA4, 0xD2,
};

//A1 第 2 步：16 kHz 采样率 + 32 点/周期 = 500 Hz 正弦表
//幅值与 sine8k1k 一致 (peak = 0x4027 = 16423), 16-bit 数据复制到 u32 高/低半字
unsigned char sine_16k_500hz[128] = {
        0x00, 0x00, 0x00, 0x00, 0x84, 0x0C, 0x84, 0x0C, 0x8D, 0x18, 0x8D, 0x18, 0xA4, 0x23, 0xA4, 0x23,
        0x5D, 0x2D, 0x5D, 0x2D, 0x57, 0x35, 0x57, 0x35, 0x45, 0x3B, 0x45, 0x3B, 0xEB, 0x3E, 0xEB, 0x3E,
        0x27, 0x40, 0x27, 0x40, 0xEB, 0x3E, 0xEB, 0x3E, 0x45, 0x3B, 0x45, 0x3B, 0x57, 0x35, 0x57, 0x35,
        0x5D, 0x2D, 0x5D, 0x2D, 0xA4, 0x23, 0xA4, 0x23, 0x8D, 0x18, 0x8D, 0x18, 0x84, 0x0C, 0x84, 0x0C,
        0x00, 0x00, 0x00, 0x00, 0x7C, 0xF3, 0x7C, 0xF3, 0x73, 0xE7, 0x73, 0xE7, 0x5C, 0xDC, 0x5C, 0xDC,
        0xA3, 0xD2, 0xA3, 0xD2, 0xA9, 0xCA, 0xA9, 0xCA, 0xBB, 0xC4, 0xBB, 0xC4, 0x15, 0xC1, 0x15, 0xC1,
        0xD9, 0xBF, 0xD9, 0xBF, 0x15, 0xC1, 0x15, 0xC1, 0xBB, 0xC4, 0xBB, 0xC4, 0xA9, 0xCA, 0xA9, 0xCA,
        0xA3, 0xD2, 0xA3, 0xD2, 0x5C, 0xDC, 0x5C, 0xDC, 0x73, 0xE7, 0x73, 0xE7, 0x7C, 0xF3, 0x7C, 0xF3,
};

//A1 第 3 步：16 kHz 采样率 + 16 点/周期 = 1000 Hz 正弦表
unsigned char sine_16k_1khz[64] = {
        0x00, 0x00, 0x00, 0x00, 0x8D, 0x18, 0x8D, 0x18,
        0x5D, 0x2D, 0x5D, 0x2D, 0x45, 0x3B, 0x45, 0x3B,
        0x27, 0x40, 0x27, 0x40, 0x45, 0x3B, 0x45, 0x3B,
        0x5D, 0x2D, 0x5D, 0x2D, 0x8D, 0x18, 0x8D, 0x18,
        0x00, 0x00, 0x00, 0x00, 0x73, 0xE7, 0x73, 0xE7,
        0xA3, 0xD2, 0xA3, 0xD2, 0xBB, 0xC4, 0xBB, 0xC4,
        0xD9, 0xBF, 0xD9, 0xBF, 0xBB, 0xC4, 0xBB, 0xC4,
        0xA3, 0xD2, 0xA3, 0xD2, 0x73, 0xE7, 0x73, 0xE7,
};

//A1 第 3 步：16 kHz 采样率 + 8 点/周期 = 2000 Hz 正弦表
unsigned char sine_16k_2khz[32] = {
        0x00, 0x00, 0x00, 0x00, 0x5D, 0x2D, 0x5D, 0x2D,
        0x27, 0x40, 0x27, 0x40, 0x5D, 0x2D, 0x5D, 0x2D,
        0x00, 0x00, 0x00, 0x00, 0xA3, 0xD2, 0xA3, 0xD2,
        0xD9, 0xBF, 0xD9, 0xBF, 0xA3, 0xD2, 0xA3, 0xD2,
};

//A1 第 3 步：编译时切换频率 (0=500Hz, 1=1kHz, 2=2kHz)
#ifndef A1_CUR_FREQ
#define A1_CUR_FREQ    0
#endif

#if (A1_CUR_FREQ == 0)
#define A1_SINE_TABLE      sine_16k_500hz
#define A1_SINE_TABLE_SIZE sizeof(sine_16k_500hz)
#elif (A1_CUR_FREQ == 1)
#define A1_SINE_TABLE      sine_16k_1khz
#define A1_SINE_TABLE_SIZE sizeof(sine_16k_1khz)
#elif (A1_CUR_FREQ == 2)
#define A1_SINE_TABLE      sine_16k_2khz
#define A1_SINE_TABLE_SIZE sizeof(sine_16k_2khz)
#else
#error "A1_CUR_FREQ must be 0 (500Hz), 1 (1kHz), or 2 (2kHz)"
#endif

void test_pcm2dac(void)
{
    WDT_DIS();
    dac_spr_set(SPR_16000);  //DAC采样率 16K
    dac_set_dvol(DIG_N0DB);  //设置数字音量,最大0DB
    dac_set_avol(50);        //设置模拟音量,0~59. (avol=50 -> N_4DB = -4 dB)
    u32 *pcm_buf = (u32*)A1_SINE_TABLE;  //A1 第 3 步: 编译时切换频率
    u32 i = 0;
    while(1) {
        print_audio_sfr_info();
        if((AUBUFCON & BIT(8)) == 0) {  //AUBUFCON BIT(8)为1时表示DACBUF已满,为0表示DACBUF未满,可以往里面写数据
            AUBUFDATA = pcm_buf[i];     //通过AUBUFDATA向DACBUF写数据,DACBUF内部大约有2K的缓存
            pcm_cnt++;
            i++;
            if (i >= A1_SINE_TABLE_SIZE/4) {
                i = 0;
            }
        }
    }
}

