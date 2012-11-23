#include <linux/input.h>
#include <linux/ctp.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <mach/gpio.h> 

static int int_cfg_addr[]={
        PIO_INT_CFG0_OFFSET,
        PIO_INT_CFG1_OFFSET,
	PIO_INT_CFG2_OFFSET,
	PIO_INT_CFG3_OFFSET,
};
static void* __iomem gpio_addr = NULL;

struct ctp_config_info config_info;
EXPORT_SYMBOL_GPL(config_info);

bool ctp_get_int_enable(u32 *enable)
{
        int ret = -1;
        ret = sw_gpio_eint_get_enable(CTP_IRQ_NUMBER,enable);
        if(ret != 0){
                return false;
        }
        return true;
}
EXPORT_SYMBOL(ctp_get_int_enable);
bool ctp_set_int_enable(u32 enable)
{
        u32 sta_enable;
        int ret = -1;
        if((enable != 0) || (enable != 1)){
                return false;
        }
        ret = ctp_get_int_enable(&sta_enable);
        if(ret == true){
                if(sta_enable == enable)
                        return true;
        }
        ret = sw_gpio_eint_set_enable(CTP_IRQ_NUMBER,enable);
        if(ret != 0){
                return false;
        }
                return true;       
}
EXPORT_SYMBOL(ctp_set_int_enable);

bool ctp_get_int_port_rate(u32 *clk)
{
        struct gpio_eint_debounce pdbc = {0,0};
        int ret = -1;
        ret = sw_gpio_eint_get_debounce(CTP_IRQ_NUMBER,&pdbc);
        if(ret != 0){
                return false;
        }
        *clk = pdbc.clk_sel;
        return true;
}
EXPORT_SYMBOL(ctp_get_int_port_rate);

bool ctp_set_int_port_rate(u32 clk)
{
        struct gpio_eint_debounce pdbc;
        int ret = -1;
        u32 sta_clk;
        if((clk != 0) || (clk != 1)){
                return false;
        }
        ret = ctp_get_int_port_rate(&sta_clk);
        if(ret ==0){
                if(sta_clk == clk){
                        return true;
                }
        }
        pdbc.clk_sel = clk;
        ret = sw_gpio_eint_set_debounce(CTP_IRQ_NUMBER,pdbc);
        if(ret != 0){
                return false;
        }
        return true;
}
EXPORT_SYMBOL(ctp_set_int_port_rate);
bool ctp_get_int_port_deb(u32 *clk_pre_scl)
{
        struct gpio_eint_debounce pdbc = {0,0};
        int ret = -1;
        ret = sw_gpio_eint_get_debounce(CTP_IRQ_NUMBER,&pdbc);
        if(ret !=0){
                return false;
        }
        *clk_pre_scl = pdbc.clk_pre_scl;
        return true;
} 
EXPORT_SYMBOL(ctp_get_int_port_deb);
bool ctp_set_int_port_deb(u32 clk_pre_scl)
{
        struct gpio_eint_debounce pdbc;
        int ret = -1;
        u32 sta_clk_pre_scl;
        ret = ctp_get_int_port_rate(&sta_clk_pre_scl);
        if(ret ==0){
                if(sta_clk_pre_scl == clk_pre_scl){
                        return true;
                }
        }
        pdbc.clk_pre_scl = clk_pre_scl;
        ret = sw_gpio_eint_set_debounce(CTP_IRQ_NUMBER,pdbc);
        if(ret != 0){
                return false;
        }
        return true;        
}
EXPORT_SYMBOL(ctp_set_int_port_deb);  
/**
 * ctp_judge_int_occur - whether interrupt occur.
 *
 * return value: 
 *              0:      int occur;
 *              others: no int occur; 
 */
int ctp_judge_int_occur(int number)
{
	//int reg_val[3];
	int reg_val;
	int ret = -1;

	reg_val = readl(gpio_addr + PIO_INT_STAT_OFFSET);
	if(reg_val&(1<<(number))){
		ret = 0;
	}
	printk("sta irq mode reg :0x%x\n",PIO_INT_STAT_OFFSET);
	return ret; 	
}
EXPORT_SYMBOL(ctp_judge_int_occur);

/**
 * ctp_clear_penirq - clear int pending
 *
 */
void ctp_clear_penirq(int number)
{
	int reg_val;

	reg_val = readl(gpio_addr + PIO_INT_STAT_OFFSET);

	if((reg_val = (reg_val&(1<<(number))))){
		printk("==CTP_IRQ_NO:%d=\n",number);              
		writel(reg_val,gpio_addr + PIO_INT_STAT_OFFSET);
	}
	printk("sta irq mode reg :0x%x\n",PIO_INT_STAT_OFFSET);
	return;
}
EXPORT_SYMBOL(ctp_clear_penirq);
int ctp_set_irq_mode(char *major_key , char *subkey, ext_int_mode int_mode)
{
	int ret = 0;
	__u32 reg_num = 0;
	__u32 reg_addr = 0;
	__u32 reg_val = 0;
	user_gpio_set_t  gpio_int_info[1];
	//config gpio to int mode
	pr_info("%s: config gpio to int mode. \n", __func__);
        if(config_info.gpio_int_hdle){
		sw_gpio_release(config_info.gpio_int_hdle, 2);
	}
	
	config_info.gpio_int_hdle = sw_gpio_request_ex(major_key, subkey);
	if(!config_info.gpio_int_hdle){
		pr_info("request tp_int_port failed. \n");
		ret = -1;
		goto request_tp_int_port_failed;
	}
	
	sw_gpio_get_one_pin_status(config_info.gpio_int_hdle, gpio_int_info, subkey, 1);
	pr_info("%s, %d: gpio_int_info, port = %d, port_num = %d. \n", __func__, __LINE__, \
		gpio_int_info[0].port, gpio_int_info[0].port_num);
        
        config_info.irq_number = gpio_int_info[0].port_num;

#ifdef AW_GPIO_INT_API_ENABLE
#else
	pr_info(" INTERRUPT CONFIG\n");
	reg_num = (gpio_int_info[0].port_num)%8;
	reg_addr = (gpio_int_info[0].port_num)/8;
	reg_val = readl(gpio_addr + int_cfg_addr[reg_addr]);
	printk("set irq mode reg :0x%x\n",int_cfg_addr[reg_addr]);
	reg_val &= (~(7 << (reg_num * 4)));
	reg_val |= (int_mode << (reg_num * 4));
	writel(reg_val,gpio_addr+int_cfg_addr[reg_addr]);
                                                               
	ctp_clear_penirq(gpio_int_info[0].port_num);
                                                               
	reg_val = readl(gpio_addr+PIO_INT_CTRL_OFFSET); 
	reg_val |= (1 << (gpio_int_info[0].port_num));
	writel(reg_val,gpio_addr+PIO_INT_CTRL_OFFSET);
	printk("enable irq mode reg :0x%x\n",PIO_INT_CTRL_OFFSET);

	udelay(1);
#endif

request_tp_int_port_failed:
	return ret;  
}
EXPORT_SYMBOL(ctp_set_irq_mode);
   
void ctp_free_platform_resource(void)
{
	if(config_info.gpio_wakeup_hdle){
		sw_gpio_release(config_info.gpio_wakeup_hdle, 2);
	}
	
	return;
}
EXPORT_SYMBOL(ctp_free_platform_resource);

/**
 * ctp_init_platform_resource - initialize platform related resource
 * return value: 0 : success
 *               -EIO :  i/o err.
 *
 */
int ctp_init_platform_resource(void)
{
	int ret = 0;	
	
	config_info.gpio_wakeup_enable = 1;
	config_info.gpio_wakeup_hdle = 0;
	config_info.gpio_int_hdle = 0;
	
	gpio_addr = ioremap(PIO_BASE_ADDRESS, PIO_RANGE_SIZE);
	if(!gpio_addr) {
                printk("gpio_addr ! fail!\n");
	}
	config_info.gpio_wakeup_hdle = sw_gpio_request_ex("ctp_para", "ctp_wakeup");
	if(!config_info.gpio_wakeup_hdle) {
		pr_info("%s: tp_wakeup request gpio fail!\n", __func__);
		config_info.gpio_wakeup_enable = 0;
	}
	
        sw_gpio_set_one_pin_io_status(config_info.gpio_wakeup_hdle, 1, "ctp_wakeup");
             
	return ret;
}
EXPORT_SYMBOL(ctp_init_platform_resource);

void ctp_print_info(struct ctp_config_info info)
{
        pr_info("info.ctp_used:%d\n",info.ctp_used);
        pr_info("info.irq_number:%d\n",info.irq_number);
        pr_info("info.twi_id:%d\n",info.twi_id);
        pr_info("info.screen_max_x:%d\n",info.screen_max_x);
        pr_info("info.screen_max_y:%d\n",info.screen_max_y);
        pr_info("info.revert_x_flag:%d",info.revert_x_flag);
        pr_info("info.revert_y_flag:%d",info.revert_y_flag);
        pr_info("info.exchange_x_y_flag:%d",info.exchange_x_y_flag);         
}
EXPORT_SYMBOL(ctp_print_info);
/**
 * ctp_fetch_sysconfig_para - get config info from sysconfig.fex file.
 * return value:  
 *                    = 0; success;
 *                    < 0; err
 */
static int ctp_fetch_sysconfig_para(void)
{
	int ret = -1;

	pr_info("%s. \n", __func__);

	if(SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_used", &config_info.ctp_used, 1)){
		pr_err("%s: script_parser_fetch err. \n", __func__);
		goto script_parser_fetch_err;
	}
	if(1 != config_info.ctp_used){
		pr_err("%s: ctp_unused. \n",  __func__);
		ret = 1;
		return ret;
	}

	if(SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_twi_id", &config_info.twi_id, sizeof(config_info.twi_id)/sizeof(__u32))){
		pr_err("%s: ctp_twi_id script_parser_fetch err. \n",__func__ );
		goto script_parser_fetch_err;
	}
	pr_info("%s: ctp_twi_id is %d. \n", __func__, config_info.twi_id);
	
	if(SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_screen_max_x", &config_info.screen_max_x, 1)){
		pr_err("%s: ctp_screen_max_x script_parser_fetch err. \n", __func__);
		goto script_parser_fetch_err;
	}
	pr_info("%s: screen_max_x = %d. \n", __func__, config_info.screen_max_x);

	if(SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_screen_max_y", &config_info.screen_max_y, 1)){
		pr_err("%s: ctp_screen_max_y script_parser_fetch err. \n", __func__);
		goto script_parser_fetch_err;
	}
	pr_info("%s: screen_max_y = %d. \n", __func__, config_info.screen_max_y);

	if(SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_revert_x_flag", &config_info.revert_x_flag, 1)){
		pr_err("%s: ctp_revert_x_flag script_parser_fetch err. \n", __func__);
		goto script_parser_fetch_err;
	}
	pr_info("%s: revert_x_flag = %d. \n", __func__, config_info.revert_x_flag);

	if(SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_revert_y_flag", &config_info.revert_y_flag, 1)){
		pr_err("%s: ctp_revert_y_flag script_parser_fetch err. \n", __func__);
		goto script_parser_fetch_err;
	}
	pr_info("%s: revert_y_flag = %d. \n", __func__, config_info.revert_y_flag);

	if(SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_exchange_x_y_flag", &config_info.exchange_x_y_flag, 1)){
		pr_err("%s: ctp_exchange_x_y_flag script_parser_fetch err. \n",__func__);
		goto script_parser_fetch_err;
	}
	pr_info("%s: exchange_x_y_flag = %d. \n", __func__, config_info.exchange_x_y_flag);

	return 0;

script_parser_fetch_err:
	pr_notice("=========script_parser_fetch_err============\n");
	return ret;
}

/**
 * ctp_wakeup - function
 *
 */
void ctp_wakeup(int status,int ms)
{
	if(1 == config_info.gpio_wakeup_enable){  
	        
		pr_info("%s. \n", __func__);
		if(EGPIO_SUCCESS != sw_gpio_write_one_pin_value(config_info.gpio_wakeup_hdle, 0, "ctp_wakeup")){
			pr_info("%s: err when operate gpio. \n", __func__);
		}
		mdelay(100);                                                       
		if(EGPIO_SUCCESS != sw_gpio_write_one_pin_value(config_info.gpio_wakeup_hdle, 1, "ctp_wakeup")){
			pr_info("%s: err when operate gpio. \n", __func__);
		}
		mdelay(100);

	}
	return;
}
EXPORT_SYMBOL(ctp_wakeup);

static int __init ctp_init(void)
{
	int err = -1;
	
	printk("==ctp_init==\n");
	printk("****************************************************************\n");
	if (ctp_fetch_sysconfig_para()){
	        printk("%s: ctp_fetch_sysconfig_para err.\n", __func__);
		return 0;
	}else{
                err = ctp_init_platform_resource();
        	if(0 != err){
        		printk("%s:ctp_ops.init_platform_resource err. \n", __func__);    
        	}
        }
        ctp_print_info(config_info);
        printk("****************************************************************\n");
        return 0;
}
static void __exit ctp_exit(void)
{
	ctp_free_platform_resource();
	return;
}

module_init(ctp_init);
module_exit(ctp_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("ctp init");
MODULE_AUTHOR("Olina yin");
MODULE_ALIAS("platform:AW");