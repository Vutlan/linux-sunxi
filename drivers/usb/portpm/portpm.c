/*
 * \file        portpm.c
 * \brief       
 *
 * \version     1.0.0
 * \date        2012-08-29
 * \author      chenjian <chenjian@allwinnertech.com>
 *
 * Copyright (c) 2012 Allwinner Technology. All Rights Reserved.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/kthread.h>
#include <linux/fs.h>
#include <asm/uaccess.h>


#include "../sun5i_usb/include/sw_usb_config.h"

#ifdef CONFIG_HAS_EARLYSUSPEND    
#include <linux/pm.h>    
#include <linux/earlysuspend.h>
#endif

#define V_1A_DEFAULT        3550000    
#define C_1A_DEFAULT        10       
#define V_500MA_DEFAULT     0      
#define C_500MA_DEFAULT     10       
#define V_DISABLE_DEFAULT   3400000       
#define C_DISABLE_DEFAULT   5 

struct pm_gpio_t{
    __u32           hdle;
    user_gpio_set_t gpio_set;	
};

struct pm_cfg{
    int restrict_1a;
    int restrict_500ma;    
    int v_1a;
    int c_1a;
    int v_500ma;
    int c_500ma;
    int v_disable;
    int c_disable;
    struct pm_gpio_t ctrl_gpio;	
};


#ifdef CONFIG_HAS_EARLYSUSPEND
static struct early_suspend standby;
#endif

static __u32 thread_run_flag = 0;
static __u32 thread_stopped_flag = 0;
static __u32 thread_suspend_flag = 0;
static __s32 vbus_status = 0;
static __s32 ctrlio_status = 0;

extern int get_hcd0_connect_status(void);
extern int get_ehci_connect_status(int usbc_no);
extern int get_ohci_connect_status(int usbc_no);

extern int hcd0_set_vbus(int is_on);
extern int hci_set_vbus(int usbc_no, int is_on);
extern int hci_get_vbus_status(int usbc_no);
extern int hcd0_get_vbus_status(void);

struct pm_cfg pm_cfg;

static int (*do_pm)(int, int, int);

static void get_pm_cfg(struct pm_cfg *cfg)
{
    s32 ret = 0;
    
    ret = script_parser_fetch("port_pm", "restrict_1a", (int *)&(cfg->restrict_1a), 64);
	if(ret != 0){
		printk("ERR: get pm_cfg:restrict_1a failed\n");
	}
	
	ret = script_parser_fetch("port_pm", "restrict_500ma", (int *)&(cfg->restrict_500ma), 64);
	if(ret != 0){
		printk("ERR: get pm_cfg:restrict_500ma failed\n");
	}
	
	ret = script_parser_fetch("port_pm", "v_1a", (int *)&(cfg->v_1a), 64);
	if(ret != 0){
		printk("ERR: get pm_cfg:v_1a failed\n");
	}
	
	ret = script_parser_fetch("port_pm", "c_1a", (int *)&(cfg->c_1a), 64);
	if(ret != 0){
		printk("ERR: get pm_cfg:c_1a failed\n");
	}
	
	ret = script_parser_fetch("port_pm", "v_500ma", (int *)&(cfg->v_500ma), 64);
	if(ret != 0){
		printk("ERR: get pm_cfg:v_500ma failed\n");
	}
	
	ret = script_parser_fetch("port_pm", "c_500ma", (int *)&(cfg->c_500ma), 64);
	if(ret != 0){
		printk("ERR: get pm_cfg:c_500ma failed\n");
	}
	
	ret = script_parser_fetch("port_pm", "v_disable", (int *)&(cfg->v_disable), 64);
	if(ret != 0){
		printk("ERR: get pm_cfg:v_disable failed\n");
	}
	
	ret = script_parser_fetch("port_pm", "c_disable", (int *)&(cfg->c_disable), 64);
	if(ret != 0){
		printk("ERR: get pm_cfg:c_disable failed\n");
	}
	
	ret = script_parser_fetch("port_pm", "ctrl_gpio", (int *)&(cfg->ctrl_gpio.gpio_set), 64);
	if(ret != 0){
		printk("ERR: get pm_cfg:ctrl_gpio failed\n");
	}
	else
	    printk("ctrl gpio_name = %s,\n"
	           "port = %d, port_num = %d,\n"
	           "mul_sel = %d, pull = %d,\n" 
	           "drv_level = %d, data = %d\n", 
	            cfg->ctrl_gpio.gpio_set.gpio_name, 
	            cfg->ctrl_gpio.gpio_set.port, cfg->ctrl_gpio.gpio_set.port_num,
	            cfg->ctrl_gpio.gpio_set.mul_sel, cfg->ctrl_gpio.gpio_set.pull,
	            cfg->ctrl_gpio.gpio_set.drv_level, cfg->ctrl_gpio.gpio_set.data);
	return;
}

static void check_pm_cfg(struct pm_cfg *cfg)
{
    if(!cfg->v_1a)
        cfg->v_1a = V_1A_DEFAULT;

    if(!cfg->c_1a)
        cfg->c_1a = C_1A_DEFAULT;

    if(!cfg->v_500ma)
        cfg->v_500ma = V_500MA_DEFAULT; 

    if(!cfg->c_500ma)
        cfg->c_500ma = C_500MA_DEFAULT;  

    if(!cfg->v_disable)
        cfg->v_disable = V_DISABLE_DEFAULT;    

    if(!cfg->c_disable)
        cfg->c_disable = C_DISABLE_DEFAULT;  

}

static void printk_pm_cfg(struct pm_cfg *cfg)
{
    printk("restrict_1a = %d, restrict_500ma = %d \n"
           "v_1a = %d, c_1a = %d, v_500ma = %d, c_500ma = %d "
           "v_disable = %d, c_disable = %d \n", 
               cfg->restrict_1a, cfg->restrict_500ma, 
               cfg->v_1a, cfg->c_1a, cfg->v_500ma, cfg->c_500ma,
               cfg->v_disable, cfg->c_disable);
}

static int get_connect_status(void)
{
    int connect_status = 0;
    
    #if defined(CONFIG_USB_SW_SUN5I_HCD0)
    connect_status |= get_hcd0_connect_status();
    #endif

    #if defined(CONFIG_USB_SW_SUN5I_EHCI0)
    connect_status |= get_ehci_connect_status(1);
    #endif

    #if defined(CONFIG_USB_SW_SUN5I_OHCI0)
    connect_status |= get_ohci_connect_status(1);
    #endif

    #if defined(CONFIG_USB_SW_SUN5I_EHCI1)
    connect_status |= get_ehci_connect_status(2);
    #endif

    #if defined(CONFIG_USB_SW_SUN5I_OHCI1)
    connect_status |= get_ohci_connect_status(2);
    #endif

    return connect_status;
}

/*
value 
0 : restrict to 500ma
1 : restrict to 1a
*/
static int set_ctrl_gpio(int is_on)
{      
    int ret = 0;
    if(ctrlio_status == is_on)
        return 0;
    if(pm_cfg.ctrl_gpio.hdle){
        int on_off;
    	printk("set ctrl gpio %s\n", is_on ? "on" : "off");        
	    ret = gpio_write_one_pin_value(pm_cfg.ctrl_gpio.hdle, 1, NULL);		    
    }

    ctrlio_status = is_on;
        
    return ret;
}

static int get_ctrl_gpio_status(void)
{    
    if(pm_cfg.ctrl_gpio.hdle)
        ctrlio_status = gpio_read_one_pin_value(pm_cfg.ctrl_gpio.hdle, NULL);	   
    
    return ctrlio_status;
}

static int get_vbus_status(void)
{
    int status = 0;

    #if defined(CONFIG_USB_SW_SUN5I_HCD0)
    status |= hcd0_get_vbus_status();
    #endif

    #if defined(CONFIG_USB_SW_SUN5I_EHCI0)
    status |= hci_get_vbus_status(1);
    #endif

    #if defined(CONFIG_USB_SW_SUN5I_OHCI0)
    status |= hci_get_vbus_status(1);
    #endif

    #if defined(CONFIG_USB_SW_SUN5I_EHCI1)
    status |= hci_get_vbus_status(2);
    #endif

    #if defined(CONFIG_USB_SW_SUN5I_OHCI1)
    status |= hci_get_vbus_status(2);
    #endif

    return status;
}

static int set_vbus(int on_off)
{
    if(get_vbus_status() == !!on_off)
        return 0;

    printk("set vbus %s\n", on_off ? "on" : "off");
    #if defined(CONFIG_USB_SW_SUN5I_HCD0)
    hcd0_set_vbus(on_off);
    #endif

    #if defined(CONFIG_USB_SW_SUN5I_EHCI0)
    hci_set_vbus(1, on_off);
    #endif

    #if defined(CONFIG_USB_SW_SUN5I_OHCI0)
    hci_set_vbus(1, on_off);
    #endif

    #if defined(CONFIG_USB_SW_SUN5I_EHCI1)
    hci_set_vbus(2, on_off);
    #endif

    #if defined(CONFIG_USB_SW_SUN5I_OHCI1)
    hci_set_vbus(2, on_off);
    #endif

    vbus_status = on_off;
    return 0;
}

static int do_1a_500ma(int connect, int voltage, int capacity)
{
    if(!connect){
        if(voltage > pm_cfg.v_1a && capacity > pm_cfg.c_1a){
            set_vbus(1);
            set_ctrl_gpio(1);
        }
        else if(voltage > pm_cfg.v_500ma && capacity > pm_cfg.c_500ma){
            set_vbus(1);
            set_ctrl_gpio(0);
        }
        else{
            set_vbus(0);            
        }            
    }
    else{        
        if(voltage > pm_cfg.v_disable && capacity > pm_cfg.c_disable)
            set_vbus(1);   
        else
            set_vbus(0);         
    }
    return 0;
}

static int do_1a(int connect, int voltage, int capacity)
{        
    if(!connect){
        if(voltage > pm_cfg.v_1a && capacity > pm_cfg.c_1a){
            set_vbus(1);
            set_ctrl_gpio(1);
        }        
        else{
            set_vbus(0);            
        }            
    }
    else{        
        if(voltage > pm_cfg.v_disable && capacity > pm_cfg.c_disable)
            set_vbus(1);   
        else
            set_vbus(0);         
    }    
    
    return 0;
}

static int do_500ma(int connect, int voltage, int capacity)
{    
    if(!connect){
        if(voltage > pm_cfg.v_500ma && capacity > pm_cfg.c_500ma){
            set_vbus(1);
            set_ctrl_gpio(0);
        }
        else{
            set_vbus(0);            
        }            
    }
    else{        
        if(voltage > pm_cfg.v_disable && capacity > pm_cfg.c_disable)
            set_vbus(1);   
        else
            set_vbus(0);         
    }            
    
    return 0;
}

#define BUFLEN 32

static int usb_port_pm_thread(void * pArg)
{
    int connect, old_connect = 0, voltage, old_voltage = 0, capacity, old_capacity = 0;
    struct file *filep;
    loff_t pos;
    char buf[BUFLEN];
    int ret;
    
    vbus_status = get_vbus_status();
    
    while(thread_run_flag){
        msleep(1000);
        
        if(thread_suspend_flag)
            continue;

        filep=filp_open("/sys/class/power_supply/battery/status", O_RDONLY, 0);        
        if(IS_ERR(filep)){
            printk("open status fail\n");
            continue;
        }
       
        pos = 0;
        vfs_read(filep, (char __user *)buf, BUFLEN, &pos);
        filp_close(filep, 0);
        
        if(!strncmp((const char *)buf, "Charging", 8)){            
            set_vbus(1);
            if(pm_cfg.restrict_1a)
                set_ctrl_gpio(1);
            else
                set_ctrl_gpio(0);
            continue;
        }    
        connect = get_connect_status();
        
        filep=filp_open("/sys/class/power_supply/battery/voltage_now", O_RDONLY, 0);        
        if(IS_ERR(filep)){
            printk("open voltage fail\n");
            continue;
        }
       
        pos = 0;
        vfs_read(filep, (char __user *)buf, BUFLEN, &pos);
        filp_close(filep, 0);
        
        ret = sscanf(buf, "%d\n", &voltage);
        if(ret != 1)
            printk("ret = %d\n", ret);
            
        filep=filp_open("/sys/class/power_supply/battery/capacity", O_RDONLY, 0);        
        if(IS_ERR(filep)){
            printk("open capacity fail\n");
            continue;
        }
        pos = 0;
        vfs_read(filep, (char __user *)buf, BUFLEN, &pos);
        filp_close(filep, 0); 
        ret = sscanf(buf, "%d\n", &capacity);
        if(ret != 1)
            printk("ret = %d\n", ret);  
          
        if(connect != old_connect || voltage != old_voltage || capacity != old_capacity){  
            printk("connect = %d, voltage = %d, capacity = %d\n", connect, voltage, capacity);
            old_connect = connect;
            old_voltage = voltage;
            old_capacity = capacity;
            do_pm(connect, voltage, capacity);
        }                
    }

    thread_stopped_flag = 1;

    return 0;
}
#ifdef CONFIG_HAS_EARLYSUSPEND
static int portpm_suspend(struct early_suspend *h)
{
    printk("portpm_suspend\n");
    thread_suspend_flag = 1;
    return 0;
}

static int portpm_resume(struct early_suspend *h)
{
    printk("portpm_resume\n");
    thread_suspend_flag = 0;
    return 0;
}
#endif

static int __init portpm_init(void)
{
    struct task_struct *pm_task = NULL;
    printk("%s(L%d):\n", __func__, __LINE__);
    #ifdef CONFIG_HAS_EARLYSUSPEND        
    standby.level     = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 5;    
    standby.suspend   = portpm_suspend;    
    standby.resume    = portpm_resume;    
    register_early_suspend(&standby);
    #endif   
    
    get_pm_cfg(&pm_cfg);
    //check_pm_cfg(&pm_cfg);
    printk_pm_cfg(&pm_cfg);
    
    pm_cfg.ctrl_gpio.hdle = gpio_request(&pm_cfg.ctrl_gpio.gpio_set, 1);
    if(pm_cfg.ctrl_gpio.hdle){
        /* set config, ouput */
        gpio_set_one_pin_io_status(pm_cfg.ctrl_gpio.hdle, 1, NULL);
        printk("0xf1c20920 : value = %x\n", readl(0xf1c20920));
    	/* default is pull down */
    	gpio_set_one_pin_pull(pm_cfg.ctrl_gpio.hdle, 2, NULL);
    }else    
		printk("request ctrl gpio failed\n");	
	    
    if(pm_cfg.restrict_1a && pm_cfg.restrict_500ma){
        do_pm = do_1a_500ma;
    }
    else if(pm_cfg.restrict_1a){
        do_pm = do_1a;
    }
    else if(pm_cfg.restrict_500ma){
        do_pm = do_500ma;
    }
    else{
        do_pm = do_1a;
    }
    
    thread_run_flag = 1;
    thread_stopped_flag = 0;
    pm_task = kthread_create(usb_port_pm_thread, &pm_cfg, "usb-port-power-management");
    if(IS_ERR(pm_task)){
		printk("ERR: usb-port-power-management thread create failed\n");
		return -1;
	}
 
	wake_up_process(pm_task);

	return 0;
}

static void __exit portpm_exit(void)
{
    thread_run_flag = 0;
	while(!thread_stopped_flag){
		printk("waitting for usb-port-power-management thread stop\n");
		msleep(10);
	}
	do_pm = NULL;
	if(pm_cfg.ctrl_gpio.hdle){
		gpio_release(pm_cfg.ctrl_gpio.hdle, 0);
		pm_cfg.ctrl_gpio.hdle = 0;
	}

	#ifdef CONFIG_HAS_EARLYSUSPEND    
	unregister_early_suspend(&standby);
	#endif
}


module_init(portpm_init);
module_exit(portpm_exit);

