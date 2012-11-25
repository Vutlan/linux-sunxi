/*
 * rtl8723as sdio wifi power management API
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <mach/sys_config.h>
#include <mach/gpio.h>

#include "wifi_pm.h"

#define SDIO_MODULE_NAME "RTL8723AS"
#define rtl8723as_msg(...)    do {printk("[RTL8723AS]: "__VA_ARGS__);} while(0)
static int rtl8723as_wl_on = 0;
static int rtl8723as_bt_on = 0;

static int rtk_suspend = 0;

static int rtl8723as_gpio_ctrl(char* name, int level)
{
	struct wifi_pm_ops *ops = &wifi_card_pm_ops;
	char* gpio_cmd[4] = {"rtk_rtl8723as_wb_pwr", "rtk_rtl8723as_wl_dis", "rtk_rtl8723as_bt_dis", "rtk_rtl8723as_wl_wps"};
	int i = 0;
	int ret = 0;
	
	for (i=0; i<4; i++) {
		if (strcmp(name, gpio_cmd[i])==0)
			break;
	}
	if (i==4) {
		rtl8723as_msg("No gpio %s for %s module\n", name, SDIO_MODULE_NAME);
		return -1;
	}
	
	rtl8723as_msg("Set GPIO %s to %d !\n", name, level);
	if (strcmp(name, "rtk_rtl8723as_wl_dis") == 0) {
		if ((level && !rtl8723as_bt_on)	|| (!level && !rtl8723as_bt_on)) {
			rtl8723as_msg("%s is powered %s by wifi\n", SDIO_MODULE_NAME, level ? "up" : "down");
			goto power_change;
		} else {
			if (level) {
				rtl8723as_msg("%s is already on by bt\n", SDIO_MODULE_NAME);
			} else {
				rtl8723as_msg("%s should stay on because of bt\n", SDIO_MODULE_NAME);
			}
			goto state_change;
		}
	}
	if (strcmp(name, "rtk_rtl8723as_bt_dis") == 0) {
		if ((level && !rtl8723as_wl_on)	|| (!level && !rtl8723as_wl_on)) {
			rtl8723as_msg("%s is powered %s by bt\n", SDIO_MODULE_NAME, level ? "up" : "down");
			goto power_change;
		} else {
			if (level) {
				rtl8723as_msg("%s is already on by wifi\n", SDIO_MODULE_NAME);
			} else {
				rtl8723as_msg("%s should stay on because of wifi\n", SDIO_MODULE_NAME);
			}
			goto state_change;
		}
	}

gpio_state_change:
	ret = sw_gpio_write_one_pin_value(ops->pio_hdle, level, name);
	if (ret) {
		rtl8723as_msg("Failed to set gpio %s to %d !\n", name, level);
		return -1;
	}
	
	return 0;
	
power_change:
	ret = sw_gpio_write_one_pin_value(ops->pio_hdle, level, "rtk_rtl8723as_wb_pwr");
	if (ret) {
		rtl8723as_msg("Failed to power off %s module!\n", SDIO_MODULE_NAME);
		return -1;
	}
	udelay(500);
	
state_change:
	if (strcmp(name, "rtk_rtl8723as_wl_dis")==0)
		rtl8723as_wl_on = level;
	if (strcmp(name, "rtk_rtl8723as_bt_dis")==0)
		rtl8723as_bt_on = level;
	rtl8723as_msg("%s power state change: wifi %d, bt %d !!\n", SDIO_MODULE_NAME, rtl8723as_wl_on, rtl8723as_bt_on);
	
	goto gpio_state_change;
}

static int rtl8723as_get_gpio_value(char* name)
{
	struct wifi_pm_ops *ops = &wifi_card_pm_ops;
	
	if (strcmp(name, "rtk_rtl8723as_wl_wps")) {
		rtl8723as_msg("No gpio %s for %s\n", name, SDIO_MODULE_NAME);
		return -1;
	}
	
	return sw_gpio_read_one_pin_value(ops->pio_hdle, name);
}

void rtl8723as_power(int mode, int *updown)
{
    if (mode) {
        if (*updown) {
        	rtl8723as_gpio_ctrl("rtk_rtl8723as_wl_dis", 1);
        } else {
        	rtl8723as_gpio_ctrl("rtk_rtl8723as_wl_dis", 0);
        }
        rtl8723as_msg("sdio wifi power state: %s\n", *updown ? "on" : "off");
    } else {
        if (rtl8723as_wl_on)
            *updown = 1;
        else
            *updown = 0;
		rtl8723as_msg("sdio wifi power state: %s\n", rtl8723as_wl_on ? "on" : "off");
    }
    return;	
}

static void rtl8723as_standby(int instadby)
{
	if (instadby) {
		if (rtl8723as_wl_on) {
			rtl8723as_gpio_ctrl("rtk_rtl8723as_wl_dis", 0);
			rtk_suspend = 1;
		}
	} else {
		if (rtk_suspend) {
			rtl8723as_gpio_ctrl("rtk_rtl8723as_wl_dis", 1);
			sw_mci_rescan_card(3, 1);
			rtk_suspend = 0;
		}
	}
	rtl8723as_msg("sdio wifi : %s\n", instadby ? "suspend" : "resume");
}

void rtl8723as_gpio_init(void)
{
	struct wifi_pm_ops *ops = &wifi_card_pm_ops;
	
	rtl8723as_msg("exec rt8723as_wifi_gpio_init\n");
	rtl8723as_wl_on = 0;
	rtl8723as_bt_on = 0;
	rtk_suspend 	= 0;
	ops->gpio_ctrl	= rtl8723as_gpio_ctrl;
	ops->get_io_val = rtl8723as_get_gpio_value;
	ops->power 		= rtl8723as_power;
	ops->standby	= rtl8723as_standby;
}

#undef SDIO_MODULE_NAME
