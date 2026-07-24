#include "include.h"
#include "bsp_dac_ext.h"
#include "audio_sfr.h"
#define DAC_DCOC        32

#define DAC_OBUF_SIZE       576
u32 dac_obuf[DAC_OBUF_SIZE] AT(.dac_obuf);
u8 sys_aupll_type = 0;
static u32 pll_band = 0;

AT(.com_text.dac_avtbl)
const u8 tbl_dac_avol_gain[60] =  {
    N_54DB, N_53DB, N_52DB, N_51DB, N_50DB, N_49DB, N_48DB, N_47DB,
    N_46DB, N_45DB, N_44DB, N_43DB, N_42DB, N_41DB, N_40DB, N_39DB,
    N_38DB, N_37DB, N_36DB, N_35DB, N_34DB, N_33DB, N_32DB, N_31DB,
    N_30DB, N_29DB, N_28DB, N_27DB, N_26DB, N_25DB, N_24DB, N_23DB,
    N_22DB, N_21DB, N_20DB, N_19DB, N_18DB, N_17DB, N_16DB, N_15DB,
    N_14DB, N_13DB, N_12DB, N_11DB, N_10DB, N_9DB,  N_8DB,  N_7DB,
    N_6DB,  N_5DB,  N_4DB,  N_3DB,  N_2DB,  N_1DB,  N_0DB,  P_1DB,
    P_2DB,  P_3DB,  P_4DB,  P_5DB
};

AT(.com_text.dac)
void dac_set_dvol(u16 vol)
{
    DACVOLCON = vol | (0x02 << 16);
}

AT(.com_text.dac)
void dac_set_avol(u16 vol_idx)
{
    if(vol_idx >= 59) {
        vol_idx = 59;
    }
    AUANGCON3 = (AUANGCON3 & ~0x7f) | tbl_dac_avol_gain[vol_idx];
}

AT(.com_text.dac)
void dac_spr_set(uint spr)
{
    DACDIGCON0  &= ~0x3C;
    DACDIGCON0  |= (spr << 2);                          //setting Src in sample
}


AT(.text.dac)
void dac_obuf_init(void)
{
    printf("%s\n",__func__);
    memset((void *)dac_obuf, 0, DAC_OBUF_SIZE * 4);
    AUBUFCON |= BIT(18);                            //DAC high sample rate count enable
    AUBUFCON |= BIT(0);                             //Reset audio Buffer (write 1 to enter reset)
    AUBUFSIZE = (DAC_OBUF_SIZE - 1);                //Configure audio buffer size
    AUBUFSIZE |= (DAC_OBUF_SIZE - 192) << 16;       //Audio Buffer Threshold
    AUBUFSTARTADDR = DMA_ADR(dac_obuf);             //Configure audio buffer start address
    AUBUFCON &= ~BIT(0);                            //Exit FIFO reset (write 0 to release)
}

//adpll_init时band从0开始，其他情况从pll_band开始
static void pll_vco_band_trimming(u8 init)
{
    u32 band = 0, trim_flag, i;
    if (init == 0) {
        band = pll_band;
    }
    PLL0CON1 |= BIT(11);                                    //DI_EN_TRIM = 1
    PLL0CON1 = (PLL0CON1 & ~0x1f) | band++;
    band = band & 0x1f;
    delay_us(200);
    trim_flag = PLL0CON1 & BIT(31);
    for (i = 1; i < 31; i++) {
        PLL0CON1 = (PLL0CON1 & ~0x1f) | band++;
        band = band & 0x1f;
        delay_us(200);                                          //delay 100us by rc
        if ((PLL0CON1 & BIT(31)) != trim_flag) {            //trimming flag翻转表示Done
            break;
        }
    }
    PLL0CON1 &= ~BIT(11);                                    //DI_EN_TRIM = 0
    if (init) {
        pll_band = (band>4)? (band-4) : 0;
    }
}

ALIGNED(256)
static void adpll_divider_update_do(u32 pll0div, u32 div2)
{
    if (div2) {
        CLKCON0 |= BIT(31);
        delay_us(2);
        CLKCON2  = (CLKCON2 & ~(0xf<<4)) | (0x06 << 4);                     //先设置一个安全的分频比
    } else {
        CLKCON2  = (CLKCON2 & ~(0xf<<4)) | (0x0E << 4);                     //先设置一个安全的分频比
        delay_us(2);
        CLKCON0 &= ~BIT(31);                                                //div2 disable
    }
    GLOBAL_INT_DISABLE();
    PLL0DIV = pll0div;
    PLL0CON |= BIT(20);                                                     //PLL divider update to SDM clock domain
    GLOBAL_INT_RESTORE();
    printf("******************adpll config\n");
}


static const uint8_t adpll_div_tbl[2][3] = {
    { 0x0D, 0x0B, 0x09 },
    { 0x06, 0x05, 0x04 },
};

AT(.rodata.dac)
const u8 dac_vcm_tbl_ext[6] = {1, 3, 5, 6, 7, 7};

static u32 adpll_divider_update(u32 out_spr, u32 init_flag)
{
    u32 adpll_div, pll0div, div2 = 0;

    if (out_spr) {
        if (DACDIGCON0 & BIT(27)) {                                             //anc 384k
            div2 = 1;
        }
        adpll_div = adpll_div_tbl[div2][0];                                     //adpll_div = 14
        pll0div = (344.064 * 16777216) / 24;
    } else {
        adpll_div = 0x0E;                                                       //adpll_div = 15
        pll0div = (338.688 * 16777216) / 24;
    }
    if (init_flag) {
        adpll_divider_update_do(pll0div, div2);
        return adpll_div;
    }
    if (adpll_div != ((CLKCON2 >> 4) & 0xf)) {
        adpll_divider_update_do(pll0div, div2);
        CLKCON2  = (CLKCON2 & ~(0xf<<4)) | (adpll_div << 4);                   //设置真实的分频比
        pll_vco_band_trimming(0);
//        printf("CLKCON2: %x, %x\n", (CLKCON2 >> 4) & 0xf, PLL0DIV);
    }
    return adpll_div;
}

void adpll_spr_set(u8 out48k_flag)
{
    if (DACDIGCON0 & BIT(1)) {
        if (!out48k_flag) {
			adpll_divider_update(0, 0);
            DACDIGCON0 &= ~BIT(1);                          //dac out sample rate 44.1K
            sys_aupll_type = 0;
        }
    } else {
        if (out48k_flag) {
            adpll_divider_update(1, 0);
            DACDIGCON0 |= BIT(1);                           //dac out sample rate 48K
            sys_aupll_type = 1;
        }
    }
}


//audio pll init
void audio_pll_init(u8 out_spr)
{
    u32 adpll_div;
    printf("%s, out_spr = %d\n",__func__,out_spr);
    PLL0CON1 |= BIT(15);                                    //PR SYSPLL SDM LDO EN
    delay_us(30);
    PLL0CON &= ~(BIT(16) | BIT(17));                        //PLL0 refclk select xosc26m
    PLL0CON = (PLL0CON & ~(7 << 3)) | (1 << 3);             //DI_CP_SEL=1
    PLL0CON1 = (PLL0CON1 & ~(7 << 5)) | (4 << 5);           //DI_VCO_GAIN
    PLL0CON1 = (PLL0CON1 & ~(7 << 8)) | (4 << 8);           //DI_TRIM_VOL 0.6V
    PLL0CON1 = (PLL0CON1 & ~(3 << 13)) | (3 << 13);         //DI_LDO2_SEL:1.2v
    PLL0CON &= ~BIT(13);                                    //DI_EN_DIV2 = 0
    PLL0CON |= BIT(12);                                     //enable pll0 ldo
    delay_us(500);
    PLL0CON |= BIT(18);                                     //pll0 sdm enable
    PLL0CON1 = (PLL0CON1 & ~0x1f) | 12;                     //默认vco band
    DACDIGCON0 = 0x00;
    if (out_spr) {
        DACDIGCON0 |= BIT(1);
        sys_aupll_type = 1;
    } else {
        DACDIGCON0 &= ~BIT(1);
        sys_aupll_type = 0;
    }
    adpll_div = adpll_divider_update(out_spr, 1);
    PLL0CON |= BIT(6);                                        //DI_EN_FUNC = 1
    delay_us(1000);
    pll_vco_band_trimming(1);
    CLKCON2  = (CLKCON2 & ~(0xf<<4)) | (adpll_div << 4);
    //-------------------------------------------------
    CLKCON1  = (CLKCON1 & ~(0x3<<0)) | (1 << 0);
    CLKGAT2 |= BIT(14);
    //delay_us(1000);
    //CLKCON1 = 0x80401;
    adpll_spr_set(out_spr);
}

typedef struct {
    union {
        struct {
            u32 type            : 4;   //0:单声道, 1:双声道, 2:VCMBUF单声道, 3:VCMBUF双声道, 4: VCMBUF_R MIX to VCMBUF
            u32 ldoh            : 4;   //VDDAUD LDO voltage, 0:2.4V, 1:2.5V, 2:2.7V, 3:2.9V, 4:3.1V, 5:3.2V
            u32 resv2           : 1;
            u32 fast_en         : 1;   //是否快速DAC初始化，用于有功放MUTE且需要快带开关机的方案。
            u32 vcm_capless     : 1;   //vcm capless
            u32 dacvdd_bypass   : 1;   //dacvdd bypass
            u32 dacrp_vusb      : 1;   //dacrp_vusb
        };
        u32 cfg;
    };
}dac_cb_t_ex;
static dac_cb_t_ex dac_cb;

AT(.text.dac)
void dac_power_on(u8 dac_type)
{
    dac_cb.type = dac_type;
    dac_cb.ldoh = 3;
    dac_cb.dacvdd_bypass = 1;
    dac_cb.fast_en = 1;

    u32 ldoh = dac_cb.ldoh;
    if (ldoh > 5) {
        ldoh = 5;
    }
    my_printf("dac_init, ldoh = %d\n",ldoh);

    DACDIGCON0 |= BIT(0);                                       //enable dac digital
    delay_us(260);

    CLKGAT2 |= BIT(15);
    delay_us(20);

    DI_EN_LDOL_CAPLESS(0x1);                                        //enable ldo12 capless mode
    DI_BYPASS_LDOH(0x0);	                                        //disable ldo33 bypass mode
    DI_VDCK_SEL(0x1);	                                            //select vdck voltage 1.04V

    DI_EN_LPF2PA(0);

    DI_PA_BIAS(0x00);                                               //DI_PA_BIAS, Audio PA bias current selection

    DI_EN_LDOH_CAPLESS(0);                                          //DI_DAC_CKDLY_SEL, ckdly = 0
    DI_EN_AUDPA_DLY(0);

    DI_EN_RIGHT(0x01);                                              //bit7: DI_EN_RIGHT,  bit8: DI_EN_LEFT; Audio SDDAC right & left channel enable
    DI_EN_LEFT(0x01);
    delay_us(15);

    DI_DACL_DCOC(DAC_DCOC);
    DI_DACR_DCOC(DAC_DCOC);
    my_printf("dac_cb.type = %d\n",dac_cb.type);
    my_printf("dac_cb.fast_en = %d\n",dac_cb.fast_en);

    DI_EN_BIAS(1);                                                  //DI_EN_BG, DI_EN_BG_FAST, bandgap, bandgap fast setup enable
    DI_EN_NBIAS_XR(0);
    DI_SEL_RCRES(0x3c);
    delay_us(800);
    DI_EN_NBIAS_XR(1);                                              //DI_EN_BG_FAST, Audio bandgap fast setup disable
    delay_ms(2);
    DI_BYPASS_LDOH(1);
    DI_LDOH_SEL(ldoh);                                          //DI_LDOH_SEL, VDDAUD LDO voltage select: 2.5/2.7/2.9/3.0/3.1/3.2
    DI_EN_LDOH(1);                                              //DI_EN_LDOH, VDDAUD LDO enable
    delay_ms(2);
    my_printf("dac_cb.dacvdd_bypass = %d\n",dac_cb.dacvdd_bypass);
    if (dac_cb.dacvdd_bypass) {
        DI_EN_LDOH_CAPLESS(1);
        delay_us(10);
    }
    DI_BYPASS_LDOH(0);                                              //DI_BYPASS_LDOH, DI_BYPASS_LDOL, bypass = 0
    DI_EN_PA_VCM_LV(1);                                             //enable DI_EN_PA_VCM_LV = 1

    DI_VCM_CON((u32)dac_vcm_tbl_ext[ldoh]);                         //DI_VCM_CON, VCM voltage control: 1.16V/1.218V/1.276V/1.334V/1.391V/1.447V/1.50V/1.55V
    if (dac_cb.vcm_capless) {
        DI_VCM_TYPE(1);                                         //DI_VCM_TYPE, VCM with internal cap
    } else {
        DI_VCM_TYPE(0);                                         //DI_VCM_TYPE, VCM with off-chip cap
    }
    DI_EN_VCM(1);                                               //DI_EN_VCM

    delay_5ms(4);
    DI_PA_TYPE(0);                                                  //DI_PA_TYPE, 1.2V differential PA
    if (dac_cb.type < DAC_DIFF_MONO) {
        DI_PA_TYPE(3);                                              //DI_PA_TYPE, 3.3V single-ended PA
    }
    DI_EN_D2A(1);                                                   //DI_EN_D2A = 1
    DACDIGCON0 |= BIT(9);                                           //Remove Dc offset Enable

    DACVOLCON = 0;
    DACVOLCON |= BIT(20);                                           //set min digital volume
    if ((dac_cb.type == DAC_MONO) || (dac_cb.type == DAC_DUAL)) {
        my_printf("no offset triming\n");
        //dac_offset_trimming(restart);
        AUANGCON3 = (AUANGCON3 & 0xffffff80) | P_5DB;               //dac analog gain +5DB
        DACVOLCON = 0x7fff;
        DACVOLCON |= BIT(20);                                       //set max digital volume
        delay_ms(2);

        DI_EN_AUDPA(1);
        delay_us(50);
        DI_EN_SC2PA(1);
        delay_us(50);
        DI_EN_AUDPA_DLY(1);
        DI_EN_PAL_OUTSTAGE(1);
        DI_EN_PAR_OUTSTAGE(1);
        delay_us(50);
        DI_EN_PAL_FB(1);
        DI_EN_PAR_FB(1);
        delay_us(50);
    }

    DI_EN_AUDPA(1);                                                 //enable DI_EN_AUDPA = 1
    delay_us(150);
    DI_EN_AUDPA_DLY(1);                                             //enable DI_EN_AUDPA_DLY = 1

    if ((dac_cb.type == DAC_MONO) || (dac_cb.type == DAC_DUAL)) {
        delay_us(500);
        DI_EN_STRONG_PD(0);                                         //disable strong pull-down
        delay_ms(5);
        DI_WEAK_PD(0);                                              //disable weak pull-down
    }
    //DI_EN_TRIM_CMP(0);                                            //Audio trimming comparator disable
    DI_MUTE_LPF(0);                                                 //DI_MUTE_LPF, DI_MUTE_PA
    DI_MUTE_PA(0);
    CLKGAT0 |= BIT(12);                 //enable dac clk
    DACDIGCON0 |= BIT(0);               //digital dac enable
    my_printf("==>Dac[%d/%d], dvol=0x%X,avol=0x%X, PHSC[%d]= 0x%X,DACDIGCON0= 0x%X\n",AUBUFFIFOCNT&0xFFFF,(u16)AUBUFSIZE,DACVOLCON&0x7FFF,AUANGCON3 &0x7F,((DACDIGCON0&BIT(6))>>6),PHASECOMP,DACDIGCON0);
    my_printf("AUANGCONx = 0x%08X,0x%08X,0x%08X,0x%08X,0x%08X, [5] = 0x%08X,0x%08X,0x%08X,0x%08X,0x%08X,0x%08X\n",AUANGCON0,AUANGCON1,AUANGCON2,AUANGCON3,AUANGCON4,AUANGCON5,AUANGCON6,AUANGCON7,AUANGCON8,AUANGCON9,AUANGCON10);
}

void pmu_ldo_init(void)
{
    u32 rtccon8, rtccon3,rtccon5,rtccon6,temp;
    my_printf("pmu_ldo_init\n");
    RTCCON3 |= 0x07;                                                    //VDDCORE, VDDIO, BUCK enable
    rtccon3 = RTCCON3;
    rtccon3 |= BIT(5);                                                  //VDDCORE short enable
    rtccon3 &= ~BIT(3);                                                 //VDDCORE AON disable
    RTCCON3 = rtccon3;                                                  //
    delay_us(80);
    RTCCON7 = (RTCCON7 & ~0xff) | 0x22;                                 //PWRUPICH = 30mA, SWICH = 30mA
    rtccon8 = RTCCON8;
    rtccon8 &= ~0x03;                                                   //RI_EN_VIO_CHG, RTCCON8[0], 5v->3.3v ldo disable; RI_CHSTOPS = 0, RTCCON8[1]
    rtccon8 |= BIT(6);                                                  //RI_EN_CHARGER = 1, RTCCON12[6]
    RTCCON8 = rtccon8;                                                  //
//  RTCCON3 &= ~BIT(4);                                                 //RI_ENB_VDDXO = 0, XOSC to VDDBT
    RTCCON4 = (RTCCON4 & ~BIT(6)) | BIT(5);                             //RI_VCORE_SVIO = 1, RTCCON4[5], RI_VCORE_SVBT = 0, RTCCON4[6]
    PWRCON0 |= BIT(23);                                                 //enhance vddcore drive when vbat is very low
    RTCCON5 &= ~(0x3<<2);                                               //vddcore mode selection, 0-normal; 1-bypass; 2-diode mode; 3-low power ldo;
    RTCCON6 |= (0x3<<2);                                                //sniff, vddcore mode selection, 0-normal; 1-bypass; 2-diode mode; 3-low power ldo;
    RTCCON5 &= ~(0x3<<4);                                               //vddio mode selection, 0-normal; 1-bypass; 2-diode mode; 3-low power ldo;
    RTCCON6 |= (0x3<<4);                                                //sniff, vddio mode selection, 0-normal; 1-bypass; 2-diode mode; 3-low power ldo;
    RTCCON5 &= ~BIT(6);                                                 //vddbt lowpower disable
    PWRCON2 = (PWRCON2 & ~0x1f) | 0x0B;                              	//vddbt set 1.25v, RI_VDDBT_VS, PWRCON2[4:0]
    PWRCON0 = (PWRCON0 & ~0x0f) | 0x0D;                                 //vddcore set 1.35v, DI_VCORES, PWRCON0[3:0]
    rtccon5  = RTCCON5 & ~0x01;
    rtccon6  = RTCCON6 & ~0x01;
    temp = RTCCON4 | BIT(7);                        //vddbt with cap
    temp &= ~BIT(7);                                //vddbt capless
    RTCCON4 = temp;
    RTCCON5 = rtccon5 | BIT(0);                     //ldo mode
    RTCCON6 = rtccon6 | BIT(0);                     //sniff ldo mode
    PWRCON0 |= BIT(20);                                                 //0=LDO mode; 1 = 软件控制
    delay_us(10);
    RTCCON3 &= ~BIT(4);                                                 //RI_EN_VDDXO = 0
}


void dac_init(void)
{
    //DAC初始化示例
    pmu_ldo_init();
    audio_pll_init(DAC_OUT_44K1);  //DAC_OUT_44K1 / DAC_OUT_48K
    //clk_test_out();              //测试DAC的时钟源或PLL0时钟是否正常,44K1和48K配置出来的时钟源是不一样的
    dac_obuf_init();
    dac_power_on(DAC_DUAL);        //DAC_DUAL配置正常后,不播放声音时,DAC左声道及右声道均会有大约1.3V ~1.4V的偏置电压.
}














