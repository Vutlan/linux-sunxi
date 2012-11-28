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
static int rtk_rtl8723as_wb_pwr = 0;
static int rtk_rtl8723as_wl_dis = 0;
static int rtk_rtl8723as_bt_dis = 0;

static int rtl8723as_gpio_ctrl(char* name, int level)
{
	int i = 0, ret1 = 0, ret2 = 0, gpio = 0;
	unsigned long flags = 0;
	char* gpio_name[3] = {"rtk_rtl8723as_wb_pwr", "rtk_rtl8723as_wl_dis", "rtk_rtl8723as_bt_dis"};

	for (i=0; i<3; i++) {
		if (strcmp(name, gpio_name[i])==0) {
			    switch (i)
			    {
			        case 0: /* rtk_rtl8723as_wb_pwr */
						gpio = rtk_rtl8723as_wb_pwr;
			            break;
			        case 1: /* rtk_rtl8723as_wl_dis */
						gpio = rtk_rtl8723as_wl_dis;
			            break;
					case 2: /* rtk_rtl8723as_bt_dis */
						gpio = rtk_rtl8723as_bt_dis;
						break;
					default:
            			rtl8723as_msg("no matched gpio!\n");
			    }
			break;
		}
	}

	if (i==3) {
		rtl8723as_msg("No gpio %s for %s module\n", name, SDIO_MODULE_NAME);
		return -1;
	}

	if (1==level)
		flags = GPIOF_OUT_INIT_HIGH;
	else 
		flags = GPIOF_OUT_INIT_LOW;
	
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

	ret1 = gpio_request(gpio, NULL);
	if (0!=ret1)
		rtl8723as_msg("warming failed to request gpio %d\n", gpio);

	ret2 = gpio_request_one(gpio, flags, NULL);
	if (ret2) {
		if (0==ret1)
			gpio_free(gpio);
		rtl8723as_msg("failed to set gpio %d to %d !\n", gpio, level);
		return -1;
	} else {
		if (0==ret1)
			gpio_free(gpio);
		rtl8723as_msg("succeed to set gpio %d to %d !\n", gpio, level);
	}
	
	return 0;
	
power_change:

	ret1 = gpio_request(rtk_rtl8723as_wb_pwr, NULL);
	if (0!=ret1)
		rtl8723as_msg("warming failed to request rtk_rtl8723as_wb_pwr gpio\n");

	ret2 = gpio_request_one(rtk_rtl8723as_wb_pwr, flags, NULL);
	if (ret2) {
		if (0==ret1)
			gpio_free(gpio);
		rtl8723as_msg("failed to set gpio rtk_rtl8723as_wb_pwr to %d !\n", level);
		return -1;
	} else {
		if (0==ret1)
			gpio_free(gpio);
		rtl8723as_msg("succeed to set gpio rtk_rtl8723as_wb_pwr to %d !\n", level);
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
	script_item_u val ;
	script_item_value_type_e type;
	struct wifi_pm_ops *ops = &wifi_card_pm_ops;
	
	rtl8723as_msg("exec rt8723as_wifi_gpio_init\n");
	
	type = script_get_item(wifi_para, "rtk_rtl8723as_wb_pwr", &val);
	if (SCIRPT_ITEM_VALUE_TYPE_PIO!=type) 
		rtl8723as_msg("get rtl8723as rtk_rtl8723as_wb_pwr gpio failed\n");
	else
		rtk_rtl8723as_wb_pwr = val.gpio.gpio;

	type = script_get_item(wifi_para, "rtk_rtl8723as_wl_dis", &val);
	if (SCIRPT_ITEM_VALUE_TYPE_PIO!=type) 
		rtl8723as_msg("get rtl8723as rtk_rtl8723as_wl_dis gpio failed\n");
	else
		rtk_rtl8723as_wl_dis = val.gpio.gpio;

	type = script_get_item(wifi_para, "rtk_rtl8723as_bt_dis", &val);
	if (SCIRPT_ITEM_VALUE_TYPE_PIO!=type) 
		rtl8723as_msg("get rtl8723as rtk_rtl8723as_bt_dis gpio failed\n");
	else
		rtk_rtl8723as_bt_dis = val.gpio.gpio;
	
	rtl8723as_wl_on = 0;
	rtl8723as_bt_on = 0;
	rtk_suspend 	= 0;
	ops->gpio_ctrl	= rtl8723as_gpio_ctrl;
	ops->power 		= rtl8723as_power;
	ops->standby	= rtl8723as_standby;
}

#undef SDIO_MODULE_NAME
