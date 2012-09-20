/*
 * bcm40181 sdio wifi power management API
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <mach/sys_config.h>
#include "../../power/axp_power/axp-gpio.h"
#include <linux/regulator/consumer.h>

#include "mmc_pm.h"

#define bcm40181_msg(...)    do {printk("[bcm40181]: "__VA_ARGS__);} while(0)

static int bcm40181_powerup = 0;
static int bcm40181_suspend = 0;

/*add by chenjd,chenjd@allwinnertech.com,20120822
* use two axp io to control the power of wifi module
* 1.the axp io handler must be got when wifi gpio init
* 2.enable the axp gpio when wifi want to get power
* 3.disable the axp gpio when wifi want to lost power
* 4.free pin when wifi exit;
*/
static user_gpio_set_t drv_vbus_gpio_set1;
static user_gpio_set_t drv_vbus_gpio_set2;
static u32 axp_handler_1 = 0;
static u32 axp_handler_2 = 0;
#define  USB_EXTERN_PIN_LDO_MASK     200
static u32 alloc_pin(user_gpio_set_t *gpio_list){
	u32 pin_handle = 0;
  char name[32];
  memset(name, 0, 32);
  
  if(gpio_list->port == 0xffff){  //axp,
        if(gpio_list->port_num >= USB_EXTERN_PIN_LDO_MASK){ //port num is above 200
  #ifdef  CONFIG_REGULATOR
				#ifdef CONFIG_AW_AXP15
            switch((gpio_list->port_num - USB_EXTERN_PIN_LDO_MASK)){
                case 1:
                    strcpy(name, "axp15_rtc");
                break;

                case 2:
                    strcpy(name, "axp15_analog/fm");
                break;
                
                case 3:
                    strcpy(name, "axp15_analog/fm2");
                break;
                
                case 4:
                    strcpy(name, "axp15_pll/sdram");
                break;

                case 5:
                    strcpy(name, "axp15_pll/hdmi");
                break;

                default:
                    printk("ERR: unkown gpio_list->port_num(%d)\n", gpio_list->port_num);
                    goto failed;
            }
				#else
						switch((gpio_list->port_num - USB_EXTERN_PIN_LDO_MASK)){
                case 1:
                    strcpy(name, "axp20_rtc");
                break;

                case 2:
                    strcpy(name, "axp20_analog/fm");
                break;

                case 3:
                    strcpy(name, "axp20_pll");
                break;

                case 4:
                    strcpy(name, "axp20_hdmi");
                break;

                default:
                    printk("ERR: unkown gpio_list->port_num(%d)\n", gpio_list->port_num);
                    goto failed;
            }
        #endif
        
            pin_handle = (u32)regulator_get(NULL, name);
            if(pin_handle == 0){
                printk("ERR: regulator_get failed\n");
                return 0;
            }
						
						printk("get AXP handle for axp port num above 200,handle is %x\n",pin_handle);
            regulator_force_disable((struct regulator*)pin_handle);
  #else
  					printk("CONFIG_REGULATOR is not define\n");
  #endif
        }else{
        		axp_gpio_set_io(gpio_list->port_num, gpio_list->mul_sel);
            axp_gpio_set_value(gpio_list->port_num, gpio_list->data);
            printk("get AXP handle for axp port num is %d\n",100+gpio_list->port_num);
            return (100 + gpio_list->port_num);
        }
  }
failed:
	return pin_handle;
}

static void free_axp_pin(u32 pin_handle, user_gpio_set_t *gpio_list){
	if(pin_handle){
		if(gpio_list->port == 0xffff) { //AXP
			if(gpio_list->port_num >= USB_EXTERN_PIN_LDO_MASK){
			#ifdef  CONFIG_REGULATOR
				printk("free axp pin");
				regulator_force_disable((struct regulator*)pin_handle);
			#else
				printk("CONFIG_REGULATOR is not define\n");
			#endif
			}
			else{
				axp_gpio_set_io(gpio_list->port_num, gpio_list->mul_sel);
        axp_gpio_set_value(gpio_list->port_num, gpio_list->data);
			}	
		}
	}
	return;
}

static void init_axp(){
	int ret = 0;
	ret = script_parser_fetch("sdio_wifi_power_para", "wifi_power_1", (int *)&drv_vbus_gpio_set1, 64);
	if(ret != 0){
		printk("Err: request wifi power 1 fail");
		return;	
	}
	ret = script_parser_fetch("sdio_wifi_power_para", "wifi_power_2", (int *)&drv_vbus_gpio_set2, 64);
	if(ret != 0){
		printk("Err: request wifi power 2 fail");
		return;	
	}
	axp_handler_1 = alloc_pin(&drv_vbus_gpio_set1);
	axp_handler_2 = alloc_pin(&drv_vbus_gpio_set2);
}


static void set_axp_power(u32 handler, user_gpio_set_t drv_vbus_gpio_set, int is_on){
		u32 on_off = 0;
		int new_vdd = 3300;
		if(handler == 0){
			printk("wrn: handler is null\n");
			return;	
		}
		if(drv_vbus_gpio_set.port == 0xffff){
			if(drv_vbus_gpio_set.port_num >= USB_EXTERN_PIN_LDO_MASK){
		#ifdef  CONFIG_REGULATOR
				printk("set axp power:%d",is_on);
      	if(is_on){
      		regulator_enable((struct regulator*)handler);
      		regulator_set_voltage((struct regulator*)handler, new_vdd*1000, new_vdd*1000);
        }else{
          regulator_disable((struct regulator*)handler);
        }
		#else
			printk("CONFIG_REGULATOR is not define\n");
		#endif
      }else{
        axp_gpio_set_value(drv_vbus_gpio_set.port_num, on_off);
      }
		}
}

static int bcm40181_gpio_ctrl(char* name, int level)
{
	int i = 0, ret = 0;
	struct mmc_pm_ops *ops = &mmc_card_pm_ops;
	char* gpio_name[4] = {"bcm40181_wakeup",
						"bcm40181_shdn",
						"bcm40181_vcc_en",
						"bcm40181_vdd_en"
						};

    for (i=0; i<4; i++) {
        if (strcmp(name, gpio_name[i])==0)
            break;
    }
    if (i==4) {
        bcm40181_msg("No gpio %s for bcm40181-wifi module\n", name);
        return -1;
    }

    ret = gpio_write_one_pin_value(ops->pio_hdle, level, name);
    if (ret) {
        bcm40181_msg("Failed to set gpio %s to %d !\n", name, level);
        return -1;
    } else
		bcm40181_msg("Succeed to set gpio %s to %d !\n", name, level);

    if (strcmp(name, "bcm40181_vdd_en") == 0) {
        bcm40181_powerup = level;
        bcm40181_msg("BCM40181 SDIO Wifi Power %s !!\n", level ? "UP" : "Off");
    }

    return 0;
}

static int bcm40181_get_io_value(char* name)
{
	int ret = -1;
	struct mmc_pm_ops *ops = &mmc_card_pm_ops;
	
    if (strcmp(name, "bcm40181_wakeup")) {
        bcm40181_msg("No gpio %s for BCM40181\n", name);
        return -1;
    }
	ret = gpio_read_one_pin_value(ops->pio_hdle, name);
	bcm40181_msg("Succeed to get gpio %s value: %d !\n", name, ret);

	return ret;
}

void bcm40181_power(int mode, int* updown)
{
	printk("bcm40181 power mode=%d, updown=%d\n", mode, *updown);
    if (mode) {
        if (*updown) {
			udelay(10);
			set_axp_power(axp_handler_1,drv_vbus_gpio_set1,1);
			set_axp_power(axp_handler_2,drv_vbus_gpio_set2,1);
			
        } else {
			set_axp_power(axp_handler_1,drv_vbus_gpio_set1,0);
			set_axp_power(axp_handler_2,drv_vbus_gpio_set2,0);
        }
    } else {
        if (bcm40181_powerup){
            *updown = 1;
		}
        else {
            *updown = 0;
		}
		bcm40181_msg("sdio wifi power state: %s\n", bcm40181_powerup ? "on" : "off");
    }
    return;
}
void bcm40181_wifi_gpio_init(void)
{
	struct mmc_pm_ops *ops = &mmc_card_pm_ops;

	bcm40181_msg("exec bcm40181_wifi_gpio_init...\n");
	bcm40181_powerup = 0;
	bcm40181_suspend = 0;
	ops->gpio_ctrl = bcm40181_gpio_ctrl;
	ops->get_io_val = bcm40181_get_io_value;
	ops->power = bcm40181_power;	
	init_axp();
}



