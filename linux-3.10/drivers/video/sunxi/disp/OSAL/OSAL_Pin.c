/*
*************************************************************************************
*                         			eBsp
*					   Operation System Adapter Layer
*
*				(c) Copyright 2006-2010, All winners Co,Ld.
*							All	Rights Reserved
*
* File Name 	: OSAL_Pin.h
*
* Author 		: javen
*
* Description 	: C库函数
*
* History 		:
*      <author>    		<time>       	<version >    		<desc>
*       javen     	   2010-09-07          1.0         create this word
*       holi     	   2010-12-02          1.1         添加具体的接口，
*************************************************************************************
*/
#include "OSAL_Pin.h"
#include <linux/pinctrl/pinconf-sunxi.h>

__hdle OSAL_GPIO_Request(disp_gpio_set_t *gpio_list, __u32 group_count_max)
{    
	int ret = 0;
	struct gpio_config pin_cfg;
	char   pin_name[32];
	u32 config;

	if (gpio_list == NULL)
		return 0;

	pin_cfg.gpio = gpio_list->gpio;
	pin_cfg.mul_sel = gpio_list->mul_sel;
	pin_cfg.pull = gpio_list->pull;
	pin_cfg.drv_level = gpio_list->drv_level;
	pin_cfg.data = gpio_list->data;
	ret = gpio_request(pin_cfg.gpio, NULL);
	if (0 != ret) {
		__wrn("%s failed, gpio_name=%s, gpio=%d, ret=%d\n", __func__, gpio_list->gpio_name, gpio_list->gpio, ret);
		return ret;
	} else {
		__inf("%s, gpio_name=%s, gpio=%d, <%d,%d,%d,%d>ret=%d\n", __func__, gpio_list->gpio_name, gpio_list->gpio,\
			gpio_list->mul_sel, gpio_list->pull, gpio_list->drv_level, gpio_list->data, ret);
	}
	ret = pin_cfg.gpio;

	if (!IS_AXP_PIN(pin_cfg.gpio)) {
		/* valid pin of sunxi-pinctrl,
		* config pin attributes individually.
		*/
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
			config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DAT, pin_cfg.data);
			pin_config_set(SUNXI_PINCTRL, pin_name, config);
		}
	} else if (IS_AXP_PIN(pin_cfg.gpio)) {
		/* valid pin of axp-pinctrl,
		* config pin attributes individually.
		*/
		sunxi_gpio_to_name(pin_cfg.gpio, pin_name);
		if (pin_cfg.data != GPIO_DATA_DEFAULT) {
			config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DAT, pin_cfg.data);
			pin_config_set(AXP_PINCTRL, pin_name, config);
		}
	} else {
		pr_warn("invalid pin [%d] from sys-config\n", pin_cfg.gpio);
	}

	return ret;
}

__hdle OSAL_GPIO_Request_Ex(char *main_name, const char *sub_name)
{
	return 0;
}

//if_release_to_default_status:
    //如果是0或者1，表示释放后的GPIO处于输入状态，输入状状态不会导致外部电平的错误。
    //如果是2，表示释放后的GPIO状态不变，即释放的时候不管理当前GPIO的硬件寄存器。
__s32 OSAL_GPIO_Release(__hdle p_handler, __s32 if_release_to_default_status)
{
	/*
	* p_handler: handle of gpio, it can be 0
	*/
	
	gpio_free(p_handler);
	return 0;
}

__s32 OSAL_Set_Disp_Port_State(char *dev_name, char *name)
{
	char compat[32];
	u32 len = 0;
	struct device_node *node;
	struct platform_device *pdev;
	struct pinctrl *pctl;
	struct pinctrl_state *state;
	int ret = -1;

	len = sprintf(compat, "allwinner,sunxi-%s", dev_name);
	if (len > 32)
		__wrn("size of mian_name is out of range\n");

	node = of_find_compatible_node(NULL, NULL, compat);
	if (!node) {
		__wrn("of_find_compatible_node %s fail\n", compat);
		goto exit;
	}

	pdev = of_find_device_by_node(node);
	if (!node) {
		__wrn("of_find_device_by_node for %s fail\n", compat);
		goto exit;
	}

	pctl = pinctrl_get(&pdev->dev);
	if (IS_ERR(pctl)) {
		__wrn("pinctrl_get for %s fail\n", compat);
		ret = PTR_ERR(pctl);
		goto exit;
	}

	state = pinctrl_lookup_state(pctl, name);
	if (IS_ERR(state)) {
		__wrn("pinctrl_lookup_state for %s fail\n", compat);
		ret = PTR_ERR(state);
		goto exit;
	}

	ret = pinctrl_select_state(pctl, state);
	if (ret < 0) {
		__wrn("pinctrl_select_state(%s) for %s fail\n", name, compat);
		goto exit;
	}
	ret = 0;

exit:
	return ret;
}


__s32 OSAL_GPIO_DevGetAllPins_Status(unsigned p_handler, disp_gpio_set_t *gpio_status, unsigned gpio_count_max, unsigned if_get_from_hardware)
{
    //return gpio_get_all_pin_status(p_handler, gpio_status, gpio_count_max, if_get_from_hardware);
    return 0;
}

__s32 OSAL_GPIO_DevGetONEPins_Status(unsigned p_handler, disp_gpio_set_t *gpio_status,const char *gpio_name,unsigned if_get_from_hardware)
{
    //return gpio_get_one_pin_status(p_handler, gpio_status,gpio_name,if_get_from_hardware);
    return 0;
}

__s32 OSAL_GPIO_DevSetONEPin_Status(u32 p_handler, disp_gpio_set_t *gpio_status, const char *gpio_name, __u32 if_set_to_current_input_status)
{
    //return gpio_set_one_pin_status(p_handler, gpio_status, gpio_name, if_set_to_current_input_status);
    return 0;
}

__s32 OSAL_GPIO_DevSetONEPIN_IO_STATUS(u32 p_handler, __u32 if_set_to_output_status, const char *gpio_name)
{
    //return gpio_set_one_pin_io_status(p_handler, if_set_to_output_status, gpio_name);
    return 0;
}

__s32 OSAL_GPIO_DevSetONEPIN_PULL_STATUS(u32 p_handler, __u32 set_pull_status, const char *gpio_name)
{
    //return gpio_set_one_pin_pull(p_handler, set_pull_status, gpio_name);
    return 0;
}

__s32 OSAL_GPIO_DevREAD_ONEPIN_DATA(u32 p_handler, const char *gpio_name)
{
    //return gpio_read_one_pin_value(p_handler, gpio_name);
    return 0;
}

__s32 OSAL_GPIO_DevWRITE_ONEPIN_DATA(u32 p_handler, __u32 value_to_gpio, const char *gpio_name)
{
	s32 ret = -1;
	
	/*
	* p_handler: handle of gpio, it can be 0
	*/
	
    	//return gpio_write_one_pin_value(p_handler, value_to_gpio, gpio_name);
	__gpio_set_value(p_handler, value_to_gpio);
	ret = 0;

	return ret;
}


