/*
 * sound\soc\sunxi\sunx8iw10_sndcodec.c
 * (C) Copyright 2014-2017
 * allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * huangxin <huangxin@allwinnertech.com>
  *
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/input.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/jack.h>
#include <linux/of.h>
#include <sound/pcm_params.h>
#include <linux/sys_config.h>
#include <linux/of_gpio.h>
#include<linux/of_irq.h>
#include <sound/soc-dapm.h>
#include "sunivw1p1_codec.h"
#include <linux/delay.h>

#include "sunxi_rw_func.h"

static struct snd_soc_jack jack;
static int jack_irq = -1;
static struct snd_soc_jack_gpio jack_gpio = {
//		.gpio = S5PV210_GPH0(6),
		.name = "headphone detc",
		.report = SND_JACK_HEADPHONE | SND_JACK_MECHANICAL,
		.debounce_time = 200,
};

static const struct snd_kcontrol_new ac_pin_controls[] = {
	SOC_DAPM_PIN_SWITCH("External Speaker"),
	SOC_DAPM_PIN_SWITCH("Headphone"),
};

static const struct snd_soc_dapm_widget sunxi_ac_dapm_widgets[] = {
	SND_SOC_DAPM_MIC("External MainMic", NULL),
};

static const struct snd_soc_dapm_route audio_map[] = {
	{"MainMic Bias", NULL, "External MainMic"},
	{"MIC1P", NULL, "MainMic Bias"},
	{"MIC1N", NULL, "MainMic Bias"},
};

/*
 * Card initialization
 */
static int sunxi_audio_init(struct snd_soc_pcm_runtime *runtime)
{
	struct snd_soc_codec *codec = runtime->codec;
//	struct snd_soc_dapm_context *dapm = &codec->dapm;
	int ret;

//	snd_soc_dapm_disable_pin(&codec->dapm,	"HPOUTR");
//	snd_soc_dapm_disable_pin(&codec->dapm,	"HPOUTL");
//	snd_soc_dapm_disable_pin(&codec->dapm,	"SPKL");
//	snd_soc_dapm_disable_pin(&codec->dapm,	"SPKR");

//	snd_soc_dapm_disable_pin(&runtime->card->dapm,	"External Speaker");
//	snd_soc_dapm_sync(dapm);

	if(jack_gpio.gpio < 0 || jack_irq <= 0) {
		printk("no jack gpio or irq\n");
		return 0;
	}

#if 1
	/* Headset jack detection */
	ret = snd_soc_jack_new(codec, "Headset Jack",
			SND_JACK_HEADPHONE | SND_JACK_MECHANICAL,
			&jack);

	if (ret)
		return ret;

//	ret = snd_soc_jack_add_pins(&jack, ARRAY_SIZE(jack_pins), jack_pins);
//	if (ret)
//		return ret;

	ret = snd_soc_jack_add_gpio(&jack, jack_irq, &jack_gpio);
	if (ret)
		return ret;
	printk("request jack ok!\n");
#endif	
	return 0;
}

static int sunxi_sndpcm_hw_params(struct snd_pcm_substream *substream,
                                       struct snd_pcm_hw_params *params)
{
	int ret  = 0;
	u32 freq_in = 22579200;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	unsigned long sample_rate = params_rate(params);

	switch (sample_rate) {
		case 8000:
		case 16000:
		case 32000:
		case 64000:
		case 128000:
		case 12000:
		case 24000:
		case 48000:
		case 96000:
        case 176400:
		case 192000:
			freq_in = 24576000;
			break;
	}

	/*set system clock source freq_in and set the mode as tdm or pcm*/
	ret = snd_soc_dai_set_sysclk(cpu_dai, 0, freq_in, 0);
	if (ret < 0) {
		pr_err("err:%s,set codec dai sysclk faided.\n", __func__);
		return ret;
	}
	/*
	* codec: slave. AP: master
	*/
	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S |
			SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0) {
		pr_err("%s,set codec dai fmt failed.\n", __func__);
		return ret;
	}

#if 0
	ret = snd_soc_dai_set_clkdiv(cpu_dai, 0, sample_rate);
	if (ret < 0) {
		pr_err("%s, set cpu dai clkdiv faided.\n", __func__);
		return ret;
	}
#endif
	return 0;
}

static struct snd_soc_ops sunxi_sndpcm_ops = {
       .hw_params              = sunxi_sndpcm_hw_params,
};

static struct snd_soc_dai_link sunxi_sndpcm_dai_link[] = {
	{
	.name 			= "audiocodec",
	.stream_name 	= "SUNXI-CODEC",
	.cpu_dai_name 	= "sunxi-internal-cpudai",
	.codec_dai_name = "sun8iw10codec",
	.platform_name 	= "sunxi-internal-cpudai",
	.codec_name 	= "sunxi-internal-codec",
	.init 			= sunxi_audio_init,
    .ops 			= &sunxi_sndpcm_ops,
	},
};

static struct snd_soc_card snd_soc_sunxi_sndpcm = {
	.name 		= "audiocodec",
	.owner 		= THIS_MODULE,
	.dai_link 	= sunxi_sndpcm_dai_link,
	.num_links 	= ARRAY_SIZE(sunxi_sndpcm_dai_link),
#if 0
	.dapm_widgets = sunxi_ac_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(sunxi_ac_dapm_widgets),
	.dapm_routes = audio_map,
	.num_dapm_routes = ARRAY_SIZE(audio_map),
	.controls = ac_pin_controls,
	.num_controls = ARRAY_SIZE(ac_pin_controls),
#endif
};

static int sunxi_machine_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device_node *np = pdev->dev.of_node;
	struct gpio_config config;

	if (!np) {
		dev_err(&pdev->dev,
			"can not get dt node for this device.\n");
		return -EINVAL;
	}

	/* register the soc card */
	snd_soc_sunxi_sndpcm.dev = &pdev->dev;
	platform_set_drvdata(pdev, &snd_soc_sunxi_sndpcm);

	sunxi_sndpcm_dai_link[0].cpu_dai_name = NULL;
	sunxi_sndpcm_dai_link[0].cpu_of_node = of_parse_phandle(np,
				"sunxi,cpudai-controller", 0);
	if (!sunxi_sndpcm_dai_link[0].cpu_of_node) {
		dev_err(&pdev->dev,
			"Property 'sunxi,cpudai-controller' missing or invalid\n");
			ret = -EINVAL;
			goto err1;
	}
	sunxi_sndpcm_dai_link[0].platform_name = NULL;
	sunxi_sndpcm_dai_link[0].platform_of_node = sunxi_sndpcm_dai_link[0].cpu_of_node;

	sunxi_sndpcm_dai_link[0].codec_name = NULL;
	sunxi_sndpcm_dai_link[0].codec_of_node = of_parse_phandle(np,"sunxi,audio-codec", 0);
	if (!sunxi_sndpcm_dai_link[0].codec_of_node) {
		dev_err(&pdev->dev,
			"Property 'sunxi,audio-codec' missing or invalid\n");
		ret = -EINVAL;
		goto err1;
	}

	jack_gpio.gpio = of_get_named_gpio_flags(np, "headphone-dect", 0, (enum of_gpio_flags *)&config);
	if (!gpio_is_valid(jack_gpio.gpio)) {
		pr_err("failed to get headphone-dect gpio from dts,headphone-dect:%d\n",jack_gpio.gpio);
		jack_gpio.gpio =-1;
	} else {
		printk("get headphone-dect gpio:%d\n", jack_gpio.gpio);
		jack_irq = gpio_to_irq(jack_gpio.gpio);
		if(jack_irq == 0) {
			printk("get headphone-dect error:%d\n", jack_irq);
		}else {
			printk("\nget headphone-dect irq:%d\n", jack_irq);
		}
	}


	ret = snd_soc_register_card(&snd_soc_sunxi_sndpcm);
	if (ret) {
		pr_err("snd_soc_register_card failed %d\n", ret);
		goto err1;
	}

	return 0;

err1:
	snd_soc_unregister_component(&pdev->dev);
	return ret;
}

static const struct of_device_id sunxi_machine_of_match[] = {
	{ .compatible = "allwinner,sunxi-codec-machine", },
	{},
};

/*method relating*/
static struct platform_driver sunxi_machine_driver = {
	.driver = {
		.name = "sunxi-codec-machine",
		.owner = THIS_MODULE,
		.pm = &snd_soc_pm_ops,
		.of_match_table = sunxi_machine_of_match,
	},
	.probe = sunxi_machine_probe,
};

static int __init sunxi_machine_init(void)
{
	int err = 0;

	if ((err = platform_driver_register(&sunxi_machine_driver)) < 0)
		return err;

	return 0;
}
module_init(sunxi_machine_init);

static void __exit sunxi_machine_exit(void)
{
	platform_driver_unregister(&sunxi_machine_driver);
}

module_exit(sunxi_machine_exit);

MODULE_AUTHOR("huangxin");
MODULE_DESCRIPTION("SUNXI_sndpcm ALSA SoC audio driver");
MODULE_LICENSE("GPL");

