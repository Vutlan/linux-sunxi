#ifndef WIFI__PM__H
#define WIFI__PM__H

#include <linux/gpio.h>
#define SDIO_WIFI_POWERUP   (1)
#define SDIO_WIFI_INSUSPEND (2)
static char* wifi_para = "sdio_wifi_para";

struct wifi_pm_ops {
	char*           mod_name;
	script_item_u   sdio_card_used;
	script_item_u   sdio_cardid;
	script_item_u   module_sel;
	int             (*gpio_ctrl)(char* name, int level);	
	void            (*standby)(int in);
	void            (*power)(int mode, int *updown);

#ifdef CONFIG_PROC_FS
	struct proc_dir_entry		*proc_root;
	struct proc_dir_entry		*proc_power;
#endif
};

void bcm40181_gpio_init(void);
void bcm40183_gpio_init(void);
void rtl8723as_gpio_init(void);
void rtl8189es_gpio_init(void);

extern struct wifi_pm_ops wifi_card_pm_ops;
extern void sw_mci_rescan_card(unsigned id, unsigned insert);

#endif
