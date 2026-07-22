#ifndef _BSP_DAC_EXT_H
#define _BSP_DAC_EXT_H

//数字音量表
#define MAX_DIG_VAL         32767
#define DIG_N0DB           (MAX_DIG_VAL / 1.000000)
#define DIG_N1DB           (MAX_DIG_VAL / 1.122018)
#define DIG_N2DB           (MAX_DIG_VAL / 1.258925)
#define DIG_N3DB           (MAX_DIG_VAL / 1.412538)
#define DIG_N4DB           (MAX_DIG_VAL / 1.584893)
#define DIG_N5DB           (MAX_DIG_VAL / 1.778279)
#define DIG_N6DB           (MAX_DIG_VAL / 1.995262)
#define DIG_N7DB           (MAX_DIG_VAL / 2.238721)
#define DIG_N8DB           (MAX_DIG_VAL / 2.511886)
#define DIG_N9DB           (MAX_DIG_VAL / 2.818383)
#define DIG_N10DB           (MAX_DIG_VAL / 3.162278)
#define DIG_N11DB           (MAX_DIG_VAL / 3.548134)
#define DIG_N12DB           (MAX_DIG_VAL / 3.981072)
#define DIG_N13DB           (MAX_DIG_VAL / 4.466836)
#define DIG_N14DB           (MAX_DIG_VAL / 5.011872)
#define DIG_N15DB           (MAX_DIG_VAL / 5.623413)
#define DIG_N16DB           (MAX_DIG_VAL / 6.309573)
#define DIG_N17DB           (MAX_DIG_VAL / 7.079458)
#define DIG_N18DB           (MAX_DIG_VAL / 7.943282)
#define DIG_N19DB           (MAX_DIG_VAL / 8.912509)
#define DIG_N20DB           (MAX_DIG_VAL / 10.000000)
#define DIG_N21DB           (MAX_DIG_VAL / 11.220185)
#define DIG_N22DB           (MAX_DIG_VAL / 12.589254)
#define DIG_N23DB           (MAX_DIG_VAL / 14.125375)
#define DIG_N24DB           (MAX_DIG_VAL / 15.848932)
#define DIG_N25DB           (MAX_DIG_VAL / 17.782794)
#define DIG_N26DB           (MAX_DIG_VAL / 19.952623)
#define DIG_N27DB           (MAX_DIG_VAL / 22.387211)
#define DIG_N28DB           (MAX_DIG_VAL / 25.118864)
#define DIG_N29DB           (MAX_DIG_VAL / 28.183829)
#define DIG_N30DB           (MAX_DIG_VAL / 31.622777)
#define DIG_N31DB           (MAX_DIG_VAL / 35.481339)
#define DIG_N32DB           (MAX_DIG_VAL / 39.810717)
#define DIG_N33DB           (MAX_DIG_VAL / 44.668359)
#define DIG_N34DB           (MAX_DIG_VAL / 50.118723)
#define DIG_N35DB           (MAX_DIG_VAL / 56.234133)
#define DIG_N36DB           (MAX_DIG_VAL / 63.095734)
#define DIG_N37DB           (MAX_DIG_VAL / 70.794578)
#define DIG_N38DB           (MAX_DIG_VAL / 79.432823)
#define DIG_N39DB           (MAX_DIG_VAL / 89.125094)
#define DIG_N40DB           (MAX_DIG_VAL / 100.000000)
#define DIG_N41DB           (MAX_DIG_VAL / 112.201845)
#define DIG_N42DB           (MAX_DIG_VAL / 125.892541)
#define DIG_N43DB           (MAX_DIG_VAL / 141.253754)
#define DIG_N44DB           (MAX_DIG_VAL / 158.489319)
#define DIG_N45DB           (MAX_DIG_VAL / 177.827941)
#define DIG_N46DB           (MAX_DIG_VAL / 199.526231)
#define DIG_N47DB           (MAX_DIG_VAL / 223.872114)
#define DIG_N48DB           (MAX_DIG_VAL / 251.188643)
#define DIG_N49DB           (MAX_DIG_VAL / 281.838293)
#define DIG_N50DB           (MAX_DIG_VAL / 316.227766)
#define DIG_N51DB           (MAX_DIG_VAL / 354.813389)
#define DIG_N52DB           (MAX_DIG_VAL / 398.107171)
#define DIG_N53DB           (MAX_DIG_VAL / 446.683592)
#define DIG_N54DB           (MAX_DIG_VAL / 501.187234)
#define DIG_N55DB           (MAX_DIG_VAL / 562.341325)
#define DIG_N56DB           (MAX_DIG_VAL / 630.957344)
#define DIG_N57DB           (MAX_DIG_VAL / 707.945784)
#define DIG_N58DB           (MAX_DIG_VAL / 794.328235)
#define DIG_N59DB           (MAX_DIG_VAL / 891.250938)
#define DIG_N60DB           0

//模拟音量表
#define N_54DB       0x0F        //000 1111
#define N_53DB       0x1F        //001 1111
#define N_52DB       0x2F        //010 1111
#define N_51DB       0x3F        //011 1111
#define N_50DB       0x4F        //100 1111
#define N_49DB       0x5F        //101 1111
#define N_48DB       0x0E        //000 1110
#define N_47DB       0x1E        //001 1110
#define N_46DB       0x2E        //010 1110
#define N_45DB       0x3E        //011 1110
#define N_44DB       0x4E        //100 1110
#define N_43DB       0x5E        //101 1110
#define N_42DB       0x0D        //000 1101
#define N_41DB       0x1D        //001 1101
#define N_40DB       0x2D        //010 1101
#define N_39DB       0x3D        //011 1101
#define N_38DB       0x4D        //100 1101
#define N_37DB       0x5D        //101 1101
#define N_36DB       0x0C        //000 1100
#define N_35DB       0x1C        //001 1100
#define N_34DB       0x2C        //010 1100
#define N_33DB       0x3C        //011 1100
#define N_32DB       0x4C        //100 1100
#define N_31DB       0x5C        //101 1100
#define N_30DB       0x0B        //000 1011
#define N_29DB       0x1B        //001 1011
#define N_28DB       0x2B        //010 1011
#define N_27DB       0x3B        //011 1011
#define N_26DB       0x4B        //100 1011
#define N_25DB       0x5B        //101 1011
#define N_24DB       0x0A        //000 1010
#define N_23DB       0x1A        //001 1010
#define N_22DB       0x2A        //010 1010
#define N_21DB       0x3A        //011 1010
#define N_20DB       0x4A        //100 1010
#define N_19DB       0x5A        //101 1010
#define N_18DB       0x09        //000 1001
#define N_17DB       0x19        //001 1001
#define N_16DB       0x29        //010 1001
#define N_15DB       0x39        //011 1001
#define N_14DB       0x49        //100 1001
#define N_13DB       0x59        //101 1001
#define N_12DB       0x08        //000 1000
#define N_11DB       0x18        //001 1000
#define N_10DB       0x28        //010 1000
#define N_9DB        0x07        //000 0111
#define N_8DB        0x06        //000 0110
#define N_7DB        0x05        //000 0101
#define N_6DB        0x04        //000 0100
#define N_5DB        0x03        //000 0011
#define N_4DB        0x02        //000 0010
#define N_3DB        0x01        //000 0001
#define N_2DB        0x00        //000 0000
#define N_1DB        0x10        //001 0000
#define N_0DB        0x20        //010 0000
#define P_1DB        0x30        //011 0000
#define P_2DB        0x40        //100 0000
#define P_3DB        0x50        //101 0000
#define P_4DB        0x60        //110 0000
#define P_5DB        0x70        //111 0000


void pmu_ldo_init(void);
void dac_set_dvol(u16 vol);
void dac_set_avol(u16 vol_idx);
void dac_spr_set(uint spr);
void dac_init(void);
void audio_pll_init(u8 out_spr);
void dac_power_on(u8 dac_type);

#endif
