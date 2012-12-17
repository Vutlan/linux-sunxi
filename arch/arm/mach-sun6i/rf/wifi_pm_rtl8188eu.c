/*
 * rtl8188eu usb wifi power management API
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <mach/sys_config.h>
#include <mach/gpio.h>
#include <linux/regulator/consumer.h>
#include "wifi_pm.h"

#define rtl8188eu_msg(...)    do {printk("[rtl8188eu]: "__VA_ARGS__);} while(0)

static int rtl8188eu_powerup = 0;
static int rtk8188eu_suspend = 0;
static char * axp_name = NULL;

// power control by axp
static int rtl8188eu_module_power(int onoff)
{
	struct regulator* wifi_ldo = NULL;
	static int first = 1;

	rtl8188eu_msg("rtl8188eu module power set by axp.\n");
	wifi_ldo = regulator_get(NULL, axp_name);
	if (!wifi_ldo)
		rtl8188eu_msg("get power regulator failed.\n");
	if (first) {
		rtl8188eu_msg("first time\n");
		regulator_force_disable(wifi_ldo);
		first = 0;
	}
	if (onoff) {
		rtl8188eu_msg("regulator on.\n");
		regulator_set_voltage(wifi_ldo, 3300000, 3300000);
		regulator_enable(wifi_ldo);
	} else {
		rtl8188eu_msg("regulator off.\n");
		regulator_disable(wifi_ldo);
	}
	return 0;
}

void rtl8188eu_power(int mode, int *updown)
{
    if (mode) {
        if (*updown) {
			rtl8188eu_module_power(1);
			udelay(50);
        } else {
			rtl8188eu_module_power(0);
        }
        rtl8188eu_msg("sdio wifi power state: %s\n", *updown ? "on" : "off");
    } else {
        if (rtl8188eu_powerup)
            *updown = 1;
        else
            *updown = 0;
		rtl8188eu_msg("sdio wifi power state: %s\n", rtl8188eu_powerup ? "on" : "off");
    }
    return;	
}

static void rtl8188eu_standby(int instadby)
{
	if (instadby) {
		if (rtl8188eu_powerup) {
			rtl8188eu_module_power(0);
			rtk8188eu_suspend = 1;
		}
	} else {
		if (rtk8188eu_suspend) {
			rtl8188eu_module_power(1);
			rtk8188eu_suspend = 0;
		}
	}
	rtl8188eu_msg("sdio wifi : %s\n", instadby ? "suspend" : "resume");
}

void rtl8188eu_gpio_init(void)
{
	script_item_u val;
	script_item_value_type_e type;
	struct wifi_pm_ops *ops = &wifi_select_pm_ops;

	rtl8188eu_msg("exec rtl8188eu_wifi_gpio_init\n");

	type = script_get_item(wifi_para, "wifi_power", &val);
	if (SCIRPT_ITEM_VALUE_TYPE_STR != type) {
		rtl8188eu_msg("failed to fetch wifi_power\n");
		return ;
	}

	axp_name = val.str;
	rtl8188eu_msg("module power name %s\n", axp_name);
	
	rtl8188eu_powerup = 0;
	rtk8188eu_suspend = 0;
	ops->power     = rtl8188eu_power;
	ops->standby   = rtl8188eu_standby;
}
