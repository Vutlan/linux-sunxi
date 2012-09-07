/*
 * rtl8189es sdio wifi power management API
 *
 *; 10 - realtek rtl8189es sdio wifi gpio config
 *;rtl8189es_shdn       = port:PA01<1><default><default><0>
 *;rtl8189es_wakeup     = port:PA02<1><default><default><1>
 *;rtl8189es_vdd_en     = port:PA03<1><default><default><0>
 *;rtl8189es_vcc_en     = port:PA04<1><default><default><0>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <mach/sys_config.h>
#include "../../power/axp_power/axp-gpio.h"
#include <linux/regulator/consumer.h>

#include "mmc_pm.h"

#define rtl8189es_msg(...)    do {printk("[rtl8189es]: "__VA_ARGS__);} while(0)

static int rtl8189es_powerup = 0;
static int rtl8189es_suspend = 0;

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
static int gpio1_used = 0;
static int gpio2_used = 0;
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
		gpio1_used = 0;
	}else{
		gpio1_used = 1;
	}
	ret = script_parser_fetch("sdio_wifi_power_para", "wifi_power_2", (int *)&drv_vbus_gpio_set2, 64);
	if(ret != 0){
		printk("Err: request wifi power 2 fail");
		gpio2_used = 0;
	}else{
		gpio2_used = 1;
	}
	if(gpio1_used) {
		axp_handler_1 = alloc_pin(&drv_vbus_gpio_set1);
	}
	if(gpio2_used) {
		axp_handler_2 = alloc_pin(&drv_vbus_gpio_set2);
	}
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



static int rtl8189es_gpio_ctrl(char* name, int level)
{
	int i = 0, ret = 0;
	struct mmc_pm_ops *ops = &mmc_card_pm_ops;
	char* gpio_name[4] = {	"rtl8189es_wakeup",
							"rtl8189es_shdn",
							"rtl8189es_vcc_en",
							"rtl8189es_vdd_en"
						};

    for (i=0; i<4; i++) {
        if (strcmp(name, gpio_name[i])==0)
            break;
    }
    if (i==4) {
        rtl8189es_msg("No gpio %s for rtl8189es-wifi module\n", name);
        return -1;
    }

    ret = gpio_write_one_pin_value(ops->pio_hdle, level, name);
    if (ret) {
        rtl8189es_msg("Failed to set gpio %s to %d !\n", name, level);
        return -1;
    } else
		rtl8189es_msg("Succeed to set gpio %s to %d !\n", name, level);

    if (strcmp(name, "rtl8189es_vdd_en") == 0) {
        rtl8189es_powerup = level;
        rtl8189es_msg("rtl8189es SDIO Wifi Power %s !!\n", level ? "UP" : "Off");
    }

    return 0;
}

static int rtl8189es_get_io_value(char* name)
{
	int ret = -1;
	struct mmc_pm_ops *ops = &mmc_card_pm_ops;

    if (strcmp(name, "rtl8189es_wakeup")) {
        rtl8189es_msg("No gpio %s for rtl8189es\n", name);
        return -1;
    }
	ret = gpio_read_one_pin_value(ops->pio_hdle, name);
	rtl8189es_msg("Succeed to get gpio %s value: %d !\n", name, ret);

	return ret;
}

static void rtl8189es_standby(int instadby)
{
		printk("rtl8189es standby:%d\n",instadby);
    if (instadby) {

        if (rtl8189es_powerup) {
            rtl8189es_gpio_ctrl("rtl8189es_shdn", 0);
            rtl8189es_gpio_ctrl("rtl8189es_vcc_en", 0);
            rtl8189es_gpio_ctrl("rtl8189es_vdd_en", 0);
            rtl8189es_suspend = 1;
        }
    } else {
        if (rtl8189es_suspend) {
            rtl8189es_gpio_ctrl("rtl8189es_vdd_en", 1);
            udelay(100);
            rtl8189es_gpio_ctrl("rtl8189es_vcc_en", 1);
            udelay(50);
            rtl8189es_gpio_ctrl("rtl8189es_shdn", 1);
            sunximmc_rescan_card(1, 1);
            rtl8189es_suspend = 0;
        }
    }
}

static void rtl8189es_power(int mode, int* updown)
{
		printk("rtl8189es power  mode:%d\n",mode);
    if (mode) {
        if (*updown) {
      printk("rtl8189es  power updown\n");
			//rtl8189es_gpio_ctrl("rtl8189es_vdd_en", 1);
			//udelay(100);
			//rtl8189es_gpio_ctrl("rtl8189es_vcc_en", 1);
			//udelay(50);
			//rtl8189es_gpio_ctrl("rtl8189es_shdn", 1);

			printk("----set 8189 power\n");
			if(gpio1_used) {
				set_axp_power(axp_handler_1,drv_vbus_gpio_set1,1);
			}
			if(gpio2_used) {
				set_axp_power(axp_handler_2,drv_vbus_gpio_set2,1);
			}

        } else {
			//rtl8189es_gpio_ctrl("rtl8189es_shdn", 0);
			//rtl8189es_gpio_ctrl("rtl8189es_vcc_en", 0);
			//rtl8189es_gpio_ctrl("rtl8189es_vdd_en", 0);

			printk("----disable 8189 power\n");
			if(gpio1_used) {
				set_axp_power(axp_handler_1,drv_vbus_gpio_set1,0);
			}
			if(gpio2_used) {
				set_axp_power(axp_handler_2,drv_vbus_gpio_set2,0);
			}
        }
    } else {
        if (rtl8189es_powerup)
            *updown = 1;
        else
            *updown = 0;
		rtl8189es_msg("sdio wifi power state: %s\n", rtl8189es_powerup ? "on" : "off");
    }
    return;
}
void rtl8189es_wifi_gpio_init(void)
{
	struct mmc_pm_ops *ops = &mmc_card_pm_ops;

	rtl8189es_msg("exec rtl8189es_wifi_gpio_init...\n");
	rtl8189es_powerup = 0;
	rtl8189es_suspend = 0;
	ops->gpio_ctrl 	  = rtl8189es_gpio_ctrl;
	ops->get_io_val   = rtl8189es_get_io_value;
    ops->standby 	  = rtl8189es_standby;
	ops->power 		  = rtl8189es_power;

	init_axp();
}
