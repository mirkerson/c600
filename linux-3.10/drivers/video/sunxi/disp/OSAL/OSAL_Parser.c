#include "OSAL_Parser.h"

int OSAL_Script_Parser_Disp_Init_Data(char *main_name, char *sub_name, int value[], int type)
{
	struct device_node *node;	
	int ret = 0;
	
	node = of_find_node_by_type(NULL, main_name);
	if (!node) {
		__wrn("of_find_node_by_type %s fail\n", main_name);
		return ret;
	}
	
	if (TYPE_INTEGER == type) {			/*integer*/
		if (of_property_read_u32_array(node, sub_name, value, 1))
			pr_info("of_property_read_u32_array %s.%s fail\n", main_name, sub_name);
		else
			ret = type;
	}

	return ret;
}

int OSAL_Script_FetchParser_Data(char *main_name, char *sub_name, int value[], int type)
{
	char compat[32];
	u32 len = 0;
	struct device_node *node;
	int ret = 0;
	struct gpio_config config;

	len = sprintf(compat, "allwinner,sunxi-%s", main_name);
	if (len > 32)
		__wrn("size of mian_name is out of range\n");

	node = of_find_compatible_node(NULL, NULL, compat);
	if (!node) {
		__wrn("of_find_compatible_node %s fail\n", compat);
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
		disp_gpio_set_t *gpio_info = (disp_gpio_set_t *)value;
		int gpio;

		gpio = of_get_named_gpio_flags(node, sub_name, 0, (enum of_gpio_flags *)&config);
		if (!gpio_is_valid(gpio))
			goto exit;

		gpio_info->gpio = config.gpio;
		gpio_info->mul_sel = config.mul_sel;
		gpio_info->pull = config.pull;
		gpio_info->drv_level = config.drv_level;
		gpio_info->data = config.data;
		memcpy(gpio_info->gpio_name, sub_name, strlen(sub_name)+1);
		__inf("%s.%s gpio=%d,mul_sel=%d,data:%d\n",main_name, sub_name, gpio_info->gpio, gpio_info->mul_sel, gpio_info->data);
		ret = type;
	}

exit:
	return ret;
}

int OSAL_sw_get_ic_ver(void)
{
    return 0;
}

