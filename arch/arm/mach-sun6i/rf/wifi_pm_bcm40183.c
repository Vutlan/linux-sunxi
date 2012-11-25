/*
 * bcm40183 sdio wifi power management API
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <mach/sys_config.h>
#include <mach/gpio.h>
#include <linux/delay.h>

#include "wifi_pm.h"

#define bcm40183_msg(...)    do {printk("[bcm40183]: "__VA_ARGS__);} while(0)
static int bcm40183_wl_on = 0;
static int bcm40183_bt_on = 0;


static int bcm40183_gpio_ctrl(char* name, int level)
{
	struct wifi_pm_ops *ops = &wifi_card_pm_ops;
	char* gpio_cmd[3] = {"bcm40183_wl_regon", "bcm40183_bt_regon", "bcm40183_bt_rst"};
	int i = 0;
	int ret = 0;

	for (i=0; i<3; i++) {
		if (strcmp(name, gpio_cmd[i])==0)
			break;
	}
	if (i==3) {
		bcm40183_msg("No gpio %s for BCM40183 module\n", name);
		return -1;
	}

	bcm40183_msg("Set GPIO %s to %d !\n", name, level);
	if (strcmp(name, "bcm40183_wl_regon") == 0) {
		if (level) {
			if (bcm40183_bt_on) {
				bcm40183_msg("BCM40183 is already powered up by bluetooth\n");
				goto change_state;
			} else {
				bcm40183_msg("BCM40183 is powered up by wifi\n");
				goto power_change;
			}
		} else {
			if (bcm40183_bt_on) {
				bcm40183_msg("BCM40183 should stay on because of bluetooth\n");
				goto change_state;
			} else {
				bcm40183_msg("BCM40183 is powered off by wifi\n");
				goto power_change;
			}
		}
	}

	if (strcmp(name, "bcm40183_bt_regon") == 0) {
		if (level) {
			if (bcm40183_wl_on) {
				bcm40183_msg("BCM40183 is already powered up by wifi\n");
				goto change_state;
			} else {
				bcm40183_msg("BCM40183 is powered up by bt\n");
				goto power_change;
			}
		} else {
			if (bcm40183_wl_on) {
				bcm40183_msg("BCM40183 should stay on because of wifi\n");
				goto change_state;
			} else {
				bcm40183_msg("BCM40183 is powered off by bt\n");
				goto power_change;
			}
		}
	}

gpio_state_change:
	ret = sw_gpio_write_one_pin_value(ops->pio_hdle, level, name);
	if (ret) {
		bcm40183_msg("Failed to set gpio %s to %d !\n", name, level);
		return -1;
	}

	return 0;

power_change:

	ret = sw_gpio_write_one_pin_value(ops->pio_hdle, level, "bcm40183_vcc_en");
	if (ret) {
		bcm40183_msg("Failed to set BCM40183 bcm40183_vcc_en %s\n", level ? "on" : "off");
		return -1;
	}
	
	ret = sw_gpio_write_one_pin_value(ops->pio_hdle, level, "bcm40183_vdd_en");
	if (ret) {
		bcm40183_msg("Failed to set BCM40183 bcm40183_vdd_en %s\n", level ? "on" : "off");
		return -1;
	}	
	
	udelay(500);

change_state:
	if (strcmp(name, "bcm40183_wl_regon")==0)
		bcm40183_wl_on = level;
	if (strcmp(name, "bcm40183_bt_regon")==0)
		bcm40183_bt_on = level;
	bcm40183_msg("BCM40183 power state change: wifi %d, bt %d !!\n", bcm40183_wl_on, bcm40183_bt_on);
	goto gpio_state_change;
}

static int bcm40183_get_gpio_value(char* name)
{
	int ret = -1;
	int i = 0;
	struct wifi_pm_ops *ops = &wifi_card_pm_ops;

	char* gpio_cmd[3] = {"bcm40183_wl_host_wake", "bcm40183_bt_wake", "bcm40183_bt_host_wake"};

	for (i=0; i<3; i++) {
		if (strcmp(name, gpio_cmd[i])) {
			bcm40183_msg("Can not get %s pin value\n", name);
			return -1;
		}
	}

	ret = sw_gpio_read_one_pin_value(ops->pio_hdle, name);
	bcm40183_msg("Succeed to get gpio %s value: %d !\n", name, ret);

	return ret;
}

static void bcm40183_power(int mode, int *updown)
{
	if (mode) {
		if (*updown) {
            bcm40183_gpio_ctrl("bcm40183_wl_regon", 1);
		} else {
            bcm40183_gpio_ctrl("bcm40183_wl_regon", 0);
		}
		bcm40183_msg("sdio wifi power state: %s\n", *updown ? "on" : "off");
	} else {
        if (bcm40183_wl_on)
            *updown = 1;
        else
            *updown = 0;
		bcm40183_msg("sdio wifi power state: %s\n", bcm40183_wl_on ? "on" : "off");
	}	
	return;
}

void bcm40183_gpio_init(void)
{
	struct wifi_pm_ops *ops = &wifi_card_pm_ops;
	
	bcm40183_msg("exec bcm40181_wifi_gpio_init\n");	
	bcm40183_wl_on = 0;
	bcm40183_bt_on = 0;
	ops->gpio_ctrl = bcm40183_gpio_ctrl;
	ops->get_io_val = bcm40183_get_gpio_value;
	ops->power = bcm40183_power;
}
