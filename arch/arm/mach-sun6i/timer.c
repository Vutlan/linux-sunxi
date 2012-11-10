/*
 * arch\arm\mach-sun6i\timer.c
 * (C) Copyright 2010-2016
 * Allwinner Technology Co., Ltd. <www.reuuimllatech.com>
 * huangxin<huangxin@reuuimllatech.com>
 *
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include <mach/hardware.h>
#include <mach/platform.h>
#include <linux/init.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <asm/sched_clock.h>
#include <mach/timer.h>

static void __iomem *timer_cpu_base = 0;
static spinlock_t timer0_spin_lock;
static int timer_set_next_clkevt(unsigned long delta, struct clock_event_device *dev);

#define TIMER0_VALUE (AW_CLOCK_SRC / (AW_CLOCK_DIV*100))

static void timer_set_mode(enum clock_event_mode mode, struct clock_event_device *clk)
{
    volatile u32 ctrl = 0;
	if (clk) {
	    switch (mode) {
	    case CLOCK_EVT_MODE_PERIODIC:
	    	ctrl &= ~(1<<0);    /* Disable timer0 */
	        ctrl = readl(timer_cpu_base + AW_TMR0_CTRL_REG);
	        ctrl &= ~(1<<7);    /* Continuous mode */
	        ctrl |= 1;  /* Enable this timer */
	        break;
	    case CLOCK_EVT_MODE_ONESHOT:
	    	ctrl &= ~(1<<0);    /* Disable timer0 */
		    ctrl = readl(timer_cpu_base + AW_TMR0_CTRL_REG);
	        ctrl &= (1<<7);    /* single mode */
	        ctrl |= 1;  /* Enable this timer */
	        break;
	        case CLOCK_EVT_MODE_UNUSED:
	        case CLOCK_EVT_MODE_SHUTDOWN:
	        default:
	            ctrl = readl(timer_cpu_base + AW_TMR0_CTRL_REG);
	            ctrl &= ~(1<<0);    /* Disable timer0 */
	            break;
	        }
	    writel(ctrl, timer_cpu_base + AW_TMR0_CTRL_REG);
    } else {
    	printk("error:%s,line:%d\n", __func__, __LINE__);
    	BUG();
    }
}


static int timer_set_next_clkevt(unsigned long delta, struct clock_event_device *dev)
{
	unsigned long flags;
	volatile u32 ctrl = 0;
	if (dev) {
		spin_lock_irqsave(&timer0_spin_lock, flags);
	    /* disable timer*/
	    ctrl= readl(timer_cpu_base + AW_TMR0_CTRL_REG);
	    ctrl |= (~(1<<0));
	    writel(ctrl, timer_cpu_base + AW_TMR0_CTRL_REG);
		udelay(1);
	    /* set timer intervalue         */
    	writel(delta, timer_cpu_base + AW_TMR0_INTV_VALUE_REG);
		/* reload the timer intervalue  */
		ctrl= readl(timer_cpu_base + AW_TMR0_CTRL_REG);
	    ctrl |= (1<<1);
	    writel(ctrl, timer_cpu_base + AW_TMR0_CTRL_REG);
	    /* enable timer */
		ctrl= readl(timer_cpu_base + AW_TMR0_CTRL_REG);
	    ctrl |= (1<<0);
	    writel(ctrl, timer_cpu_base + AW_TMR0_CTRL_REG);
	    spin_unlock_irqrestore(&timer0_spin_lock, flags);
	} else {
	    printk("error:%s,line:%d\n", __func__, __LINE__);
    	BUG();
    	return -EINVAL;
	}
    return 0;
}

static struct clock_event_device sun6i_timer0_clockevent = {
    .name = "timer0",
    .shift = 32,
    .rating = 100,
    .features = CLOCK_EVT_FEAT_PERIODIC|CLOCK_EVT_FEAT_ONESHOT,
    .set_mode = timer_set_mode,
    .set_next_event = timer_set_next_clkevt,
};

static irqreturn_t sun6i_timer_interrupt(int irq, void *dev_id)
{
    struct clock_event_device *evt;
    if (dev_id) {
	    evt = (struct clock_event_device *)dev_id;
	
	    /* Clear interrupt */
	    writel(0x1, timer_cpu_base + AW_TMR_IRQ_STA_REG);
	
	    /*
	     * timer_set_next_event will be called only in ONESHOT mode
	     */
	    evt->event_handler(evt);
	} else {
	   	printk("error:%s,line:%d\n", __func__, __LINE__);
    	BUG();
    	return IRQ_NONE;
	}
    return IRQ_HANDLED;
}

static struct irqaction sun6i_timer_irq = {
    .name = "timer0",
    .flags = IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL,
    .handler = sun6i_timer_interrupt,
    .dev_id = &sun6i_timer0_clockevent,
    .irq = AW_IRQ_TIMER0,
};

void __init sun6i_timer_init(void)
{
	int ret = 0;
	volatile u32 ctrl = 0;
	
	timer_cpu_base = ioremap_nocache(AW_TIMER_BASE, 0x1000);
	printk("hx-tickless-hrtimer:[%s] base=%p\n", __FUNCTION__,timer_cpu_base);

    /* Disable & clear all timers */
	writel(0x0, timer_cpu_base + AW_TMR_IRQ_EN_REG);
    writel(0x3f, timer_cpu_base + AW_TMR_IRQ_STA_REG);

    /* Init timer0 */
    writel(TIMER0_VALUE, timer_cpu_base + AW_TMR0_INTV_VALUE_REG);
    
    ctrl = readl(timer_cpu_base + AW_TMR0_CTRL_REG);
#ifdef AW_ASIC_PLATFORM
	/*OSC24m*/
	printk("fpga,%s,line:%d\n", __func__, __LINE__);
    ctrl |= (1<<2);
#else 
	/*internalOSC/N*/
	printk("fpga,%s,line:%d\n", __func__, __LINE__);
    ctrl |= (0<<2);
#endif
	ctrl |= (1<<1);
	writel(ctrl, timer_cpu_base + AW_TMR0_CTRL_REG);
    ret = setup_irq(AW_IRQ_TIMER0, &sun6i_timer_irq);
    if (ret) {
    	early_printk("failed to setup irq %d\n", AW_IRQ_TIMER0);
    }

    /* Enable timer0 */
    writel(0x1, timer_cpu_base + AW_TMR_IRQ_EN_REG);

    sun6i_timer0_clockevent.mult = div_sc(AW_CLOCK_SRC/AW_CLOCK_DIV, NSEC_PER_SEC, sun6i_timer0_clockevent.shift);
    sun6i_timer0_clockevent.max_delta_ns = clockevent_delta2ns(0x80000000, &sun6i_timer0_clockevent);
    sun6i_timer0_clockevent.min_delta_ns = clockevent_delta2ns(0x1, &sun6i_timer0_clockevent)+100000;
    sun6i_timer0_clockevent.cpumask = cpu_all_mask;
    sun6i_timer0_clockevent.irq = sun6i_timer_irq.irq;
    clockevents_register_device(&sun6i_timer0_clockevent);
}
