
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>
#include <sound/tlv.h>
#include <linux/sys_config.h>
#include "sta33x.h"
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/pinconf-sunxi.h>



#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/irq.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <media/rc-core.h>


/*
#define STA33X_RATES (SNDRV_PCM_RATE_32000 | \
                      SNDRV_PCM_RATE_44100 | \
                      SNDRV_PCM_RATE_48000 | \
                      SNDRV_PCM_RATE_88200 | \
                      SNDRV_PCM_RATE_96000 | \
                      SNDRV_PCM_RATE_176400 | \
                      SNDRV_PCM_RATE_192000)
*/
#define STA33X_RATES (SNDRV_PCM_RATE_8000_192000 | SNDRV_PCM_RATE_KNOT)

/*
#define STA33X_FORMATS \
        (SNDRV_PCM_FMTBIT_S16_LE  | SNDRV_PCM_FMTBIT_S16_BE  | \
         SNDRV_PCM_FMTBIT_S18_3LE | SNDRV_PCM_FMTBIT_S18_3BE | \
         SNDRV_PCM_FMTBIT_S20_3LE | SNDRV_PCM_FMTBIT_S20_3BE | \
         SNDRV_PCM_FMTBIT_S24_3LE | SNDRV_PCM_FMTBIT_S24_3BE | \
         SNDRV_PCM_FMTBIT_S24_LE  | SNDRV_PCM_FMTBIT_S24_BE  | \
         SNDRV_PCM_FMTBIT_S32_LE  | SNDRV_PCM_FMTBIT_S32_BE) */
#define STA33X_FORMATS (SNDRV_PCM_FMTBIT_S8 | SNDRV_PCM_FMTBIT_S16_LE | \
		                     SNDRV_PCM_FMTBIT_S18_3LE | SNDRV_PCM_FMTBIT_S20_3LE |\
                                SNDRV_PCM_FMTBIT_S24_LE)




#define TYPE_INTEGER	1
#define TYPE_STRING		2
#define TYPE_GPIO		3
#define TYPE_EXT_IRQ	4

typedef struct
{
	char  gpio_name[32];
	int port;
	int port_num;
	int mul_sel;
	int pull;
	int drv_level;
	int data;
	int gpio;
} codec_gpio_set_t;



static __u32 twi_id = 2;

static u32 sta33x_pwrdn_gpio = 0;


/* regulator power supply names */
static const char *sta33x_supply_names[] = {
        "Vdda", /* analog supply, 3.3VV */
        "Vdd3", /* digital supply, 3.3V */
        "Vcc"   /* power amp spply, 10V - 36V */
};

/* codec private data */
struct sta33x_priv {
        struct regmap *regmap;
        struct regulator_bulk_data supplies[ARRAY_SIZE(sta33x_supply_names)];
        struct snd_soc_codec *codec;
        struct sta33x_platform_data *pdata;
	     void *control_data;

        unsigned int mclk;
        unsigned int mult_fs;
        unsigned int format;

        u32 coef_shadow[STA33X_COEF_COUNT];
        struct delayed_work watchdog_work;
        int shutdown;
};

struct sta33x_priv sta33x_p;

static __u32   sta33x_reset_gpio_hd;

//static const unsigned short normal_i2c[2] = {0x38,I2C_CLIENT_END};
static const unsigned short normal_i2c[2] = {0x1c,I2C_CLIENT_END};




static const DECLARE_TLV_DB_SCALE(mvol_tlv, -12700, 50, 1);
static const DECLARE_TLV_DB_SCALE(chvol_tlv, -7950, 50, 1);
static const DECLARE_TLV_DB_SCALE(tone_tlv, -120, 200, 0);

static const char *sta33x_drc_ac[] = {
        "Anti-Clipping", "Dynamic Range Compression" };
static const char *sta33x_auto_gc_mode[] = {
        "User", "AC no clipping", "AC limited clipping (10%)",
        "DRC nighttime listening mode" };
static const char *sta33x_auto_xo_mode[] = {
        "User", "80Hz", "100Hz", "120Hz", "140Hz", "160Hz", "180Hz", "200Hz",
        "220Hz", "240Hz", "260Hz", "280Hz", "300Hz", "320Hz", "340Hz", "360Hz" };
static const char *sta33x_limiter_select[] = {
        "Limiter Disabled", "Limiter #1", "Limiter #2" };
static const char *sta33x_limiter_attack_rate[] = {
        "3.1584", "2.7072", "2.2560", "1.8048", "1.3536", "0.9024",
        "0.4512", "0.2256", "0.1504", "0.1123", "0.0902", "0.0752",
        "0.0645", "0.0564", "0.0501", "0.0451" };
static const char *sta33x_limiter_release_rate[] = {
        "0.5116", "0.1370", "0.0744", "0.0499", "0.0360", "0.0299",
        "0.0264", "0.0208", "0.0198", "0.0172", "0.0147", "0.0137",
        "0.0134", "0.0117", "0.0110", "0.0104" };

static const unsigned int sta33x_limiter_ac_attack_tlv[] = {
        TLV_DB_RANGE_HEAD(2),
        0, 7, TLV_DB_SCALE_ITEM(-1200, 200, 0),
        8, 16, TLV_DB_SCALE_ITEM(300, 100, 0),
};

static const unsigned int sta33x_limiter_ac_release_tlv[] = {
        TLV_DB_RANGE_HEAD(5),
        0, 0, TLV_DB_SCALE_ITEM(TLV_DB_GAIN_MUTE, 0, 0),
        1, 1, TLV_DB_SCALE_ITEM(-2900, 0, 0),
        2, 2, TLV_DB_SCALE_ITEM(-2000, 0, 0),
        3, 8, TLV_DB_SCALE_ITEM(-1400, 200, 0),
        8, 16, TLV_DB_SCALE_ITEM(-700, 100, 0),
};

static const unsigned int sta33x_limiter_drc_attack_tlv[] = {
        TLV_DB_RANGE_HEAD(3),
        0, 7, TLV_DB_SCALE_ITEM(-3100, 200, 0),
        8, 13, TLV_DB_SCALE_ITEM(-1600, 100, 0),
        14, 16, TLV_DB_SCALE_ITEM(-1000, 300, 0),
};

static const unsigned int sta33x_limiter_drc_release_tlv[] = {
        TLV_DB_RANGE_HEAD(5),
        0, 0, TLV_DB_SCALE_ITEM(TLV_DB_GAIN_MUTE, 0, 0),
        1, 2, TLV_DB_SCALE_ITEM(-3800, 200, 0),
        3, 4, TLV_DB_SCALE_ITEM(-3300, 200, 0),
        5, 12, TLV_DB_SCALE_ITEM(-3000, 200, 0),
        13, 16, TLV_DB_SCALE_ITEM(-1500, 300, 0),
};

static SOC_ENUM_SINGLE_DECL(sta33x_drc_ac_enum,
                            STA33X_CONFD, STA33X_CONFD_DRC_SHIFT,
                            sta33x_drc_ac);
static SOC_ENUM_SINGLE_DECL(sta33x_auto_gc_enum,
                            STA33X_AUTO1, STA33X_AUTO1_AMGC_SHIFT,
                            sta33x_auto_gc_mode);
static SOC_ENUM_SINGLE_DECL(sta33x_auto_xo_enum,
                            STA33X_AUTO2, STA33X_AUTO2_XO_SHIFT,
                            sta33x_auto_xo_mode);
static SOC_ENUM_SINGLE_DECL(sta33x_limiter_ch1_enum,
                            STA33X_C1CFG, STA33X_CxCFG_LS_SHIFT,
                            sta33x_limiter_select);
static SOC_ENUM_SINGLE_DECL(sta33x_limiter_ch2_enum,
                            STA33X_C2CFG, STA33X_CxCFG_LS_SHIFT,
                            sta33x_limiter_select);
static SOC_ENUM_SINGLE_DECL(sta33x_limiter_ch3_enum,
                            STA33X_C3CFG, STA33X_CxCFG_LS_SHIFT,
                            sta33x_limiter_select);
static SOC_ENUM_SINGLE_DECL(sta33x_limiter1_attack_rate_enum,
                            STA33X_L1AR, STA33X_LxA_SHIFT,
                            sta33x_limiter_attack_rate);
static SOC_ENUM_SINGLE_DECL(sta33x_limiter2_attack_rate_enum,
                            STA33X_L2AR, STA33X_LxA_SHIFT,
                            sta33x_limiter_attack_rate);
static SOC_ENUM_SINGLE_DECL(sta33x_limiter1_release_rate_enum,
                            STA33X_L1AR, STA33X_LxR_SHIFT,
                            sta33x_limiter_release_rate);
static SOC_ENUM_SINGLE_DECL(sta33x_limiter2_release_rate_enum,
                            STA33X_L2AR, STA33X_LxR_SHIFT,
                            sta33x_limiter_release_rate);


/* byte array controls for setting biquad, mixer, scaling coefficients;
 * for biquads all five coefficients need to be set in one go,
 * mixer and pre/postscale coefs can be set individually;
 * each coef is 24bit, the bytes are ordered in the same way
 * as given in the STA33X data sheet (big endian; b1, b2, a1, a2, b0)
 */

static int sta33x_coefficient_info(struct snd_kcontrol *kcontrol,
                                   struct snd_ctl_elem_info *uinfo)
{
        int numcoef = kcontrol->private_value >> 16;
        uinfo->type = SNDRV_CTL_ELEM_TYPE_BYTES;
        uinfo->count = 3 * numcoef;
        return 0;
}

static int sta33x_coefficient_get(struct snd_kcontrol *kcontrol,
                                  struct snd_ctl_elem_value *ucontrol)
{
        struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
        int numcoef = kcontrol->private_value >> 16;
        int index = kcontrol->private_value & 0xffff;
        unsigned int cfud;
        int i;

        /* preserve reserved bits in STA33X_CFUD */
        cfud = snd_soc_read(codec, STA33X_CFUD) & 0xf0;
        /* chip documentation does not say if the bits are self clearing,
         * so do it explicitly */
        snd_soc_write(codec, STA33X_CFUD, cfud);

        snd_soc_write(codec, STA33X_CFADDR2, index);
        if (numcoef == 1)
                snd_soc_write(codec, STA33X_CFUD, cfud | 0x04);
        else if (numcoef == 5)
                snd_soc_write(codec, STA33X_CFUD, cfud | 0x08);
        else
                return -EINVAL;
        for (i = 0; i < 3 * numcoef; i++)
                ucontrol->value.bytes.data[i] =
                        snd_soc_read(codec, STA33X_B1CF1 + i);

        return 0;
}

static int sta33x_coefficient_put(struct snd_kcontrol *kcontrol,
                                  struct snd_ctl_elem_value *ucontrol)
{
        struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
        struct sta33x_priv *sta33x = snd_soc_codec_get_drvdata(codec);
        int numcoef = kcontrol->private_value >> 16;
        int index = kcontrol->private_value & 0xffff;
        unsigned int cfud;
        int i;

        cfud = snd_soc_read(codec, STA33X_CFUD) & 0xf0;

        snd_soc_write(codec, STA33X_CFUD, cfud);

        snd_soc_write(codec, STA33X_CFADDR2, index);
        for (i = 0; i < numcoef && (index + i < STA33X_COEF_COUNT); i++)
                sta33x->coef_shadow[index + i] =
                          (ucontrol->value.bytes.data[3 * i] << 16)
                        | (ucontrol->value.bytes.data[3 * i + 1] << 8)
                        | (ucontrol->value.bytes.data[3 * i + 2]);
        for (i = 0; i < 3 * numcoef; i++)
                snd_soc_write(codec, STA33X_B1CF1 + i,
                              ucontrol->value.bytes.data[i]);
        if (numcoef == 1)
                snd_soc_write(codec, STA33X_CFUD, cfud | 0x01);
        else if (numcoef == 5)
                snd_soc_write(codec, STA33X_CFUD, cfud | 0x02);
        else
                return -EINVAL;

        return 0;
}

#define SINGLE_COEF(xname, index) \
{       .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
        .info = sta33x_coefficient_info, \
        .get = sta33x_coefficient_get,\
        .put = sta33x_coefficient_put, \
        .private_value = index | (1 << 16) }

#define BIQUAD_COEFS(xname, index) \
{       .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
        .info = sta33x_coefficient_info, \
        .get = sta33x_coefficient_get,\
        .put = sta33x_coefficient_put, \
        .private_value = index | (5 << 16) }


static const struct snd_kcontrol_new sta33x_snd_controls[] = {
SOC_SINGLE_TLV("Master Volume", STA33X_MVOL, 0, 0xff, 1, mvol_tlv),
//SOC_SINGLE_TLV_MONO("Mono Master Volume", STA33X_MVOL, 0, 0xff, 1, mvol_tlv),
SOC_SINGLE("Master Switch", STA33X_MMUTE, 0, 1, 1), // MUTE
//SOC_SINGLE_MONO("Mono Master Switch", STA33X_MMUTE, 0, 1, 1), // MUTE
SOC_SINGLE("Ch1 Switch", STA33X_MMUTE, 1, 1, 1),
SOC_SINGLE("Ch2 Switch", STA33X_MMUTE, 2, 1, 1),
SOC_SINGLE("Ch3 Switch", STA33X_MMUTE, 3, 1, 1),
SOC_SINGLE_TLV("Ch1 Volume", STA33X_C1VOL, 0, 0xc8, 1, chvol_tlv),
SOC_SINGLE_TLV("Ch2 Volume", STA33X_C2VOL, 0, 0xc8, 1, chvol_tlv),
SOC_SINGLE_TLV("Ch3 Volume", STA33X_C3VOL, 0, 0xc8, 1, chvol_tlv),
SOC_SINGLE("De-emphasis Filter Switch", STA33X_CONFD, STA33X_CONFD_DEMP_SHIFT, 1, 0), // De-emphasis enable    ??
SOC_ENUM("Compressor/Limiter Switch", sta33x_drc_ac_enum),       // anti-clip or dynamic compress            !?
SOC_SINGLE("Miami Mode Switch", STA33X_CONFD, STA33X_CONFD_MME_SHIFT, 1, 0), // submix mode enable            ?
SOC_SINGLE("Zero Cross Switch", STA33X_CONFE, STA33X_CONFE_ZCE_SHIFT, 1, 0),  // Zero-crossing volume enable     !!
SOC_SINGLE("Soft Ramp Switch", STA33X_CONFE, STA33X_CONFE_SVE_SHIFT, 1, 0),  // Soft volume update enable
SOC_SINGLE("Auto-Mute Switch", STA33X_CONFF, STA33X_CONFF_IDE_SHIFT, 1, 0),    // Invalid input detect mute enable !!
SOC_ENUM("Automode GC", sta33x_auto_gc_enum),                                  //gc
SOC_ENUM("Automode XO", sta33x_auto_xo_enum),                                 // xo
SOC_SINGLE("Ch1 Tone Control Bypass Switch", STA33X_C1CFG, STA33X_CxCFG_TCB_SHIFT, 1, 0),  // Tone control bypass?
SOC_SINGLE("Ch2 Tone Control Bypass Switch", STA33X_C2CFG, STA33X_CxCFG_TCB_SHIFT, 1, 0),
SOC_SINGLE("Ch1 EQ Bypass Switch", STA33X_C1CFG, STA33X_CxCFG_EQBP_SHIFT, 1, 0),           // EQ bypass
SOC_SINGLE("Ch2 EQ Bypass Switch", STA33X_C2CFG, STA33X_CxCFG_EQBP_SHIFT, 1, 0),
SOC_SINGLE("Ch1 Master Volume Bypass Switch", STA33X_C1CFG, STA33X_CxCFG_VBP_SHIFT, 1, 0),  // Volume bypass
SOC_SINGLE("Ch2 Master Volume Bypass Switch", STA33X_C1CFG, STA33X_CxCFG_VBP_SHIFT, 1, 0),
SOC_SINGLE("Ch3 Master Volume Bypass Switch", STA33X_C1CFG, STA33X_CxCFG_VBP_SHIFT, 1, 0),
SOC_ENUM("Ch1 Limiter Select", sta33x_limiter_ch1_enum),                                  // Limiter select
SOC_ENUM("Ch2 Limiter Select", sta33x_limiter_ch2_enum),
SOC_ENUM("Ch3 Limiter Select", sta33x_limiter_ch3_enum),
SOC_SINGLE_TLV("Bass Tone Control", STA33X_TONE, STA33X_TONE_BTC_SHIFT, 15, 0, tone_tlv),  //Tone control
SOC_SINGLE_TLV("Treble Tone Control", STA33X_TONE, STA33X_TONE_TTC_SHIFT, 15, 0, tone_tlv),
SOC_ENUM("Limiter1 Attack Rate (dB/ms)", sta33x_limiter1_attack_rate_enum),           // Limiter 1 attack/release rate
SOC_ENUM("Limiter2 Attack Rate (dB/ms)", sta33x_limiter2_attack_rate_enum),
SOC_ENUM("Limiter1 Release Rate (dB/ms)", sta33x_limiter1_release_rate_enum),
SOC_ENUM("Limiter2 Release Rate (dB/ms)", sta33x_limiter2_release_rate_enum),

SOC_SINGLE_TLV("Device status", STA33X_DEVSTA, 0, 0xff, 0,mvol_tlv),
//SOC_SINGLE_TLV_MONO("Mono Device status", STA33X_DEVSTA, 0, 0xff, 0,mvol_tlv),
/* depending on mode, the attack/release thresholds have
 * two different enum definitions; provide both
 */
SOC_SINGLE_TLV("Limiter1 Attack Threshold (AC Mode)", STA33X_L1ATRT, STA33X_LxA_SHIFT,
               16, 0, sta33x_limiter_ac_attack_tlv),
SOC_SINGLE_TLV("Limiter2 Attack Threshold (AC Mode)", STA33X_L2ATRT, STA33X_LxA_SHIFT,
               16, 0, sta33x_limiter_ac_attack_tlv),
SOC_SINGLE_TLV("Limiter1 Release Threshold (AC Mode)", STA33X_L1ATRT, STA33X_LxR_SHIFT,
               16, 0, sta33x_limiter_ac_release_tlv),
SOC_SINGLE_TLV("Limiter2 Release Threshold (AC Mode)", STA33X_L2ATRT, STA33X_LxR_SHIFT,
               16, 0, sta33x_limiter_ac_release_tlv),
SOC_SINGLE_TLV("Limiter1 Attack Threshold (DRC Mode)", STA33X_L1ATRT, STA33X_LxA_SHIFT,
               16, 0, sta33x_limiter_drc_attack_tlv),
SOC_SINGLE_TLV("Limiter2 Attack Threshold (DRC Mode)", STA33X_L2ATRT, STA33X_LxA_SHIFT,
               16, 0, sta33x_limiter_drc_attack_tlv),
SOC_SINGLE_TLV("Limiter1 Release Threshold (DRC Mode)", STA33X_L1ATRT, STA33X_LxR_SHIFT,
               16, 0, sta33x_limiter_drc_release_tlv),
SOC_SINGLE_TLV("Limiter2 Release Threshold (DRC Mode)", STA33X_L2ATRT, STA33X_LxR_SHIFT,
               16, 0, sta33x_limiter_drc_release_tlv),

BIQUAD_COEFS("Ch1 - Biquad 1", 0),
BIQUAD_COEFS("Ch1 - Biquad 2", 5),
BIQUAD_COEFS("Ch1 - Biquad 3", 10),
BIQUAD_COEFS("Ch1 - Biquad 4", 15),
BIQUAD_COEFS("Ch2 - Biquad 1", 20),
BIQUAD_COEFS("Ch2 - Biquad 2", 25),
BIQUAD_COEFS("Ch2 - Biquad 3", 30),
BIQUAD_COEFS("Ch2 - Biquad 4", 35),
BIQUAD_COEFS("High-pass", 40),
BIQUAD_COEFS("Low-pass", 45),
SINGLE_COEF("Ch1 - Prescale", 50),
SINGLE_COEF("Ch2 - Prescale", 51),
SINGLE_COEF("Ch1 - Postscale", 52),
SINGLE_COEF("Ch2 - Postscale", 53),
SINGLE_COEF("Ch3 - Postscale", 54),
SINGLE_COEF("Thermal warning - Postscale", 55),
SINGLE_COEF("Ch1 - Mix 1", 56),
SINGLE_COEF("Ch1 - Mix 2", 57),
SINGLE_COEF("Ch2 - Mix 1", 58),
SINGLE_COEF("Ch2 - Mix 2", 59),
SINGLE_COEF("Ch3 - Mix 1", 60),
SINGLE_COEF("Ch3 - Mix 2", 61),

};

static const struct snd_soc_dapm_widget sta33x_dapm_widgets[] = {
SND_SOC_DAPM_DAC("DAC", "Playback", SND_SOC_NOPM, 0, 0),
SND_SOC_DAPM_OUTPUT("LEFT"),
SND_SOC_DAPM_OUTPUT("RIGHT"),
SND_SOC_DAPM_OUTPUT("SUB"),
};

static const struct snd_soc_dapm_route sta33x_dapm_routes[] = {
        { "LEFT", NULL, "DAC" },
        { "RIGHT", NULL, "DAC" },
        { "SUB", NULL, "DAC" },
};

/* MCLK interpolation ratio per fs */
static struct {
        int fs;
        int ir;
} interpolation_ratios[] = {
        { 32000, 0 },
        { 44100, 0 },
        { 48000, 0 },
        { 88200, 1 },
        { 96000, 1 },
        { 176400, 2 },
        { 192000, 2 },
};

/* MCLK to fs clock ratios */
static struct {
        int ratio;
        int mcs;
} mclk_ratios[3][7] = {
        { { 768, 0 }, { 512, 1 }, { 384, 2 }, { 256, 3 },{ 128, 4 }, { 576, 5 }, { 0, 0 } },
        { { 384, 0 }, { 256, 1 }, { 192, 2 }, { 128, 3 }, {64, 4 }, { 0, 0 } },
        { {192, 0 },{ 128, 1 }, {96, 2 }, {64, 3 }, { 32, 4 }, { 0, 0 } },
};

static void sta33x_output_config(struct snd_soc_codec *codec,unsigned char configuration){
	unsigned char i2c_buf;
	
	i2c_buf = snd_soc_read(codec, STA33X_CONFF);
	//i2C_Buf=I2Cm_Rx(STA33X_CONFIGURE_F,STA33X_I2C_ADDR);
	i2c_buf &= 0xFC;
	i2c_buf |= configuration;
	snd_soc_write(codec, STA33X_CONFF, i2c_buf);
	//I2Cm_Tx(,STA33X_CONFIGURE_F,STA33X_I2C_ADDR);
}

static void sta33x_power_onoff(struct snd_soc_codec *codec,unsigned char power_flag){
	unsigned char i2c_buf;
	
	i2c_buf = snd_soc_read(codec, STA33X_CONFF);
	//i2c_buf=I2Cm_Rx(STA33X_CONFIGURE_F,STA33X_I2C_ADDR);
	i2c_buf &= 0x7F;
	i2c_buf |= ((power_flag)<<7);
	snd_soc_write(codec, STA33X_CONFF, i2c_buf);
	//I2Cm_Tx(i2c_buf,STA33X_CONFIGURE_F,STA33X_I2C_ADDR);
}

static int sta33x_mute(struct snd_soc_dai *codec_dai, int mute)
{
	printk("digital mute: %u\n", mute);
	struct snd_soc_codec *codec = codec_dai->codec;
	u8 mmute = snd_soc_read(codec, STA33X_MMUTE);
	
	mmute &= 0xF1;
	if(mute){
		mmute |= 0x0E;
		snd_soc_write(codec, STA33X_MMUTE, mmute);
	}else{
		snd_soc_write(codec, STA33X_MMUTE, mmute);		
	}

	void __iomem *pd17 = ioremap(0x01c2087c, 0x100);
	printk("pd17 = 0x%x\n", readl(pd17));	
	iounmap(pd17);
	
	//msleep(30);
	return 0;
}

static int sta33x_startup(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	return 0;
}

static void sta33x_shutdown(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{

}

static const int backup_regs_index[] = {
	0x00,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,

	0x06,
	0x07,
	0x08,
	0x09,
	0x0a,

	0x0b,
	0x0c,
	0x0d,

	0x0e,
	0x0f,
	0x10,

	0x11,

	0x12,
	0x13,
	0x14,
	0x15,

	0x27,
	0x28,
	0x29,
	0x2a,
	0x2b,
	0x2c,

	0x31,
	0x36,
	0x37,
	0x38,
};
static unsigned char backup_regs_val[ARRAY_SIZE(backup_regs_index)];
/* registers should be saved */
static void sta33x_store_config(struct snd_soc_codec *codec)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(backup_regs_index); i++)
	{
		backup_regs_val[i] = snd_soc_read(codec, backup_regs_index[i]);
	}
	printk("==read==\n");
}

static void sta33x_restore_config(struct snd_soc_codec *codec)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(backup_regs_index); i++)
	{
		snd_soc_write(codec, backup_regs_index[i], backup_regs_val[i]);
	}
	printk("==write==\n");
}
static int sta33x_set_dai_sysclk(struct snd_soc_dai *codec_dai,
                int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct sta33x_priv *sta33x = snd_soc_codec_get_drvdata(codec);
	int i, j, ir, fs;
	unsigned int rates = 0;
	unsigned int rate_min = -1;
	unsigned int rate_max = 0;


	//printk("mclk=%u\n", freq);
	if(!freq)
	{
	   sta33x->mclk = 24576000;
	}else{
	   sta33x->mclk = 22579200;
	}
	
//	printk("mclk=%u\n", sta33x->mclk);
	if (sta33x->mclk) {
	        for (i = 0; i < ARRAY_SIZE(interpolation_ratios); i++) {
	                ir = interpolation_ratios[i].ir;
	                fs = interpolation_ratios[i].fs;
	                for (j = 0; mclk_ratios[ir][j].ratio; j++) {
	                        if (mclk_ratios[ir][j].ratio * fs == sta33x->mclk) {
	                                rates |= snd_pcm_rate_to_rate_bit(fs);
	                                if (fs < rate_min)
	                                        rate_min = fs;
	                                if (fs > rate_max)
	                                        rate_max = fs;
	                                break;
	                        }
	                }
	        }
	        //soc should support a rate list
	        rates &= ~SNDRV_PCM_RATE_KNOT;

	        if (!rates) {
	                dev_err(codec->dev, "could not find a valid sample rate\n");
	                return -EINVAL;
	        }
	} else {
	        // enable all possible rates
	        rates = STA33X_RATES;
	        rate_min = 32000;
	        rate_max = 192000;
	}

	return 0;
}

static int sta33x_set_dai_fmt(struct snd_soc_dai *codec_dai,
                              unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct sta33x_priv *sta33x = snd_soc_codec_get_drvdata(codec);

	//printk("lyhxxw sta33x_set_dai_fmt: %x\n", fmt);

	//return 0;//20160308
	u8 confb = snd_soc_read(codec, STA33X_CONFB);

	//pr_debug("\n");
	confb &= ~(STA33X_CONFB_C1IM | STA33X_CONFB_C2IM);

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {//4001 &0xf000
	case SND_SOC_DAIFMT_CBS_CFS:
	        break;
	default:
	        return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		printk("sta33x_set_dai_fmt: SND_SOC_DAIFMT_I2S\n");
	case SND_SOC_DAIFMT_RIGHT_J:
	//	printk("sta33x_set_dai_fmt: SND_SOC_DAIFMT_RIGHT_J\n");
	case SND_SOC_DAIFMT_LEFT_J:
	//	printk("sta33x_set_dai_fmt: SND_SOC_DAIFMT_LEFT_J\n");
	        sta33x->format = fmt & SND_SOC_DAIFMT_FORMAT_MASK;
         //	printk("sta33x_set_dai_fmt:val = %d\n", sta33x->format);
	        break;
	default:
	        return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {//4001 & 0x0f00
	case SND_SOC_DAIFMT_NB_NF:
	        confb |= STA33X_CONFB_C2IM;
	        printk("sta33x_set_dai_fmt: STA33X_CONFB_C2IM\n");
	        break;
	case SND_SOC_DAIFMT_NB_IF:
	        confb |= STA33X_CONFB_C1IM;
	        printk("sta33x_set_dai_fmt: STA33X_CONFB_C1IM\n");
	        break;
	default:
	        return -EINVAL;
	}

	snd_soc_write(codec, STA33X_CONFB, confb);

	sta33x_store_config(codec);

	return 0;
}

static int sta33x_hw_params(struct snd_pcm_substream *substream,
                            struct snd_pcm_hw_params *params,
                            struct snd_soc_dai *dai)
{       
    struct snd_soc_codec *codec = dai->codec;
    struct sta33x_priv *sta33x = snd_soc_codec_get_drvdata(codec);
    unsigned int rate, format;
    int i, mcs = -1, ir = -1;
    static unsigned int old_rate = 0;
    u8 confa, confb;


    u8 conftest;


    format = params_format(params);

    rate = params_rate(params);
   // printk("\nlyhxxw sta33x_hw_params rate:%u channels:%u period_size:%u format:%d\n", rate,channels,period_size,format);

	sta33x_restore_config(codec);
    old_rate = rate;

    for (i = 0; i < ARRAY_SIZE(interpolation_ratios); i++)
            if (interpolation_ratios[i].fs == rate) {
                    ir = interpolation_ratios[i].ir;
                    break;
            }
			
    if (ir < 0)
            return -EINVAL;
    for (i = 0; mclk_ratios[ir][i].ratio; i++)
            if (mclk_ratios[ir][i].ratio == sta33x->mult_fs) {
                    mcs = mclk_ratios[ir][i].mcs;
                    break;
            }
    if (mcs < 0)
            return -EINVAL;

    confa = snd_soc_read(codec, STA33X_CONFA);
    confa &= ~(STA33X_CONFA_MCS_MASK | STA33X_CONFA_IR_MASK);
    confa |= (ir << STA33X_CONFA_IR_SHIFT) | (mcs << STA33X_CONFA_MCS_SHIFT);
    printk("ir: %u mcs: %u\n", ir, mcs);

    confb = snd_soc_read(codec, STA33X_CONFB);
    confb &= ~(STA33X_CONFB_SAI_MASK | STA33X_CONFB_SAIFB);
    printk("format: %u\n", params_format(params));
	
    switch (format & SND_SOC_DAIFMT_FORMAT_MASK) {
    case SNDRV_PCM_FORMAT_S24_LE:
        //    printk("24bit\n");
            // fall through
    case 32:
            printk("24bit or 32bit\n");
            switch (sta33x->format) {
            case SND_SOC_DAIFMT_I2S:
                    confb |= 0x0;
		    //printk("fmt: SND_SOC_DAIFMT_I2S\n");
                    break;
            case SND_SOC_DAIFMT_LEFT_J:
                    confb |= 0x1;
                    break;
            case SND_SOC_DAIFMT_RIGHT_J:
                    confb |= 0x2;
                    break;
            }

            break;
    case 20:
            //printk("20bit\n");
            switch (sta33x->format) {
            case SND_SOC_DAIFMT_I2S:
                    confb |= 0x4;
		     printk("fmt: SND_SOC_DAIFMT_I2S\n");
                    break;
            case SND_SOC_DAIFMT_LEFT_J:
                    confb |= 0x5;
                    break;
            case SND_SOC_DAIFMT_RIGHT_J:
                    confb |= 0x6;
                    break;
            }

            break;
    case 18:
            //printk("18bit\n");
            switch (sta33x->format) {
            case SND_SOC_DAIFMT_I2S:
                    confb |= 0x8;
		    //printk("fmt: SND_SOC_DAIFMT_I2S\n");
                    break;
            case SND_SOC_DAIFMT_LEFT_J:
                    confb |= 0x9;
                    break;
            case SND_SOC_DAIFMT_RIGHT_J:
                    confb |= 0xa;
                    break;
            }

            break;
    case SNDRV_PCM_FORMAT_S16_LE:
            printk("16bit\n");
				switch (sta33x->format)
				{
					case SND_SOC_DAIFMT_I2S:
						confb |= 0x0;
						//printk("fmt: SND_SOC_DAIFMT_I2S\n");
						break;
					case SND_SOC_DAIFMT_LEFT_J:
						confb |= 0xd;//0xd
						break;
					case SND_SOC_DAIFMT_RIGHT_J:
						confb |= 0xe;//0xe
						break;
				}

            break;
    default:
            return -EINVAL;
    }

    snd_soc_write(codec, STA33X_CONFA, confa);
    snd_soc_write(codec, STA33X_CONFB, confb);


    return 0;
}
static int sta33x_set_dai_clkdiv(struct snd_soc_dai *codec_dai, int div_id, int div)
{

    struct snd_soc_codec *codec = codec_dai->codec;
    struct sta33x_priv   *sta33x  = snd_soc_codec_get_drvdata(codec);

	 if(div_id == 2)
	 {
		 sta33x->mult_fs = div;
	 }

	 return 0;
}

static int sta33x_set_bias_level(struct snd_soc_codec *codec,
                                 enum snd_soc_bias_level level)
{
			
        struct sta33x_priv *sta33x = snd_soc_codec_get_drvdata(codec);

        switch (level) {
        case SND_SOC_BIAS_ON:
                break;

        case SND_SOC_BIAS_PREPARE:
                /* Full power on */
                snd_soc_update_bits(codec, STA33X_CONFF,
                                    STA33X_CONFF_PWDN | STA33X_CONFF_EAPD,
                                    STA33X_CONFF_PWDN | STA33X_CONFF_EAPD);
                break;

        case SND_SOC_BIAS_STANDBY:
                if (codec->dapm.bias_level == SND_SOC_BIAS_OFF) {
                	snd_soc_update_bits(codec, STA33X_CONFF,
                                    STA33X_CONFF_PWDN | STA33X_CONFF_EAPD,
                                    STA33X_CONFF_PWDN | STA33X_CONFF_EAPD);
		        }

				snd_soc_update_bits(codec, STA33X_CONFF,
                                    STA33X_CONFF_PWDN | STA33X_CONFF_EAPD,
                                    STA33X_CONFF_PWDN | STA33X_CONFF_EAPD);

                break;

        case SND_SOC_BIAS_OFF:
                /* The chip runs through the power down sequence for us. */

				printk("SND_SOC_BIAS_OFF !!!!!\n");
                snd_soc_update_bits(codec, STA33X_CONFF,
                                    STA33X_CONFF_PWDN | STA33X_CONFF_EAPD,
                                    STA33X_CONFF_PWDN);

                break;
        }
        codec->dapm.bias_level = level;
        return 0;
}

static const struct snd_soc_dai_ops sta33x_dai_ops = {
	.startup   =     sta33x_startup,
	.shutdown  =     sta33x_shutdown,
	.hw_params =     sta33x_hw_params,
	.digital_mute =  sta33x_mute,
	.set_clkdiv =    sta33x_set_dai_clkdiv,
	.set_sysclk =    sta33x_set_dai_sysclk,
	.set_fmt    =    sta33x_set_dai_fmt,
};

static struct snd_soc_dai_driver sta33x_dai = {
        .name = "STA33X",
        .playback = {
                .stream_name = "Playback",
                .channels_min = 1,
                .channels_max = 2,
                .rates   = STA33X_RATES,
                .formats = STA33X_FORMATS,
        },
        .ops = &sta33x_dai_ops,
	.symmetric_rates = 1,
};
static int codec_set_gpio_val(u32 gpio, u32 val);

#ifdef CONFIG_PM
static int sta33x_suspend(struct snd_soc_codec *codec)
{
	printk("sta33x_suspend-------\n");
		codec_set_gpio_val(sta33x_pwrdn_gpio,0);
        sta33x_set_bias_level(codec, SND_SOC_BIAS_OFF);
        return 0;
}

static int sta33x_resume(struct snd_soc_codec *codec)
{
		printk("sta33x_resume-------\n");
		codec_set_gpio_val(sta33x_pwrdn_gpio,1);
        sta33x_set_bias_level(codec, SND_SOC_BIAS_STANDBY);
        return 0;
}
#else
#define sta33x_suspend NULL
#define sta33x_resume NULL
#endif

static int sta33x_probe(struct snd_soc_codec *codec)
{
    struct sta33x_priv *sta33x = snd_soc_codec_get_drvdata(codec);
    int i,  thermal = 0;
    unsigned int ret = 0,ret0=0;
    //sta33x_reset_gpio_hd = (int *)codec->card->drvdata;

	//printk("lyh sun5i_sndi2s start init codec %s\n",codec->name);


	/*gpio_request(136, "sta33x_reset_gpio");//xxw 20160307
	gpio_direction_output(136, 0);
	gpio_set_value(136, 0);
	msleep(10);
	gpio_set_value(136, 1);
      msleep(5);
	sta33x_reset_gpio_hd = 136;*/


	sta33x->codec = codec;
    sta33x->pdata = dev_get_platdata(codec->dev);

	codec->hw_write = (hw_write_t) i2c_master_send;
        codec->control_data  =sta33x->control_data;


    
	snd_soc_write(codec, STA33X_CONFA, 0x63);
	snd_soc_write(codec, STA33X_CONFB, 0x80);
   snd_soc_write(codec, STA33X_CONFC, 0x97);//9F
	snd_soc_write(codec, STA33X_CONFD, 0x50);//18
    snd_soc_write(codec, STA33X_CONFE, 0xC2);//82

    //snd_soc_write(codec, STA33X_CONFF, 0xdd);//0x5e

    //snd_soc_write(codec, STA33X_CONFF, 0x5d);//xxw 20160808 add for noise


    snd_soc_write(codec, STA33X_MMUTE, 0x00);//0x40
	snd_soc_write(codec, STA33X_MVOL,  0xc8);
	snd_soc_write(codec, STA33X_C1VOL, 0x00);
	snd_soc_write(codec, STA33X_C2VOL, 0x00);
	snd_soc_write(codec, STA33X_C3VOL, 0x00);

   //snd_soc_write(codec, STA33X_C1CFG, 0x00);
	//snd_soc_write(codec, STA33X_C2CFG, 0x88);
   snd_soc_write(codec, STA33X_C3CFG, 0x60);
	snd_soc_write(codec, STA33X_AUTO1, 0x30);
	//snd_soc_write(codec, STA33X_AUTO2, 0x30);
	snd_soc_write(codec, STA33X_AUTO3, 0x00);
	snd_soc_write(codec, STA33X_TONE,  0x6a);
	snd_soc_write(codec, STA33X_L1AR,  0x69);
	snd_soc_write(codec, STA33X_L1ATRT,0x6a);
	snd_soc_write(codec, STA33X_L2AR,  0x69);


	//printk("[I2S]snd_soc_write2222222 \n");	
	snd_soc_write(codec, STA33X_EXTEND_CONFIG, 0x07);

	sta33x_output_config(codec,STA33X_2_1_SE_Config);
	
	//sta33x_power_onoff(codec,1);//xxw 20160805 del for noise
	
	snd_soc_write(codec, STA33X_MVOL, 0x4c);// middle volume


    /* initialize coefficient shadow RAM with reset values */
    for (i = 4; i <= 49; i += 5)
            sta33x->coef_shadow[i] = 0x400000;
    for (i = 50; i <= 54; i++)
            sta33x->coef_shadow[i] = 0x7fffff;
    sta33x->coef_shadow[55] = 0x5a9df7;
    sta33x->coef_shadow[56] = 0x7fffff;
    sta33x->coef_shadow[59] = 0x7fffff;
    sta33x->coef_shadow[60] = 0x400000;
    sta33x->coef_shadow[61] = 0x400000;
   
    //sta33x_set_bias_level(codec, SND_SOC_BIAS_STANDBY);//xxw 20160805 del for noise

     
	ret = snd_soc_read(codec,STA33X_DEVSTA);
	ret0= snd_soc_read(codec,STA33X_CONFA);
	
    //printk("lyh sun5i_sndi2s codec init success %u %u\n",ret,ret0);
    return 0;
}

static int sta33x_remove(struct snd_soc_codec *codec)
{
        struct sta33x_priv *sta33x = snd_soc_codec_get_drvdata(codec);

        sta33x_set_bias_level(codec, SND_SOC_BIAS_OFF);

        return 0;
}

static bool sta33x_reg_is_volatile(struct device *dev, unsigned int reg)
{
        switch (reg) {
        case STA33X_CONFA ... STA33X_L2ATRT:
        case STA33X_MPCC1 ... STA33X_FDRC2:
                return 0;
        }
        return 1;
}

static int sta33x_write(struct snd_soc_codec *codec, unsigned int reg,
				unsigned int val)
{
	u8 data[2];
	int ret;

	data[0] = reg & 0xff;
	data[1] = val & 0xff;

	if (codec->hw_write(codec->control_data, data, 2) == 2)
		return 0;
	else
		return -EIO;
}

static unsigned int sta33x_read(struct snd_soc_codec *codec, unsigned int reg)
{
	int ret;

	return i2c_smbus_read_byte_data(codec->control_data, reg & 0xff);
}

static const struct snd_soc_codec_driver sta33x_codec = {
	    .read =                 sta33x_read,
	    .write =                sta33x_write,
        .probe =                sta33x_probe,
        .remove =               sta33x_remove,
        .suspend =              sta33x_suspend,
        .resume =               sta33x_resume,
        .set_bias_level =       sta33x_set_bias_level,
        .controls =             sta33x_snd_controls,
        .num_controls =         ARRAY_SIZE(sta33x_snd_controls),
        .dapm_widgets =         sta33x_dapm_widgets,
        .num_dapm_widgets =     ARRAY_SIZE(sta33x_dapm_widgets),
        .dapm_routes =          sta33x_dapm_routes,
        .num_dapm_routes =      ARRAY_SIZE(sta33x_dapm_routes),
};

static int sta33x_i2c_probe(struct i2c_client *i2c,
                            const struct i2c_device_id *id)
{
    struct sta33x_priv *sta33x = &sta33x_p;
    int ret;   
    //printk("\nlyh sta33x_i2c_probe\n");
	//printk("[I2S]sun5i_sndi2s start codec register id :%s:%u\n",id->name,id->driver_data);

	sta33x->control_data = i2c;
	i2c_set_clientdata(i2c, sta33x);
	//printk("[I2S]snd_soc_register_codec ...... \n");
	ret = snd_soc_register_codec(&i2c->dev, &sta33x_codec, &sta33x_dai, 1);
	if (ret != 0)
		dev_err(&i2c->dev, "Failed to register codec (%d)\n", ret);

   
	//printk("[I2S]sun5i_sndi2s register codec \n");
	return ret;
}

static int sta33x_i2c_remove(struct i2c_client *client)
{
         snd_soc_unregister_codec(&client->dev);
         return 0;
}

static const struct i2c_device_id sta33x_i2c_id[] = {
         { "sta339", 0 },
         { }
};
MODULE_DEVICE_TABLE(i2c, sta33x_i2c_id);

//Function as i2c_master_send, and return 1 if operation is successful. 
static int i2c_write_bytes(struct i2c_client *client, uint8_t *data, uint16_t len)
{
	struct i2c_msg msg;
	int ret=-1;
	
	msg.flags = !I2C_M_RD;
	msg.addr = client->addr;
	msg.len = len;
	msg.buf = data;		
	
	ret=i2c_transfer(client->adapter, &msg,1);
	return ret;
}


static bool sta33x_i2c_test(struct i2c_client * client)
{
	int ret, retry;
	uint8_t test_data[1] = { 0 };	//only write a data address.
	
	for(retry=0; retry < 2; retry++)
	{
		ret =i2c_write_bytes(client, test_data, 1);	//Test i2c.
		if (ret == 1)
			break;
		msleep(5);
	}
	
	return ret==1 ? true : false;
}


static int sta33x_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;
	int ret;

	//printk("\nsta33x_detect\n");
	ret = i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA);
	if (!ret)
        	return -ENODEV;
    //printk("adapter->nr =%d",adapter->nr);
	if(twi_id == adapter->nr){
            //pr_info("%s: addr= %x\n",__func__,client->addr);

            ret = sta33x_i2c_test(client);
        	if(!ret){
        		pr_info("%s:I2C connection might be something wrong or maybe the other gsensor equipment! \n",__func__);
        		return -ENODEV;
        		//strlcpy(info->type, "sta339", I2C_NAME_SIZE);//xxw for test
        		//return 0;//xxw for test
        	}else{           	    
            	//pr_info("I2C connection sucess!\n");
            	//strlcpy(info->type, SENSOR_NAME, I2C_NAME_SIZE);
            	strlcpy(info->type, "sta339", I2C_NAME_SIZE);
    		    return 0;	
	        }

	}else{
		return -ENODEV;
	}
}



static struct i2c_driver sta33x_i2c_driver = {
	     .class = I2C_CLASS_HWMON,
         .driver = {
                 .name = "sta339",
                 .owner = THIS_MODULE,
                 //.of_match_table = "allwinner,sun50i-gsensor-para",
         },
         .probe  =   sta33x_i2c_probe,
         .remove =   sta33x_i2c_remove,
         .detect =   sta33x_detect,
         .id_table = sta33x_i2c_id,
         .address_list	= normal_i2c,
};


//#define CODEC_CONFIG_OF
//#ifdef CODEC_CONFIG_OF

static int  codec_request_gpio(struct gpio_config *pinConfig,__u32 group_count_max)
{
	struct gpio_config pin_cfg;
	char   pin_name[32];
	int ret ;
	u32 config;

	pr_info("%s: test1\n", __func__);
	pin_cfg.gpio = pinConfig->gpio;
	pin_cfg.mul_sel = pinConfig->mul_sel;
	pin_cfg.pull = pinConfig->pull;
	pin_cfg.drv_level = pinConfig->drv_level;
	pin_cfg.data = pinConfig->data;

	pr_info("%s: request gpio=%d\n", __func__, pin_cfg.gpio);
	ret = gpio_request(pin_cfg.gpio, NULL);
    if (0 == ret) {
		sunxi_gpio_to_name(pin_cfg.gpio, pin_name);
		config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC, pin_cfg.mul_sel);
		pin_config_set(SUNXI_PINCTRL, pin_name, config);
		if (pin_cfg.pull != GPIO_PULL_DEFAULT) {
			config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_PUD, pin_cfg.pull);
			pin_config_set(SUNXI_PINCTRL, pin_name, config);
		}
		if (pin_cfg.drv_level != GPIO_DRVLVL_DEFAULT) {
			config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DRV, pin_cfg.drv_level);
			pin_config_set(SUNXI_PINCTRL, pin_name, config);
		}
		if (pin_cfg.data != GPIO_DATA_DEFAULT) {
			pr_info("%s: L%d Pin=%d\n", __func__, __LINE__, pin_cfg.data);
			config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DAT, pin_cfg.data);
			pin_config_set(SUNXI_PINCTRL, pin_name, config);
		}
    }

	pr_info("%s: request gpio(%d) %s\n", __func__, pin_cfg.gpio, ((ret?"fail":"ok")));
	return ret;

}

static int codec_set_gpio_val(u32 gpio, u32 val)
{
	struct gpio_config pin_cfg;
	int ret = -1;

	pin_cfg.gpio=gpio;
	pin_cfg.drv_level=1;//SUNXI_DRV_LEVEL1;
	pin_cfg.mul_sel=1;  //OUTPUT
	pin_cfg.data=((val)?1:0);
	pin_cfg.pull=1;

	ret = codec_request_gpio(&pin_cfg,0);
	if (0 == ret)
		gpio_free(gpio);
	return ret;
}


static int codec_get_para(char *main_name, char *sub_name, int value[], int type)
{
	char compat[32];
	u32 len = 0;
	struct device_node *node;
	struct device_node *node_parent;
	int ret = 0;
	struct gpio_config config;

	node = of_find_node_by_type(NULL, "sndi2scodec_conf");
	if (!node) {
		printk(KERN_WARNING "of_find_node_by_name %s fail\n", "sndi2scodec_conf");
		return ret;
	}

	if (TYPE_INTEGER == type) {			/*integer*/
		if (of_property_read_u32_array(node, sub_name, value, 1))
			pr_info("of_property_read_u32_array %s.%s fail\n", main_name, sub_name);
		else
			ret = type;
	} else if (TYPE_STRING == type) {	/*string*/
		const char *str;

		if (of_property_read_string(node, sub_name, &str))
			pr_info("of_property_read_string %s.%s fail\n", main_name, sub_name);
		else {
			ret = type;
			memcpy((void*)value, str, strlen(str)+1);
		}
	} else if (TYPE_GPIO == type) {	/*gpio*/
	    int gpio;
		codec_gpio_set_t *gpio_info = (codec_gpio_set_t *)value;
		
		gpio = of_get_named_gpio_flags(node, sub_name, 0, (enum of_gpio_flags *)&config);
		if (!gpio_is_valid(gpio))
			goto exit;

		gpio_info->gpio      = config.gpio;
		gpio_info->mul_sel   = config.mul_sel;
		gpio_info->pull      = config.pull;
		gpio_info->drv_level = config.drv_level;
		gpio_info->data      = config.data;
		memcpy(gpio_info->gpio_name, sub_name, strlen(sub_name)+1);
		printk(KERN_INFO "%s.%s gpio=%d,mul_sel=%d,data:%d\n",main_name, sub_name, gpio_info->gpio, gpio_info->mul_sel, gpio_info->data);
		ret = type;
	} else if (TYPE_EXT_IRQ == type) {
		//*value = irq_of_parse_and_map(node, 0);
		//ret = type;
	}

exit:
	return ret;
}

static int codec_config(void)
{
	int ret = -1;
	int value = 0;
	codec_gpio_set_t pin;

    printk("codec_config\n");

	ret = codec_get_para(NULL, "sta33x_pwrdn_gpio", &pin, TYPE_GPIO);
	if(TYPE_GPIO != ret) {
		printk("%s: fail to get sta33x_pwrdn_gpio pin\n", __func__);
		sta33x_pwrdn_gpio = 0;
	} else {
		sta33x_pwrdn_gpio = pin.gpio;
		printk("%s: sta33x_pwrdn_gpio gpio num=%d\n", __func__, sta33x_pwrdn_gpio);
	}

	return 0;
}
//#endif


static int __init sta33x_modinit(void)
{
	int ret;

/*#ifdef CODEC_CONFIG_OF
	gpio_request(135, "sta33x_pwrdn_gpio");
	gpio_direction_output(135, 0);
	gpio_set_value(135, 1);
	msleep(10);
#else*/
    codec_config();
	codec_set_gpio_val(sta33x_pwrdn_gpio,1);
	msleep(10);
//#endif


	ret = i2c_add_driver(&sta33x_i2c_driver);
	if (ret != 0) {
		printk(KERN_ERR "Failed to register sta339 I2C driver: %d\n",ret);
	}
	
	return ret;
} 

static void __exit sta33x_exit(void)
{
	i2c_del_driver(&sta33x_i2c_driver);
}

fs_initcall(sta33x_modinit);
module_exit(sta33x_exit);
MODULE_DESCRIPTION("ASoC STA33X driver");
MODULE_AUTHOR("Yuan Kexiong <yuankexiong@tp-link.net>");


