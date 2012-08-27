/*
 * arch\arm\mach-sun6i\timer.c
 * (C) Copyright 2010-2016
 * Allwinner Technology Co., Ltd. <www.reuuimllatech.com>
 * chenpailin <huangxin@reuuimllatech.com>
 *
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#ifndef __AW_TIMER_H__
#define __AW_TIMER_H__

#undef TIMER_DBG
#undef TIMER_ERR
#if (1)
    #define TIMER_DBG(format,args...)  printk("[TIMER1SRC] "format,##args)
    #define TIMER_ERR(format,args...)  printk("[TIMER1SRC] "format,##args)
#else
    #define TIMER_DBG(...)
    #define TIMER_ERR(...)
#endif

#ifndef __tmr_reg
    #define __tmr_reg(x)    (*(volatile __u32 *)(x))
#endif  /*#ifndef __tmr_reg */

/*define the timer1 irq value*/
#define SW_INT_IRQNO_TIMER1 37//timer1's irq is 37 in fpga,however the timer1's irq is 51 in ic

/* define timer io base on aw chips */
#define AW_TMR_IO_BASE          0xf1c20c00
/* define timer io register address */
#define TMR_REG_o_IRQ_EN        (AW_TMR_IO_BASE + 0x0000)
#define TMR_REG_o_IRQ_STAT      (AW_TMR_IO_BASE + 0x0004)

#define TMR_REG_o_TMR0_CTL      (AW_TMR_IO_BASE + 0x0010)
#define TMR_REG_o_TMR0_INTV     (AW_TMR_IO_BASE + 0x0014)
#define TMR_REG_o_TMR0_CUR      (AW_TMR_IO_BASE + 0x0018)

#define TMR_REG_o_TMR1_CTL      (AW_TMR_IO_BASE + 0x0020)
#define TMR_REG_o_TMR1_INTV     (AW_TMR_IO_BASE + 0x0024)
#define TMR_REG_o_TMR1_CUR      (AW_TMR_IO_BASE + 0x0028)

/*define timer counter base on aw chips*/
#define AW_TMR_IO_COUNT_BASE	0xf1f01c00
/*define timer counter io register address*/
#define TMR_REG_o_CNT64_CTL     (AW_TMR_IO_COUNT_BASE + 0x0280)
#define TMR_REG_o_CNT64_LO      (AW_TMR_IO_COUNT_BASE + 0x0284)
#define TMR_REG_o_CNT64_HI      (AW_TMR_IO_COUNT_BASE + 0x0288)

/* define timer io register value */
#define TMR_REG_IRQ_EN          __tmr_reg(TMR_REG_o_IRQ_EN   )
#define TMR_REG_IRQ_STAT        __tmr_reg(TMR_REG_o_IRQ_STAT )

#define TMR_REG_TMR0_CTL        __tmr_reg(TMR_REG_o_TMR0_CTL )
#define TMR_REG_TMR0_INTV       __tmr_reg(TMR_REG_o_TMR0_INTV)
#define TMR_REG_TMR0_CUR        __tmr_reg(TMR_REG_o_TMR0_CUR )


#define TMR_REG_TMR1_CTL        __tmr_reg(TMR_REG_o_TMR1_CTL )
#define TMR_REG_TMR1_INTV       __tmr_reg(TMR_REG_o_TMR1_INTV)
#define TMR_REG_TMR1_CUR        __tmr_reg(TMR_REG_o_TMR1_CUR )

#define TMR_REG_CNT64_CTL       __tmr_reg(TMR_REG_o_CNT64_CTL)
#define TMR_REG_CNT64_LO        __tmr_reg(TMR_REG_o_CNT64_LO )
#define TMR_REG_CNT64_HI        __tmr_reg(TMR_REG_o_CNT64_HI )

/* define timer clock source */
#define TMR_CLK_SRC_32KLOSC     (0)
#define TMR_CLK_SRC_24MHOSC     (1)
#define TMR_CLK_SRC_PLL         (2)

/* config clock frequency   */
#define AW_HPET_CLK_SRC     TMR_CLK_SRC_24MHOSC
//#define AW_HPET_CLK_EVT     TMR_CLK_SRC_24MHOSC

/* aw HPET clock source frequency */
#ifndef AW_HPET_CLK_SRC
    #error "AW_HPET_CLK_SRC is not define!!"
#endif
#if(AW_HPET_CLK_SRC == TMR_CLK_SRC_24MHOSC)
    #define AW_HPET_CLOCK_SOURCE_HZ         (24000000)
#else
    #error "AW_HPET_CLK_SRC config is invalid!!"
#endif


#endif  /* #ifndef __AW_CLOCKSRC_H__ */

