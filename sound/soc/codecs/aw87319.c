// SPDX-License-Identifier: GPL-2.0-only

#include <linux/bits.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/regmap.h>
#include <sound/soc.h>

#define AW87319_REG_SYSCTRL		0x01
#define AW87319_SYSCTRL_EN_PA		BIT(2)

struct aw87319_priv {
	struct regmap *regmap;
	struct gpio_desc *gpiod_reset;
};

static int aw87319_drv_event(struct snd_soc_dapm_widget *w,
			     struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *c = snd_soc_dapm_to_component(w->dapm);
	struct aw87319_priv *aw = snd_soc_component_get_drvdata(c);
	int ret = 0;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		gpiod_set_value_cansleep(aw->gpiod_reset, 1);
		usleep_range(1000, 2000);
		ret = regmap_update_bits(aw->regmap, AW87319_REG_SYSCTRL,
					 AW87319_SYSCTRL_EN_PA,
					 AW87319_SYSCTRL_EN_PA);
		msleep(5);
		break;
	case SND_SOC_DAPM_PRE_PMD:
		regmap_update_bits(aw->regmap, AW87319_REG_SYSCTRL,
				   AW87319_SYSCTRL_EN_PA, 0);
		gpiod_set_value_cansleep(aw->gpiod_reset, 0);
		break;
	default:
		WARN(1, "Unexpected event");
		return -EINVAL;
	}

	return ret;
}

static const struct snd_soc_dapm_widget aw87319_dapm_widgets[] = {
	SND_SOC_DAPM_INPUT("IN"),
	SND_SOC_DAPM_OUT_DRV_E("DRV", SND_SOC_NOPM, 0, 0, NULL, 0, aw87319_drv_event,
			       SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD),
	SND_SOC_DAPM_OUTPUT("OUT"),
};

static const struct snd_soc_dapm_route aw87319_dapm_routes[] = {
	{ "DRV", NULL, "IN" },
	{ "OUT", NULL, "DRV" },
};

static const struct snd_soc_component_driver aw87319_component_driver = {
	.dapm_widgets = aw87319_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(aw87319_dapm_widgets),
	.dapm_routes = aw87319_dapm_routes,
	.num_dapm_routes = ARRAY_SIZE(aw87319_dapm_routes),
};

static const struct regmap_config aw87319_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = 0x09,
};

static int aw87319_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct aw87319_priv *aw;

	aw = devm_kzalloc(dev, sizeof(*aw), GFP_KERNEL);
	if (!aw)
		return -ENOMEM;
	i2c_set_clientdata(client, aw);

	aw->regmap = devm_regmap_init_i2c(client, &aw87319_regmap_config);
	if (IS_ERR(aw->regmap))
		return dev_err_probe(dev, PTR_ERR(aw->regmap),
				     "Failed to init regmap\n");

	aw->gpiod_reset = devm_gpiod_get(dev, "reset", GPIOD_OUT_LOW);
	if (IS_ERR(aw->gpiod_reset))
		return dev_err_probe(dev, PTR_ERR(aw->gpiod_reset),
				     "Failed to get 'reset' gpio\n");

	return devm_snd_soc_register_component(dev, &aw87319_component_driver,
					       NULL, 0);
}

static const struct of_device_id aw87319_of_match[] = {
	{ .compatible = "awinic,aw87319" },
	{ }
};
MODULE_DEVICE_TABLE(of, aw87319_of_match);

static struct i2c_driver aw87319_driver = {
	.driver = {
		.name = "aw87319",
		.of_match_table = aw87319_of_match,
	},
	.probe = aw87319_probe,
};
module_i2c_driver(aw87319_driver);

MODULE_DESCRIPTION("Awinic AW87319 Amplifier Driver");
MODULE_LICENSE("GPL");
