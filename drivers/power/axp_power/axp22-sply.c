/*
 * Battery charger driver for X-Powers AXP22X
 *
 * Copyright (C) 2012 X-Powers, Ltd.
 *  Weijin Zhong <zhwj@x-powers.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/workqueue.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>

#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/slab.h>

#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/input.h>
#include <linux/mfd/axp-mfd.h>
#include <asm/div64.h>

#include <mach/sys_config.h>

#include <asm-generic/gpio.h>
#include <mach/gpio.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#include "axp-cfg.h"
#include "axp-sply.h"

#include <linux/gpio.h>
//#include <drivers/char/gpio_test/sun6i_gpio_test.h>

#define DBG_AXP_PSY 1
#if  DBG_AXP_PSY
#define DBG_PSY_MSG(format,args...)   printk("[AXP]"format,##args)
#else
#define DBG_PSY_MSG(format,args...)   do {} while (0)
#endif

//const uint32_t AXP22_NOTIFIER_ON =	(AXP22_IRQ_USBIN |AXP22_IRQ_USBRE |AXP22_IRQ_ACIN |AXP22_IRQ_ACRE |AXP22_IRQ_BATIN |
// 									 AXP22_IRQ_BATRE |
// 									 AXP22_IRQ_CHAST |
// 									 AXP22_IRQ_PEKFE |
// 									 AXP22_IRQ_PEKRE |
// 									 AXP22_IRQ_CHAOV );

static int axp_debug = 0;
static int pmu_used2 = 0;
static int gpio_adp_hdle = 0;
static int pmu_suspendpwroff_vol = 0;
static int pmu_earlysuspend_chgcur = 0;
static int pmu_batdeten = 0;
struct axp_adc_res adc;
static int count_rdc = 0;
static int count_dis = 0;
struct delayed_work usbwork;
#ifdef CONFIG_HAS_EARLYSUSPEND
static struct early_suspend axp_early_suspend;
int early_suspend_flag = 0;
#endif

int pmu_usbvolnew = 0;
int pmu_usbcurnew = 0;
int axp_usbcurflag = 0;
int axp_usbvolflag = 0;

int axp_usbvol(void)
{
	axp_usbvolflag = 1;
    return 0;
}
EXPORT_SYMBOL_GPL(axp_usbvol);

int axp_usbcur(void)
{
    axp_usbcurflag = 1;
    return 0;
}
EXPORT_SYMBOL_GPL(axp_usbcur);

int axp_usbvol_restore(void)
{
 	axp_usbvolflag = 0;
    return 0;
}
EXPORT_SYMBOL_GPL(axp_usbvol_restore);

int axp_usbcur_restore(void)
{
	axp_usbcurflag = 0;
    return 0;
}
EXPORT_SYMBOL_GPL(axp_usbcur_restore);

static ssize_t axpdebug_store(struct class *class, 
			struct class_attribute *attr,	const char *buf, size_t count)
{
	if(buf[0] == '1'){
	   axp_debug = 1; 
    }
    else{
	   axp_debug = 0;         
    }        
	return count;
}

static ssize_t axpdebug_show(struct class *class, 
			struct class_attribute *attr,	char *buf)
{
	return sprintf(buf, "bat-debug value is %d\n", axp_debug);
}

static struct class_attribute axppower_class_attrs[] = {
	__ATTR(axpdebug,S_IRUGO|S_IWUSR,axpdebug_show,axpdebug_store),
	__ATTR_NULL
};
static struct class axppower_class = {
    .name = "axppower",
    .class_attrs = axppower_class_attrs,
};

int ADC_Freq_Get(struct axp_charger *charger)
{
	uint8_t  temp;
	int  rValue = 25;

	axp_read(charger->master, AXP22_ADC_CONTROL3,&temp);
	temp &= 0xc0;
	switch(temp >> 6)
	{
		case 0:
			rValue = 100;
			break;
		case 1:
			rValue = 200;
			break;
		case 2:
			rValue = 400;
			break;
		case 3:
			rValue = 800;
			break;
		default:
			break;
	}
	return rValue;
}

static inline int axp22_vbat_to_mV(uint16_t reg)
{
  return ((int)((( reg >> 8) << 4 ) | (reg & 0x000F))) * 1100 / 1000;
}

static inline int axp22_vdc_to_mV(uint16_t reg)
{
  return ((int)(((reg >> 8) << 4 ) | (reg & 0x000F))) * 1700 / 1000;
}


static inline int axp22_ibat_to_mA(uint16_t reg)
{
    return ((int)(((reg >> 8) << 5 ) | (reg & 0x001F))) ;
}

static inline int axp22_icharge_to_mA(uint16_t reg)
{
    return ((int)(((reg >> 8) << 4 ) | (reg & 0x000F)));
}

static inline int axp22_iac_to_mA(uint16_t reg)
{
    return ((int)(((reg >> 8) << 4 ) | (reg & 0x000F))) * 625 / 1000;
}

static inline int axp22_iusb_to_mA(uint16_t reg)
{
    return ((int)(((reg >> 8) << 4 ) | (reg & 0x000F))) * 375 / 1000;
}


static inline void axp_read_adc(struct axp_charger *charger,
  struct axp_adc_res *adc)
{
  uint8_t tmp[8];
//
//  axp_reads(charger->master,AXP22_VACH_RES,8,tmp);
  adc->vac_res = 0;
  adc->iac_res = 0;
  adc->vusb_res = 0;
  adc->iusb_res = 0;
  axp_reads(charger->master,AXP22_VBATH_RES,6,tmp);
  adc->vbat_res = ((uint16_t) tmp[0] << 8 )| tmp[1];
  adc->ichar_res = ((uint16_t) tmp[2] << 8 )| tmp[3];
  adc->idischar_res = ((uint16_t) tmp[4] << 8 )| tmp[5];
}


static void axp_charger_update_state(struct axp_charger *charger)
{
  uint8_t val[2];
  uint16_t tmp;

  axp_reads(charger->master,AXP22_CHARGE_STATUS,2,val);
  tmp = (val[1] << 8 )+ val[0];
  charger->is_on = (val[1] & AXP22_IN_CHARGE) ? 1 : 0;
  charger->fault = val[1];
  charger->bat_det = (tmp & AXP22_STATUS_BATEN)?1:0;
  charger->ac_det = (tmp & AXP22_STATUS_ACEN)?1:0;
  charger->usb_det = (tmp & AXP22_STATUS_USBEN)?1:0;
  charger->usb_valid = (tmp & AXP22_STATUS_USBVA)?1:0;
  charger->ac_valid = (tmp & AXP22_STATUS_ACVA)?1:0;
  charger->ext_valid = charger->ac_valid | charger->usb_valid;
  charger->bat_current_direction = (tmp & AXP22_STATUS_BATCURDIR)?1:0;
  charger->in_short = (tmp& AXP22_STATUS_ACUSBSH)?1:0;
  charger->batery_active = (tmp & AXP22_STATUS_BATINACT)?1:0;
  charger->int_over_temp = (tmp & AXP22_STATUS_ICTEMOV)?1:0;
  axp_read(charger->master,AXP22_CHARGE_CONTROL1,val);
  charger->charge_on = ((val[0] >> 7) & 0x01);
}

static void axp_charger_update(struct axp_charger *charger)
{
  uint16_t tmp;
  uint8_t val[2];
  //struct axp_adc_res adc;
  charger->adc = &adc;
  axp_read_adc(charger, &adc);
  tmp = charger->adc->vbat_res;
  charger->vbat = axp22_vbat_to_mV(tmp);
   //tmp = charger->adc->ichar_res + charger->adc->idischar_res;
  charger->ibat = ABS(axp22_icharge_to_mA(charger->adc->ichar_res)-axp22_ibat_to_mA(charger->adc->idischar_res));
  tmp = 00;
  charger->vac = axp22_vdc_to_mV(tmp);
  tmp = 00;
  charger->iac = axp22_iac_to_mA(tmp);
  tmp = 00;
  charger->vusb = axp22_vdc_to_mV(tmp);
  tmp = 00;
  charger->iusb = axp22_iusb_to_mA(tmp);
  axp_reads(charger->master,AXP22_INTTEMP,2,val);
  //DBG_PSY_MSG("TEMPERATURE:val1=0x%x,val2=0x%x\n",val[1],val[0]);
  tmp = (val[0] << 4 ) + (val[1] & 0x0F);
  charger->ic_temp = (int) tmp  - 1447;
  if(!charger->ext_valid){
  	charger->disvbat =  charger->vbat;
  	charger->disibat =  charger->ibat;
  }
}

#if defined  (CONFIG_AXP_CHARGEINIT)
static void axp_set_charge(struct axp_charger *charger)
{
  uint8_t val=0x00;
  uint8_t tmp=0x00;
    if(charger->chgvol < 4200000){
      val &= ~(3 << 5);
	  //val |= 1 << 5;
      }
    else if (charger->chgvol<4220000)		
		{
			  val &= ~(3 << 5);
			  val |= 1 << 6;
		}

    else if (charger->chgvol<4240000){
      val &= ~(3 << 5);
      val |= 1 << 5;
      }
    else
      val |= 3 << 5;

		if(charger->chgcur == 0)
			charger->chgen = 0;

    if(charger->chgcur< 300000)
      charger->chgcur = 300000;
    else if(charger->chgcur > 2550000)
     charger->chgcur = 2550000;

    val |= (charger->chgcur - 300000) / 150000 ;
    if(charger ->chgend == 10){
      val &= ~(1 << 4);
    }
    else {
      val |= 1 << 4;
    }
    val &= 0x7F;
    val |= charger->chgen << 7;
      if(charger->chgpretime < 30)
      charger->chgpretime = 30;
    if(charger->chgcsttime < 360)
      charger->chgcsttime = 360;

    tmp = ((((charger->chgpretime - 40) / 10) << 6)  \
      | ((charger->chgcsttime - 360) / 120));
	axp_write(charger->master, AXP22_CHARGE_CONTROL1,val);
	axp_update(charger->master, AXP22_CHARGE_CONTROL2,tmp,0xC2);
}
#else
static void axp_set_charge(struct axp_charger *charger)
{

}
#endif

static enum power_supply_property axp_battery_props[] = {
  POWER_SUPPLY_PROP_MODEL_NAME,
  POWER_SUPPLY_PROP_STATUS,
  POWER_SUPPLY_PROP_PRESENT,
  POWER_SUPPLY_PROP_ONLINE,
  POWER_SUPPLY_PROP_HEALTH,
  POWER_SUPPLY_PROP_TECHNOLOGY,
  POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,
  POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN,
  POWER_SUPPLY_PROP_VOLTAGE_NOW,
  POWER_SUPPLY_PROP_CURRENT_NOW,
  //POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
  //POWER_SUPPLY_PROP_CHARGE_FULL,
  POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN,
  POWER_SUPPLY_PROP_CAPACITY,
  //POWER_SUPPLY_PROP_TIME_TO_EMPTY_NOW,
  //POWER_SUPPLY_PROP_TIME_TO_FULL_NOW,
  POWER_SUPPLY_PROP_TEMP,
};

static enum power_supply_property axp_ac_props[] = {
  POWER_SUPPLY_PROP_MODEL_NAME,
  POWER_SUPPLY_PROP_PRESENT,
  POWER_SUPPLY_PROP_ONLINE,
  POWER_SUPPLY_PROP_VOLTAGE_NOW,
  POWER_SUPPLY_PROP_CURRENT_NOW,
};

static enum power_supply_property axp_usb_props[] = {
  POWER_SUPPLY_PROP_MODEL_NAME,
  POWER_SUPPLY_PROP_PRESENT,
  POWER_SUPPLY_PROP_ONLINE,
  POWER_SUPPLY_PROP_VOLTAGE_NOW,
  POWER_SUPPLY_PROP_CURRENT_NOW,
};

static void axp_battery_check_status(struct axp_charger *charger,
            union power_supply_propval *val)
{
  if (charger->bat_det) {
    if (charger->ext_valid){
    	if( charger->rest_vol == 100)
        val->intval = POWER_SUPPLY_STATUS_FULL;
    	else if(charger->charge_on)
    		val->intval = POWER_SUPPLY_STATUS_CHARGING;
    	else
    		val->intval = POWER_SUPPLY_STATUS_NOT_CHARGING;
    }
    else
      val->intval = POWER_SUPPLY_STATUS_DISCHARGING;
  }
  else
    val->intval = POWER_SUPPLY_STATUS_FULL;
}

static void axp_battery_check_health(struct axp_charger *charger,
            union power_supply_propval *val)
{
    if (charger->fault & AXP22_FAULT_LOG_BATINACT)
    val->intval = POWER_SUPPLY_HEALTH_DEAD;
  else if (charger->fault & AXP22_FAULT_LOG_OVER_TEMP)
    val->intval = POWER_SUPPLY_HEALTH_OVERHEAT;
  else if (charger->fault & AXP22_FAULT_LOG_COLD)
    val->intval = POWER_SUPPLY_HEALTH_COLD;
  else
    val->intval = POWER_SUPPLY_HEALTH_GOOD;
}

static int axp_battery_get_property(struct power_supply *psy,
           enum power_supply_property psp,
           union power_supply_propval *val)
{
  struct axp_charger *charger;
  int ret = 0;
  charger = container_of(psy, struct axp_charger, batt);

  switch (psp) {
  case POWER_SUPPLY_PROP_STATUS:
    axp_battery_check_status(charger, val);
    break;
  case POWER_SUPPLY_PROP_HEALTH:
    axp_battery_check_health(charger, val);
    break;
  case POWER_SUPPLY_PROP_TECHNOLOGY:
    val->intval = charger->battery_info->technology;
    break;
  case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
    val->intval = charger->battery_info->voltage_max_design;
    break;
  case POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN:
    val->intval = charger->battery_info->voltage_min_design;
    break;
  case POWER_SUPPLY_PROP_VOLTAGE_NOW:
    val->intval = charger->ocv * 1000;
    break;
  case POWER_SUPPLY_PROP_CURRENT_NOW:
    val->intval = charger->ibat * 1000;
    break;
  case POWER_SUPPLY_PROP_MODEL_NAME:
    val->strval = charger->batt.name;
    break;
/*  case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
  case POWER_SUPPLY_PROP_CHARGE_FULL:
    val->intval = charger->battery_info->charge_full_design;
        break;
*/
  case POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN:
    val->intval = charger->battery_info->energy_full_design;
  //  DBG_PSY_MSG("POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN:%d\n",val->intval);
       break;
  case POWER_SUPPLY_PROP_CAPACITY:
    val->intval = charger->rest_vol;
    break;
/*  case POWER_SUPPLY_PROP_TIME_TO_EMPTY_NOW:
    if(charger->bat_det && !(charger->is_on) && !(charger->ext_valid))
      val->intval = charger->rest_time;
    else
      val->intval = 0;
    break;
  case POWER_SUPPLY_PROP_TIME_TO_FULL_NOW:
    if(charger->bat_det && charger->is_on)
      val->intval = charger->rest_time;
    else
      val->intval = 0;
    break;
*/
  case POWER_SUPPLY_PROP_ONLINE:
    val->intval = (!charger->is_on)&&(charger->bat_det) && (! charger->ext_valid);
    break;
  case POWER_SUPPLY_PROP_PRESENT:
    val->intval = charger->bat_det;
    break;
  case POWER_SUPPLY_PROP_TEMP:
    //val->intval = charger->ic_temp - 200;
    val->intval =  300;
    break;
  default:
    ret = -EINVAL;
    break;
  }

  return ret;
}

static int axp_ac_get_property(struct power_supply *psy,
           enum power_supply_property psp,
           union power_supply_propval *val)
{
  struct axp_charger *charger;
  int ret = 0;
  charger = container_of(psy, struct axp_charger, ac);

  switch(psp){
  case POWER_SUPPLY_PROP_MODEL_NAME:
    val->strval = charger->ac.name;break;
  case POWER_SUPPLY_PROP_PRESENT:
    val->intval = charger->ac_det;
    break;
  case POWER_SUPPLY_PROP_ONLINE:
    val->intval = charger->ac_valid;break;
  case POWER_SUPPLY_PROP_VOLTAGE_NOW:
    val->intval = charger->vac * 1000;
    break;
  case POWER_SUPPLY_PROP_CURRENT_NOW:
    val->intval = charger->iac * 1000;
    break;
  default:
    ret = -EINVAL;
    break;
  }
   return ret;
}

static int axp_usb_get_property(struct power_supply *psy,
           enum power_supply_property psp,
           union power_supply_propval *val)
{
  struct axp_charger *charger;
  int ret = 0;
  charger = container_of(psy, struct axp_charger, usb);

  switch(psp){
  case POWER_SUPPLY_PROP_MODEL_NAME:
    val->strval = charger->usb.name;break;
  case POWER_SUPPLY_PROP_PRESENT:
    val->intval = charger->usb_det;
    break;
  case POWER_SUPPLY_PROP_ONLINE:
    val->intval = charger->usb_valid;
    break;
  case POWER_SUPPLY_PROP_VOLTAGE_NOW:
    val->intval = charger->vusb * 1000;
    break;
  case POWER_SUPPLY_PROP_CURRENT_NOW:
    val->intval = charger->iusb * 1000;
    break;
  default:
    ret = -EINVAL;
    break;
  }
   return ret;
}

static void axp_change(struct axp_charger *charger)
{
  uint8_t val,tmp;
  int var;
  DBG_PSY_MSG("battery state change\n");
  axp_charger_update_state(charger);
  axp_charger_update(charger);
  printk("charger->usb_valid = %d\n",charger->usb_valid);
	if(!charger->usb_valid){
		printk("set usb vol-lim to %d mV, cur-lim to %d mA\n",pmu_usbvol,pmu_usbcur);
        cancel_delayed_work_sync(&usbwork);
		//reset usb-ac after usb removed 
		if((pmu_usbcur) && (pmu_usbcur_limit)){
			axp_clr_bits(charger->master, AXP22_CHARGE_VBUS, 0x01);
			var = pmu_usbcur * 1000;
			if(var >= 900000)
				axp_clr_bits(charger->master, AXP22_CHARGE_VBUS, 0x03);
			else if ((var >= 500000)&& (var < 900000)){
				axp_clr_bits(charger->master, AXP22_CHARGE_VBUS, 0x02);
				axp_set_bits(charger->master, AXP22_CHARGE_VBUS, 0x01);
			}
			else if ((var >= 100000)&& (var < 500000)){
				axp_clr_bits(charger->master, AXP22_CHARGE_VBUS, 0x01);
				axp_set_bits(charger->master, AXP22_CHARGE_VBUS, 0x02);
			}
			else
				printk("set usb limit current error,%d mA\n",pmu_usbcur);	
		}
		else
			axp_set_bits(charger->master, AXP22_CHARGE_VBUS, 0x03);
			
		if((pmu_usbvol) && (pmu_usbvol_limit)){
			axp_set_bits(charger->master, AXP22_CHARGE_VBUS, 0x40);
			var = pmu_usbvol * 1000;
			if(var >= 4000000 && var <=4700000){
				tmp = (var - 4000000)/100000;
			    axp_read(charger->master, AXP22_CHARGE_VBUS,&val);
			    val &= 0xC7;
			    val |= tmp << 3;
			    axp_write(charger->master, AXP22_CHARGE_VBUS,val);
			}
			else
				printk("set usb limit voltage error,%d mV\n",pmu_usbvol);	
		}
		else
			axp_clr_bits(charger->master, AXP22_CHARGE_VBUS, 0x40);
	}
  flag_state_change = 1;
  power_supply_changed(&charger->batt);
}

static void axp_presslong(struct axp_charger *charger)
{
	DBG_PSY_MSG("press long\n");
	input_report_key(powerkeydev, KEY_POWER, 1);
	input_sync(powerkeydev);
	ssleep(2);
	DBG_PSY_MSG("press long up\n");
	input_report_key(powerkeydev, KEY_POWER, 0);
	input_sync(powerkeydev);
}

static void axp_pressshort(struct axp_charger *charger)
{
	DBG_PSY_MSG("press short\n");
  	input_report_key(powerkeydev, KEY_POWER, 1);
 	input_sync(powerkeydev);
 	msleep(100);
 	input_report_key(powerkeydev, KEY_POWER, 0);
 	input_sync(powerkeydev);
}

static void axp_keyup(struct axp_charger *charger)
{
	DBG_PSY_MSG("power key up\n");
	input_report_key(powerkeydev, KEY_POWER, 0);
	input_sync(powerkeydev);
}

static void axp_keydown(struct axp_charger *charger)
{
	DBG_PSY_MSG("power key down\n");
	input_report_key(powerkeydev, KEY_POWER, 1);
	input_sync(powerkeydev);
}

static void axp_capchange(struct axp_charger *charger)
{
	uint8_t val;
	int k;

	DBG_PSY_MSG("battery change\n");
	ssleep(2);
    axp_charger_update_state(charger);
    axp_charger_update(charger);
    axp_read(charger->master, AXP22_CAP,&val);
    charger->rest_vol = (int) (val & 0x7F);

    if((charger->bat_det == 0) || (charger->rest_vol == 127)){
  	charger->rest_vol = 100;
  }

  DBG_PSY_MSG("rest_vol = %d\n",charger->rest_vol);
  memset(Bat_Cap_Buffer, 0, sizeof(Bat_Cap_Buffer));
  for(k = 0;k < AXP22_VOL_MAX; k++){
    Bat_Cap_Buffer[k] = charger->rest_vol;
  }
  Total_Cap = charger->rest_vol * AXP22_VOL_MAX;
  power_supply_changed(&charger->batt);
}

static void axp_close(struct axp_charger *charger)
{
	charger->rest_vol = 5;
	axp_write(charger->master,AXP22_DATA_BUFFER1,0x85);
	DBG_PSY_MSG("\n==================event in close==============\n");
	power_supply_changed(&charger->batt);
}


static int axp_battery_event(struct notifier_block *nb, unsigned long event,
        void *data)
{
    struct axp_charger *charger =
    container_of(nb, struct axp_charger, nb);

    uint8_t w[9];
	printk("axp_battery_event enter...\n");
    if((bool)data==0){
    		printk("low 32bit status...\n");
			if(event & (AXP22_IRQ_BATIN|AXP22_IRQ_BATRE)) {
				axp_capchange(charger);
			}
	
			if(event & (AXP22_IRQ_ACIN|AXP22_IRQ_USBIN|AXP22_IRQ_ACOV|AXP22_IRQ_USBOV|AXP22_IRQ_CHAOV
						|AXP22_IRQ_CHAST|AXP22_IRQ_TEMOV|AXP22_IRQ_TEMLO)) {
				axp_change(charger);
			}
	
			if(event & (AXP22_IRQ_ACRE|AXP22_IRQ_USBRE)) {
				axp_change(charger);
			}
	
			if(event & AXP22_IRQ_POKLO) {
				axp_presslong(charger);
			}
	
			if(event & AXP22_IRQ_POKSH) {
				axp_pressshort(charger);
			}
			w[0] = (uint8_t) ((event) & 0xFF);
    		w[1] = AXP22_INTSTS2;
    		w[2] = (uint8_t) ((event >> 8) & 0xFF);
    		w[3] = AXP22_INTSTS3;
    		w[4] = (uint8_t) ((event >> 16) & 0xFF);
    		w[5] = AXP22_INTSTS4;
    		w[6] = (uint8_t) ((event >> 24) & 0xFF);
    		w[7] = AXP22_INTSTS5;
    		w[8] = 0;
	} else {
		if((event) & (AXP22_IRQ_PEKFE>>32)) {
			axp_keydown(charger);
		}

		if((event) & (AXP22_IRQ_PEKRE>>32)) {
			axp_keyup(charger);
		}
		printk("high 32bit status...\n");
		w[0] = 0;
    	w[1] = AXP22_INTSTS2;
    	w[2] = 0;
    	w[3] = AXP22_INTSTS3;
    	w[4] = 0;
    	w[5] = AXP22_INTSTS4;
    	w[6] = 0;
    	w[7] = AXP22_INTSTS5;
    	w[8] = (uint8_t) ((event) & 0xFF);;
	}
    printk("event = 0x%x\n",(int) event);
    axp_writes(charger->master,AXP22_INTSTS1,9,w);

    return 0;
}

static char *supply_list[] = {
  "battery",
};



static void axp_battery_setup_psy(struct axp_charger *charger)
{
  struct power_supply *batt = &charger->batt;
  struct power_supply *ac = &charger->ac;
  struct power_supply *usb = &charger->usb;
  struct power_supply_info *info = charger->battery_info;

  batt->name = "battery";
  batt->use_for_apm = info->use_for_apm;
  batt->type = POWER_SUPPLY_TYPE_BATTERY;
  batt->get_property = axp_battery_get_property;

  batt->properties = axp_battery_props;
  batt->num_properties = ARRAY_SIZE(axp_battery_props);

  ac->name = "ac";
  ac->type = POWER_SUPPLY_TYPE_MAINS;
  ac->get_property = axp_ac_get_property;

  ac->supplied_to = supply_list,
  ac->num_supplicants = ARRAY_SIZE(supply_list),

  ac->properties = axp_ac_props;
  ac->num_properties = ARRAY_SIZE(axp_ac_props);

  usb->name = "usb";
  usb->type = POWER_SUPPLY_TYPE_USB;
  usb->get_property = axp_usb_get_property;

  usb->supplied_to = supply_list,
  usb->num_supplicants = ARRAY_SIZE(supply_list),

  usb->properties = axp_usb_props;
  usb->num_properties = ARRAY_SIZE(axp_usb_props);
};

#if defined  (CONFIG_AXP_CHARGEINIT)
static int axp_battery_adc_set(struct axp_charger *charger)
{
   int ret ;
   uint8_t val;

  /*enable adc and set adc */
  val= AXP22_ADC_BATVOL_ENABLE | AXP22_ADC_BATCUR_ENABLE;

	ret = axp_update(charger->master, AXP22_ADC_CONTROL, val , val);
  if (ret)
    return ret;
    ret = axp_read(charger->master, AXP22_ADC_CONTROL3, &val);
  switch (charger->sample_time/100){
  case 1: val &= ~(3 << 6);break;
  case 2: val &= ~(3 << 6);val |= 1 << 6;break;
  case 4: val &= ~(3 << 6);val |= 2 << 6;break;
  case 8: val |= 3 << 6;break;
  default: break;
  }
  ret = axp_write(charger->master, AXP22_ADC_CONTROL3, val);
  if (ret)
    return ret;

  return 0;
}
#else
static int axp_battery_adc_set(struct axp_charger *charger)
{
  return 0;
}
#endif

static int axp_battery_first_init(struct axp_charger *charger)
{
   int ret;
   uint8_t val;
   axp_set_charge(charger);
   ret = axp_battery_adc_set(charger);
   if(ret)
    return ret;

   ret = axp_read(charger->master, AXP22_ADC_CONTROL3, &val);
   switch ((val >> 6) & 0x03){
  case 0: charger->sample_time = 100;break;
  case 1: charger->sample_time = 200;break;
  case 2: charger->sample_time = 400;break;
  case 3: charger->sample_time = 800;break;
  default:break;
  }
  return ret;
}

static ssize_t chgen_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
  struct axp_charger *charger = dev_get_drvdata(dev);
  uint8_t val;
  axp_read(charger->master, AXP22_CHARGE_CONTROL1, &val);
  charger->chgen  = val >> 7;
  return sprintf(buf, "%d\n",charger->chgen);
}

static ssize_t chgen_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
  struct axp_charger *charger = dev_get_drvdata(dev);
  int var;
  var = simple_strtoul(buf, NULL, 10);
  if(var){
    charger->chgen = 1;
    axp_set_bits(charger->master,AXP22_CHARGE_CONTROL1,0x80);
  }
  else{
    charger->chgen = 0;
    axp_clr_bits(charger->master,AXP22_CHARGE_CONTROL1,0x80);
  }
  return count;
}

static ssize_t chgmicrovol_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
  struct axp_charger *charger = dev_get_drvdata(dev);
  uint8_t val;
  axp_read(charger->master, AXP22_CHARGE_CONTROL1, &val);
  switch ((val >> 5) & 0x03){
    case 0: charger->chgvol = 4100000;break;
    case 1: charger->chgvol = 4220000;break;
    case 2: charger->chgvol = 4200000;break;
    case 3: charger->chgvol = 4240000;break;
  }
  return sprintf(buf, "%d\n",charger->chgvol);
}

static ssize_t chgmicrovol_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
  struct axp_charger *charger = dev_get_drvdata(dev);
  int var;
  uint8_t tmp, val;
  var = simple_strtoul(buf, NULL, 10);
  switch(var){
    case 4100000:tmp = 0;break;
    case 4220000:tmp = 1;break;
    case 4200000:tmp = 2;break;
    case 4240000:tmp = 3;break;
    default:  tmp = 4;break;
  }
  if(tmp < 4){
    charger->chgvol = var;
    axp_read(charger->master, AXP22_CHARGE_CONTROL1, &val);
    val &= 0x9F;
    val |= tmp << 5;
    axp_write(charger->master, AXP22_CHARGE_CONTROL1, val);
  }
  return count;
}

static ssize_t chgintmicrocur_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
  struct axp_charger *charger = dev_get_drvdata(dev);
  uint8_t val;
  axp_read(charger->master, AXP22_CHARGE_CONTROL1, &val);
  charger->chgcur = (val & 0x0F) * 150000 +300000;
  return sprintf(buf, "%d\n",charger->chgcur);
}

static ssize_t chgintmicrocur_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
  struct axp_charger *charger = dev_get_drvdata(dev);
  int var;
  uint8_t val,tmp;
  var = simple_strtoul(buf, NULL, 10);
  if(var >= 300000 && var <= 2550000){
    tmp = (var -200001)/150000;
    charger->chgcur = tmp *150000 + 300000;
    axp_read(charger->master, AXP22_CHARGE_CONTROL1, &val);
    val &= 0xF0;
    val |= tmp;
    axp_write(charger->master, AXP22_CHARGE_CONTROL1, val);
  }
  return count;
}

static ssize_t chgendcur_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
  struct axp_charger *charger = dev_get_drvdata(dev);
  uint8_t val;
  axp_read(charger->master, AXP22_CHARGE_CONTROL1, &val);
  charger->chgend = ((val >> 4)& 0x01)? 15 : 10;
  return sprintf(buf, "%d\n",charger->chgend);
}

static ssize_t chgendcur_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
  struct axp_charger *charger = dev_get_drvdata(dev);
  int var;
  var = simple_strtoul(buf, NULL, 10);
  if(var == 10 ){
    charger->chgend = var;
    axp_clr_bits(charger->master ,AXP22_CHARGE_CONTROL1,0x10);
  }
  else if (var == 15){
    charger->chgend = var;
    axp_set_bits(charger->master ,AXP22_CHARGE_CONTROL1,0x10);

  }
  return count;
}

static ssize_t chgpretimemin_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
  struct axp_charger *charger = dev_get_drvdata(dev);
  uint8_t val;
  axp_read(charger->master,AXP22_CHARGE_CONTROL2, &val);
  charger->chgpretime = (val >> 6) * 10 +40;
  return sprintf(buf, "%d\n",charger->chgpretime);
}

static ssize_t chgpretimemin_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
  struct axp_charger *charger = dev_get_drvdata(dev);
  int var;
  uint8_t tmp,val;
  var = simple_strtoul(buf, NULL, 10);
  if(var >= 40 && var <= 70){
    tmp = (var - 40)/10;
    charger->chgpretime = tmp * 10 + 40;
    axp_read(charger->master,AXP22_CHARGE_CONTROL2,&val);
    val &= 0x3F;
    val |= (tmp << 6);
    axp_write(charger->master,AXP22_CHARGE_CONTROL2,val);
  }
  return count;
}

static ssize_t chgcsttimemin_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
  struct axp_charger *charger = dev_get_drvdata(dev);
  uint8_t val;
  axp_read(charger->master,AXP22_CHARGE_CONTROL2, &val);
  charger->chgcsttime = (val & 0x03) *120 + 360;
  return sprintf(buf, "%d\n",charger->chgcsttime);
}

static ssize_t chgcsttimemin_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
  struct axp_charger *charger = dev_get_drvdata(dev);
  int var;
  uint8_t tmp,val;
  var = simple_strtoul(buf, NULL, 10);
  if(var >= 360 && var <= 720){
    tmp = (var - 360)/120;
    charger->chgcsttime = tmp * 120 + 360;
    axp_read(charger->master,AXP22_CHARGE_CONTROL2,&val);
    val &= 0xFC;
    val |= tmp;
    axp_write(charger->master,AXP22_CHARGE_CONTROL2,val);
  }
  return count;
}

static ssize_t adcfreq_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
  struct axp_charger *charger = dev_get_drvdata(dev);
  uint8_t val;
  axp_read(charger->master, AXP22_ADC_CONTROL3, &val);
  switch ((val >> 6) & 0x03){
     case 0: charger->sample_time = 100;break;
     case 1: charger->sample_time = 200;break;
     case 2: charger->sample_time = 400;break;
     case 3: charger->sample_time = 800;break;
     default:break;
  }
  return sprintf(buf, "%d\n",charger->sample_time);
}

static ssize_t adcfreq_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
  struct axp_charger *charger = dev_get_drvdata(dev);
  int var;
  uint8_t val;
  var = simple_strtoul(buf, NULL, 10);
  axp_read(charger->master, AXP22_ADC_CONTROL3, &val);
  switch (var/25){
    case 1: val &= ~(3 << 6);charger->sample_time = 100;break;
    case 2: val &= ~(3 << 6);val |= 1 << 6;charger->sample_time = 200;break;
    case 4: val &= ~(3 << 6);val |= 2 << 6;charger->sample_time = 400;break;
    case 8: val |= 3 << 6;charger->sample_time = 800;break;
    default: break;
    }
  axp_write(charger->master, AXP22_ADC_CONTROL3, val);
  return count;
}


static ssize_t vholden_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
  struct axp_charger *charger = dev_get_drvdata(dev);
  uint8_t val;
  axp_read(charger->master,AXP22_CHARGE_VBUS, &val);
  val = (val>>6) & 0x01;
  return sprintf(buf, "%d\n",val);
}

static ssize_t vholden_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
  struct axp_charger *charger = dev_get_drvdata(dev);
  int var;
  var = simple_strtoul(buf, NULL, 10);
  if(var)
    axp_set_bits(charger->master, AXP22_CHARGE_VBUS, 0x40);
  else
    axp_clr_bits(charger->master, AXP22_CHARGE_VBUS, 0x40);

  return count;
}

static ssize_t vhold_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
  struct axp_charger *charger = dev_get_drvdata(dev);
  uint8_t val;
  int vhold;
  axp_read(charger->master,AXP22_CHARGE_VBUS, &val);
  vhold = ((val >> 3) & 0x07) * 100000 + 4000000;
  return sprintf(buf, "%d\n",vhold);
}

static ssize_t vhold_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
  struct axp_charger *charger = dev_get_drvdata(dev);
  int var;
  uint8_t val,tmp;
  var = simple_strtoul(buf, NULL, 10);
  if(var >= 4000000 && var <=4700000){
    tmp = (var - 4000000)/100000;
    //printk("tmp = 0x%x\n",tmp);
    axp_read(charger->master, AXP22_CHARGE_VBUS,&val);
    val &= 0xC7;
    val |= tmp << 3;
    //printk("val = 0x%x\n",val);
    axp_write(charger->master, AXP22_CHARGE_VBUS,val);
  }
  return count;
}

static ssize_t iholden_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
  struct axp_charger *charger = dev_get_drvdata(dev);
  uint8_t val;
  axp_read(charger->master,AXP22_CHARGE_VBUS, &val);
  return sprintf(buf, "%d\n",((val & 0x03) == 0x03)?0:1);
}

static ssize_t iholden_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
  struct axp_charger *charger = dev_get_drvdata(dev);
  int var;
  var = simple_strtoul(buf, NULL, 10);
  if(var)
    axp_clr_bits(charger->master, AXP22_CHARGE_VBUS, 0x01);
  else
    axp_set_bits(charger->master, AXP22_CHARGE_VBUS, 0x03);

  return count;
}

static ssize_t ihold_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
  struct axp_charger *charger = dev_get_drvdata(dev);
  uint8_t val,tmp;
  int ihold;
  axp_read(charger->master,AXP22_CHARGE_VBUS, &val);
  tmp = (val) & 0x03;
  switch(tmp){
    case 0: ihold = 900000;break;
    case 1: ihold = 500000;break;
    default: ihold = 0;break;
  }
  return sprintf(buf, "%d\n",ihold);
}

static ssize_t ihold_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
  struct axp_charger *charger = dev_get_drvdata(dev);
  int var;
  var = simple_strtoul(buf, NULL, 10);
  if(var == 900000)
    axp_clr_bits(charger->master, AXP22_CHARGE_VBUS, 0x03);
  else if (var == 500000){
    axp_clr_bits(charger->master, AXP22_CHARGE_VBUS, 0x02);
    axp_set_bits(charger->master, AXP22_CHARGE_VBUS, 0x01);
  }
  return count;
}

static struct device_attribute axp_charger_attrs[] = {
  AXP_CHG_ATTR(chgen),
  AXP_CHG_ATTR(chgmicrovol),
  AXP_CHG_ATTR(chgintmicrocur),
  AXP_CHG_ATTR(chgendcur),
  AXP_CHG_ATTR(chgpretimemin),
  AXP_CHG_ATTR(chgcsttimemin),
  AXP_CHG_ATTR(adcfreq),
  AXP_CHG_ATTR(vholden),
  AXP_CHG_ATTR(vhold),
  AXP_CHG_ATTR(iholden),
  AXP_CHG_ATTR(ihold),
};

#if defined CONFIG_HAS_EARLYSUSPEND
static void axp_earlysuspend(struct early_suspend *h)
{
	uint8_t tmp;
	DBG_PSY_MSG("======early suspend=======\n");

#if defined (CONFIG_AXP_CHGCHANGE)
  	early_suspend_flag = 1;
  	if(pmu_earlysuspend_chgcur == 0)
  		axp_clr_bits(axp_charger->master,AXP22_CHARGE_CONTROL1,0x80);
  	else
  		axp_set_bits(axp_charger->master,AXP22_CHARGE_CONTROL1,0x80);

    if(pmu_earlysuspend_chgcur >= 300000 && pmu_earlysuspend_chgcur <= 2550000){
    	tmp = (pmu_earlysuspend_chgcur -200001)/150000;
    	axp_update(axp_charger->master, AXP22_CHARGE_CONTROL1, tmp,0x0F);
    }
#endif

}
static void axp_lateresume(struct early_suspend *h)
{
	uint8_t tmp;
	DBG_PSY_MSG("======late resume=======\n");

#if defined (CONFIG_AXP_CHGCHANGE)
	early_suspend_flag = 0;
	if(pmu_resume_chgcur == 0)
  		axp_clr_bits(axp_charger->master,AXP22_CHARGE_CONTROL1,0x80);
  else
  		axp_set_bits(axp_charger->master,AXP22_CHARGE_CONTROL1,0x80);

    if(pmu_resume_chgcur >= 300000 && pmu_resume_chgcur <= 2550000){
        tmp = (pmu_resume_chgcur -200001)/150000;
        axp_update(axp_charger->master, AXP22_CHARGE_CONTROL1, tmp,0x0F);
    }
#endif

}
#endif

int axp_charger_create_attrs(struct power_supply *psy)
{
  int j,ret;
  for (j = 0; j < ARRAY_SIZE(axp_charger_attrs); j++) {
    ret = device_create_file(psy->dev,
          &axp_charger_attrs[j]);
    if (ret)
      goto sysfs_failed;
  }
    goto succeed;

sysfs_failed:
  while (j--)
    device_remove_file(psy->dev,
         &axp_charger_attrs[j]);
succeed:
  return ret;
}

static void axp_charging_monitor(struct work_struct *work)
{
	struct axp_charger *charger;
	uint8_t	val;
	uint8_t v[5];
	int	pre_rest_vol;
	uint16_t tmp;
/// for test GPIO 20121130

//	__gpio_set_value(GPIO_AXP(2), 1); /* __gpio_set_value */
/// for test GPIO 20121130

	charger = container_of(work, struct axp_charger, work.work);
	pre_rest_vol = charger->rest_vol;
	axp_charger_update_state(charger);
	axp_charger_update(charger);

	axp_read(charger->master, AXP22_CAP,&val);
	charger->rest_vol	= (int)	(val & 0x7F);
	
	if(axp_debug){
		DBG_PSY_MSG("charger->ic_temp = %d\n",charger->ic_temp);
		DBG_PSY_MSG("charger->vbat = %d\n",charger->vbat);
		DBG_PSY_MSG("charger->ibat = %d\n",charger->ibat);
		DBG_PSY_MSG("charger->vusb = %d\n",charger->vusb);
		DBG_PSY_MSG("charger->iusb = %d\n",charger->iusb);
		DBG_PSY_MSG("charger->vac = %d\n",charger->vac);
		DBG_PSY_MSG("charger->iac = %d\n",charger->iac);
		DBG_PSY_MSG("charger->ocv = %d\n",charger->ocv);
		DBG_PSY_MSG("charger->disvbat = %d\n",charger->disvbat);
		DBG_PSY_MSG("charger->disibat = %d\n",charger->disibat);
		//DBG_PSY_MSG("rt_rest_vol = %d\n",rt_rest_vol);
		DBG_PSY_MSG("charger->rest_vol = %d\n",charger->rest_vol);
		axp_reads(charger->master,0xba,2,v);
		//rdc = (((v[0] & 0x1F) << 8) | v[1]) * 10742 / 10000;
		//DBG_PSY_MSG("rdc = %d\n",rdc);
		DBG_PSY_MSG("bat_cap = %d\n",bat_cap);
		DBG_PSY_MSG("charger->is_on = %d\n",charger->is_on);
		DBG_PSY_MSG("charger->charge_on = %d\n",charger->charge_on);
		DBG_PSY_MSG("charger->ext_valid = %d\n",charger->ext_valid);
		DBG_PSY_MSG("count_dis = %d\n",count_dis);
		DBG_PSY_MSG("count_rdc = %d\n",count_rdc);
		DBG_PSY_MSG("pmu_init_chgcur           = %d\n",pmu_runtime_chgcur);
		DBG_PSY_MSG("pmu_earlysuspend_chgcur   = %d\n",pmu_earlysuspend_chgcur);
		DBG_PSY_MSG("pmu_suspend_chgcur        = %d\n",pmu_suspend_chgcur);
		DBG_PSY_MSG("pmu_resume_chgcur         = %d\n",pmu_resume_chgcur);
		DBG_PSY_MSG("pmu_shutdown_chgcur       = %d\n",pmu_shutdown_chgcur);
		//axp_reads(charger->master,AXP22_DATA_BUFFER0,12,data_mm);
		//for( mm = 0; mm < 12; mm++){
		//	DBG_PSY_MSG("REG[0x%x] = 0x%x\n",mm+AXP22_DATA_BUFFER0,data_mm[mm]);	
		//}
	}
	
	/* if battery volume changed, inform uevent */
	if(charger->rest_vol - pre_rest_vol){
		printk("battery vol change: %d->%d \n", pre_rest_vol, charger->rest_vol);
		pre_rest_vol = charger->rest_vol;
		axp_write(charger->master,AXP22_DATA_BUFFER1,charger->rest_vol | 0x80);
		power_supply_changed(&charger->batt);
	}
	/* reschedule for the next time */
	schedule_delayed_work(&charger->work, charger->interval);
}

static int axp_battery_probe(struct platform_device *pdev)
{
  struct axp_charger *charger;
  struct axp_supply_init_data *pdata = pdev->dev.platform_data;
  int ret,k,var;
  uint8_t val1,val2,tmp,val;
  uint8_t ocv_cap[31],v[2];
  int Cur_CoulombCounter,rdc;
  
  printk("axp_battery_probe enter...\n");
  powerkeydev = input_allocate_device();
  if (!powerkeydev) {
    kfree(powerkeydev);
    return -ENODEV;
  }

  powerkeydev->name = pdev->name;
  powerkeydev->phys = "m1kbd/input2";
  powerkeydev->id.bustype = BUS_HOST;
  powerkeydev->id.vendor = 0x0001;
  powerkeydev->id.product = 0x0001;
  powerkeydev->id.version = 0x0100;
  powerkeydev->open = NULL;
  powerkeydev->close = NULL;
  powerkeydev->dev.parent = &pdev->dev;

  set_bit(EV_KEY, powerkeydev->evbit);
  set_bit(EV_REL, powerkeydev->evbit);
  //set_bit(EV_REP, powerkeydev->evbit);
  set_bit(KEY_POWER, powerkeydev->keybit);

  ret = input_register_device(powerkeydev);
  if(ret) {
    printk("Unable to Register the power key\n");
    }

  if (pdata == NULL)
    return -EINVAL;

  printk("axp charger not limit now\n");
  if (pdata->chgcur > 2550000 ||
      pdata->chgvol < 4100000 ||
      pdata->chgvol > 4240000){
        printk("charger milliamp is too high or target voltage is over range\n");
        return -EINVAL;
    }

  if (pdata->chgpretime < 40 || pdata->chgpretime >70 ||
    pdata->chgcsttime < 360 || pdata->chgcsttime > 720){
            printk("prechaging time or constant current charging time is over range\n");
        return -EINVAL;
  }

  charger = kzalloc(sizeof(*charger), GFP_KERNEL);
  if (charger == NULL)
    return -ENOMEM;

  charger->master = pdev->dev.parent;

  charger->chgcur      = pdata->chgcur;
  charger->chgvol     = pdata->chgvol;
  charger->chgend           = pdata->chgend;
  charger->sample_time          = pdata->sample_time;
  charger->chgen                   = pdata->chgen;
  charger->chgpretime      = pdata->chgpretime;
  charger->chgcsttime = pdata->chgcsttime;
  charger->battery_info         = pdata->battery_info;
  charger->disvbat			= 0;
  charger->disibat			= 0;

  ret = axp_battery_first_init(charger);
  if (ret)
    goto err_charger_init;

  printk("add axp_battery_event to notifier[%x]\n", axp_battery_event);
  charger->nb.notifier_call = axp_battery_event;
  ret = axp_register_notifier(charger->master, &charger->nb, AXP22_NOTIFIER_ON);
  if (ret)
    goto err_notifier;

  axp_battery_setup_psy(charger);
  ret = power_supply_register(&pdev->dev, &charger->batt);
  if (ret)
    goto err_ps_register;

	axp_read(charger->master,AXP22_CHARGE_STATUS,&val);
	if(!((val >> 1) & 0x01)){
  	ret = power_supply_register(&pdev->dev, &charger->ac);
  	if (ret){
    	power_supply_unregister(&charger->batt);
    	goto err_ps_register;
  	}
  }
  ret = power_supply_register(&pdev->dev, &charger->usb);
  if (ret){
    power_supply_unregister(&charger->ac);
    power_supply_unregister(&charger->batt);
    goto err_ps_register;
  }

  ret = axp_charger_create_attrs(&charger->batt);
  if(ret){
    return ret;
  }

  platform_set_drvdata(pdev, charger);

  /* initial restvol*/

  /* usb current and voltage limit */
  if((pmu_usbvol) && (pmu_usbvol_limit)){
    axp_set_bits(charger->master, AXP22_CHARGE_VBUS, 0x40);
  	var = pmu_usbvol * 1000;
  	if(var >= 4000000 && var <=4700000){
    	tmp = (var - 4000000)/100000;
    	axp_read(charger->master, AXP22_CHARGE_VBUS,&val);
    	val &= 0xC7;
    	val |= tmp << 3;
    	axp_write(charger->master, AXP22_CHARGE_VBUS,val);
  	}
  }
  else
    axp_clr_bits(charger->master, AXP22_CHARGE_VBUS, 0x40);

  if((pmu_usbcur) && (pmu_usbcur_limit)){
    axp_clr_bits(charger->master, AXP22_CHARGE_VBUS, 0x01);
    var = pmu_usbcur * 1000;
  	if(var == 900000)
    	axp_clr_bits(charger->master, AXP22_CHARGE_VBUS, 0x03);
  	else if (var == 500000){
    	axp_clr_bits(charger->master, AXP22_CHARGE_VBUS, 0x02);
    	axp_set_bits(charger->master, AXP22_CHARGE_VBUS, 0x01);
  	}
  }
  else
    axp_set_bits(charger->master, AXP22_CHARGE_VBUS, 0x03);


  /* set lowe power warning/shutdown level
  var = script_parser_fetch("pmu_para", "pmu_battery_warning_level1", &pmu_battery_warning_level1, sizeof(int));
  if (var)
  {
     printk("[AXP]axp driver uning configuration failed(%d)\n", __LINE__);
     pmu_battery_warning_level1 = 15;
     printk("[AXP]pmu_warning_lev = %d\n",pmu_battery_warning_level1);
  }
  var = script_parser_fetch("pmu_para", "pmu_battery_warning_level2", &pmu_battery_warning_level2, sizeof(int));
  if (var)
  {
	 printk("[AXP]axp driver uning configuration failed(%d)\n", __LINE__);
	 pmu_battery_warning_level2 = 00;
	 printk("[AXP]pmu_pwroff_lev = %d\n",pmu_battery_warning_level2);
  }*/
  axp_write(charger->master, AXP22_WARNING_LEVEL,(pmu_battery_warning_level1 << 4)+pmu_battery_warning_level2);

  ocv_cap[0]  = pmu_bat_para1;
  ocv_cap[1]  = 0xC1;
  ocv_cap[2]  = pmu_bat_para2;
  ocv_cap[3]  = 0xC2;
  ocv_cap[4]  = pmu_bat_para3;
  ocv_cap[5]  = 0xC3;
  ocv_cap[6]  = pmu_bat_para4;
  ocv_cap[7]  = 0xC4;
  ocv_cap[8]  = pmu_bat_para5;
  ocv_cap[9]  = 0xC5;
  ocv_cap[10] = pmu_bat_para6;
  ocv_cap[11] = 0xC6;
  ocv_cap[12] = pmu_bat_para7;
  ocv_cap[13] = 0xC7;
  ocv_cap[14] = pmu_bat_para8;
  ocv_cap[15] = 0xC8;
  ocv_cap[16] = pmu_bat_para9;
  ocv_cap[17] = 0xC9;
  ocv_cap[18] = pmu_bat_para10;
  ocv_cap[19] = 0xCA;
  ocv_cap[20] = pmu_bat_para11;
  ocv_cap[21] = 0xCB;
  ocv_cap[22] = pmu_bat_para12;
  ocv_cap[23] = 0xCC;
  ocv_cap[24] = pmu_bat_para13;
  ocv_cap[25] = 0xCD;
  ocv_cap[26] = pmu_bat_para14;
  ocv_cap[27] = 0xCE;
  ocv_cap[28] = pmu_bat_para15;
  ocv_cap[29] = 0xCF;
  ocv_cap[30] = pmu_bat_para16;
  axp_writes(charger->master, 0xC0,31,ocv_cap);

  /* open/close set */
  printk("pmu_pokoff_time = %d\n",pmu_pokoff_time);
  printk("pmu_pokoff_en = %d\n",pmu_pokoff_en);
  printk("pmu_poklong_time = %d\n",pmu_poklong_time);
  printk("pmu_pokon_time = %d\n",pmu_pokon_time);
  printk("pmu_pwrok_time = %d\n",pmu_pwrok_time);

  /* n_oe delay time set 
	if (pmu_pwrnoe_time < 1000)
		pmu_pwrnoe_time = 128;
	if (pmu_pwrnoe_time > 3000)
		pmu_pwrnoe_time = 3000;
	axp_read(charger->master,AXP22_OFF_CTL,&val);
	val &= 0xfc;
	val |= ((pmu_pwrnoe_time) / 1000);
	axp_write(charger->master,AXP22_OFF_CTL,val);
	DBG_PSY_MSG("%d-->0x%x\n",__LINE__,val);
*/
	/* pok open time set */
	axp_read(charger->master,AXP22_POK_SET,&val);
	if (pmu_pokon_time < 1000)
		val &= 0x3f;
	else if(pmu_pokon_time < 2000){
		val &= 0x3f;
		val |= 0x80;
	}
	else if(pmu_pokon_time < 3000){
		val &= 0x3f;
		val |= 0xc0;
	}
	else {
		val &= 0x3f;
		val |= 0x40;
	}
	axp_write(charger->master,AXP22_POK_SET,val);
	printk("AXP22_POK_SET:%d-->0x%x\n",__LINE__,val);

	/* pok long time set*/
	if(pmu_poklong_time < 1000)
		pmu_poklong_time = 1000;
	if(pmu_poklong_time > 2500)
		pmu_poklong_time = 2500;
	axp_read(charger->master,AXP22_POK_SET,&val);
	val &= 0xcf;
	val |= (((pmu_poklong_time - 1000) / 500) << 4);
	axp_write(charger->master,AXP22_POK_SET,val);
	printk("AXP22_POK_SET:%d-->0x%x\n",__LINE__,val);

	/* pok en set*/
	if(pmu_pokoff_en)
		pmu_pokoff_en = 1;
	axp_read(charger->master,AXP22_POK_SET,&val);
	val &= 0xf7;
	val |= (pmu_pokoff_en << 3);
	axp_write(charger->master,AXP22_POK_SET,val);
	printk("AXP22_POK_SET:%d-->0x%x\n",__LINE__,val);

	/* pok delay set */
	if(pmu_pwrok_time <= 8)
		pmu_pwrok_time = 0;
	else
		pmu_pwrok_time = 1;
	axp_read(charger->master,AXP22_POK_SET,&val);
	val &= 0xfb;
	val |= pmu_pwrok_time << 2;
	axp_write(charger->master,AXP22_POK_SET,val);
	printk("AXP22_POK_SET:%d-->0x%x\n",__LINE__,val);

	/* pok off time set */
	if(pmu_pokoff_time < 4000)
		pmu_pokoff_time = 4000;
	if(pmu_pokoff_time > 10000)
		pmu_pokoff_time =10000;
	pmu_pokoff_time = (pmu_pokoff_time - 4000) / 2000 ;
	axp_read(charger->master,AXP22_POK_SET,&val);
	val &= 0xfc;
	val |= pmu_pokoff_time ;
	axp_write(charger->master,AXP22_POK_SET,val);
	printk("AXP22_POK_SET:%d-->0x%x\n",__LINE__,val);

	/* RDC initial */
	axp_read(charger->master,AXP22_HOTOVER_CTL,&val);
	if(!(val & 0x40))			//如果没正确检测过RDC，则initial RDC
	{
		rdc = (pmu_battery_rdc * 10000 + 5371) / 10742;
		axp_write(charger->master,AXP22_RDC1,rdc & 0x00FF);
		axp_write(charger->master, AXP22_RDC0, ((rdc >> 8) & 0x1F));
	}
  axp_set_bits(charger->master,0x8F,0x88); //enable IRQ waku and 16's restart up
//  axp_clr_bits(charger->master,0x81,0x04);
  /* set N_VBUSEN as an input 20121203 add by zhongweijin */
  axp_set_bits(charger->master,0x8F,0x10);
  /* set USB not current limit 2012-12-03 add by zhongweijin*/
  axp_set_bits(charger->master,0x30,0x02);
  
  axp_charger_update_state((struct axp_charger *)charger);

  axp_read(charger->master, AXP22_CAP,&val2);
	charger->rest_vol = (int) (val2 & 0x7F);

  printk("last_rest_vol = %d, now_rest_vol = %d\n",(val1 & 0x7F),(val2 & 0x7F));
  charger->interval = msecs_to_jiffies(10 * 1000);
  INIT_DELAYED_WORK(&charger->work, axp_charging_monitor);
  schedule_delayed_work(&charger->work, charger->interval);
/*
  var = script_parser_fetch("pmu_para", "pmu_used2", &pmu_used2, sizeof(int));
  if (var)
  {
     printk("axp driver uning configuration failed(%d)\n", __LINE__);
     pmu_used2 = 0;
     printk("pmu_used2 = %d\n",pmu_used2);
  }
*/
/*
  var = script_parser_fetch("pmu_para", "pmu_earlysuspend_chgcur", &pmu_earlysuspend_chgcur, sizeof(int));
  if (var)
  {
     printk("axp driver uning configuration failed(%d)\n", __LINE__);
     pmu_earlysuspend_chgcur = pmu_suspend_chgcur / 1000;
     printk("pmu_earlysuspend_chgcur = %d\n",pmu_earlysuspend_chgcur);
  }
  pmu_earlysuspend_chgcur = pmu_earlysuspend_chgcur * 1000;
  
  var = script_parser_fetch("pmu_para", "pmu_batdeten", &pmu_batdeten, sizeof(int));
  if (var)
  {
     printk("axp driver uning configuration failed(%d)\n", __LINE__);
     pmu_batdeten = 1;
     printk("pmu_batdeten = %d\n",pmu_batdeten);
  }
  if(!pmu_batdeten)
  	axp_clr_bits(charger->master,AXP22_PDBC,0x40);
  else
  	axp_set_bits(charger->master,AXP22_PDBC,0x40);
  	
  axp usb-pc limite
  
  var = script_parser_fetch("pmu_para", "pmu_usbvol_pc", &pmu_usbvolnew, sizeof(int));
  if (var)
  {
     printk("axp driver uning configuration failed-pmu_usbvol_pc\n");
     pmu_usbvolnew = 4000;
     printk("pmu_usbvolnew = %d\n",pmu_usbvolnew);
  }
  
  var = script_parser_fetch("pmu_para", "pmu_usbcur_pc", &pmu_usbcurnew, sizeof(int));
  if (var)
  {
     printk("axp driver uning configuration failed-pmu_usbcurnew\n");
     pmu_usbcurnew = 200;
     printk("pmu_usbcurnew = %d\n",pmu_usbcurnew);
  }
*/
//probe 时初始化RDC，使其提前计算正确的OCV，然后在此处启动计量系统
	if(pmu_battery_cap)
	{
		Cur_CoulombCounter = pmu_battery_cap * 1000 / 1456;
		axp_write(charger->master,AXP22_BATCAP1,Cur_CoulombCounter & 0x00FF);
		axp_write(charger->master, AXP22_BATCAP0, ((Cur_CoulombCounter >> 8) | 0x80));		
	}
	else
	{
		axp_write(charger->master,AXP22_BATCAP1,0x00);
		axp_write(charger->master, AXP22_BATCAP0, 0x00);	
	}

//#if defined (CONFIG_AXP_CHGCHANGE)
//  if(pmu_used2){
//  	gpio_adp_hdle = gpio_request_ex("pmu_para", "pmu_adpdet");
//  	if (!gpio_adp_hdle)
//    {
//       DBG_PSY_MSG("get adapter parameter failed\n");
//    }
//  }
//#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	axp_charger = charger;
    axp_early_suspend.suspend = axp_earlysuspend;
    axp_early_suspend.resume = axp_lateresume;
    axp_early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 2;
    register_early_suspend(&axp_early_suspend);
#endif
	/* 调试接口注册 */
	class_register(&axppower_class);
    printk("axp_battery_probe ok\n");
    return ret;

err_ps_register:
  axp_unregister_notifier(charger->master, &charger->nb, AXP22_NOTIFIER_ON);

err_notifier:
  cancel_delayed_work_sync(&charger->work);

err_charger_init:
  kfree(charger);
  input_unregister_device(powerkeydev);
  kfree(powerkeydev);
  return ret;
}

static int axp_battery_remove(struct platform_device *dev)
{
    struct axp_charger *charger = platform_get_drvdata(dev);

    if(main_task){
        kthread_stop(main_task);
        main_task = NULL;
    }

    axp_unregister_notifier(charger->master, &charger->nb, AXP22_NOTIFIER_ON);
    cancel_delayed_work_sync(&charger->work);
    power_supply_unregister(&charger->usb);
    power_supply_unregister(&charger->ac);
    power_supply_unregister(&charger->batt);

    kfree(charger);
    input_unregister_device(powerkeydev);
    kfree(powerkeydev);

    return 0;
}


static int axp22_suspend(struct platform_device *dev, pm_message_t state)
{
    uint8_t irq_w[9];
    uint8_t tmp;

    struct axp_charger *charger = platform_get_drvdata(dev);


	cancel_delayed_work_sync(&charger->work);

    /*clear all irqs events*/
    irq_w[0] = 0xff;
    irq_w[1] = AXP22_INTSTS2;
    irq_w[2] = 0xff;
    irq_w[3] = AXP22_INTSTS3;
    irq_w[4] = 0xff;
    irq_w[5] = AXP22_INTSTS4;
    irq_w[6] = 0xff;
    irq_w[7] = AXP22_INTSTS5;
    irq_w[8] = 0xff;
    axp_writes(charger->master, AXP22_INTSTS1, 9, irq_w);

    /* close all irqs*/
    axp_unregister_notifier(charger->master, &charger->nb, AXP22_NOTIFIER_ON);

#if defined (CONFIG_AXP_CHGCHANGE)
		if(pmu_suspend_chgcur == 0)
  		axp_clr_bits(charger->master,AXP22_CHARGE_CONTROL1,0x80);
  	else
  		axp_set_bits(charger->master,AXP22_CHARGE_CONTROL1,0x80);

  	printk("pmu_suspend_chgcur = %d\n", pmu_suspend_chgcur);

    if(pmu_suspend_chgcur >= 300000 && pmu_suspend_chgcur <= 2550000){
    tmp = (pmu_suspend_chgcur -200001)/150000;
    charger->chgcur = tmp *150000 + 300000;
    axp_update(charger->master, AXP22_CHARGE_CONTROL1, tmp,0x0F);
    }
#endif

    return 0;
}

static int axp22_resume(struct platform_device *dev)
{
    struct axp_charger *charger = platform_get_drvdata(dev);

    int pre_rest_vol,k;
    uint8_t val,val1,tmp;
    uint8_t v[2];
    int rt_rest_vol;
    int Cur_CoulombCounter;

    axp_register_notifier(charger->master, &charger->nb, AXP22_NOTIFIER_ON);

    axp_charger_update_state(charger);

		pre_rest_vol = charger->rest_vol;

		axp_read(charger->master, AXP22_CAP,&val);
		charger->rest_vol = val & 0x7f;

		if(charger->rest_vol - pre_rest_vol){
			printk("battery vol change: %d->%d \n", pre_rest_vol, charger->rest_vol);
			pre_rest_vol = charger->rest_vol;
			axp_write(charger->master,AXP22_DATA_BUFFER1,charger->rest_vol | 0x80);
			power_supply_changed(&charger->batt);
		}

#if defined (CONFIG_AXP_CHGCHANGE)
  	if(pmu_resume_chgcur == 0)
  		axp_clr_bits(charger->master,AXP22_CHARGE_CONTROL1,0x80);
  	else
  		axp_set_bits(charger->master,AXP22_CHARGE_CONTROL1,0x80);

  	printk("pmu_resume_chgcur = %d\n", pmu_resume_chgcur);

    if(pmu_resume_chgcur >= 300000 && pmu_resume_chgcur <= 2550000){
        tmp = (pmu_resume_chgcur -200001)/150000;
        charger->chgcur = tmp *150000 + 300000;
        axp_update(charger->master, AXP22_CHARGE_CONTROL1, tmp,0x0F);
    }
#endif

	charger->disvbat = 0;
	charger->disibat = 0;
    schedule_delayed_work(&charger->work, charger->interval);

    return 0;
}

static void axp22_shutdown(struct platform_device *dev)
{
    uint8_t tmp;
    struct axp_charger *charger = platform_get_drvdata(dev);
    
    cancel_delayed_work_sync(&charger->work);

#if defined (CONFIG_AXP_CHGCHANGE)
  	if(pmu_shutdown_chgcur == 0)
  		axp_clr_bits(charger->master,AXP22_CHARGE_CONTROL1,0x80);
  	else
  		axp_set_bits(charger->master,AXP22_CHARGE_CONTROL1,0x80);

		printk("pmu_shutdown_chgcur = %d\n", pmu_shutdown_chgcur);

    if(pmu_shutdown_chgcur >= 300000 && pmu_shutdown_chgcur <= 2550000){
    	tmp = (pmu_shutdown_chgcur -200001)/150000;
    	charger->chgcur = tmp *150000 + 300000;
    	axp_update(charger->master, AXP22_CHARGE_CONTROL1, tmp, 0x0F);
    }
#endif
}

static struct platform_driver axp_battery_driver = {
  .driver = {
    .name = "axp22-supplyer",
    .owner  = THIS_MODULE,
  },
  .probe = axp_battery_probe,
  .remove = axp_battery_remove,
  .suspend = axp22_suspend,
  .resume = axp22_resume,
  .shutdown = axp22_shutdown,
};

static int axp_battery_init(void)
{
  return platform_driver_register(&axp_battery_driver);
}

static void axp_battery_exit(void)
{
  platform_driver_unregister(&axp_battery_driver);
}

module_init(axp_battery_init);
module_exit(axp_battery_exit);

MODULE_DESCRIPTION("AXP22 battery charger driver");
MODULE_AUTHOR("Kyle Cheung");
MODULE_LICENSE("GPL");
