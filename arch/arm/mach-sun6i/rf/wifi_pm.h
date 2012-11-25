#ifndef WIFI__PM__H
#define WIFI__PM__H
#define SDIO_WIFI_POWERUP   (1)
#define SDIO_WIFI_INSUSPEND (2)

struct wifi_pm_ops {
	char*   mod_name;
	u32     sdio_card_used;
	u32     sdio_cardid;
	u32     module_sel;
	u32     pio_hdle;
	int     (*gpio_ctrl)(char* name, int level);
	int     (*get_io_val)(char* name);
	void    (*standby)(int in);
	void    (*power)(int mode, int *updown);

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
