/*
 * SoftWinners 3G module core Linux support
 *
 * Copyright (C) 2012 SoftWinners Incorporated
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/kmemcheck.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/idr.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/signal.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/pm.h>
#include <linux/earlysuspend.h>
#endif

#include <linux/time.h>
#include <linux/timer.h>
#include <linux/input.h>
#include <linux/ioport.h>
#include <linux/io.h>

#include <mach/platform.h>
#include <mach/sys_config.h>
#include <linux/clk.h>

#include "sw_module.h"


#define PIO_INT_CFG0_OFFSET 	(0x200)
#define PIO_INT_CFG1_OFFSET 	(0x204)
#define PIO_INT_CFG2_OFFSET 	(0x208)
#define PIO_INT_CFG3_OFFSET 	(0x20c)

#define PIO_INT_STAT_OFFSET          (0x214)
#define PIO_INT_CTRL_OFFSET          (0x210)

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void sw_module_delay(u32 time)
{
    spinlock_t lock;
	unsigned long flags = 0;

	spin_lock_init(&lock);
	spin_lock_irqsave(&lock, flags);
	mdelay(time);
	spin_unlock_irqrestore(&lock, flags);
}
EXPORT_SYMBOL(sw_module_delay);

s32 modem_get_config(struct sw_modem *modem)
{
    int ret = 0;
    int int_temp=0;
    char str_temp[64];
    script_parser_value_type_t type = SCIRPT_PARSER_VALUE_TYPE_STRING;
    /* 3g_used */
    ret = script_parser_fetch("2g_para", "2g_used", &int_temp,1);
    if(ret == SCRIPT_PARSER_OK){
        modem->used =int_temp;
    }else{
        modem_err("ERR: get 2g_used failed\n");
        modem->used = 0;
    }

    /* 3g_usbc_num */
    ret= script_parser_fetch("2g_para", "2g_usbc_num", &int_temp,1);
    if(ret == SCRIPT_PARSER_OK){
        modem->usbc_no =int_temp;
    }else{
        modem_err("ERR: get 2g_usbc_num failed\n");
        modem->usbc_no = 0;
    }

    /* 3g_uart_num */
    ret = script_parser_fetch("2g_para", "2g_uart_num", &int_temp,1);
    if(ret == SCRIPT_PARSER_OK){
        modem->uart_no = int_temp;
    }else{
        modem_err("ERR: get 2g_uart_num failed\n");
        modem->uart_no = 0;
    }

    /* bb_name */
    ret = script_parser_fetch_ex("2g_para", "bb_name", (int*)(&str_temp),&type,sizeof(str_temp)/sizeof(int));
    if(ret == SCRIPT_PARSER_OK){
        strcpy(modem->name, str_temp);
        modem_dbg("%s modem support\n", modem->name);
    }else{
        modem_err("ERR: get bb_name failed\n");
    }

    /* bb_vbat */
    modem->bb_vbat.pio= gpio_request_ex("2g_para", "bb_vbat");
    if(modem->bb_vbat.pio){
        modem->bb_vbat.valid = 1;
    }else{
        modem_err("ERR: get bb_vbat failed\n");
        modem->bb_vbat.valid = 0;
    }

    /* bb_pwr_on */
    modem->bb_pwr_on.pio = gpio_request_ex("2g_para", "bb_pwr_on");
    if(modem->bb_pwr_on.pio){
        modem->bb_pwr_on.valid = 1;
    }else{
        modem_err("ERR: get bb_pwr_on failed\n");
        modem->bb_pwr_on.valid  = 0;
    }

    /* bb_rst */
    modem->bb_rst.pio = gpio_request_ex("2g_para", "bb_rst");
    if(modem->bb_rst.pio){
        modem->bb_rst.valid = 1;
    }else{
        modem_err("ERR: get bb_rst failed\n");
        modem->bb_rst.valid  = 0;
    }
  
    /* bb_rf_dis */
    modem->bb_rf_dis.pio = gpio_request_ex("2g_para", "bb_rf_dis");
    if(modem->bb_rf_dis.pio){
        modem->bb_rf_dis.valid = 1;
    }else{
        modem_err("ERR: get bb_rf_dis failed\n");
        modem->bb_rf_dis.valid  = 0;
    }

    /* bb_host_wake */
    modem->bb_host_wake.pio=gpio_request_ex("2g_para", "bb_host_wake");
    if(modem->bb_host_wake.pio){
        modem->bb_host_wake.valid = 1;
    }else{
        modem_err("ERR: get bb_host_wake failed\n");
        modem->bb_host_wake.valid  = 0;
    }

    /* bb_wake */
    modem->bb_wake.pio= gpio_request_ex("2g_para", "bb_wake");
    if(modem->bb_wake.pio){
        modem->bb_wake.valid = 1;
    }else{
        modem_err("ERR: get bb_wake failed\n");
        modem->bb_wake.valid  = 0;
    }

    
    return 0;
}
EXPORT_SYMBOL(modem_get_config);

s32 modem_pin_exit(struct sw_modem *modem)
{
    //---------------------------------
    //  bb_vbat
    //---------------------------------
    if(modem->bb_vbat.valid){
        gpio_release(modem->bb_vbat.pio,1);
        modem->bb_vbat.valid = 0;
    }

    //---------------------------------
    //  bb_pwr_on
    //---------------------------------
    if(modem->bb_pwr_on.valid){
        gpio_release(modem->bb_pwr_on.pio,1);
        modem->bb_pwr_on.valid = 0;
    }

    //---------------------------------
    //  bb_rst
    //---------------------------------
    if(modem->bb_rst.valid){
        gpio_release(modem->bb_rst.pio,1);
        modem->bb_rst.valid = 0;
    }

    //---------------------------------
    //  bb_wake
    //---------------------------------
    if(modem->bb_wake.valid){
        gpio_release(modem->bb_wake.pio,1);
        modem->bb_wake.valid = 0;
    }

    //---------------------------------
    //  bb_rf_dis
    //---------------------------------
    if(modem->bb_rf_dis.valid){
        gpio_release(modem->bb_rf_dis.pio,1);
        modem->bb_rf_dis.valid = 0;
    }

    return 0;
}
EXPORT_SYMBOL(modem_pin_exit);

void modem_vbat(struct sw_modem *modem, u32 value)
{

    if(modem->bb_vbat.valid)
        if(0!=gpio_write_one_pin_value(modem->bb_vbat.pio, value,"bb_vbat"))
        {
            modem_err("write vbat pin error\n");
        }

    return;
}
EXPORT_SYMBOL(modem_vbat);

/* modem reset delay is 100 */
void modem_reset(struct sw_modem *modem, u32 value)
{

    if(modem->bb_rst.valid)
    {   
        if(0!=gpio_write_one_pin_value(modem->bb_rst.pio, value,"bb_rst"))
        {
            modem_err("write reset pin error\n");
        }

    }
    return;
}
EXPORT_SYMBOL(modem_reset);

void modem_sleep(struct sw_modem *modem, u32 value)
{
   
    if(modem->bb_wake.valid)
    {   
        if(0!=gpio_write_one_pin_value(modem->bb_wake.pio,value,"bb_wake"))
        {
            modem_err("write wake pin error\n");
        }

    }   
    return;
}
EXPORT_SYMBOL(modem_sleep);

void modem_power_on_off(struct sw_modem *modem, u32 value)
{
    if(modem->bb_pwr_on.valid)
    {
        if(0!=gpio_write_one_pin_value(modem->bb_pwr_on.pio, value,"bb_pwr_on"))
        {
            modem_err("write pwr_on pin error\n");  
        }
    }
    return;
}
EXPORT_SYMBOL(modem_power_on_off);

void modem_rf_disable(struct sw_modem *modem, u32 value)
{

    if(modem->bb_rf_dis.valid)
    {
        if(0!=gpio_write_one_pin_value(modem->bb_rf_dis.pio, value,"bb_rf_dis"))
        {
           modem_err("write rf_dis pin error\n");   
        }
    }
    return;
}
EXPORT_SYMBOL(modem_rf_disable);

static int modem_create_input_device(struct sw_modem *modem)
{
    int ret = 0;

    modem->key = input_allocate_device();
    if (!modem->key) {
        modem_err("err: not enough memory for input device\n");
        return -ENOMEM;
    }

    modem->key->name          = "sw_2g_modem";
    modem->key->phys          = "modem/input0";
    modem->key->id.bustype    = BUS_HOST;
    modem->key->id.vendor     = 0xf001;
    modem->key->id.product    = 0xffff;
    modem->key->id.version    = 0x0100;

    modem->key->evbit[0] = BIT_MASK(EV_KEY);
    set_bit(KEY_POWER, modem->key->keybit);

    ret = input_register_device(modem->key);
    if (ret) {
        modem_err("err: input_register_device failed\n");
        input_free_device(modem->key);
        return -ENOMEM;
    }

    return 0;
}

static int modem_free_input_device(struct sw_modem *modem)
{
    if(modem->key){
        input_unregister_device(modem->key);
        input_free_device(modem->key);
    }

    return 0;
}

/* 通知android，唤醒系统 */
static void modem_wakeup_system(struct sw_modem *modem)
{
    modem_dbg("---------%s modem wakeup system----------\n", modem->name);

    input_report_key(modem->key, KEY_POWER, 1);
    input_sync(modem->key);
    msleep(100);
    input_report_key(modem->key, KEY_POWER, 0);
    input_sync(modem->key);

    return ;
}


static int	int_cfg_addr[]={PIO_INT_CFG0_OFFSET,PIO_INT_CFG1_OFFSET,
			PIO_INT_CFG2_OFFSET, PIO_INT_CFG3_OFFSET};

static int modem_irq_config(struct sw_modem *modem)
{
    __u32 reg_num = 0;
	__u32 reg_addr = 0;
	__u32 reg_val = 0;
    user_gpio_set_t gpio_int_info[1];
    gpio_get_one_pin_status(modem->bb_host_wake.pio,gpio_int_info,"bb_host_wake",1);
    modem_dbg("%s, %d: gpio_int_info, port = %d, port_num = %d. \n", __func__, __LINE__, \
		gpio_int_info[0].port, gpio_int_info[0].port_num);

    //set nagetive trige
    reg_num = (gpio_int_info[0].port_num)%8;
	reg_addr = (gpio_int_info[0].port_num)/8;
	reg_val = readl(SW_VA_PORTC_IO_BASE + int_cfg_addr[reg_addr]);
	reg_val &= (~(7 << (reg_num * 4)));
	reg_val |= (1 << (reg_num * 4));//nagtive trige
	writel(reg_val,SW_VA_PORTC_IO_BASE+int_cfg_addr[reg_addr]);
                                                               
		
	//clear the interrupt pending
	reg_val = readl(SW_VA_PORTC_IO_BASE + PIO_INT_STAT_OFFSET);
	if((reg_val = (reg_val&(reg_num<<(0))))){//PG0 eint0
		modem_dbg("==MODEM_IRQ_PENDING==\n");              
		writel(reg_val,SW_VA_PORTC_IO_BASE + PIO_INT_STAT_OFFSET);
	}

     //disable the gpio irq
	reg_val = readl(SW_VA_PORTC_IO_BASE+PIO_INT_CTRL_OFFSET); 
	reg_val &= ~(1 << (gpio_int_info[0].port_num));
	writel(reg_val,SW_VA_PORTC_IO_BASE+PIO_INT_CTRL_OFFSET);
    

    return 0;
}

static int modem_irq_config_clear(struct sw_modem *modem)
{
    
    __u32 reg_num = 0;
	__u32 reg_addr = 0;
	__u32 reg_val = 0;
    user_gpio_set_t gpio_int_info[1];
    gpio_get_one_pin_status(modem->bb_host_wake.pio,gpio_int_info,"bb_host_wake",1);
    modem_dbg("%s, %d: gpio_int_info, port = %d, port_num = %d. \n", __func__, __LINE__, \
		gpio_int_info->port, gpio_int_info->port_num);

    //set nagetive trige
    reg_num = (gpio_int_info[0].port_num)%8;
	reg_addr = (gpio_int_info[0].port_num)/8;
	reg_val = readl(SW_VA_PORTC_IO_BASE + int_cfg_addr[reg_addr]);
	reg_val &= (~(7 << (reg_num * 4)));
	reg_val |= (1 << (reg_num * 4));//nagtive trige
	writel(reg_val,SW_VA_PORTC_IO_BASE+int_cfg_addr[reg_addr]);
                                                               
		
	//clear the interrupt pending
	reg_val = readl(SW_VA_PORTC_IO_BASE + PIO_INT_STAT_OFFSET);
	if((reg_val = (reg_val&(reg_num<<(0))))){//PG0 eint0
		modem_dbg("==MODEM_IRQ_NO=\n");              
		writel(reg_val,SW_VA_PORTC_IO_BASE + PIO_INT_STAT_OFFSET);
	}

     //disable the gpio irq
	reg_val = readl(SW_VA_PORTC_IO_BASE+PIO_INT_CTRL_OFFSET); 
	reg_val &= ~(1 << (gpio_int_info[0].port_num));
	writel(reg_val,SW_VA_PORTC_IO_BASE+PIO_INT_CTRL_OFFSET);


    return 0;
}

static void modem_irq_enable(struct sw_modem *modem)
{
    user_gpio_set_t gpio_int_info[1];
	__u32 reg_val = 0;
    
    gpio_get_one_pin_status(modem->bb_host_wake.pio,gpio_int_info,"bb_host_wake",1);
    
     //enable the gpio irq
	reg_val = readl(SW_VA_PORTC_IO_BASE+PIO_INT_CTRL_OFFSET); 
	reg_val |=(1 << (gpio_int_info[0].port_num));
	writel(reg_val,SW_VA_PORTC_IO_BASE+PIO_INT_CTRL_OFFSET);

    return;
}

static void modem_irq_disable(struct sw_modem *modem)
{
    user_gpio_set_t gpio_int_info[1];
	__u32 reg_val = 0;
    
    gpio_get_one_pin_status(modem->bb_host_wake.pio,gpio_int_info,"bb_host_wake",1);
    
     //disable the gpio irq
	reg_val = readl(SW_VA_PORTC_IO_BASE+PIO_INT_CTRL_OFFSET); 
	reg_val &= ~(1 << (gpio_int_info[0].port_num));
	writel(reg_val,SW_VA_PORTC_IO_BASE+PIO_INT_CTRL_OFFSET);

    return;
}

static u32 modem_irq_is_enable(struct sw_modem *modem)
{

    __u32 result = 0;

    user_gpio_set_t gpio_int_info[1];
	__u32 reg_val = 0;
    
    gpio_get_one_pin_status(modem->bb_host_wake.pio,gpio_int_info,"bb_host_wake",1);
    
	reg_val = readl(SW_VA_PORTC_IO_BASE+PIO_INT_CTRL_OFFSET); 
    reg_val &=(1 << (gpio_int_info[0].port_num));
    if(reg_val)
    {
        result=1;
    }else
    {
        result=0;
    }


    return result;
}

static u32 modem_irq_is_pending(struct sw_modem *modem)
{

    __u32 result = 0;

    user_gpio_set_t gpio_int_info[1];
	__u32 reg_val = 0;
    
    gpio_get_one_pin_status(modem->bb_host_wake.pio,gpio_int_info,"bb_host_wake",1);
    
	reg_val = readl(SW_VA_PORTC_IO_BASE+PIO_INT_STAT_OFFSET); 
    reg_val &=(1 << (gpio_int_info[0].port_num));
    
    if(reg_val)
    {
        result=1;
    }else
    {
        result=0;
    }
    return result;
}

static void modem_irq_clear_pending(struct sw_modem *modem)
{

     user_gpio_set_t gpio_int_info[1];
	__u32 reg_val = 0;
    
    gpio_get_one_pin_status(modem->bb_host_wake.pio,gpio_int_info,"bb_host_wake",1);
    
     //clear the gpio pending
	reg_val = readl(SW_VA_PORTC_IO_BASE+PIO_INT_STAT_OFFSET); 
    reg_val |=(1 << (gpio_int_info[0].port_num));
    

}

static void modem_irq_work(struct work_struct *data)
{
	struct sw_modem *modem = container_of(data, struct sw_modem, irq_work);

	modem_wakeup_system(modem);

	return;
}

static irqreturn_t modem_irq_interrupt(int irq, void *dev)
{
	struct sw_modem *modem = (struct sw_modem *)dev;
  
	if(modem_irq_is_pending(modem)){
       modem_dbg(" modem_irq_is_pending ....\n");
	   modem_irq_disable(modem);
       modem_irq_clear_pending(modem);
       schedule_work(&modem->irq_work);
	}

	return 0;
}

int modem_irq_init(struct sw_modem *modem)
{
    int ret = 0;

    ret = modem_create_input_device(modem);
    if(ret != 0){
        modem_err("err: modem_create_input_device failed\n");
        return -1;
    }

	INIT_WORK(&modem->irq_work, modem_irq_work);

    modem->irq_hd =request_irq(SW_INT_IRQNO_PIO,
                                        modem_irq_interrupt,
                                        IRQF_TRIGGER_FALLING | IRQF_SHARED,
                                        SW_DEVICE_NODE_NAME,
                                        modem);
    if(modem->irq_hd < 0){
        modem_err("err: request_irq failed\n");
        modem_free_input_device(modem);
        return -1;
    }
   
    modem_irq_disable(modem);
    modem_irq_config_clear(modem);
    modem_irq_clear_pending(modem);
    

    return 0;
}
EXPORT_SYMBOL(modem_irq_init);

int modem_irq_exit(struct sw_modem *modem)
{
	modem_irq_disable(modem);
    modem_irq_config_clear(modem);
    modem_irq_clear_pending(modem);

    free_irq(SW_INT_IRQNO_PIO,modem);
    cancel_work_sync(&modem->irq_work);
    modem_free_input_device(modem);

    return 0;
}
EXPORT_SYMBOL(modem_irq_exit);

void modem_early_suspend(struct sw_modem *modem)
{
    modem_irq_config(modem);
    modem_irq_clear_pending(modem);
    modem_irq_enable(modem);

    return;
}
EXPORT_SYMBOL(modem_early_suspend);

void modem_early_resume(struct sw_modem *modem)
{
    modem_irq_disable(modem);
    modem_irq_config_clear(modem);
    modem_irq_clear_pending(modem);

    return;
}
EXPORT_SYMBOL(modem_early_resume);


