/*
 * rtl8192cu usb wifi power management API
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <mach/sys_config.h>
#include <mach/gpio.h>
#include <linux/regulator/consumer.h>
#include "wifi_pm.h"

#define rtl8192cu_msg(...)    do {printk("[rtl8192cu]: "__VA_ARGS__);} while(0)

static int rtl8192cu_powerup = 0;
static int rtl8192cu_power_pin = 0;
static int rtk8192cu_suspend = 0;

// power control by axp
static int rtl8192cu_module_power(int onoff)
{
	struct regulator* wifi_ldo = NULL;
	static int first = 1;

	rtl8192cu_msg("rtl8192cu module power set by axp.\n");
	wifi_ldo = regulator_get(NULL, "axp22_aldo1");
	if (!wifi_ldo)
		rtl8192cu_msg("get power regulator failed.\n");
	if (first) {
		rtl8192cu_msg("first time\n");
		regulator_force_disable(wifi_ldo);
		first = 0;
	}
	if (onoff) {
		rtl8192cu_msg("regulator on.\n");
		regulator_set_voltage(wifi_ldo, 3300000, 3300000);
		regulator_enable(wifi_ldo);
	} else {
		rtl8192cu_msg("regulator off.\n");
		regulator_disable(wifi_ldo);
	}
	return 0;
}

void rtl8192cu_power(int mode, int *updown)
{
    if (mode) {
        if (*updown) {
			rtl8192cu_module_power(1);
			udelay(50);
        } else {
			rtl8192cu_module_power(0);
        }
        rtl8192cu_msg("sdio wifi power state: %s\n", *updown ? "on" : "off");
    } else {
        if (rtl8192cu_powerup)
            *updown = 1;
        else
            *updown = 0;
		rtl8192cu_msg("sdio wifi power state: %s\n", rtl8192cu_powerup ? "on" : "off");
    }
    return;	
}

static void rtl8192cu_standby(int instadby)
{
	if (instadby) {
		if (rtl8192cu_powerup) {
			rtl8192cu_module_power(0);
			rtk8192cu_suspend = 1;
		}
	} else {
		if (rtk8192cu_suspend) {
			rtl8192cu_module_power(1);
			rtk8192cu_suspend = 0;
		}
	}
	rtl8192cu_msg("sdio wifi : %s\n", instadby ? "suspend" : "resume");
}

void rtl8192cu_gpio_init(void)
{
	script_item_u val ;
	script_item_value_type_e type;
	struct wifi_pm_ops *ops = &wifi_select_pm_ops;

	rtl8192cu_msg("exec rtl8192cu_wifi_gpio_init\n");

	rtl8192cu_powerup = 0;
	rtk8192cu_suspend = 0;
	ops->gpio_ctrl = rtl8192cu_gpio_init;
	ops->power     = rtl8192cu_power;
	ops->standby   = rtl8192cu_standby;
}
