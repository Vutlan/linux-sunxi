/*
 * drivers/media/pa/pa.c
 * (C) Copyright 2010-2016
 * reuuimllatech Technology Co., Ltd. <www.reuuimllatech.com>
 * huangxin <huangxin@reuuimllatech.com>
 *
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/preempt.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/dma.h>
#include <mach/hardware.h>
#include <asm/system.h>
#include <linux/rmap.h>
#include <linux/string.h>
#include <mach/gpio.h>
#include <linux/gpio.h>
#ifdef CONFIG_PM
#include <linux/pm.h>
#endif
#include <mach/sys_config.h>
#include <mach/system.h>

static void __iomem *pa_base;
#define pa_rdreg(reg)	    readl((pa_base+(reg)))
#define pa_wrreg(reg,val)  writel((val),(pa_base+(reg)))
#define SUN6I_MIC_CTRL			   	 (0x28)
#define SUN6I_DAC_ACTL				 (0x20)		//Output Mixer & DAC Analog Control Register
#define LINEOUTL_EN			  	  (19)
#define LINEOUTR_EN			  	  (18)
#define LINEOUTL_SRC_SEL		  (17)
#define	LINEOUTR_SRC_SEL		  (16)
#define	LINEOUT_VOL				  (11)

#define LHPIS					  (8)
#define RHPIS					  (9)
#define LMIXEN					  (28)
#define RMIXEN					  (29)
static bool gpio_pa_count = false;
static struct class *pa_dev_class;
static struct cdev *pa_dev;
static dev_t dev_num ;

static int req_status;
static script_item_u item;
static script_item_value_type_e  type;

typedef enum PA_OPT
{
	PA_OPEN = 200,
	PA_CLOSE,
	PA_DEV_
}__ace_ops_e;

/**
* codec_wrreg_bits - update codec register bits
* @reg: codec register
* @mask: register mask
* @value: new value
*
* Writes new register value.
* Return 1 for change else 0.
*/
int pa_wrreg_bits(unsigned short reg, unsigned int	mask,	unsigned int value)
{
	unsigned int old, new;
		
	old	=	pa_rdreg(reg);
	new	=	(old & ~mask) | value;

	pa_wrreg(reg,new);

	return 0;
}

int pa_wr_control(u32 reg, u32 mask, u32 shift, u32 val)
{
	u32 reg_val;
	reg_val = val << shift;
	mask = mask << shift;
	pa_wrreg_bits(reg, mask, reg_val);
	return 0;
}

static int pa_dev_open(struct inode *inode, struct file *filp) {
    return 0;
}

static int pa_dev_release(struct inode *inode, struct file *filp) {  
 	return 0;
}

static long pa_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{	
	switch (cmd) {	
		case PA_OPEN:
			gpio_pa_count = true;
			item.gpio.data = 1;
			/*config gpio info of audio_pa_ctrl open*/
			if (0 != sw_gpio_setall_range(&item.gpio, 1)) {
				printk("sw_gpio_setall_range failed\n");
			}
			pa_wr_control(SUN6I_MIC_CTRL, 0x1, LINEOUTR_EN, 0x1);
			pa_wr_control(SUN6I_MIC_CTRL, 0x1, LINEOUTL_EN, 0x1);
			pa_wr_control(SUN6I_MIC_CTRL, 0x1, LINEOUTL_SRC_SEL, 0x0);
			pa_wr_control(SUN6I_MIC_CTRL, 0x1, LINEOUTR_SRC_SEL, 0x1);
			pa_wr_control(SUN6I_MIC_CTRL, 0x1f, LINEOUT_VOL, 0x1f);
			break;
		case PA_CLOSE:
			default:
			gpio_pa_count = false;
			item.gpio.data = 0;
			pa_wr_control(SUN6I_MIC_CTRL, 0x1f, LINEOUT_VOL, 0x0);
			pa_wr_control(SUN6I_MIC_CTRL, 0x1, LINEOUTL_EN, 0x0);
			pa_wr_control(SUN6I_MIC_CTRL, 0x1, LINEOUTR_EN, 0x0);
			
			/*config gpio info of audio_pa_ctrl close*/
			if (0 != sw_gpio_setall_range(&item.gpio, 1)) {
				printk("sw_gpio_setall_range failed\n");
			}
			break;
	}

	return 0;
}

static int snd_pa_suspend(struct platform_device *pdev,pm_message_t state)
{
	item.gpio.data = 0;
	pa_wr_control(SUN6I_MIC_CTRL, 0x1, LINEOUTL_EN, 0x0);
	pa_wr_control(SUN6I_MIC_CTRL, 0x1, LINEOUTR_EN, 0x0);
	/*config gpio info of audio_pa_ctrl close*/
	if (0 != sw_gpio_setall_range(&item.gpio, 1)) {
		printk("sw_gpio_setall_range failed\n");
	}
	return 0;
}

static int snd_pa_resume(struct platform_device *pdev)
{
	if (true == gpio_pa_count) {
		item.gpio.data = 1;
		/*config gpio info of audio_pa_ctrl open*/
		if (0 != sw_gpio_setall_range(&item.gpio, 1)) {
			printk("sw_gpio_setall_range failed\n");
		}
	}
	return 0;
}

static struct file_operations pa_dev_fops = {
    .owner 			= THIS_MODULE,
    .unlocked_ioctl = pa_dev_ioctl,
    .open           = pa_dev_open,
    .release        = pa_dev_release,
};

/*data relating*/
static struct platform_device device_pa = {
	.name = "pa",   	   
};

/*method relating*/
static struct platform_driver pa_driver = {
#ifdef CONFIG_PM
	.suspend	= snd_pa_suspend,
	.resume		= snd_pa_resume,
#endif
	.driver		= {
		.name	= "pa",
	},
};

static int __init pa_dev_init(void)
{
    int err = 0;
	printk("[pa_drv] start!!!\n");

	if ((platform_device_register(&device_pa)) < 0) {
		return err;
	}
	if ((err = platform_driver_register(&pa_driver)) < 0) {
		return err;
	}
	pa_base = (void __iomem *)(0xf1c22c00);
    alloc_chrdev_region(&dev_num, 0, 1, "pa_chrdev");
    pa_dev = cdev_alloc();
    cdev_init(pa_dev, &pa_dev_fops);
    pa_dev->owner = THIS_MODULE;
    err = cdev_add(pa_dev, dev_num, 1);
    if (err) {
    	printk(KERN_NOTICE"Error %d adding pa_dev!\n", err); 
        return -1;
    }
    pa_dev_class = class_create(THIS_MODULE, "pa_cls");
    device_create(pa_dev_class, NULL, dev_num, NULL, "pa_dev");

	/*get the default pa val(close)*/
    type = script_get_item("audio_para", "audio_pa_ctrl", &item);
	if (SCIRPT_ITEM_VALUE_TYPE_PIO != type) {
		printk("script_get_item return type err\n");
		return -EFAULT;
	}
	/*request gpio*/
	req_status = gpio_request(item.gpio.gpio, NULL);
	if (0 != req_status) {
		printk("request gpio failed!\n");
	}
	/*config gpio info of audio_pa_ctrl, the default pa config is close(check pa sys_config1.fex).*/
	if (0 != sw_gpio_setall_range(&item.gpio, 1)) {
		printk("sw_gpio_setall_range failed\n");
	}
    printk("[pa_drv] init end!!!\n");
    return 0;
}
module_init(pa_dev_init);

static void __exit pa_dev_exit(void)
{
    device_destroy(pa_dev_class,  dev_num);
    class_destroy(pa_dev_class);
    platform_driver_unregister(&pa_driver);
   	/*while set the pgio value, release gpio handle*/
	if (0 == req_status) {
		gpio_free(item.gpio.gpio);
	}

}
module_exit(pa_dev_exit);

MODULE_AUTHOR("huangxin");
MODULE_DESCRIPTION("User mode encrypt device interface");
MODULE_LICENSE("GPL");
