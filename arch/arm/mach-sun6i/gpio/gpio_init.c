/*
 * arch/arm/mach-sun6i/gpio/gpio_init.c
 * (C) Copyright 2010-2015
 * Reuuimlla Technology Co., Ltd. <www.reuuimllatech.com>
 * liugang <liugang@reuuimllatech.com>
 *
 * sun6i gpio driver
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include "gpio_include.h"

//#define GPIO_SUPPORT_STANDBY /* noramlly, gpio need not deal standby */

static struct clk *g_apb_pio_clk = NULL;

/**
 * gpio_save - save somethig for the chip before enter sleep
 * @chip:	aw_gpio_chip which will be saved
 *
 * Returns 0 if sucess, the err line number if failed.
 */
u32 gpio_save(struct aw_gpio_chip *pchip)
{
	/* save something before suspend */
	printk("%s: not implete yet, line %d\n", __func__, __LINE__);
	return 0;
}

/**
 * gpio_resume - restore somethig for the chip after wake up
 * @chip:	aw_gpio_chip which will be saved
 *
 * Returns 0 if sucess, the err line number if failed.
 */
u32 gpio_resume(struct aw_gpio_chip *pchip)
{
	/* restore something after wakeup */
	printk("%s: not implete yet, line %d\n", __func__, __LINE__);
	return 0;
}

/*
 * gpio power api struct
 */
struct gpio_pm_t g_pm = {
	gpio_save,
	gpio_resume
};

/*
 * gpio config api struct
 */
struct gpio_cfg_t g_cfg = {
	gpio_set_cfg,
	gpio_get_cfg,
	gpio_set_pull,
	gpio_get_pull,
	gpio_set_drvlevel,
	gpio_get_drvlevel,
};

/*
 * gpio eint config api struct
 */
struct gpio_eint_cfg_t g_eint_cfg = {
	gpio_eint_set_trig,
	gpio_eint_get_trig,
	gpio_eint_set_enable,
	gpio_eint_get_enable,
	gpio_eint_get_irqpd_sta,
	gpio_eint_clr_irqpd_sta,
	gpio_eint_set_debounce,
	gpio_eint_get_debounce,
};

/*
 * gpio chips for the platform
 */
struct aw_gpio_chip gpio_chips[] = {
	{
		.cfg	= &g_cfg,
		.pm	= &g_pm,
		.chip	= {
			.base	= PA_NR_BASE,
			.ngpio	= PA_NR,
			.label	= "GPA",
			.to_irq = __pio_to_irq,
		},
		.vbase  = (void __iomem *)PIO_VBASE(0),
		/* cfg for eint */
		.irq_num = AW_IRQ_EINTA,
		.vbase_eint = (void __iomem *)PIO_VBASE_EINT_PA,
		.cfg_eint = &g_eint_cfg,
	}, {
		.cfg	= &g_cfg,
		.pm	= &g_pm,
		.chip	= {
			.base	= PB_NR_BASE,
			.ngpio	= PB_NR,
			.label	= "GPB",
			.to_irq = __pio_to_irq,
		},
		.vbase  = (void __iomem *)PIO_VBASE(1),
		/* cfg for eint */
		.irq_num = AW_IRQ_EINTB,
		.vbase_eint = (void __iomem *)PIO_VBASE_EINT_PB,
		.cfg_eint = &g_eint_cfg,
	}, {
		.cfg	= &g_cfg,
		.pm	= &g_pm,
		.chip	= {
			.base	= PC_NR_BASE,
			.ngpio	= PC_NR,
			.label	= "GPC",
		},
		.vbase  = (void __iomem *)PIO_VBASE(2),
	}, {
		.cfg	= &g_cfg,
		.pm	= &g_pm,
		.chip	= {
			.base	= PD_NR_BASE,
			.ngpio	= PD_NR,
			.label	= "GPD",
		},
		.vbase  = (void __iomem *)PIO_VBASE(3),
	}, {
		.cfg	= &g_cfg,
		.pm	= &g_pm,
		.chip	= {
			.base	= PE_NR_BASE,
			.ngpio	= PE_NR,
			.label	= "GPE",
			.to_irq = __pio_to_irq,
		},
		.vbase  = (void __iomem *)PIO_VBASE(4),
		/* cfg for eint */
		.irq_num = AW_IRQ_EINTE,
		.vbase_eint = (void __iomem *)PIO_VBASE_EINT_PE,
		.cfg_eint = &g_eint_cfg,
	}, {
		.cfg	= &g_cfg,
		.pm	= &g_pm,
		.chip	= {
			.base	= PF_NR_BASE,
			.ngpio	= PF_NR,
			.label	= "GPF",
		},
		.vbase  = (void __iomem *)PIO_VBASE(5),
	}, {
		.cfg	= &g_cfg,
		.pm	= &g_pm,
		.chip	= {
			.base	= PG_NR_BASE,
			.ngpio	= PG_NR,
			.label	= "GPG",
			.to_irq = __pio_to_irq,
		},
		.vbase  = (void __iomem *)PIO_VBASE(6),
		/* cfg for eint */
		.irq_num = AW_IRQ_EINTG,
		.vbase_eint = (void __iomem *)PIO_VBASE_EINT_PG,
		.cfg_eint = &g_eint_cfg,
	}, {
		.cfg	= &g_cfg,
		.pm	= &g_pm,
		.chip	= {
			.base	= PH_NR_BASE,
			.ngpio	= PH_NR,
			.label	= "GPH",
		},
		.vbase  = (void __iomem *)PIO_VBASE(7),
	}, {
		.cfg	= &g_cfg,
		.pm	= &g_pm,
		.chip	= {
			.base	= PL_NR_BASE,
			.ngpio	= PL_NR,
			.label	= "GPL",
			.to_irq = __pio_to_irq,
		},
		.vbase  = (void __iomem *)RPIO_VBASE(0),
		/* cfg for eint */
		.irq_num = AW_IRQ_EINTL,
		.vbase_eint = (void __iomem *)PIO_VBASE_EINT_R_PL,
		.cfg_eint = &g_eint_cfg,
	}, {
		.cfg	= &g_cfg,
		.pm	= &g_pm,
		.chip	= {
			.base	= PM_NR_BASE,
			.ngpio	= PM_NR,
			.label	= "GPM",
			.to_irq = __pio_to_irq,
		},
		.vbase  = (void __iomem *)RPIO_VBASE(1),
		/* cfg for eint */
		.irq_num = AW_IRQ_EINTM,
		.vbase_eint = (void __iomem *)PIO_VBASE_EINT_R_PM,
		.cfg_eint = &g_eint_cfg,
	}
};

u32 gpio_clk_init(void)
{
	PIO_INF("%s todo: cpus pio clock init, line %d\n", __func__, __LINE__);
	//r_gpio_clk_init();

	if(NULL != g_apb_pio_clk)
		PIO_INF("%s maybe err: g_apb_pio_clk not NULL, line %d\n", __func__, __LINE__);

	g_apb_pio_clk = clk_get(NULL, CLK_APB_PIO);
	PIO_DBG("%s: get g_apb_pio_clk 0x%08x\n", __func__, (u32)g_apb_pio_clk);
	if(NULL == g_apb_pio_clk || IS_ERR(g_apb_pio_clk)) {
		printk("%s err: clk_get %s failed\n", __func__, CLK_APB_PIO);
		goto err;
	} else {
		if(0 != clk_enable(g_apb_pio_clk)) {
			printk("%s err: clk_enable failed\n", __func__);
			goto err;
		}
		PIO_DBG("%s: clk_enable g_apb_pio_clk success\n", __func__);
		if(0 != clk_reset(g_apb_pio_clk, AW_CCU_CLK_NRESET)) {
			printk("%s err: clk_reset failed\n", __func__);
			goto err;
		}
		PIO_DBG("%s: clk_reset g_apb_pio_clk-AW_CCU_CLK_NRESET success\n", __func__);
	}
	PIO_DBG("%s success\n", __func__);
	return 0;
err:
	return -EPERM;
}

u32 gpio_clk_deinit(void)
{
	//r_gpio_clk_deinit(); /* cpus pio clock deinit here */
	if(NULL == g_apb_pio_clk || IS_ERR(g_apb_pio_clk)) {
		PIO_INF("%s: g_apb_pio_clk 0x%08x invalid, just return\n", __func__, (u32)g_apb_pio_clk);
		return 0;
	}

	if(0 != clk_reset(g_apb_pio_clk, AW_CCU_CLK_RESET))
		printk("%s err: clk_reset failed\n", __func__);
	clk_disable(g_apb_pio_clk);
	clk_put(g_apb_pio_clk);
	g_apb_pio_clk = NULL;

	PIO_DBG("%s success\n", __func__);
	return 0;
}

#ifdef GPIO_SUPPORT_STANDBY
int gpio_drv_suspend(struct device *dev)
{
	if(NORMAL_STANDBY == standby_type) /* process for normal standby */
 		PIO_INF("%s: normal standby, line %d\n", __func__, __LINE__);
	else if(SUPER_STANDBY == standby_type) { /* process for super standby */
 		PIO_INF("%s: super standby, line %d\n", __func__, __LINE__);
		if(0 != gpio_clk_deinit())
			printk("%s err, gpio_clk_deinit failed\n", __func__);
	}
	return 0;
}

int gpio_drv_resume(struct device *dev)
{
	if(NORMAL_STANDBY == standby_type) /* process for normal standby */
 		PIO_INF("%s: normal standby, line %d\n", __func__, __LINE__);
	else if(SUPER_STANDBY == standby_type) { /* process for super standby */
 		PIO_INF("%s: super standby, line %d\n", __func__, __LINE__);
		if(0 != gpio_clk_init())
			printk("%s err, gpio_clk_init failed\n", __func__);
	}
	return 0;
}

static const struct dev_pm_ops sw_gpio_pm = {
	.suspend	= gpio_drv_suspend,
	.resume		= gpio_drv_resume,
};
#endif /* GPIO_SUPPORT_STANDBY */

static struct platform_device sw_gpio_device = {
	.name = "sw_gpio",
};
static struct platform_driver sw_gpio_driver = {
	.driver.name 	= "sw_gpio",
	.driver.owner 	= THIS_MODULE,
#ifdef GPIO_SUPPORT_STANDBY
	.driver.pm 	= &sw_gpio_pm,
#endif /* GPIO_SUPPORT_STANDBY */
};


void sw_gpio_hold_output_enabled(bool enabled)
{
	script_item_u		used, *list = NULL;
	script_item_value_type_e type;
	u32					pl_values, pm_values;
	u32					gpio_index, gpio_pl_bits = 0, gpio_pm_bits = 0;
	int					gpio_hold_count;

	type = script_get_item("gpio_hold_para", "gpio_hold_used", &used);
	if (SCIRPT_ITEM_VALUE_TYPE_INT != type || !used.val) {
		printk("[gpio_init] gpio_hold_para.gpio_hold_used type error or == 0\n");
		return;
	}

	gpio_hold_count = script_get_pio_list("gpio_hold_para", &list);
	// printk("[gpio_init] gpio hold count is : %d.\n", gpio_hold_count);
	if (!gpio_hold_count) {
		printk("[gpio_init] gpio_hold_para get gpio list failed\n");
		return;
	}

	while (gpio_hold_count--) {
		gpio_index = (list + gpio_hold_count)->gpio.gpio;

		if (gpio_index >= GPIOL(0) && gpio_index <= GPIOL(8))
			gpio_pl_bits |= 1 << (gpio_index - GPIOL(0));

		if (gpio_index >= GPIOM(0) && gpio_index <= GPIOM(7))
			gpio_pm_bits |= 1 << (gpio_index - GPIOM(0));
	}

	// printk("[gpio_init] gpio_pl_bits = %#x, gpio_pm_bits = %#x\n",
	//         gpio_pl_bits, gpio_pm_bits);

	pl_values = ioread32(AW_VIR_RTC_BASE + 0x180);
	pm_values = ioread32(AW_VIR_RTC_BASE + 0x184);

	if (enabled) {
		pl_values |= gpio_pl_bits;
		pm_values |= gpio_pm_bits;
	} else {
		pl_values &= ~gpio_pl_bits;
		pm_values &= ~gpio_pm_bits;
	}

	iowrite32(pl_values, AW_VIR_RTC_BASE + 0x180);
	iowrite32(pm_values, AW_VIR_RTC_BASE + 0x184);
}
EXPORT_SYMBOL(sw_gpio_hold_output_enabled);

/**
 * aw_gpio_init - gpio driver init function
 *
 * Returns 0 if sucess, the err line number if failed.
 */
static __init int aw_gpio_init(void)
{
	u32	uret = 0;
	u32 	i = 0;

	// disable gpio hold output
	sw_gpio_hold_output_enabled(false);

	/* init gpio clock */
	if(0 != gpio_clk_init())
		printk("%s err: line %d\n", __func__, __LINE__);
	/* register gpio chips */
	for(i = 0; i < ARRAY_SIZE(gpio_chips); i++) {
		PIO_CHIP_LOCK_INIT(&gpio_chips[i].lock);
		/* register gpio_chip */
		if(0 != aw_gpiochip_add(&gpio_chips[i].chip)) {
			uret = __LINE__;
			goto end;
		}
	}
	/* register gpio platform driver */
	if(platform_device_register(&sw_gpio_device))
		printk("%s(%d) err: platform_device_register failed\n", __func__, __LINE__);
	if(platform_driver_register(&sw_gpio_driver))
		printk("%s(%d) err: platform_driver_register failed\n", __func__, __LINE__);

#if (CONFIG_ARCH_SUN6I == 1) /* pull up all pl pin, in case electricity leak, sunny */
	for(i = PL_NR_BASE; i < PL_NR_BASE + PL_NR - 1; i++) {
		// reduce PL GPIO current when shutdown
		// for app5 inetq70 board: PL0~PL1 pull-up, PL2~PL8 pull-down
		if (i <= PL_NR_BASE + 1)
			WARN_ON(sw_gpio_setpull(i, 0x01));
		if (i >= PL_NR_BASE + 2)
			WARN_ON(sw_gpio_setpull(i, 0x02));
	}
#endif

end:
	if(0 != uret)
		printk("%s err, line %d\n", __func__, uret);
	return uret;
}
subsys_initcall(aw_gpio_init);

