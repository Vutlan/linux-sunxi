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
	PIO_DBG_FUN_LINE_TODO;

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
	PIO_DBG_FUN_LINE_TODO;

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

	if(NULL != g_apb_pio_clk) {
		PIO_INF("%s maybe err: g_apb_pio_clk not NULL, line %d\n", __func__, __LINE__);
	}

	g_apb_pio_clk = clk_get(NULL, CLK_APB_PIO);
	PIO_DBG("%s: get g_apb_pio_clk 0x%08x\n", __func__, (u32)g_apb_pio_clk);
	if(NULL == g_apb_pio_clk || IS_ERR(g_apb_pio_clk)) {
		PIO_ERR("%s err: clk_get %s failed\n", __func__, CLK_APB_PIO);
		return -EPERM;
	} else {
		if(0 != clk_enable(g_apb_pio_clk)) {
			PIO_ERR("%s err: clk_enable failed\n", __func__);
			return -EPERM;
		}
		PIO_DBG("%s: clk_enable g_apb_pio_clk success\n", __func__);
		if(0 != clk_reset(g_apb_pio_clk, AW_CCU_CLK_NRESET)) {
			PIO_ERR("%s err: clk_reset failed\n", __func__);
			return -EPERM;
		}
		PIO_DBG("%s: clk_reset g_apb_pio_clk-AW_CCU_CLK_NRESET success\n", __func__);
	}

	PIO_DBG("%s success\n", __func__);
	return 0;
}

u32 gpio_clk_deinit(void)
{
	PIO_INF("%s todo: cpus pio clock deinit, line %d\n", __func__, __LINE__);
	//r_gpio_clk_deinit();

	if(NULL == g_apb_pio_clk || IS_ERR(g_apb_pio_clk)) {
		PIO_INF("%s: g_apb_pio_clk 0x%08x invalid, just return\n", __func__, (u32)g_apb_pio_clk);
		return 0;
	}

	if(0 != clk_reset(g_apb_pio_clk, AW_CCU_CLK_RESET)) {
		PIO_ERR("%s err: clk_reset failed\n", __func__);
	}
	clk_disable(g_apb_pio_clk);
	clk_put(g_apb_pio_clk);
	g_apb_pio_clk = NULL;

	PIO_DBG("%s success\n", __func__);
	return 0;
}

#ifdef ADD_AXP_PIN_20121117
extern int axp_gpio_set_io(int gpio, int io_state);
extern int axp_gpio_get_io(int gpio, int *io_state);
extern int axp_gpio_set_value(int gpio, int value);
extern int axp_gpio_get_value(int gpio, int *value);
static int __axp_gpio_input(struct gpio_chip *chip, unsigned offset)
{
	u32  index = chip->base + offset;

	if(GPIO_AXP(0) == index)
		return axp_gpio_set_io(0, 0);
	else if(GPIO_AXP(1) == index)
		return axp_gpio_set_io(1, 0);
	else
		return -EINVAL;
}
static int __axp_gpio_output(struct gpio_chip *chip, unsigned offset, int value)
{
	u32  index = chip->base + offset;
	int  ret = 0;

	if(GPIO_AXP(0) == index) {
		ret = axp_gpio_set_io(0, 1); /* set to output */
		if(ret)
			return ret;
		return axp_gpio_set_value(0, value); /* set value */
	} else if(GPIO_AXP(1) == index) {
		ret = axp_gpio_set_io(1, 1); /* set to output */
		if(ret)
			return ret;
		return axp_gpio_set_value(1, value); /* set value */
	}
	else
		return -EINVAL;
}
static void __axp_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	u32  index = chip->base + offset;

	if(GPIO_AXP(0) == index)
		axp_gpio_set_value(0, value);
	else if(GPIO_AXP(1) == index)
		axp_gpio_set_value(1, value);
	else
		WARN_ON(1);
}
static int __axp_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	u32  index = chip->base + offset;
	int  value = 0;

	if(GPIO_AXP(0) == index) {
		WARN_ON(0 != axp_gpio_get_value(0, &value));
		return value;
	} else if(GPIO_AXP(1) == index) {
		WARN_ON(0 != axp_gpio_get_value(1, &value));
		return value;
	} else {
		printk("%s err: line %d\n", __func__, __LINE__);;
		return 0;
	}
}

struct gpio_chip axp_gpio_chip = {
	.base	= AXP_NR_BASE,
	.ngpio	= AXP_NR,
	.label	= "axp_pin",
	.direction_input = __axp_gpio_input,
	.direction_output = __axp_gpio_output,
	.set	= __axp_gpio_set,
	.get	= __axp_gpio_get,
};
#endif /* ADD_AXP_PIN_20121117 */

/**
 * aw_gpio_init - gpio driver init function
 *
 * Returns 0 if sucess, the err line number if failed.
 */
static __init int aw_gpio_init(void)
{
	u32	uret = 0;
	u32 	i = 0;

	/* init gpio clock */
	if(0 != gpio_clk_init()) {
		PIO_ERR("%s err: line %d\n", __func__, __LINE__);
	}
	/* register gpio chips */
	for(i = 0; i < ARRAY_SIZE(gpio_chips); i++) {
		PIO_CHIP_LOCK_INIT(&gpio_chips[i].lock);
		/* register gpio_chip */
		if(0 != aw_gpiochip_add(&gpio_chips[i].chip)) {
			uret = __LINE__;
			goto End;
		}
	}
#ifdef ADD_AXP_PIN_20121117
	/* register axp gpio chip */
	if(0 != gpiochip_add(&axp_gpio_chip))
		printk("%s err, line %d\n", __func__, __LINE__);
#endif /* ADD_AXP_PIN_20121117 */

End:
	if(0 != uret) {
		PIO_ERR("%s err, line %d\n", __func__, uret);
	}

	return uret;
}
subsys_initcall(aw_gpio_init);

