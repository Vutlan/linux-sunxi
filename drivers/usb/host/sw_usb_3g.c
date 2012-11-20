/*
*************************************************************************************
*                         			      Linux
*					                 USB Host Driver
*
*				        (c) Copyright 2006-2012, SoftWinners Co,Ld.
*							       All Rights Reserved
*
* File Name 	: sw_usb_3g.c
*
* Author 		: javen
*
* Description 	: 3G驱动模板是针对MU509设计的
*
* History 		:
*      <author>    		<time>       	<version >    		<desc>
*       javen     	  2012-4-10            1.0          create this file
*
*************************************************************************************
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/signal.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/pm.h>
#include <linux/earlysuspend.h>
#endif

#include <linux/time.h>
#include <linux/timer.h>

#include <mach/sys_config.h>
#include <mach/gpio.h>
#include <linux/clk.h>

#include  <mach/clock.h>
#include "sw_hci_sun6i.h"
#include "sw_usb_3g.h"

#define  SW_VA_PORTC_IO_BASE    0x01c20800
#define  SW_INT_IRQNO_PIO       28



//-----------------------------------------------------------------------------
//   debug
//-----------------------------------------------------------------------------
#define  USB_3G_DEBUG

#ifdef  USB_3G_DEBUG
#define  usb_3g_dbg(stuff...)		printk(stuff)
#define  usb_3g_err(...) (usb_3g_dbg("err:L%d(%s):", __LINE__, __FILE__), usb_3g_dbg(__VA_ARGS__))
#else
#define  usb_3g_dbg(...)
#define  usb_3g_err(...)
#endif


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

static struct sw_usb_3g g_usb_3g;


/*
*******************************************************************************
*                     usb_3g_get_config
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static s32 usb_3g_get_config(struct sw_usb_3g *usb_3g)
{
    __s32 ret = 0;

    /* 3g_used */
	ret = script_parser_fetch("3g_para",
	                          "3g_used",
	                          (int *)&usb_3g->used,
	                          64);
	if(ret != 0){
		usb_3g_err("ERR: get 3g_used failed\n");
		//return -1;
	}

    /* 3g_usbc_num */
	ret = script_parser_fetch("3g_para",
	                          "3g_usbc_num",
	                          (int *)&usb_3g->usbc_no,
	                          64);
	if(ret != 0){
		usb_3g_err("ERR: get 3g_usbc_num failed\n");
		//return -1;
	}

    /* 3g_usbc_type */
	ret = script_parser_fetch("3g_para",
	                          "3g_usbc_type",
	                          (int *)&usb_3g->usbc_type,
	                          64);
	if(ret != 0){
		usb_3g_err("ERR: get 3g_usbc_type failed\n");
		//return -1;
	}

    /* 3g_uart_num */
	ret = script_parser_fetch("3g_para",
	                          "3g_uart_num",
	                          (int *)&usb_3g->uart_no,
	                          64);
	if(ret != 0){
		usb_3g_err("ERR: get 3g_uart_num failed\n");
		//return -1;
	}

    /* 3g_vbat_gpio */
	ret = script_parser_fetch("3g_para",
	                          "3g_vbat_gpio",
	                          (int *)&usb_3g->vbat_set,
	                          64);
	if(ret != 0){
		usb_3g_err("ERR: get 3g_vbat_gpio failed\n");
		//return -1;
	}

	if(usb_3g->vbat_set.port){
	    usb_3g->vbat_valid = 1;
	}else{
	    usb_3g_err("ERR: 3g_vbat_gpio is invalid\n");
	    usb_3g->vbat_valid = 0;
	}

    /* 3g_power_on_off_gpio */
	ret = script_parser_fetch("3g_para",
	                          "3g_power_on_off_gpio",
	                          (int *)&usb_3g->power_on_off_set,
	                          64);
	if(ret != 0){
		usb_3g_err("ERR: get 3g_power_on_off_gpio failed\n");
		//return -1;
	}

	if(usb_3g->power_on_off_set.port){
	    usb_3g->power_on_off_valid = 1;
	}else{
	    usb_3g_err("ERR: 3g_power_on_off_gpio is invalid\n");
	    usb_3g->power_on_off_valid  = 0;
	}

    /* 3g_reset_gpio */
	ret = script_parser_fetch("3g_para",
	                          "3g_reset_gpio",
	                          (int *)&usb_3g->reset_set,
	                          64);
	if(ret != 0){
		usb_3g_err("ERR: get 3g_reset_gpio failed\n");
		//return -1;
	}

	if(usb_3g->reset_set.port){
	    usb_3g->reset_valid = 1;
	}else{
	    usb_3g_err("ERR: 3g_reset_gpio is invalid\n");
	    usb_3g->reset_valid  = 0;
	}

    /* 3g_wakeup_in_gpio */
	ret = script_parser_fetch("3g_para",
	                          "3g_wakeup_in_gpio",
	                          (int *)&usb_3g->wakeup_in_set,
	                          64);
	if(ret != 0){
		usb_3g_err("ERR: get 3g_wakeup_in_gpio failed\n");
		//return -1;
	}

	if(usb_3g->wakeup_in_set.port){
	    usb_3g->wakeup_in_valid = 1;
	}else{
	    usb_3g_err("ERR: 3g_wakeup_in_gpio is invalid\n");
	    usb_3g->wakeup_in_valid  = 0;
	}

    return 0;
}

/*
*******************************************************************************
*                     usb_3g_pin_init
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static s32 usb_3g_pin_init(struct sw_usb_3g *usb_3g)
{

    //---------------------------------
    //  3g vbat
    //---------------------------------
    if(usb_3g->vbat_valid){
		usb_3g->vbat_hd = sw_gpio_request(&(usb_3g->vbat_set), 1);
		if(usb_3g->vbat_hd == 0){
			usb_3g_err("ERR: gpio_request vbat_set failed\n");
			return 0;
		}

		/* set config, ouput */
		sw_gpio_set_one_pin_io_status(usb_3g->vbat_hd, 1, NULL);

		/* reserved is pull down */
		sw_gpio_set_one_pin_pull(usb_3g->vbat_hd, 2, NULL);
    }else{
        usb_3g->vbat_hd = 0;
    }

    //---------------------------------
    //  3g power_on_off
    //---------------------------------
    if(usb_3g->power_on_off_valid){
		usb_3g->power_on_off_hd = sw_gpio_request(&(usb_3g->power_on_off_set), 1);
		if(usb_3g->power_on_off_hd == 0){
			usb_3g_err("ERR: sw_gpio_request power_on_off_set failed\n");
			return 0;
		}

		/* set config, ouput */
		sw_gpio_set_one_pin_io_status(usb_3g->power_on_off_hd, 1, NULL);

		/* reserved is pull up */
		sw_gpio_set_one_pin_pull(usb_3g->power_on_off_hd, 1, NULL);
    }else{
        usb_3g->power_on_off_hd = 0;
    }

    //---------------------------------
    //  3g reset
    //---------------------------------
    if(usb_3g->reset_valid){
		usb_3g->reset_hd = sw_gpio_request(&(usb_3g->reset_set), 1);
		if(usb_3g->reset_hd == 0){
			usb_3g_err("ERR: sw_gpio_request reset_set failed\n");
			return 0;
		}

		/* set config, ouput */
		sw_gpio_set_one_pin_io_status(usb_3g->reset_hd, 1, NULL);

		/* reserved is pull down */
		sw_gpio_set_one_pin_pull(usb_3g->reset_hd, 2, NULL);
    }else{
        usb_3g->reset_hd = 0;
    }

    //---------------------------------
    //  3g wakeup_out
    //---------------------------------
    if(usb_3g->wakeup_in_valid){
		usb_3g->wakeup_in_hd = sw_gpio_request(&(usb_3g->wakeup_in_set), 1);
		if(usb_3g->wakeup_in_hd == 0){
			usb_3g_err("ERR: sw_gpio_request wakeup_in_set failed\n");
			return 0;
		}

		/* set config, ouput */
		sw_gpio_set_one_pin_io_status(usb_3g->wakeup_in_hd, 1, NULL);

		/* reserved is pull down */
		sw_gpio_set_one_pin_pull(usb_3g->wakeup_in_hd, 2, NULL);
    }else{
        usb_3g->wakeup_in_hd = 0;
    }

    return 0;
}

/*
*******************************************************************************
*                     usb_3g_pin_exit
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static s32 usb_3g_pin_exit(struct sw_usb_3g *usb_3g)
{

    if(usb_3g->vbat_hd){
        sw_gpio_release(usb_3g->vbat_hd, 0);
        usb_3g->vbat_hd = 0;
    }

    if(usb_3g->power_on_off_hd){
        sw_gpio_release(usb_3g->power_on_off_hd, 0);
        usb_3g->power_on_off_hd = 0;
    }

    if(usb_3g->reset_hd){
        sw_gpio_release(usb_3g->reset_hd, 0);
        usb_3g->reset_hd = 0;
    }

    if(usb_3g->wakeup_in_hd){
        sw_gpio_release(usb_3g->wakeup_in_hd, 0);
        usb_3g->wakeup_in_hd = 0;
    }

    return 0;
}

#ifndef CONFIG_USB_3G_SLEEP_BY_USB_WAKEUP_BY_USB

/*
*******************************************************************************
*                     usb_3g_wakeup_irq_enable
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static void usb_3g_wakeup_irq_enable(void)
{
    __u32 gpio_base = SW_VA_PORTC_IO_BASE;
    __u32 reg_val = 0;
	unsigned long flags = 0;
    struct sw_usb_3g *usb_3g = &g_usb_3g;

    /* interrupt enable */
	spin_lock_irqsave(&usb_3g->lock, flags);
    reg_val = USBC_Readl(gpio_base + 0x210);
    reg_val |= (1 << 2);
    USBC_Writel(reg_val, (gpio_base + 0x210));
	spin_unlock_irqrestore(&usb_3g->lock, flags);

    return;
}

/*
*******************************************************************************
*                     usb_3g_wakeup_irq_disable
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static void usb_3g_wakeup_irq_disable(void)
{
    __u32 gpio_base = SW_VA_PORTC_IO_BASE;
    __u32 reg_val = 0;
	unsigned long flags = 0;
    struct sw_usb_3g *usb_3g = &g_usb_3g;

    /* interrupt disable */
	spin_lock_irqsave(&usb_3g->lock, flags);
    reg_val = USBC_Readl(gpio_base + 0x210);
    reg_val &= ~(1 << 2);
    USBC_Writel(reg_val, (gpio_base + 0x210));
	spin_unlock_irqrestore(&usb_3g->lock, flags);

    return;
}

/*
*******************************************************************************
*                     usb_3g_wakeup_irq_clear
*
* Description:
*    Only used to provide driver mode change events
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static void usb_3g_wakeup_irq_clear(void)
{
    __u32 gpio_base = SW_VA_PORTC_IO_BASE;
    __u32 reg_val = 0;
	unsigned long flags = 0;
    struct sw_usb_3g *usb_3g = &g_usb_3g;

    /* clear interrupt pending */
	spin_lock_irqsave(&usb_3g->lock, flags);
    reg_val = USBC_Readl(gpio_base + 0x214);
    reg_val |= (1 << 2);
    USBC_Writel(reg_val, (gpio_base + 0x214));
	spin_unlock_irqrestore(&usb_3g->lock, flags);

    return;
}

/*
*******************************************************************************
*                     usb_3g_wakeup_irq_config
*
* Description:
*    Only used to provide driver mode change events
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static int usb_3g_wakeup_irq_config(void)
{
    __u32 gpio_base = SW_VA_PORTC_IO_BASE;
    __u32 reg_val = 0;
	unsigned long flags = 0;
    struct sw_usb_3g *usb_3g = &g_usb_3g;

	spin_lock_irqsave(&usb_3g->lock, flags);

    /* pull up, PH2 */
    reg_val = USBC_Readl(gpio_base + 0x118);
    reg_val &= ~(0x03 << 4);
    reg_val |= (0x01 << 4);      //EINT2
    USBC_Writel(reg_val, (gpio_base + 0x118));

    /* set port configure register, PH2 */
    reg_val = USBC_Readl(gpio_base + 0xFC);
    reg_val &= ~(0x07 << 8);
    reg_val |= (0x6 << 8);      //EINT2
    USBC_Writel(reg_val, (gpio_base + 0xFC));

    /* PIO interrupt configure register */
    reg_val = USBC_Readl(gpio_base + 0x200);
    reg_val &= ~(0x07 << 8);
    reg_val |= (0x1 << 8);
    USBC_Writel(reg_val, (gpio_base + 0x200));

	spin_unlock_irqrestore(&usb_3g->lock, flags);

    return 0;
}

/*
*******************************************************************************
*                     usb_3g_wakeup_irq_config_clear
*
* Description:
*    Only used to provide driver mode change events
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static int usb_3g_wakeup_irq_config_clear(void)
{
    __u32 gpio_base = SW_VA_PORTC_IO_BASE;
    __u32 reg_val = 0;
	unsigned long flags = 0;
    struct sw_usb_3g *usb_3g = &g_usb_3g;

	spin_lock_irqsave(&usb_3g->lock, flags);

    /* set port configure register, PH2 */
    reg_val = USBC_Readl(gpio_base + 0xFC);
    reg_val &= ~(0x07 << 8);
    USBC_Writel(reg_val, (gpio_base + 0xFC));

    /* PIO interrupt configure register */
    reg_val = USBC_Readl(gpio_base + 0x200);
    reg_val &= ~(0x07 << 8);
    USBC_Writel(reg_val, (gpio_base + 0x200));

	spin_unlock_irqrestore(&usb_3g->lock, flags);

    return 0;
}

extern void axp_pressshort_ex(void);

/*
*******************************************************************************
*                     usb_3g_wakeup_irq_work
*
* Description:
*    Only used to provide driver mode change events
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static void usb_3g_wakeup_irq_work(struct work_struct *data)
{
    usb_3g_dbg("---------usb_3g_wakeup_irq_work----------\n");

    /* 通知android，唤醒系统 */
	//axp_pressshort_ex();

	return;
}

/*
*******************************************************************************
*                     is_usb_3g_wakeup_irq_pending
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static u32 is_usb_3g_wakeup_irq_pending(void)
{
    __u32 gpio_base = SW_VA_PORTC_IO_BASE;
    __u32 reg_val = 0;
    __u32 result = 0;

    /* interrupt pending */
    reg_val = USBC_Readl(gpio_base + 0x214);
    result = (reg_val & (1 << 2)) ? 1 : 0;

	return result;
}

/*
*******************************************************************************
*                     usb_3g_wakeup_irq_clear_pending
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static void usb_3g_wakeup_irq_clear_pending(void)
{
    __u32 gpio_base = SW_VA_PORTC_IO_BASE;
    __u32 reg_val = 0;
	unsigned long flags = 0;
    struct sw_usb_3g *usb_3g = &g_usb_3g;

    /* clear interrupt pending */
    spin_lock_irqsave(&usb_3g->lock, flags);
    reg_val = USBC_Readl(gpio_base + 0x214);
    reg_val |= (1 << 2);
    USBC_Writel(reg_val, (gpio_base + 0x214));
    spin_unlock_irqrestore(&usb_3g->lock, flags);

    return ;
}

/*
*******************************************************************************
*                     is_usb_3g_wakeup_irq_enable
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static u32 is_usb_3g_wakeup_irq_enable(void)
{
    __u32 gpio_base = SW_VA_PORTC_IO_BASE;
    __u32 reg_val = 0;
    __u32 result = 0;

    /* interrupt pending */
    reg_val = USBC_Readl(gpio_base + 0x210);
    result = (reg_val & (1 << 2)) ? 1 : 0;

    return result;
}

/*
*******************************************************************************
*                     usb_3g_wakeup_irq_interrupt
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static irqreturn_t usb_3g_wakeup_irq_interrupt(int irq, void *__hci)
{
    __u32 result = 0;

    if(!is_usb_3g_wakeup_irq_pending()){
        return IRQ_NONE;
    }

	if(is_usb_3g_wakeup_irq_enable()){
	    result = 1;
	}

    usb_3g_wakeup_irq_disable();
    usb_3g_wakeup_irq_config_clear();
    usb_3g_wakeup_irq_clear_pending();

    if(result){
        schedule_work(&g_usb_3g.irq_work);
    }

	return IRQ_HANDLED;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void usb_3g_early_suspend(struct early_suspend *h)
{
    usb_3g_wakeup_irq_config();
    usb_3g_wakeup_irq_clear();
    usb_3g_wakeup_irq_enable();
}

static void usb_3g_early_resume(struct early_suspend *h)
{
    usb_3g_wakeup_irq_disable();
    usb_3g_wakeup_irq_config_clear();
    usb_3g_wakeup_irq_clear();
}
#endif

/*
*******************************************************************************
*                     usb_3g_wakeup_irq_init
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
int usb_3g_wakeup_irq_init(void)
{
	struct sw_usb_3g *usb_3g = &g_usb_3g;
    int ret = 0;
    int nIrq = SW_INT_IRQNO_PIO;

    ret = request_irq(nIrq, usb_3g_wakeup_irq_interrupt, IRQF_TRIGGER_FALLING | IRQF_SHARED, "usb_3g", usb_3g);
    if(ret != 0){
        usb_3g_err("request_irq failed, ret=%d\n", ret);
        goto failed;
    }

	/* Init IRQ workqueue before request_irq */
	INIT_WORK(&usb_3g->irq_work, usb_3g_wakeup_irq_work);

    usb_3g_wakeup_irq_config();

#ifdef CONFIG_HAS_EARLYSUSPEND
    usb_3g->early_suspend.suspend = usb_3g_early_suspend;
    usb_3g->early_suspend.resume = usb_3g_early_resume;
	register_early_suspend(&usb_3g->early_suspend);
#endif

    //enable_irq(nIrq);

    return 0;

failed:
    return -1;
}

/*
*******************************************************************************
*                     usb_3g_wakeup_irq_exit
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
int usb_3g_wakeup_irq_exit(void)
{
	struct sw_usb_3g *usb_3g = &g_usb_3g;

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&usb_3g->early_suspend);
#endif

	usb_3g_wakeup_irq_disable();

    usb_3g_wakeup_irq_config_clear();

    usb_3g_wakeup_irq_clear();

    //free_irq(SW_INT_IRQNO_PIO, usb_3g_wakeup_irq_interrupt);

    return 0;
}
#else
int usb_3g_wakeup_irq_init(void)
{
    return 0;
}

/*
*******************************************************************************
*                     usb_3g_wakeup_irq_exit
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
int usb_3g_wakeup_irq_exit(void)
{
    return 0;
}

#endif

EXPORT_SYMBOL(usb_3g_wakeup_irq_init);
EXPORT_SYMBOL(usb_3g_wakeup_irq_exit);

/*
*******************************************************************************
*                     usb_3g_vbat
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*
*******************************************************************************
*/
void usb_3g_vbat(struct sw_usb_3g *usb_3g, u32 on)
{
    if(!usb_3g->vbat_hd){
        //usb_3g_err("err: drv_vbus_ext_Handle == NULL\n");
        return;
    }

    if(on){
        usb_3g_dbg("Set USB usb_3g vBat on\n");
    }else{
        usb_3g_dbg("Set USB usb_3g vBat off\n");
    }

    sw_gpio_write_one_pin_value(usb_3g->vbat_hd, on, NULL);

    return;
}
EXPORT_SYMBOL(usb_3g_vbat);

/*
*******************************************************************************
*                     usb_3g_reset
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void usb_3g_reset(u32 usbc_no, u32 time)
{
    struct sw_usb_3g *usb_3g = &g_usb_3g;

    if(!usb_3g->reset_hd){
        //usb_3g_err("err: drv_vbus_ext_Handle == NULL\n");
        return;
    }

    sw_gpio_write_one_pin_value(usb_3g->reset_hd, 1, NULL);
    mdelay(time);
    sw_gpio_write_one_pin_value(usb_3g->reset_hd, 0, NULL);

    return;
}
EXPORT_SYMBOL(usb_3g_reset);

/*
*******************************************************************************
*                     usb_3g_wakeup_sleep
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
#ifdef CONFIG_USB_3G_SLEEP_BY_GPIO_WAKEUP_BY_GPIO
void usb_3g_wakeup_sleep(u32 usbc_no, u32 sleep)
{
    struct sw_usb_3g *usb_3g = &g_usb_3g;

    if(!usb_3g->wakeup_in_hd){
        //usb_3g_err("err: drv_vbus_ext_Handle == NULL\n");
        return;
    }

    if(sleep){
        usb_3g_dbg("sleep usb_3g\n");
    }else{
        usb_3g_dbg("wakeup usb_3g\n");
    }

    sw_gpio_write_one_pin_value(usb_3g->wakeup_in_hd, sleep, NULL);

    return;
}
#else
void usb_3g_wakeup_sleep(u32 usbc_no, u32 sleep)
{
    return;
}
#endif
EXPORT_SYMBOL(usb_3g_wakeup_sleep);

/*
*******************************************************************************
*                     usb_3g_power_on_off
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static void usb_3g_power_on_off(struct sw_usb_3g *usb_3g, u32 is_on)
{
    u32 on_off = 0;

    usb_3g_dbg("Set usb_3g Power %s\n", (is_on ? "ON" : "OFF"));

    /* set power */
    if(usb_3g->power_on_off_set.data == 0){
        on_off = is_on ? 1 : 0;
    }else{
        on_off = is_on ? 0 : 1;
    }

    sw_gpio_write_one_pin_value(usb_3g->power_on_off_hd, on_off, NULL);

    return;
}

/*
*******************************************************************************
*                     usb_3g_power
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*
* 1、vbat通过GPIO操作时，vbat关掉后，整个3G模组都掉电了，因此 power_on_off 可以省去
*    power on:
*		1、vbat pin pull up
*		2、delay 4000ms
*		3、power_on_off pin pull down
*		4、delay 700ms
*		5、power_on_off pin pull up
*
*	  power off:
*		1、vbat pin pull down
*
*
*******************************************************************************
*/
void usb_3g_power(u32 usbc_no, u32 on)
{
    struct sw_usb_3g *usb_3g = &g_usb_3g;

    usb_3g_dbg("usb_3g power %s with gpio\n", (on ? "ON" : "OFF"));

    if(on){
		usb_3g_vbat(usb_3g, 1);
		mdelay(4000);
        usb_3g_power_on_off(usb_3g, 0);
        mdelay(700);
        usb_3g_power_on_off(usb_3g, 1);
    }else{
		usb_3g_vbat(usb_3g, 0);
    }

    return;
}
EXPORT_SYMBOL(usb_3g_power);

/*
*******************************************************************************
*                     is_suspport_usb_3g
*
* Description:
*    void
*
* Parameters:
*    usbc_no   : input. 控制器编号, usb0,usb1,usb2
*    usbc_type : input. 控制器类型, 0: unkown, 1: ehci, 2: ohci
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
u32 is_suspport_usb_3g(u32 usbc_no, u32 usbc_type)
{
    if(!g_usb_3g.used){
        //usb_3g_err("err: not support, (%d, %d)\n", g_usb_3g.usbc_no, usbc_no);
        return 0;
    }

    if(g_usb_3g.usbc_no != usbc_no){
        //usb_3g_err("err: not support, (%d, %d)\n", g_usb_3g.usbc_no, usbc_no);
        return 0;
    }

    if(g_usb_3g.usbc_type != usbc_type){
        //usb_3g_err("err: not support, (%d, %d)\n", g_usb_3g.usbc_type, usbc_type);
        return 0;
    }

    return 1;
}
EXPORT_SYMBOL(is_suspport_usb_3g);

/*
*******************************************************************************
*                     usb_3g_init
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
int usb_3g_init(void)
{
    int ret = 0;

    memset(&g_usb_3g, 0, sizeof(struct sw_usb_3g));
	spin_lock_init(&g_usb_3g.lock);

    ret = usb_3g_get_config(&g_usb_3g);
    if(ret != 0){
        usb_3g_err("err: usb_3g_get_config failed\n");
        goto failed0;
    }

    ret =  usb_3g_pin_init(&g_usb_3g);
    if(ret != 0){
       usb_3g_err("err: usb_3g_pin_init failed\n");
       goto failed1;
    }

    return 0;

failed1:
failed0:

    return -1;
}
EXPORT_SYMBOL(usb_3g_init);

/*
*******************************************************************************
*                     usb_3g_exit
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
int usb_3g_exit(void)
{
    usb_3g_pin_exit(&g_usb_3g);

    return 0;
}
EXPORT_SYMBOL(usb_3g_exit);




