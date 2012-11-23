/* drivers/input/touchscreen/zet6221_i2c.c
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * ZEITEC Semiconductor Co., Ltd
 * Tel: +886-3-579-0045
 * Fax: +886-3-579-9960
 * http://www.zeitecsemi.com
 */




#include <linux/i2c.h>
#include <linux/input.h>
    #include <linux/pm.h>
    #include <linux/earlysuspend.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/async.h>
#include <linux/hrtimer.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include <mach/irqs.h>
#include <mach/system.h>
#include <mach/hardware.h>
#include <mach/sys_config.h>
#include "ctp_platform_ops.h"

//#define for smit
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <asm/io.h>
#include <linux/platform_device.h>
//#include <mach/gpio.h>
#include <linux/irq.h>
#include <linux/irq.h>
#include <asm/irq.h>
#include <linux/syscalls.h>
#include <linux/reboot.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/completion.h>
#include <asm/uaccess.h>
#include <mach/irqs.h>
#include <mach/system.h>
#include <mach/hardware.h>
#include <mach/sys_config.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/errno.h>

//#include "zet6221_fw.h"
#include "zet6221.h"
//#include "zet6221_fw_dianchu.h"
//#include "zet6221-fw-czy.h"
//#include "zet6221-fw-xdc.h"
//#include "zet6221-hh.h"
//#include "zet6221-fw-czy-htp.h"
//#include "zet6221_deyi.h"
//#include "zet6221_fw_ATC070A08_weituo.h"
//#include "zet6221_fw_DPC0263_deyi3.h"
//#include "zet6221_fw_zc_nc.h"
//#include "zet6221_fw_xdc_nc.h"
//#include "zet6221_fw_zc.h"
//#include "zet6221_fw_huahao.h"
//#include "zet6221_fw_thf.h"
//#include "zet6221_fw_jinjia.h"
//#include "zet6221_fw_dc1109.h"

static user_gpio_set_t  gpio_int_info[1];

///////////////////////////////////////////////
//specific tp related macro: need be configured for specific tp
//#ifdef CONFIG_ARCH_SUN4I
//#define CTP_IRQ_NO			(IRQ_EINT21)
#define CTP_IRQ_NO	(gpio_int_info[0].port_num)

//#else ifdef CONFIG_ARCH_SUN5I
//#define CTP_IRQ_NO			(IRQ_EINT9)
//#define CTP_IRQ_NO	(gpio_int_info[0].port_num)

//#endif

#define CTP_IRQ_MODE			(NEGATIVE_EDGE)
#define FTS_FALSE			0
#define FTS_TRUE			1
  
#define TS_RESET_LOW_PERIOD		(1)
#define TS_INITIAL_HIGH_PERIOD		(30)
#define TS_WAKEUP_LOW_PERIOD	(10)
#define TS_WAKEUP_HIGH_PERIOD	(20)
#define TS_POLL_DELAY			(10)	/* ms delay between samples */
#define TS_POLL_PERIOD			(10)	/* ms delay between samples */
#define PRESS_MAX			(255)

#ifdef PRINT_POINT_INFO 
#define print_point_info(fmt, args...)   \
        do{                              \
                pr_info(fmt, ##args);     \
        }while(0)
#else
#define print_point_info(fmt, args...)   //
#endif

//#define PRINT_INT_INFO
#ifdef PRINT_INT_INFO 
#define print_int_info(fmt, args...)     \
        do{                              \
                pr_info(fmt, ##args);     \
        }while(0)
#else
#define print_int_info(fmt, args...)   //
#endif

int enable_cmd = 0;

 
static u8 ChargeChange = 0;//discharge

void ts_write_charge_enable_cmd(void);
void ts_write_charge_disable_cmd(void);
u8 zet6221_ts_version(void);

struct timer_list write_timer; 
static void* __iomem gpio_addr = NULL;
static int gpio_int_hdle = 0;
static int gpio_wakeup_hdle = 0;
static int gpio_reset_hdle = 0;
static int gpio_wakeup_enable = 1;
static int gpio_reset_enable = 1;

//static int gpio_int_hdle_read = 0;
static int gpio_int_hdle_read = 3; 

static struct i2c_client *this_client; 

struct i2c_dev{
struct list_head list;	
struct i2c_adapter *adap;
struct device *dev;
};

s32 zet6221_i2c_write_tsdata(struct i2c_client *client, u8 *data, u8 length);
static struct class *i2c_dev_class;
static LIST_HEAD (i2c_dev_list);
static DEFINE_SPINLOCK(i2c_dev_list_lock);

#define I2C_MINORS 	256
#define I2C_MAJOR 	125
static int	int_cfg_addr[]={PIO_INT_CFG0_OFFSET,PIO_INT_CFG1_OFFSET,
			PIO_INT_CFG2_OFFSET, PIO_INT_CFG3_OFFSET};
/* Addresses to scan */
static const unsigned short normal_i2c[2] = {0x76,I2C_CLIENT_END};
static __u32 twi_id = 0;

/* -------------- global variable definition -----------*/
#define _MACH_MSM_TOUCH_H_

#define ZET_TS_ID_NAME "zet6221_ts"

#define MJ5_TS_NAME	ZET_TS_ID_NAME 

#define RSTPIN_ENABLE
#define TPINFO	1
//#define X_MAX	1536
//#define Y_MAX	832
//#define X_MAX	1216
//#define Y_MAX	704
#define Y_MAX 480
#define X_MAX 790
//#define FINGER_NUMBER 4
#define FINGER_NUMBER 5
#define KEY_NUMBER 0	//please assign correct default key number 0/8 
//#define KEY_NUMBER 8  
#define P_MAX	1
#define D_POLLING_TIME	25000
#define U_POLLING_TIME	25000
#define S_POLLING_TIME  100
#define REPORT_POLLING_TIME  5

#define MAX_KEY_NUMBER      	8
#define MAX_FINGER_NUMBER	16
#define TRUE 		1
#define FALSE 		0

//#define debug_mode 1
//#define DPRINTK(fmt,args...)	do { if (debug_mode) printk(KERN_EMERG "[%s][%d] "fmt"\n", __FUNCTION__, __LINE__, ##args);} while(0)
#define DPRINTK(fmt,args...)	printk(KERN_EMERG "[%s][%d] "fmt"\n", __FUNCTION__, __LINE__, ##args)

//#define TRANSLATE_ENABLE 1
#define TOPRIGHT 	0
#define TOPLEFT  	1
#define BOTTOMRIGHT	2
#define BOTTOMLEFT	3
#define ORIGIN		BOTTOMRIGHT

//Jack.wu 03/02/2012 for debug INT
static void* __iomem gpio_addr10 = NULL;
#define gpio_base                (0x01c20800)
#define gpio_range               (0x400)
#define ph2_ctrl_offset          0x104
#define ph_data_offset           0x10c
 __u32 temp_data1,iii;
//Jack.wu 03/02/2012 for debug INT

struct msm_ts_platform_data {
	unsigned int x_max;
	unsigned int y_max;
	unsigned int pressure_max;
};

struct zet6221_tsdrv {
	struct i2c_client *i2c_ts;
	struct work_struct work1;
	struct work_struct work2; 
	struct workqueue_struct *ts_workqueue; 
	struct workqueue_struct *ts_workqueue1; 
	struct input_dev *input;
	struct timer_list polling_timer;
	struct early_suspend early_suspend;
	unsigned int gpio; /* GPIO used for interrupt of TS1*/
	unsigned int irq;
	unsigned int x_max;
	unsigned int y_max;
	unsigned int pressure_max;
};

static u16 polling_time = S_POLLING_TIME;

static int __devinit zet6221_ts_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int __devexit zet6221_ts_remove(struct i2c_client *dev);

static unsigned char zeitec_zet6221_page[130] __initdata;
static unsigned char zeitec_zet6221_page_in[130] __initdata;

static int filterCount = 0; 
static u32 filterX[MAX_FINGER_NUMBER][2], filterY[MAX_FINGER_NUMBER][2]; 

static u8  key_menu_pressed = 0x0; //0x1;
static u8  key_back_pressed = 0x0; //0x1;
static u8  key_home_pressed = 0x0; //0x1;
static u8  key_search_pressed = 0x0; //0x1;

static u16 ResolutionX=X_MAX;
static u16 ResolutionY=Y_MAX;
static u16 FingerNum=0;
static u16 KeyNum=0;
static int bufLength=0;	
static u8 inChargerMode=0;
static u8 xyExchange=0;
static int f_up_cnt=0;
static u8 pc[8];
//static u16 fb[8] = {0x3EEA,0x3EED,0x3EF0,0x3EF3,0x3EF6,0x3EF9,0x3EFC,0x3EFF};
static u16 fb[8] = {0x3DF1,0x3DF4,0x3DF7,0x3DFA,0x3EF6,0x3EF9,0x3EFC,0x3EFF};
static int resetCount = 0;

static struct i2c_client *this_client;

extern bool i2c_test(struct i2c_client * client);
//Touch Screen
static const struct i2c_device_id zet6221_ts_idtable[] = {
       { ZET_TS_ID_NAME, 0 },
       { }
};

static struct i2c_driver zet6221_ts_driver = {
	.class = I2C_CLASS_HWMON,
	.driver = {
		.owner = THIS_MODULE,
		.name  = ZET_TS_ID_NAME,
	},
	.probe	  = zet6221_ts_probe,
	.remove		= __devexit_p(zet6221_ts_remove),
	.id_table = zet6221_ts_idtable,
	.address_list	= normal_i2c,
};

#if 1 //other i2c downloader.Don't need now

#define I2C_CTPM_ADDRESS        (0x76)

/*
[function]: 
    callback: write data to ctpm by i2c interface,implemented by special user;
[parameters]:
    bt_ctpm_addr[in]    :the address of the ctpm;
    pbt_buf[in]        :data buffer;
    dw_lenth[in]        :the length of the data buffer;
[return]:
    FTS_TRUE     :success;
    FTS_FALSE    :fail;
*/
int i2c_write_interface(u8 bt_ctpm_addr, u8* pbt_buf, u16 dw_lenth)
{
	int ret;
	ret=i2c_master_send(this_client, pbt_buf, dw_lenth);
	if(ret != dw_lenth){
		pr_info("i2c_write_interface error\n");
		return FTS_FALSE;
	}

	return FTS_TRUE;
}

/*
[function]: 
    callback: read data from ctpm by i2c interface,implemented by special user;
[parameters]:
    bt_ctpm_addr[in]    :the address of the ctpm;
    pbt_buf[out]        :data buffer;
    dw_lenth[in]        :the length of the data buffer;
[return]:
    FTS_TRUE     :success;
    FTS_FALSE    :fail;
*/
int i2c_read_interface(u8 bt_ctpm_addr, u8* pbt_buf, u16 dw_lenth)
{
	int ret;

	ret=i2c_master_recv(this_client, pbt_buf, dw_lenth);

	if(ret != dw_lenth){
		pr_info("ret = %d. \n", ret);
		pr_info("i2c_read_interface error\n");
		return FTS_FALSE;
	}

	return FTS_TRUE;
}

#endif //other i2c downloader

///////////////

/*
 * ctp_get_pendown_state  : get the int_line data state, 
 * 
 * return value:
 *             return PRESS_DOWN: if down
 *             return FREE_UP: if up,
 *             return 0: do not need process, equal free up.
 */
static int ctp_get_pendown_state(void)
{
	unsigned int reg_val;
	static int state = FREE_UP;

	//get the input port state
	reg_val = readl(gpio_addr + PIOH_DATA);
	//pr_info("reg_val = %x\n",reg_val);
	if(!(reg_val & (1<<CTP_IRQ_NO))){
		state = PRESS_DOWN;
		print_int_info("pen down. \n");
	}else{ //touch panel is free up
		state = FREE_UP;
		print_int_info("free up. \n");
	}
	return state;
}


__u32 get_gpio_value(void)
{
	//Jack.wu 03/02/2012 for debug INT
         gpio_addr10 = ioremap(gpio_base, gpio_range); 
         temp_data1 = readl(gpio_addr10+ph2_ctrl_offset);
         temp_data1 &= 0xff0fffff;
         writel(temp_data1,gpio_addr10+ph2_ctrl_offset);   //input  
	 temp_data1 = readl(gpio_addr10+ph_data_offset) & 0x00200000;  
	//Jack.wu 03/02/2012 for debug INT
	return temp_data1;
}


/**
 * ctp_clear_penirq - clear int pending
 *
 */
static void ctp_clear_penirq(void)
{
	int reg_val;
	//clear the IRQ_EINT29 interrupt pending
	//pr_info("clear pend irq pending\n");
	reg_val = readl(gpio_addr + PIO_INT_STAT_OFFSET);
	//writel(reg_val,gpio_addr + PIO_INT_STAT_OFFSET);
	//writel(reg_val&(1<<(IRQ_EINT21)),gpio_addr + PIO_INT_STAT_OFFSET);
	if((reg_val = (reg_val&(1<<(CTP_IRQ_NO))))){
		print_int_info("==CTP_IRQ_NO=\n");              
		writel(reg_val,gpio_addr + PIO_INT_STAT_OFFSET);
	}
	return;
}

/**
 * ctp_set_irq_mode - according sysconfig's subkey "ctp_int_port" to config int port.
 * 
 * return value: 
 *              0:      success;
 *              others: fail; 
 */
static int ctp_set_irq_mode(char *major_key , char *subkey,ext_int_mode int_mode)
{
	int ret = 0;
	__u32 reg_num = 0;
	__u32 reg_addr = 0;
	__u32 reg_val = 0;
	//config gpio to int mode
	pr_info("%s: config gpio to int mode. \n", __func__);
#ifndef SYSCONFIG_GPIO_ENABLE
#else
	if(gpio_int_hdle){
		gpio_release(gpio_int_hdle, 2);
	}
	gpio_int_hdle = gpio_request_ex(major_key, subkey);
	if(!gpio_int_hdle){
		pr_info("request tp_int_port failed. \n");
		ret = -1;
		goto request_tp_int_port_failed;
	}
	
	gpio_get_one_pin_status(gpio_int_hdle, gpio_int_info, subkey, 1);	
	pr_info("%s, %d: gpio_int_info, port = %d, port_num = %d. \n", __func__, __LINE__,gpio_int_info[0].port, gpio_int_info[0].port_num);

#endif

#ifdef AW_GPIO_INT_API_ENABLE
#else
	pr_info(" INTERRUPT CONFIG\n");
//	reg_num = ext_int_num%8;
//	reg_addr = ext_int_num/8;
	reg_num = (gpio_int_info[0].port_num)%8;
	reg_addr = (gpio_int_info[0].port_num)/8;

	reg_val = readl(gpio_addr + int_cfg_addr[reg_addr]);
	reg_val &= (~(7 << (reg_num * 4)));
	reg_val |= (int_mode << (reg_num * 4));
	writel(reg_val,gpio_addr+int_cfg_addr[reg_addr]);
                          
	ctp_clear_penirq();

	reg_val = readl(gpio_addr+PIO_INT_CTRL_OFFSET); 
//	reg_val |= (1 << ext_int_num);
	reg_val |= (1 << (gpio_int_info[0].port_num));

	writel(reg_val,gpio_addr+PIO_INT_CTRL_OFFSET);
	
	udelay(1);
#endif

request_tp_int_port_failed:

	return ret;  
}

/**
 * ctp_set_gpio_mode - according sysconfig's subkey "ctp_io_port" to config io port.
 *
 * return value: 
 *              0:      success;
 *              others: fail; 
 */
static int ctp_set_gpio_mode(void)
{
	//int reg_val;
	int ret = 0;
	//config gpio to io mode
	pr_info("%s: config gpio to io mode. \n", __func__);
#ifndef SYSCONFIG_GPIO_ENABLE
#else
	if(gpio_int_hdle){
		gpio_release(gpio_int_hdle, 2);
	}
	gpio_int_hdle = gpio_request_ex("ctp_para", "ctp_io_port");
	if(!gpio_int_hdle){
		pr_info("request ctp_io_port failed. \n");
		ret = -1;
		goto request_tp_io_port_failed;
	}
#endif
	return ret;

request_tp_io_port_failed:
	return ret;
}

/**
 * ctp_judge_int_occur - whether interrupt occur.
 *
 * return value: 
 *              0:      int occur;
 *              others: no int occur; 
 */
static int ctp_judge_int_occur(void)
{
	//int reg_val[3];
	int reg_val;
	int ret = -1;
	reg_val = readl(gpio_addr + PIO_INT_STAT_OFFSET);
//	printk("the reg_val = %x ===========\n"); 
	if(reg_val&(1<<(CTP_IRQ_NO))){
		ret = 0;
	}

	return ret; 	
}

/**
 * ctp_free_platform_resource - corresponding with ctp_init_platform_resource
 *
 */
static void ctp_free_platform_resource(void)
{
	if(gpio_addr){
		iounmap(gpio_addr);
	}
	
	if(gpio_int_hdle){
		gpio_release(gpio_int_hdle, 2);
	}
	
	if(gpio_wakeup_hdle){
		gpio_release(gpio_wakeup_hdle, 2);
	}
	
	if(gpio_reset_hdle){
		gpio_release(gpio_reset_hdle, 2);
	}

	return;
}


/**
 * ctp_init_platform_resource - initialize platform related resource
 * return value: 0 : success
 *               -EIO :  i/o err.
 *
 */
static int ctp_init_platform_resource(void)
{
	int ret = 0;

	gpio_addr = ioremap(PIO_BASE_ADDRESS, PIO_RANGE_SIZE);
	//pr_info("%s, gpio_addr = 0x%x. \n", __func__, gpio_addr);
	if(!gpio_addr) {
		ret = -EIO;
		goto exit_ioremap_failed;	
	}
	//    gpio_wakeup_enable = 1;
	gpio_wakeup_hdle = gpio_request_ex("ctp_para", "ctp_wakeup");
	if(!gpio_wakeup_hdle) {
		pr_warning("%s: tp_wakeup request gpio fail!\n", __func__);
		gpio_wakeup_enable = 0;
	}

	gpio_reset_hdle = gpio_request_ex("ctp_para", "ctp_reset");
	if(!gpio_reset_hdle) {
		pr_warning("%s: tp_reset request gpio fail!\n", __func__);
		gpio_reset_enable = 0;
	}
	
	gpio_int_hdle_read = gpio_request_ex("ctp_para", "ctp_io_port");
	if(!gpio_int_hdle_read){
		pr_warning("%s: request ctp_io_port failed. \n", __func__);
	}

	return ret;

exit_ioremap_failed:
	ctp_free_platform_resource();
	return ret;
}


/**
 * ctp_fetch_sysconfig_para - get config info from sysconfig.fex file.
 * return value:  
 *                    = 0; success;
 *                    < 0; err
 */
static int ctp_fetch_sysconfig_para(void)
{
	int ret = -1;
	int ctp_used = -1;
	char name[I2C_NAME_SIZE];
	__u32 twi_addr = 0;
	//__u32 twi_id = 0;
	script_parser_value_type_t type = SCIRPT_PARSER_VALUE_TYPE_STRING;

	pr_info("%s. \n", __func__);

	memset(name, 0, I2C_NAME_SIZE);


	if(SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_used", &ctp_used, 1)){
		pr_err("%s: script_parser_fetch err. \n", __func__);
		goto script_parser_fetch_err;
	}
	if(1 != ctp_used){
		pr_err("%s: ctp_unused. \n",  __func__);
		//ret = 1;
		return ret;
	}


	if(SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_twi_id", &twi_id, sizeof(twi_id)/sizeof(__u32))){
		pr_err("%s: script_parser_fetch err. \n", name);
		goto script_parser_fetch_err;
	}
	pr_info("%s: ctp_twi_id is %d. \n", __func__, twi_id);


	return 0;

script_parser_fetch_err:
	pr_notice("=========script_parser_fetch_err============\n");
	return ret;
}

/**
 * ctp_reset - function
 *
 */
static void ctp_reset(void)
{
	if(gpio_reset_enable){
		pr_info("%s. \n", __func__);
		if(EGPIO_SUCCESS != gpio_write_one_pin_value(gpio_reset_hdle, 0, "ctp_reset")){
			pr_info("%s: err when operate gpio. \n", __func__);
		}
		mdelay(TS_RESET_LOW_PERIOD);
		if(EGPIO_SUCCESS != gpio_write_one_pin_value(gpio_reset_hdle, 1, "ctp_reset")){
			pr_info("%s: err when operate gpio. \n", __func__);
		}
		mdelay(TS_INITIAL_HIGH_PERIOD);
	}
}

/**
 * ctp_wakeup - function
 *
 */
static void ctp_wakeup(void)
{
	if(1 == gpio_wakeup_enable){  
		pr_info("%s. \n", __func__);
		if(EGPIO_SUCCESS != gpio_write_one_pin_value(gpio_wakeup_hdle, 0, "ctp_wakeup")){
			pr_info("%s: err when operate gpio. \n", __func__);
		}
		mdelay(TS_WAKEUP_LOW_PERIOD);
		if(EGPIO_SUCCESS != gpio_write_one_pin_value(gpio_wakeup_hdle, 1, "ctp_wakeup")){
			pr_info("%s: err when operate gpio. \n", __func__);
		}
		mdelay(TS_WAKEUP_HIGH_PERIOD);

	}
	return;
}
/**
 * ctp_detect - Device detection callback for automatic device creation
 * return value:  
 *                    = 0; success;
 *                    < 0; err
 */
static int ctp_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;
	int ret;

        if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
                return -ENODEV;
    
	if(twi_id == adapter->nr){
                pr_info("%s: addr= %x\n",__func__,client->addr);

                ret = i2c_test(client);
                if(!ret){
        		pr_info("%s:I2C connection might be something wrong or maybe the other gsensor equipment! \n",__func__);
        		return -ENODEV;
        	}else{           	    
            	        pr_info("I2C connection sucess!\n");
            	        strlcpy(info->type, ZET_TS_ID_NAME, I2C_NAME_SIZE);
    		    return 0;	
	             }

	}else{
		return -ENODEV;
	}
}

static struct i2c_dev *get_free_i2c_dev(struct i2c_adapter *adap) 
{
	struct i2c_dev *i2c_dev;

	if (adap->nr >= I2C_MINORS){
		pr_info("i2c-dev:out of device minors (%d) \n",adap->nr);
		return ERR_PTR (-ENODEV);
	}

	i2c_dev = kzalloc(sizeof(*i2c_dev), GFP_KERNEL);
	if (!i2c_dev){
		return ERR_PTR(-ENOMEM);
	}
	i2c_dev->adap = adap;

	spin_lock(&i2c_dev_list_lock);
	list_add_tail(&i2c_dev->list, &i2c_dev_list);
	spin_unlock(&i2c_dev_list_lock);
	
	return i2c_dev;
}

static struct ctp_platform_ops ctp_ops = {
	.get_pendown_state = ctp_get_pendown_state,
	.clear_penirq	   = ctp_clear_penirq,
	.set_irq_mode      = ctp_set_irq_mode,
	.set_gpio_mode     = ctp_set_gpio_mode,	
	.judge_int_occur   = ctp_judge_int_occur,
	.init_platform_resource = ctp_init_platform_resource,
	.free_platform_resource = ctp_free_platform_resource,
	.fetch_sysconfig_para = ctp_fetch_sysconfig_para,
	.ts_reset =          ctp_reset,
	.ts_wakeup =         ctp_wakeup,
	.ts_detect = ctp_detect,
};
static struct i2c_dev *i2c_dev_get_by_minor(unsigned index)
{ 
	struct i2c_dev *i2c_dev;
	spin_lock(&i2c_dev_list_lock);
	
	list_for_each_entry(i2c_dev,&i2c_dev_list,list){
		pr_info("--line = %d ,i2c_dev->adapt->nr = %d,index = %d.\n",__LINE__,i2c_dev->adap->nr,index);
		if(i2c_dev->adap->nr == index){
		     goto found;
		}
	}
	i2c_dev = NULL;
	
found: 
	spin_unlock(&i2c_dev_list_lock);
	
	return i2c_dev ;
}

///////////////

/***********************************************************************
    [function]: 
		        callback: Timer Function if there is no interrupt fuction;
    [parameters]:
			    arg[in]:  arguments;
    [return]:
			    NULL;
************************************************************************/

static void polling_timer_func(unsigned long arg)
{
	struct zet6221_tsdrv *ts_drv = (struct zet6221_tsdrv *)arg;
//	schedule_work(&ts_drv->work2);
	queue_work(ts_drv->ts_workqueue1, &ts_drv->work2);
	mod_timer(&ts_drv->polling_timer,jiffies + msecs_to_jiffies(polling_time));
}

void write_cmd_work(void)
{
	if(enable_cmd != ChargeChange)
	{	
		if(enable_cmd == 1) {
			ts_write_charge_enable_cmd();
			
		}else if(enable_cmd == 0)
		{
			ts_write_charge_disable_cmd();
		}
		ChargeChange = enable_cmd;
	}

}

/***********************************************************************
    [function]: 
		        callback: read data by i2c interface;
    [parameters]:
			    client[in]:  struct i2c_client ??represent an I2C slave device;
			    data [out]:  data buffer to read;
			    length[in]:  data length to read;
    [return]:
			    Returns negative errno, else the number of messages executed;
************************************************************************/
s32 zet6221_i2c_read_tsdata(struct i2c_client *client, u8 *data, u8 length)
{
	struct i2c_msg msg;
	msg.addr = client->addr;
	msg.flags = I2C_M_RD;
	msg.len = length;
	msg.buf = data;
	return i2c_transfer(client->adapter,&msg, 1);
}

/***********************************************************************
    [function]: 
		        callback: write data by i2c interface;
    [parameters]:
			    client[in]:  struct i2c_client ??represent an I2C slave device;
			    data [out]:  data buffer to write;
			    length[in]:  data length to write;
    [return]:
			    Returns negative errno, else the number of messages executed;
************************************************************************/
s32 zet6221_i2c_write_tsdata(struct i2c_client *client, u8 *data, u8 length)
{
	struct i2c_msg msg;
	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = length;
	msg.buf = data;
	return i2c_transfer(client->adapter,&msg, 1);
}

/***********************************************************************
    [function]: 
		        callback: coordinate traslating;
    [parameters]:
			    px[out]:  value of X axis;
			    py[out]:  value of Y axis;
				p [in]:   pressed of released status of fingers;
    [return]:
			    NULL;
************************************************************************/
void touch_coordinate_traslating(u32 *px, u32 *py, u8 p)
{
	int i;
	u8 pressure;

	#if ORIGIN == TOPRIGHT
	for(i=0;i<MAX_FINGER_NUMBER;i++){
		pressure = (p >> (MAX_FINGER_NUMBER-i-1)) & 0x1;
		if(pressure)
		{
			px[i] = X_MAX - px[i];
		}
	}
	#elif ORIGIN == BOTTOMRIGHT
	for(i=0;i<MAX_FINGER_NUMBER;i++){
		pressure = (p >> (MAX_FINGER_NUMBER-i-1)) & 0x1;
		if(pressure)
		{
			px[i] = X_MAX - px[i];
			py[i] = Y_MAX - py[i];
		}
	}
	#elif ORIGIN == BOTTOMLEFT
	for(i=0;i<MAX_FINGER_NUMBER;i++){
		pressure = (p >> (MAX_FINGER_NUMBER-i-1)) & 0x1;
		if(pressure)
		{
			py[i] = Y_MAX - py[i];
		}
	}
	#endif
}

/***********************************************************************
    [function]: 
		        callback: read finger information from TP;
    [parameters]:
    			client[in]:  struct i2c_client ??represent an I2C slave device;
			    x[out]:  values of X axis;
			    y[out]:  values of Y axis;
			    z[out]:  values of Z axis;
				pr[out]:  pressed of released status of fingers;
				ky[out]:  pressed of released status of keys;
    [return]:
			    Packet ID;
************************************************************************/
u8 zet6221_ts_get_xy_from_panel(struct i2c_client *client, u32 *x, u32 *y, u32 *z, u32 *pr, u32 *ky)
{
	u8  ts_data[70];
	int ret;
	int i;
	
	memset(ts_data,0,70);

	ret=zet6221_i2c_read_tsdata(client, ts_data, bufLength);
	
	*pr = ts_data[1];
	*pr = (*pr << 8) | ts_data[2];
		
	for(i=0;i<FingerNum;i++)
	{
		x[i]=(u8)((ts_data[3+4*i])>>4)*256 + (u8)ts_data[(3+4*i)+1];
		y[i]=(u8)((ts_data[3+4*i]) & 0x0f)*256 + (u8)ts_data[(3+4*i)+2];
		z[i]=(u8)((ts_data[(3+4*i)+3]) & 0x0f);
	}
		
	//if key enable
	if(KeyNum > 0)
		*ky = ts_data[3+4*FingerNum];

	return ts_data[0];
}

/***********************************************************************
    [function]: 
		        callback: get dynamic report information;
    [parameters]:
    			client[in]:  struct i2c_client ??represent an I2C slave device;

    [return]:
			    1;
************************************************************************/
u8 zet6221_ts_get_report_mode(struct i2c_client *client)
{
	u8 ts_report_cmd[1] = {178};
	u8 ts_reset_cmd[1] = {176};
	u8 ts_in_data[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	
	__u32 tmp ;//= get_gpio_value;

	int ret;
	int i;

	ret=zet6221_i2c_write_tsdata(client, ts_report_cmd, 1);
//	printk("the i2c_write_tsdata's ret is %d \n",ret);

	if (ret > 0)
	{
		while(1)
		{
			udelay(1);

//			if (gpio_get_value(S3C64XX_GPN(9)) == 0)
//			if( gpio_read_one_pin_value(gpio_int_hdle, NULL) == 0)
			if(get_gpio_value() == 0)
			{
				printk ("=============== int low ===============\n");
				ret=zet6221_i2c_read_tsdata(client, ts_in_data, 17);
				
				for(i=0;i<8;i++)
				{
					pc[i]=ts_in_data[i] & 0xff;
				}

				xyExchange = (ts_in_data[16] & 0x8) >> 3;
				if(xyExchange == 1)
				{
					ResolutionY= ts_in_data[9] & 0xff;
					ResolutionY= (ResolutionY << 8)|(ts_in_data[8] & 0xff);
					ResolutionX= ts_in_data[11] & 0xff;
					ResolutionX= (ResolutionX << 8) | (ts_in_data[10] & 0xff);
				}
				else
				{
					ResolutionX = ts_in_data[9] & 0xff;
					ResolutionX = (ResolutionX << 8)|(ts_in_data[8] & 0xff);
					ResolutionY = ts_in_data[11] & 0xff;
					ResolutionY = (ResolutionY << 8) | (ts_in_data[10] & 0xff);
				}
				
				FingerNum = (ts_in_data[15] & 0x7f);
				KeyNum = (ts_in_data[15] & 0x80);
				inChargerMode = (ts_in_data[16] & 0x2) >> 1;

				if(KeyNum==0)
					bufLength  = 3+4*FingerNum;
				else
					bufLength  = 3+4*FingerNum+1;

				//DPRINTK( "bufLength=%d\n",bufLength);
				
				break;
				

			}/*else
				DPRINTK( "int high\n");*/
		}

	}
	return 1;
}

u8 zet6221_ts_get_report_mode_t(struct i2c_client *client)
{
	u8 ts_report_cmd[1] = {178};
	u8 ts_reset_cmd[1] = {176};
	u8 ts_in_data[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	
	__u32 tmp ;//= get_gpio_value;

	int ret;
	int i;
	
	ret=zet6221_i2c_write_tsdata(client, ts_report_cmd, 1);
//	printk("the i2c_write_tsdata's ret is %d \n",ret);

	if (ret > 0)
	{
			//udelay(10);
			msleep(10);
			printk ("=============== zet6221_ts_get_report_mode_t ===============\n");
			ret=zet6221_i2c_read_tsdata(client, ts_in_data, 17);
			
			if(ret > 0)
			{
				
				for(i=0;i<8;i++)
				{
					pc[i]=ts_in_data[i] & 0xff;
				}

				xyExchange = (ts_in_data[16] & 0x8) >> 3;
				if(xyExchange == 1)
				{
					ResolutionY= ts_in_data[9] & 0xff;
					ResolutionY= (ResolutionY << 8)|(ts_in_data[8] & 0xff);
					ResolutionX= ts_in_data[11] & 0xff;
					ResolutionX= (ResolutionX << 8) | (ts_in_data[10] & 0xff);
				}
				else
				{
					ResolutionX = ts_in_data[9] & 0xff;
					ResolutionX = (ResolutionX << 8)|(ts_in_data[8] & 0xff);
					ResolutionY = ts_in_data[11] & 0xff;
					ResolutionY = (ResolutionY << 8) | (ts_in_data[10] & 0xff);
				}
					
				FingerNum = (ts_in_data[15] & 0x7f);
				KeyNum = (ts_in_data[15] & 0x80);
				inChargerMode = (ts_in_data[16] & 0x2) >> 1;

				if(KeyNum==0)
					bufLength  = 3+4*FingerNum;
				else
					bufLength  = 3+4*FingerNum+1;
				
			}else
			{
				printk ("=============== zet6221_ts_get_report_mode_t READ ERROR ===============\n");
			}
			

			if(zet6221_ts_version()==0)
			{
				ResolutionX = X_MAX;
				ResolutionY = Y_MAX;
				FingerNum = FINGER_NUMBER;
				KeyNum = KEY_NUMBER;
				if(KeyNum==0)
					bufLength  = 3+4*FingerNum;
				else
					bufLength  = 3+4*FingerNum+1;
			}
				
	}
	return 1;
}


/***********************************************************************
    [function]: 
		        callback: interrupt function;
    [parameters]:
    			irq[in]:  irq value;
    			dev_id[in]: dev_id;

    [return]:
			    NULL;
************************************************************************/
static irqreturn_t zet6221_ts_interrupt(int irq, void *dev_id)
{
	struct zet6221_tsdrv *ts_drv = dev_id;

	print_int_info("==========------zet6221_ts TS Interrupt-----============\n"); 
	if(!ctp_ops.judge_int_occur()){
		print_int_info("==IRQ_EINT21=  %d\n",gpio_read_one_pin_value(gpio_int_hdle_read, NULL));
		ctp_ops.clear_penirq();
//		schedule_work(&ts_drv->work1);
		queue_work(ts_drv->ts_workqueue, &ts_drv->work1);
#if 0
		
	while(1){
			zet6221_i2c_write_tsdata(this_client, ts_wakeup_cmd, 1);	
		}
#endif
#if 0
		if (!work_pending(&ft5x_ts->pen_event_work)) 
		{
			print_int_info("Enter work\n");
			queue_work(ft5x_ts->ts_workqueue, &ft5x_ts->pen_event_work);
		}
	}else{
		print_int_info("Other Interrupt\n");
		return IRQ_NONE;
	}
#endif
	}
	return IRQ_HANDLED;
}

/***********************************************************************
    [function]: 
		        callback: touch information handler;
    [parameters]:
    			_work[in]:  struct work_struct;

    [return]:
			    NULL;
************************************************************************/
static void zet6221_ts_work(struct work_struct *_work)
{
	u32 x[MAX_FINGER_NUMBER], y[MAX_FINGER_NUMBER], z[MAX_FINGER_NUMBER], pr, ky, points;
	u32 px,py,pz;
	u8 ret;
	u8 pressure;
	int i;

	if (bufLength == 0)
	{
		return;
	}
	if(resetCount == 1)
	{
		resetCount = 0;
		return;
	}

//	if (gpio_get_value(S3C64XX_GPN(9)) != 0)
#if 0	
	if( gpio_read_one_pin_value(gpio_int_hdle, NULL) != 0)
	{
		return;
	}
#endif

	struct zet6221_tsdrv *ts =
		container_of(_work, struct zet6221_tsdrv, work1);

	struct i2c_client *tsclient1 = ts->i2c_ts;

	ret = zet6221_ts_get_xy_from_panel(tsclient1, x, y, z, &pr, &ky);

	if(ret == 0x3C)
	{

		//DPRINTK( " [KY] = %d\n", ky);
		
		points = pr;
		
		#if defined(TRANSLATE_ENABLE)
		touch_coordinate_traslating(x, y, points);
		#endif
		
		if(points == 0)
		{
			f_up_cnt++;
			if(f_up_cnt>=5)
			{
				//printk("==finger up==\n");
				f_up_cnt = 0;
				input_report_abs(ts->input, ABS_MT_TOUCH_MAJOR, 0);
				input_mt_sync(ts->input);
			}
			goto no_finger;
		}

		f_up_cnt = 0;
		for(i=0;i<FingerNum;i++){  
			pressure = (points >> (MAX_FINGER_NUMBER-i-1)) & 0x1;
			//DPRINTK( "valid=%d pressure[%d]= %d x= %d y= %d,X_MAX=%d\n",points , i, pressure,x[i],y[i],ResolutionX);

			if(pressure)
			{
				px = ResolutionX-x[i];
				py = y[i];
				//	py = ResolutionY-y[i]; //change direct y 
				pz = z[i];

				input_report_abs(ts->input, ABS_MT_TRACKING_ID, i);
	    		input_report_abs(ts->input, ABS_MT_TOUCH_MAJOR, P_MAX);
	    		//input_report_abs(ts->input, ABS_MT_POSITION_X, x[i]);
	    		//input_report_abs(ts->input, ABS_MT_POSITION_Y, y[i]);
	    		input_report_abs(ts->input, ABS_MT_POSITION_X, px);
	    		input_report_abs(ts->input, ABS_MT_POSITION_Y, py);
	    		input_mt_sync(ts->input);

			}/*else
			{
				input_report_abs(ts->input, ABS_MT_TRACKING_ID, i);
				input_report_abs(ts->input, ABS_MT_TOUCH_MAJOR, 0);
				input_mt_sync(ts->input);
				
			}*/
		}

no_finger:
		if(KeyNum > 0)
		{
			for(i=0;i<MAX_KEY_NUMBER;i++)
			//for(i=0;i<3;i++)
			{			
				pressure = ky & ( 0x01 << i );
				switch(i)
				{
					case 0:
						if(pressure)
						{
							/*if(key_search_pressed)
							{
								input_report_key(ts->input, KEY_SEARCH, 1);
								input_report_key(ts->input, KEY_SEARCH, 0);
								key_search_pressed = 0x0;
							}*/
							
							if(!key_search_pressed)
							{
								printk("key_search is pressed ================\n");
                                input_report_key(ts->input, KEY_SEARCH, 1);
                                //input_sync(ts->input);
                                key_search_pressed = 0x1;
							}
						}else
						{
							/*key_search_pressed = 0x1;*/
							if(key_search_pressed)
							{
                                input_report_key(ts->input, KEY_SEARCH, 0);
                                //input_sync(ts->input);
                                key_search_pressed = 0x0;
							}
						}
						
						break;
					case 1:
						if(pressure)
						{
							/*if(key_back_pressed)
							{
								input_report_key(ts->input, KEY_BACK, 1);
								input_report_key(ts->input, KEY_BACK, 0);
								key_back_pressed = 0x0;
							}*/
							if(!key_back_pressed)
							{
                                input_report_key(ts->input, KEY_BACK, 1);
                                //input_sync(ts->input);
                                key_back_pressed = 0x1;
							}
						}else
						{
							/*key_back_pressed = 0x1;*/
							if(key_back_pressed)
							{
                                input_report_key(ts->input, KEY_BACK, 0);
                                //input_sync(ts->input);
                                key_back_pressed = 0x0;
							}
						}
						
						break;
					case 2:
						if(pressure)
						{
							//input_report_key(ts->input, KEY_HOME, 1);
							//input_report_key(ts->input, KEY_HOME, 0);
							if(!key_home_pressed)
							{
								printk("key_home is pressed ================\n");
                                input_report_key(ts->input, KEY_HOME, 1);
                                //input_sync(ts->input);
                                key_home_pressed = 0x1;
							}
						}else
						{
							if(key_home_pressed)
							{
                                input_report_key(ts->input, KEY_HOME, 0);
                                //input_sync(ts->input);
                                key_home_pressed = 0x0;
							}
						}
						
						break;
					case 3:
						if(pressure)
						{
							/*if(key_menu_pressed)
							{
								input_report_key(ts->input, KEY_MENU, 1);
								input_report_key(ts->input, KEY_MENU, 0);
								key_menu_pressed = 0x0;
							}*/
							if(!key_menu_pressed)
							{
                                input_report_key(ts->input, KEY_MENU, 1);
                                //input_sync(ts->input);
                                key_menu_pressed = 0x1;
							}
						}else
						{
							/*key_menu_pressed = 0x1;*/
							if(key_menu_pressed)
							{
                                input_report_key(ts->input, KEY_MENU, 0);
                                //input_sync(ts->input);
                                key_menu_pressed = 0x0;
							}
						}
						break;
					case 4:
						break;
					case 5:
						break;
					case 6:
						break;
					case 7:
						break;
				}

			}
		}

		input_sync(ts->input);		
	}

}

#if 1

void ts_write_charge_enable_cmd(void)
{
	printk("%s is running ==========",__FUNCTION__);
		
	struct zet6221_tsdrv *zet6221_ts;
	u8 ts_write_charge_cmd[1] = {0xb5}; 
	int ret=0;
	ret=zet6221_i2c_write_tsdata(this_client, ts_write_charge_cmd, 1);
}
EXPORT_SYMBOL_GPL(ts_write_charge_enable_cmd);


void ts_write_charge_disable_cmd()
{
	printk("%s is running ==========",__FUNCTION__);
	struct zet6221_tsdrv *zet6221_ts;
	u8 ts_write_cmd[1] = {0xb6}; 
	int ret=0;
	ret=zet6221_i2c_write_tsdata(this_client, ts_write_cmd, 1);
}
EXPORT_SYMBOL_GPL(ts_write_charge_disable_cmd);
#endif

static void ts_early_suspend(struct early_suspend *handler)
{
	//Sleep Mode
#if 1
	struct zet6221_tsdrv *zet6221_ts;
	u8 ts_sleep_cmd[1] = {0xb1}; 
	int ret=0;
	ret=zet6221_i2c_write_tsdata(this_client, ts_sleep_cmd, 1);
#endif

//	printk("early_suspend ===========\n");
	return;	        
}

static void ts_late_resume(struct early_suspend *handler)
{
#if 1
	resetCount = 1;
	ctp_ops.ts_wakeup();
	msleep(100);
#else
	struct zet6221_tsdrv *zet6221_ts;
	u8 ts_wakeup_cmd[1] = {0xb4};
	zet6221_i2c_write_tsdata(this_client, ts_wakeup_cmd, 1);
#endif

	ChargeChange = 0;
//	printk("early_resume ============\n");
	return;
}

/********************************************************/
#if 1

u8 zet6221_ts_sndpwd(struct i2c_client *client)
{
	u8 ts_sndpwd_cmd[3] = {0x20,0xC5,0x9D};
	
	int ret;

#if defined(I2C_CTPM_ADDRESS)
	ret=i2c_write_interface(I2C_CTPM_ADDRESS, ts_sndpwd_cmd, 3);
#else
	ret=zet6221_i2c_write_tsdata(client, ts_sndpwd_cmd, 3);
#endif
	
	return 1;
}

u8 zet6221_ts_sfr(struct i2c_client *client)
{
	u8 ts_cmd[1] = {0x2C};
	u8 ts_in_data[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	u8 ts_cmd17[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	u8 ts_sfr_data[16] = {0x18,0x76,0x27,0x27,0xFF,0x03,0x8E,0x14,0x00,0x38,0x82,0xEC,0x00,0x00,0x7d,0x03};
	int ret;
	int i;
	
#if 0
	u8 ts_cmd_27[1] = {0x27};
	u8 ts_in_data_27[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	
	u8 ts_sfr_cmd17[17] = {0x2b,0x18,0x86,0x27,0x27,0xff,0x03,0x8e,0x14,0x60,0x38,0x82,0xec,0x00,0x00,0x3d,0x03};
	ret=zet6221_i2c_write_tsdata(client, ts_sfr_cmd17, 17);
	msleep(100);
	
	ret=zet6221_i2c_write_tsdata(client, ts_cmd_27, 1);
	ret=zet6221_i2c_read_tsdata(client, ts_in_data_27, 16);
	printk("CO: %02x \n",ts_in_data_27[2]); 
#endif

	printk("\nwrite : "); 
#if defined(I2C_CTPM_ADDRESS)
	ret=i2c_write_interface(I2C_CTPM_ADDRESS, ts_cmd, 1);
#else
	ret=zet6221_i2c_write_tsdata(client, ts_cmd, 1);
#endif
	msleep(1);
	
	printk("%02x ",ts_cmd[0]); 
	
	printk("\nread : "); 
#if defined(I2C_CTPM_ADDRESS)
	ret=i2c_read_interface(I2C_CTPM_ADDRESS, ts_in_data, 16);
#else
	ret=zet6221_i2c_read_tsdata(client, ts_in_data, 16);
#endif
	msleep(1);

	for(i=0;i<16;i++)
	{
		ts_cmd17[i+1]=ts_in_data[i];
		printk("%02x ",ts_in_data[i]); 

#if 1
		if(i>1 && i<8)
		{
			if(ts_in_data[i]!=ts_sfr_data[i])
				return 0;
		}
#endif

	}
	printk("\n"); 


	if(ts_in_data[14]==0x3D)
	{
		/* 
		ts_cmd[0]=0x2D;		
		printk("write %02x\n",ts_cmd[0]); 
		ret=zet6221_i2c_write_tsdata(client, ts_cmd, 1);
		msleep(500);

		ts_cmd17[15]=0x7D;
		
		ts_cmd17[0]=0x2E;		
		printk("write \n"); 
		ret=zet6221_i2c_write_tsdata(client, ts_cmd17, 17);
		
		printk("ts_in_data[14]==0x3D\n"); 
		for(i=0;i<17;i++)
		{
			printk("%02x ",ts_cmd17[i]); 
		}
		printk("\n"); */
			
		ts_cmd17[i+1]=ts_in_data[i];
		
	}else
	{
		ts_cmd17[15]=0x3D;
		
		ts_cmd17[0]=0x2B;	
		
		printk("write \n"); 
#if defined(I2C_CTPM_ADDRESS)
		ret=i2c_write_interface(I2C_CTPM_ADDRESS, ts_cmd17, 17);
#else
		ret=zet6221_i2c_write_tsdata(client, ts_cmd17, 17);
#endif
	
		printk("ts_in_data[14]!=0x3D\n"); 
		for(i=0;i<17;i++)
		{
			printk("%02x ",ts_cmd17[i]); 
		}
		printk("\n"); 
	}
	
	return 1;
}

u8 zet6221_ts_masserase(struct i2c_client *client)
{
	u8 ts_cmd[1] = {0x24};
	
	int ret;

#if defined(I2C_CTPM_ADDRESS)
	ret=i2c_write_interface(I2C_CTPM_ADDRESS, ts_cmd, 1);
#else
	ret=zet6221_i2c_write_tsdata(client, ts_cmd, 1);
#endif
	
	return 1;
}

u8 zet6221_ts_pageerase(struct i2c_client *client,int npage)
{
	u8 ts_cmd[2] = {0x23,0x00};
	
	int ret;

	ts_cmd[1]=npage;
#if defined(I2C_CTPM_ADDRESS)
	ret=i2c_write_interface(I2C_CTPM_ADDRESS, ts_cmd, 2);
#else
	ret=zet6221_i2c_write_tsdata(client, ts_cmd, 2);
#endif
	
	return 1;
}

u8 zet6221_ts_resetmcu(struct i2c_client *client)
{
	u8 ts_cmd[1] = {0x29};
	
	int ret;

#if defined(I2C_CTPM_ADDRESS)
	ret=i2c_write_interface(I2C_CTPM_ADDRESS, ts_cmd, 1);
#else
	ret=zet6221_i2c_write_tsdata(client, ts_cmd, 1);
#endif
	
	return 1;
}

u8 zet6221_ts_hwcmd(struct i2c_client *client)
{
	u8 ts_cmd[1] = {0xB9};
	
	int ret;

#if defined(I2C_CTPM_ADDRESS)
	ret=i2c_write_interface(I2C_CTPM_ADDRESS, ts_cmd, 1);
#else
	ret=zet6221_i2c_write_tsdata(client, ts_cmd, 1);
#endif
	
	return 1;
}

u8 zet6221_ts_version()
{	
	int i;
	printk("pc: ");
	for(i=0;i<8;i++)
		printk("%02x ",pc[i]);
	printk("\n");
	
	printk("src: ");
	for(i=0;i<8;i++)
		printk("%02x ",zeitec_zet6221_firmware[fb[i]]);
	printk("\n");
	
	for(i=0;i<8;i++)
		if(pc[i]!=zeitec_zet6221_firmware[fb[i]])
			return 0;
			
	return 1;
}

int __init zet6221_downloader( struct i2c_client *client, unsigned short ver, unsigned char * data )
{
	int BufLen=0;
	int BufPage=0;
	int BufIndex=0;
	int ret;
	int i;
	
	int nowBufLen=0;
	int nowBufPage=0;
	int nowBufIndex=0;
	int retryCount=0;
	
begin_download:
	
#if defined(RSTPIN_ENABLE)
	//reset mcu
	/*msleep(5);
	gpio_direction_output(TS_RST_GPIO, 0);
	msleep(5);
	gpio_set_value(TS_RST_GPIO,GPIO_LOW);*/
	if(EGPIO_SUCCESS != gpio_write_one_pin_value(gpio_wakeup_hdle, 0, "ctp_wakeup")){
			pr_info("%s: err when operate gpio. \n", __func__);
	}
	msleep(20);
#else
	zet6221_ts_hwcmd(client);
	msleep(200);
#endif

	//send password
//	DPRINTK( "send passwd\n");
	zet6221_ts_sndpwd(client);
	msleep(200);

/*****compare version*******/

	//0~3
	memset(zeitec_zet6221_page_in,0x00,130);
	zeitec_zet6221_page_in[0]=0x25;
	zeitec_zet6221_page_in[1]=(fb[0] >> 7);//(fb[0]/128);
#if defined(I2C_CTPM_ADDRESS)
		ret=i2c_write_interface(I2C_CTPM_ADDRESS, zeitec_zet6221_page_in, 2);
#else
		ret=zet6221_i2c_write_tsdata(client, zeitec_zet6221_page_in, 2);
#endif
	
	zeitec_zet6221_page_in[0]=0x0;
	zeitec_zet6221_page_in[1]=0x0;
#if defined(I2C_CTPM_ADDRESS)
	ret=i2c_read_interface(I2C_CTPM_ADDRESS, zeitec_zet6221_page_in, 128);
#else
	ret=zet6221_i2c_read_tsdata(client, zeitec_zet6221_page_in, 128);
#endif

	printk("page=%d ",(fb[0] >> 7));//(fb[0]/128));
	for(i=0;i<4;i++)
	{
		pc[i]=zeitec_zet6221_page_in[(fb[i] & 0x7f)];//[(fb[i]%128)];
		printk("offset[%d]=%d ",i,(fb[i] & 0x7f));//(fb[i]%128));
	}
	printk("\n");
	
	/*
	printk("page=%d ",(fb[0] >> 7));
	for(i=0;i<4;i++)
	{
		printk("offset[%d]=%d ",i,(fb[i] & 0x7f));
	}
	printk("\n");
	*/
	
	//4~7
	memset(zeitec_zet6221_page_in,0x00,130);
	zeitec_zet6221_page_in[0]=0x25;
	zeitec_zet6221_page_in[1]=(fb[4] >> 7);//(fb[4]/128);
#if defined(I2C_CTPM_ADDRESS)
	ret=i2c_write_interface(I2C_CTPM_ADDRESS, zeitec_zet6221_page_in, 2);
#else
	ret=zet6221_i2c_write_tsdata(client, zeitec_zet6221_page_in, 2);
#endif
	
	zeitec_zet6221_page_in[0]=0x0;
	zeitec_zet6221_page_in[1]=0x0;
#if defined(I2C_CTPM_ADDRESS)
		ret=i2c_read_interface(I2C_CTPM_ADDRESS, zeitec_zet6221_page_in, 128);
#else
		ret=zet6221_i2c_read_tsdata(client, zeitec_zet6221_page_in, 128);
#endif

	printk("page=%d ",(fb[4] >> 7)); //(fb[4]/128));
	for(i=4;i<8;i++)
	{
		pc[i]=zeitec_zet6221_page_in[(fb[i] & 0x7f)]; //[(fb[i]%128)];
		printk("offset[%d]=%d ",i,(fb[i] & 0x7f));  //(fb[i]%128));
	}
	printk("\n");
	
	//page 127
	memset(zeitec_zet6221_page_in,0x00,130);
	zeitec_zet6221_page_in[0]=0x25;
	zeitec_zet6221_page_in[1]=127;
#if defined(I2C_CTPM_ADDRESS)
	ret=i2c_write_interface(I2C_CTPM_ADDRESS, zeitec_zet6221_page_in, 2);
#else
	ret=zet6221_i2c_write_tsdata(client, zeitec_zet6221_page_in, 2);
#endif
	
	zeitec_zet6221_page_in[0]=0x0;
	zeitec_zet6221_page_in[1]=0x0;
#if defined(I2C_CTPM_ADDRESS)
		ret=i2c_read_interface(I2C_CTPM_ADDRESS, zeitec_zet6221_page_in, 128);
#else
		ret=zet6221_i2c_read_tsdata(client, zeitec_zet6221_page_in, 128);
#endif

	for(i=0;i<128;i++)
	{
		if(0x3F80+i < sizeof(zeitec_zet6221_firmware)/sizeof(char))
		{
			if(zeitec_zet6221_page_in[i]!=zeitec_zet6221_firmware[0x3F80+i])
			{
				printk("page 127 [%d] doesn't match! continue to download!\n",i);
				goto proc_sfr;
			}
		}
	}

	if(zet6221_ts_version()!=0)
		goto exit_download;
		
/*****compare version*******/

proc_sfr:
	//sfr
//	DPRINTK( "check sfr\n");
	if(zet6221_ts_sfr(client)==0)
	{

#if 1

#if defined(RSTPIN_ENABLE)
	
		if(EGPIO_SUCCESS != gpio_write_one_pin_value(gpio_wakeup_hdle, 1, "ctp_wakeup")){
				pr_info("%s: err when operate gpio. \n", __func__);
		}
		msleep(20);
		
		if(EGPIO_SUCCESS != gpio_write_one_pin_value(gpio_wakeup_hdle, 0, "ctp_wakeup")){
			pr_info("%s: err when operate gpio. \n", __func__);
		}
		msleep(20);
		
		if(EGPIO_SUCCESS != gpio_write_one_pin_value(gpio_wakeup_hdle, 1, "ctp_wakeup")){
				pr_info("%s: err when operate gpio. \n", __func__);
		}
#else
		zet6221_ts_resetmcu(client);
#endif	
		msleep(20);
		goto begin_download;
		
#endif

	}
	msleep(20);
	
	//erase
	if(BufLen==0)
	{
		//mass erase
		//DPRINTK( "mass erase\n");
		zet6221_ts_masserase(client);
		msleep(200);

		BufLen=sizeof(zeitec_zet6221_firmware)/sizeof(char);
	}else
	{
		zet6221_ts_pageerase(client,BufPage);
		msleep(200);
	}
	
	
	while(BufLen>0)
	{
download_page:

		memset(zeitec_zet6221_page,0x00,130);
		
		DPRINTK( "Start: write page%d\n",BufPage);
		nowBufIndex=BufIndex;
		nowBufLen=BufLen;
		nowBufPage=BufPage;
		
		if(BufLen>128)
		{
			for(i=0;i<128;i++)
			{
				zeitec_zet6221_page[i+2]=zeitec_zet6221_firmware[BufIndex];
				BufIndex+=1;
			}
			zeitec_zet6221_page[0]=0x22;
			zeitec_zet6221_page[1]=BufPage;
			BufLen-=128;
		}
		else
		{
			for(i=0;i<BufLen;i++)
			{
				zeitec_zet6221_page[i+2]=zeitec_zet6221_firmware[BufIndex];
				BufIndex+=1;
			}
			zeitec_zet6221_page[0]=0x22;
			zeitec_zet6221_page[1]=BufPage;
			BufLen=0;
		}
//		DPRINTK( "End: write page%d\n",BufPage);

#if defined(I2C_CTPM_ADDRESS)
		ret=i2c_write_interface(I2C_CTPM_ADDRESS, zeitec_zet6221_page, 130);
#else
		ret=zet6221_i2c_write_tsdata(client, zeitec_zet6221_page, 130);
#endif
		msleep(200);
		
#if 1

		memset(zeitec_zet6221_page_in,0x00,130);
		zeitec_zet6221_page_in[0]=0x25;
		zeitec_zet6221_page_in[1]=BufPage;
#if defined(I2C_CTPM_ADDRESS)
		ret=i2c_write_interface(I2C_CTPM_ADDRESS, zeitec_zet6221_page_in, 2);
#else
		ret=zet6221_i2c_write_tsdata(client, zeitec_zet6221_page_in, 2);
#endif
	
		zeitec_zet6221_page_in[0]=0x0;
		zeitec_zet6221_page_in[1]=0x0;
#if defined(I2C_CTPM_ADDRESS)
		ret=i2c_read_interface(I2C_CTPM_ADDRESS, zeitec_zet6221_page_in, 128);
#else
		ret=zet6221_i2c_read_tsdata(client, zeitec_zet6221_page_in, 128);
#endif
		
		for(i=0;i<128;i++)
		{
			if(i < nowBufLen)
			{
				if(zeitec_zet6221_page[i+2]!=zeitec_zet6221_page_in[i])
				{
					BufIndex=nowBufIndex;
					BufLen=nowBufLen;
					BufPage=nowBufPage;
				
					if(retryCount < 5)
					{
						retryCount++;
						goto download_page;
					}else
					{
						//BufIndex=0;
						//BufLen=0;
						//BufPage=0;
						retryCount=0;
						
#if defined(RSTPIN_ENABLE)
						if(EGPIO_SUCCESS != gpio_write_one_pin_value(gpio_wakeup_hdle, 1, "ctp_wakeup")){
							pr_info("%s: err when operate gpio. \n", __func__);
						}
						msleep(20);
		
						if(EGPIO_SUCCESS != gpio_write_one_pin_value(gpio_wakeup_hdle, 0, "ctp_wakeup")){
							pr_info("%s: err when operate gpio. \n", __func__);
						}
						msleep(20);
		
						if(EGPIO_SUCCESS != gpio_write_one_pin_value(gpio_wakeup_hdle, 1, "ctp_wakeup")){
							pr_info("%s: err when operate gpio. \n", __func__);
						}
#else
						zet6221_ts_resetmcu(client);
#endif	
						msleep(20);
						goto begin_download;
					}

				}
			}
		}
		
#endif
		retryCount=0;
		BufPage+=1;
	}

exit_download:

#if defined(RSTPIN_ENABLE)
//	gpio_set_value(TS_RST_GPIO,GPIO_HIGH);
//	msleep(100);
	
	if(EGPIO_SUCCESS != gpio_write_one_pin_value(gpio_wakeup_hdle, 1, "ctp_wakeup")){
			pr_info("%s: err when operate gpio. \n", __func__);
	}
	msleep(200);

#endif

	zet6221_ts_resetmcu(client);
	msleep(100);

}


#endif
/********************************************************/

static int __devinit zet6221_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int result;
	struct input_dev *input_dev;
	struct zet6221_tsdrv *zet6221_ts;

	struct i2c_dev *i2c_dev;
	struct device *dev;
	
	unsigned short ver;
	unsigned char * data;
	int count=0;

//	DPRINTK( "[TS] zet6221_ts_probe \n");
	zet6221_ts = kzalloc(sizeof(struct zet6221_tsdrv), GFP_KERNEL);
	zet6221_ts->i2c_ts = client;
	//zet6221_ts->gpio = S3C64XX_GPN(9); /*s3c6410*/
	//zet6221_ts->gpio = TS1_INT_GPIO;
	
	this_client = client;
	
	i2c_set_clientdata(client, zet6221_ts);

	client->driver = &zet6221_ts_driver;

	INIT_WORK(&zet6221_ts->work1, zet6221_ts_work);
	zet6221_ts->ts_workqueue = create_singlethread_workqueue(dev_name(&client->dev)); 
	if (!zet6221_ts->ts_workqueue) {
	//	err = -ESRCH;
		printk("ts_workqueue ts_probe error ==========\n");
		return;
	}

    INIT_WORK(&zet6221_ts->work2, write_cmd_work);
	zet6221_ts->ts_workqueue1 = create_singlethread_workqueue(dev_name(&client->dev)); //zhongwei++ workqueue
	if (!zet6221_ts->ts_workqueue1) {
	//	err = -ESRCH;
		printk("ts_workqueue1 ts_probe error ==========\n");
		return;
	}
	
	input_dev = input_allocate_device();
	if (!input_dev || !zet6221_ts) {
		result = -ENOMEM;
		goto fail_alloc_mem;
	}
	
	i2c_set_clientdata(client, zet6221_ts);

	input_dev->name = MJ5_TS_NAME;
	input_dev->phys = "zet6221_touch/input0";
	input_dev->id.bustype = BUS_HOST;
	input_dev->id.vendor = 0x0001;
	input_dev->id.product = 0x0002;
	input_dev->id.version = 0x0100;
	
zet_download:
	zet6221_downloader(client,ver,data);
	
#if defined(TPINFO)
	udelay(100);
	zet6221_ts_get_report_mode(client);
	 
	count=0;
	do{
		//wakeup pin for reset in zeitec6221
		ctp_ops.ts_wakeup();  
	
		zet6221_ts_get_report_mode_t(client);
		if(zet6221_ts_version()!=0)
			break;
		count++;
	}while(count<REPORT_POLLING_TIME);
	
	if(count==REPORT_POLLING_TIME)
		goto zet_download;
	
#else
	ResolutionX = X_MAX;
	ResolutionY = Y_MAX;
	FingerNum = FINGER_NUMBER;
	KeyNum = KEY_NUMBER;
	if(KeyNum==0)
		bufLength  = 3+4*FingerNum;
	else
		bufLength  = 3+4*FingerNum+1;
#endif

//	DPRINTK( "ResolutionX=%d ResolutionY=%d FingerNum=%d KeyNum=%d\n",ResolutionX,ResolutionY,FingerNum,KeyNum);

#if 0
	while(zet6221_ts_version()==0)
	{
		zet6221_downloader(client,ver,data);
#if defined(RSTPIN_ENABLE)
		//wakeup pin for reset in zeitec6221
		ctp_ops.ts_wakeup();
#endif
		zet6221_ts_get_report_mode_t(client);
	}
	
#endif
        set_bit(ABS_MT_TOUCH_MAJOR, input_dev->absbit); 
        set_bit(ABS_MT_POSITION_X, input_dev->absbit); 
        set_bit(ABS_MT_POSITION_Y, input_dev->absbit); 
        set_bit(ABS_MT_WIDTH_MAJOR, input_dev->absbit); 
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, P_MAX, 0, 0);
	//if(ResolutionX==0 && ResolutionY==0)
	//{		
	//	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, X_MAX, 0, 0);
	//	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, Y_MAX, 0, 0);
	//}else
	//{
		input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, ResolutionX, 0, 0);
		input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, ResolutionY, 0, 0);
	//}	
	//input_set_abs_params(input_dev, ABS_MT_WIDTH_MAJOR, 0, 10, 0, 0);

	set_bit(KEY_BACK, input_dev->keybit);
	set_bit(KEY_MENU, input_dev->keybit);
	set_bit(KEY_HOME, input_dev->keybit);
	set_bit(KEY_SEARCH, input_dev->keybit);

	input_dev->evbit[0] = BIT(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
	//input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
	//input_dev->keybit[BIT_WORD (KEY_HOME)] = BIT_MASK(KEY_HOME)|BIT_MASK(KEY_BACK);

	result = input_register_device(input_dev);
	if (result)
		goto fail_ip_reg;

//config early_suspend
	pr_info("==register_early_suspend =\n");
	zet6221_ts->early_suspend.suspend = ts_early_suspend;
	zet6221_ts->early_suspend.resume = ts_late_resume;
	register_early_suspend(&zet6221_ts->early_suspend);
//end config early_suspend

	zet6221_ts->input = input_dev;

	input_set_drvdata(zet6221_ts->input, zet6221_ts);


	setup_timer(&zet6221_ts->polling_timer, polling_timer_func, (unsigned long)zet6221_ts);
	mod_timer(&zet6221_ts->polling_timer,jiffies + msecs_to_jiffies(800));
	
	int err;
	err = ctp_ops.set_irq_mode("ctp_para", "ctp_int_port", CTP_IRQ_MODE);
	if(0 != err){
		pr_info("%s:ctp_ops.set_irq_mode err. \n", __func__);
		goto exit_set_irq_mode;
	}

	err = request_irq(SW_INT_IRQNO_PIO, zet6221_ts_interrupt, IRQF_TRIGGER_FALLING | IRQF_SHARED, ZET_TS_ID_NAME, zet6221_ts);
   
    
	if (err < 0) {
		//dev_err(&client->dev, "ft5x_ts_probe: request irq failed\n");
//		DPRINTK( "[TS] zet6221_ts_probe.request_irq failed. err=\n",err);
		goto exit_irq_request_failed;
	}

    i2c_dev = get_free_i2c_dev(client->adapter);	
	if (IS_ERR(i2c_dev)){	
		err = PTR_ERR(i2c_dev);		
		return err;	
	}
	dev = device_create(i2c_dev_class, &client->adapter->dev, MKDEV(I2C_MAJOR,client->adapter->nr), NULL, "aw_i2c_ts%d", client->adapter->nr);	
	if (IS_ERR(dev))	{		
			err = PTR_ERR(dev);		
			return err;	
	}

	return 0;

//request_irq_fail:
//	gpio_free(zet6221_ts->gpio);
gpio_request_fail:
	free_irq(zet6221_ts->irq, zet6221_ts);
	input_unregister_device(input_dev);
	input_dev = NULL;
fail_ip_reg:
fail_alloc_mem:
	input_free_device(input_dev);
	kfree(zet6221_ts);
	return result;

//////////////////////////////
exit_irq_request_failed:
exit_set_irq_mode:
	enable_irq(SW_INT_IRQNO_PIO);
exit_input_register_device_failed:
	input_free_device(input_dev);
exit_input_dev_alloc_failed:
	free_irq(SW_INT_IRQNO_PIO, zet6221_ts);
exit_create_singlethread:
	pr_info("==singlethread error =\n");
	i2c_set_clientdata(client, NULL);
	kfree(zet6221_ts);
exit_alloc_data_failed:
exit_check_functionality_failed:
	return err;
}

static int __devexit zet6221_ts_remove(struct i2c_client *dev)
{
	struct zet6221_tsdrv *zet6221_ts = i2c_get_clientdata(dev);
#if 0 

	free_irq(zet6221_ts->irq, zet6221_ts);
	gpio_free(zet6221_ts->gpio);
	del_timer_sync(&zet6221_ts->polling_timer);
	input_unregister_device(zet6221_ts->input);
	kfree(zet6221_ts);
#endif 
		
	del_timer_sync(&write_timer);
	pr_info("==zet6221_ts_remove=\n");
	free_irq(SW_INT_IRQNO_PIO, zet6221_ts);

//CONFIG HAS_EARLYSUSPEND
	unregister_early_suspend(&zet6221_ts->early_suspend);
//END CONFIG HAS_EARLYSUSPEND

	input_unregister_device(zet6221_ts->input);
	input_free_device(zet6221_ts->input);
	destroy_workqueue(zet6221_ts->ts_workqueue); 
	kfree(zet6221_ts);
    
	i2c_set_clientdata(dev, NULL);
	ctp_ops.free_platform_resource();

	return 0;
}

static int aw_open(struct inode *inode, struct file *file)
{
	int subminor;
	int ret = 0;	
	struct i2c_client *client;
	struct i2c_adapter *adapter;	
	struct i2c_dev *i2c_dev;	

	pr_info("====%s======.\n", __func__);
	
	#ifdef AW_DEBUG	
	        pr_info("enter aw_open function\n");
	#endif	
	
	subminor = iminor(inode);
	#ifdef AW_DEBUG	
	      pr_info("subminor=%d\n",subminor);
	#endif	
	
	//lock_kernel();	
	i2c_dev = i2c_dev_get_by_minor(2);	
	if (!i2c_dev)	{	
		pr_info("error i2c_dev\n");		
		return -ENODEV;	
	}
	
	adapter = i2c_get_adapter(i2c_dev->adap->nr);	
	if (!adapter)	{		
		return -ENODEV;	
	}	
	
	client = kzalloc(sizeof(*client), GFP_KERNEL);	
	
	if (!client)	{		
		i2c_put_adapter(adapter);		
		ret = -ENOMEM;	
	}	
	snprintf(client->name, I2C_NAME_SIZE, "pctp_i2c_ts%d", adapter->nr);
	client->driver = &zet6221_ts_driver;
	client->adapter = adapter;		
	file->private_data = client;
		
	return 0;
}

static int aw_release (struct inode *inode, struct file *file) 
{
	struct i2c_client *client = file->private_data;
	#ifdef AW_DEBUG
	    pr_info("enter aw_release function.\n");
	#endif
	
	i2c_put_adapter(client->adapter);
	kfree(client);
	file->private_data = NULL;
	return 0;	  
}

static const struct file_operations aw_i2c_ts_fops ={	
	.owner = THIS_MODULE, 	
	.open = aw_open, 	
	.release = aw_release, 
};

static int __init zet6221_ts_init(void)
{
#if 1 

	int ret = -1;
	int err = -1;

	pr_info("===========================%s=====================\n", __func__);

	if (ctp_ops.fetch_sysconfig_para)
	{
		if(ctp_ops.fetch_sysconfig_para()){
			pr_info("%s: err.\n", __func__);
			return -1;
		}
	}
	pr_info("%s: after fetch_sysconfig_para:  normal_i2c: 0x%hx. normal_i2c[1]: 0x%hx \n", \
	__func__, normal_i2c[0], normal_i2c[1]);

	err = ctp_ops.init_platform_resource();
	if(0 != err){
	    pr_info("%s:ctp_ops.init_platform_resource err. \n", __func__);    
	}

	//reset
	ctp_ops.ts_reset();
	//wakeup
	ctp_ops.ts_wakeup();  
	
	zet6221_ts_driver.detect = ctp_ops.ts_detect;

	ret= register_chrdev(I2C_MAJOR,"aw_i2c_ts",&aw_i2c_ts_fops );	
	if(ret) {	
		pr_info(KERN_ERR "%s:register chrdev failed\n",__FILE__);	
		return ret;
	}
	
	i2c_dev_class = class_create(THIS_MODULE,"aw_i2c_dev");
	if (IS_ERR(i2c_dev_class)) {		
		ret = PTR_ERR(i2c_dev_class);		
		class_destroy(i2c_dev_class);	
	}

#endif

//	setup_timer(&write_timer, polling_timer_func, 0);
//	mod_timer(&write_timer,jiffies + msecs_to_jiffies(200));
	i2c_add_driver(&zet6221_ts_driver);

//	return 0;
	return ret;
}
module_init(zet6221_ts_init);

static void __exit zet6221_ts_exit(void)
{
    i2c_del_driver(&zet6221_ts_driver);
}
module_exit(zet6221_ts_exit);

void zet6221_set_ts_mode(u8 mode)
{
//	DPRINTK( "[Touch Screen]ts mode = %d \n", mode);
}
EXPORT_SYMBOL_GPL(zet6221_set_ts_mode);


MODULE_DESCRIPTION("ZET6221 I2C Touch Screen driver");
MODULE_LICENSE("GPL v2");
