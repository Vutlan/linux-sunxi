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

#define PIO_CLK_NAME		"apb_pio"

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
		.irq_num = AW_IRQ_EINT_PA,
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
		.irq_num = AW_IRQ_EINT_PB,
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
		.irq_num = AW_IRQ_EINT_PE,
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
		.irq_num = AW_IRQ_EINT_PG,
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
		.irq_num = AW_IRQ_EINT_R_PL,
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
		.irq_num = AW_IRQ_EINT_R_PM,
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

	g_apb_pio_clk = clk_get(NULL, PIO_CLK_NAME);
	PIO_DBG("%s: get g_apb_pio_clk 0x%08x\n", __func__, (u32)g_apb_pio_clk);
	if(NULL == g_apb_pio_clk || IS_ERR(g_apb_pio_clk)) {
		PIO_ERR("%s err: clk_get %s failed\n", __func__, PIO_CLK_NAME);
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

/**
 * aw_gpio_init - gpio driver init function
 *
 * Returns 0 if sucess, the err line number if failed.
 */
static __init int aw_gpio_init(void)
{
	u32	uret = 0;
	u32 	i = 0;

	if(0 != gpio_clk_init()) {
		PIO_ERR("%s err: line %d\n", __func__, __LINE__);
	}

	for(i = 0; i < ARRAY_SIZE(gpio_chips); i++) {
		/* lock init */
		PIO_CHIP_LOCK_INIT(&gpio_chips[i].lock);

		/* register gpio_chip */
		if(0 != aw_gpiochip_add(&gpio_chips[i].chip)) {
			uret = __LINE__;
			goto End;
		}
	}

End:
	if(0 != uret) {
		PIO_ERR("%s err, line %d\n", __func__, uret);
	}

	return uret;
}

subsys_initcall(aw_gpio_init);

