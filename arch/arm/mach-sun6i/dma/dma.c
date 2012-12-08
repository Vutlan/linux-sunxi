/*
 * arch/arm/mach-sun6i/dma/dma.c
 * (C) Copyright 2010-2015
 * Reuuimlla Technology Co., Ltd. <www.reuuimllatech.com>
 * liugang <liugang@reuuimllatech.com>
 *
 * sun6i dma driver interface
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include "dma_include.h"

/**
 * dma_drv_probe - dma driver inital function.
 * @dev:	platform device pointer
 *
 * Returns 0 if success, otherwise return the err line number.
 */
static int __devinit dma_drv_probe(struct platform_device *dev)
{
	int 	ret = 0;

	ret = dma_init(dev);
	if(ret)
		DMA_ERR("%s err, line %d\n", __func__, __LINE__);
	return ret;
}

/**
 * dma_drv_remove - dma driver deinital function.
 * @dev:	platform device pointer
 *
 * Returns 0 if success, otherwise means err.
 */
static int __devexit dma_drv_remove(struct platform_device *dev)
{
	int 	ret = 0;

	ret = dma_deinit();
	if(ret)
		DMA_ERR("%s err, line %d\n", __func__, __LINE__);
	return ret;
}

/**
 * dma_drv_suspend - dma driver suspend function.
 * @dev:	platform device pointer
 * @state:	power state
 *
 * Returns 0 if success, otherwise means err.
 */
int dma_drv_suspend(struct device *dev)
{
	if(NORMAL_STANDBY == standby_type) { /* process for normal standby */
 		DMA_INF("%s: normal standby, line %d\n", __func__, __LINE__);
		/* close dma mode clock */
		if(NULL != g_dma_mod_clk && !IS_ERR(g_dma_mod_clk)) {
			if(0 != clk_reset(g_dma_mod_clk, AW_CCU_CLK_RESET))
				printk("%s err: clk_reset failed\n", __func__);
			clk_disable(g_dma_mod_clk);
			clk_put(g_dma_mod_clk);
			g_dma_mod_clk = NULL;
			DMA_INF("%s: close dma mod clock success\n", __func__);
		}
	} else if(SUPER_STANDBY == standby_type) { /* process for super standby */
 		DMA_INF("%s: super standby, line %d\n", __func__, __LINE__);
		/* close dma clock */
		if(0 != dma_clk_deinit())
			DMA_ERR("%s err, dma_clk_deinit failed\n", __func__);
	}
	return 0;
}

/**
 * dma_drv_resume - dma driver resume function.
 * @dev:	platform device pointer
 *
 * Returns 0 if success, otherwise means err.
 */
int dma_drv_resume(struct device *dev)
{
	if(NORMAL_STANDBY == standby_type) { /* process for normal standby */
 		DMA_INF("%s: normal standby, line %d\n", __func__, __LINE__);
		/* enable dma mode clock */
		g_dma_mod_clk = clk_get(NULL, CLK_MOD_DMA);
		if(NULL == g_dma_mod_clk || IS_ERR(g_dma_mod_clk)) {
			printk("%s err: clk_get %s failed\n", __func__, CLK_MOD_DMA);
			return -EPERM;
		}
		WARN_ON(0 != clk_enable(g_dma_mod_clk));
		WARN_ON(0 != clk_reset(g_dma_mod_clk, AW_CCU_CLK_NRESET));
		DMA_INF("%s: open dma mod clock success\n", __func__);
	} else if(SUPER_STANDBY == standby_type) { /* process for super standby */
 		DMA_INF("%s: super standby, line %d\n", __func__, __LINE__);
		/* enable dma clock */
		if(0 != dma_clk_init())
			DMA_ERR("%s err, dma_clk_init failed\n", __func__);
	}
	return 0;
}

static const struct dev_pm_ops sw_dmac_pm = {
	.suspend	= dma_drv_suspend,
	.resume		= dma_drv_resume,
};
static struct platform_driver sw_dmac_driver = {
	.probe          = dma_drv_probe,
	.remove         = __devexit_p(dma_drv_remove),
	.driver         = {
		.name   = "sw_dmac",
		.owner  = THIS_MODULE,
		.pm 	= &sw_dmac_pm,
		},
};

/**
 * drv_dma_init - dma driver register function
 *
 * Returns 0 if success, otherwise means err.
 */
static int __init drv_dma_init(void)
{
	if(platform_driver_register(&sw_dmac_driver))
		printk("%s(%d) err: platform_driver_register failed\n", __func__, __LINE__);
	return 0;
}


arch_initcall(drv_dma_init);

