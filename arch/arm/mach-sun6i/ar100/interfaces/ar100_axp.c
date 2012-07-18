/*
 *  arch/arm/mach-sun6i/ar100/ar100_axp.c
 *
 * Copyright (c) 2012 Allwinner.
 * sunny (sunny@allwinnertech.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "../ar100_i.h"

typedef struct axp_isr
{
	ar100_cb_t   handler;
	void        *arg;
} axp_isr_t;

//pmu isr node, record current pmu interrupt handler and argument
axp_isr_t axp_isr_node;

/*
 * axp power off.
 * para:  none;
 * return: result, 0 - power off successed, !0 - power off failed;
 */
int ar100_axp_power_off(void)
{
	struct ar100_message *pmessage;
	
	//allocate a message frame
	pmessage = ar100_message_allocate(0);
	if (pmessage == NULL) {
		AR100_ERR("allocate message for power-off request failed\n");
		return -ENOMEM;
	}
	//initialize message
	pmessage->type     = AR100_AXP_POWEROFF_REQ;
	pmessage->attr     = 0;
	pmessage->state    = AR100_MESSAGE_INITIALIZED;
	
	//send power-off request to ar100
	ar100_hwmsgbox_send_message(pmessage, AR100_SEND_MSG_TIMEOUT);
	
	return 0;
}
EXPORT_SYMBOL(ar100_axp_power_off);

/*
 * read axp register data.
 * addr: point of registers address;
 * data: point of registers data;
 * len : number of read registers;
 * return: result, 0 - read register successed, !0 - read register failed;
 */
int ar100_axp_read_reg(unsigned char *addr, unsigned char *data, unsigned long len)
{
	int                   i;
	struct ar100_message *pmessage;
	
	if ((addr == NULL) || (data == NULL) || (len > AXP_TRANS_BYTE_MAX)) {
		AR100_WRN("pmu read reg para error\n");
		return -EINVAL;
	}
	
	//allocate a message frame
	pmessage = ar100_message_allocate(AR100_MESSAGE_ATTR_SYN);
	if (pmessage == NULL) {
		AR100_WRN("allocate message failed\n");
		return -ENOMEM;
	}
	//initialize message
	pmessage->type     = AR100_AXP_READ_REGS;
	pmessage->attr     = AR100_MESSAGE_ATTR_SYN;
	pmessage->state    = AR100_MESSAGE_INITIALIZED;
	
	//package address and data to message->paras,
	//message->paras data layout: 
	//|para[0]|para[1]|para[2]|para[3]|para[4]|
	//|addr0~3|addr4~7|data0~3|data4~7|  len  |
	pmessage->paras[0] = 0;
	pmessage->paras[1] = 0;
	pmessage->paras[2] = 0;
	pmessage->paras[3] = 0;
	pmessage->paras[4] = len;
	for (i = 0; i < len; i++) {
		if (i < 4) {
			//pack 8bit addr0~addr3 into 32bit paras[0]
			pmessage->paras[0] |= (addr[i] << (i * 8));
		} else {
			//pack 8bit addr4~addr7 into 32bit paras[1]
			pmessage->paras[1] |= (addr[i] << ((i - 4) * 8));
		}
	}
	//send message use hwmsgbox
	ar100_hwmsgbox_send_message(pmessage, AR100_SEND_MSG_TIMEOUT);
	
	//copy message readout data to user data buffer
	for (i = 0; i < len; i++) {
		if (i < 4) {
			data[i] = ((pmessage->paras[2]) >> (i * 8)) & 0xff;
		} else {
			data[i] = ((pmessage->paras[3]) >> ((i - 4) * 8)) & 0xff;
		}
	}
	
	//free message
	ar100_message_free(pmessage);
	
	return 0;
}
EXPORT_SYMBOL(ar100_axp_read_reg);


/*
 * write axp register data.
 * addr: point of registers address;
 * data: point of registers data;
 * len : number of write registers;
 * return: result, 0 - write register successed, !0 - write register failed;
 */
int ar100_axp_write_reg(unsigned char *addr, unsigned char *data, unsigned long len)
{
	int                   i;
	struct ar100_message *pmessage;
	
	if ((addr == NULL) || (data == NULL) || (len > AXP_TRANS_BYTE_MAX)) {
		AR100_WRN("pmu write reg para error\n");
		return -EINVAL;
	}
	
	//allocate a message frame
	pmessage = ar100_message_allocate(AR100_MESSAGE_ATTR_SYN);
	if (pmessage == NULL) {
		AR100_WRN("allocate message failed\n");
		return -ENOMEM;
	}
	//initialize message
	pmessage->type  = AR100_AXP_WRITE_REGS;
	pmessage->attr  = AR100_MESSAGE_ATTR_SYN;
	pmessage->state = AR100_MESSAGE_INITIALIZED;
	
	//package address and data to message->paras,
	//message->paras data layout: 
	//|para[0]|para[1]|para[2]|para[3]|para[4]|
	//|addr0~3|addr4~7|data0~3|data4~7|  len  |
	pmessage->paras[0] = 0;
	pmessage->paras[1] = 0;
	pmessage->paras[2] = 0;
	pmessage->paras[3] = 0;
	pmessage->paras[4] = len;
	for (i = 0; i < len; i++) {
		if (i < 4) {
			//pack 8bit addr0~addr3 into 32bit paras[0]
			pmessage->paras[0] |= (addr[i] << (i * 8));
			
			//pack 8bit data0~data3 into 32bit paras[2]
			pmessage->paras[2] |= (data[i] << (i * 8));
		} else {
			//pack 8bit addr4~addr7 into 32bit paras[1]
			pmessage->paras[1] |= (addr[i] << ((i - 4) * 8));
			
			//pack 8bit data4~data7 into 32bit paras[3]
			pmessage->paras[3] |= (data[i] << ((i - 4) * 8));
		}
	}
	//send message use hwmsgbox
	ar100_hwmsgbox_send_message(pmessage, AR100_SEND_MSG_TIMEOUT);
	
	//free message
	ar100_message_free(pmessage);
	
	return 0;
}
EXPORT_SYMBOL(ar100_axp_write_reg);


/*
 * axp get battery paramter.
 * para:  battery parameter;
 * return: result, 0 - get battery successed, !0 - get battery failed;
 */
int ar100_axp_get_battery(void *para)
{
	struct ar100_message *pmessage;
	
	//allocate a message frame
	pmessage = ar100_message_allocate(AR100_MESSAGE_ATTR_SYN);
	if (pmessage == NULL) {
		AR100_ERR("allocate message for get battery request failed\n");
		return -ENOMEM;
	}
	//initialize message
	pmessage->type  = AR100_AXP_GET_BATTERY;
	pmessage->attr  = AR100_MESSAGE_ATTR_SYN;
	pmessage->state = AR100_MESSAGE_INITIALIZED;
	
	//send set battery request to ar100
	ar100_hwmsgbox_send_message(pmessage, AR100_SEND_MSG_TIMEOUT);
	
	//syn message, free message
	ar100_message_free(pmessage);
	
	return 0;
}
EXPORT_SYMBOL(ar100_axp_get_battery);


/*
 * axp set battery paramter.
 * para:  battery parameter;
 * return: result, 0 - set battery successed, !0 - set battery failed;
 */
int ar100_axp_set_battery(void *para)
{
	struct ar100_message *pmessage;
	
	//allocate a message frame
	pmessage = ar100_message_allocate(AR100_MESSAGE_ATTR_SYN);
	if (pmessage == NULL) {
		AR100_ERR("allocate message for set battery request failed\n");
		return -ENOMEM;
	}
	//initialize message
	pmessage->type  = AR100_AXP_SET_BATTERY;
	pmessage->attr  = AR100_MESSAGE_ATTR_SYN;
	pmessage->state = AR100_MESSAGE_INITIALIZED;
	
	//send set battery request to ar100
	ar100_hwmsgbox_send_message(pmessage, AR100_SEND_MSG_TIMEOUT);
	
	//syn message, free message
	ar100_message_free(pmessage);
	
	return 0;
}
EXPORT_SYMBOL(ar100_axp_set_battery);


/*
 * register call-back function, call-back function is for ar100 notify some event to ac327,
 * axp interrupt for ex.
 * func:  call-back function;
 * para:  parameter for call-back function;
 * return: result, 0 - register call-back function successed;
 *                !0 - register call-back function failed;
 * NOTE: the function is like "int callback(void *para)";
 */
int ar100_cb_register(ar100_cb_t func, void *para)
{
	if (axp_isr_node.handler) {
		//just output warning message, overlay handler.
		AR100_WRN("pmu interrupt handler register already\n");
		return -EINVAL;
	}
	axp_isr_node.handler = func;
	axp_isr_node.arg     = para;
	
	return 0;
}
EXPORT_SYMBOL(ar100_cb_register);


/*
 * unregister call-back function.
 * func:  call-back function which need be unregister;
 */
void ar100_cb_unregister(ar100_cb_t func)
{
	if ((u32)(axp_isr_node.handler) != (u32)(func)) {
		//invalid handler.
		AR100_WRN("invalid handler for unreg\n\n");
		return ;
	}
	axp_isr_node.handler = NULL;
	axp_isr_node.arg     = NULL;
}
EXPORT_SYMBOL(ar100_cb_unregister);


int ar100_axp_int_notify(struct ar100_message *pmessage)
{
	if (axp_isr_node.handler == NULL) {
		AR100_WRN("pmu isr not install\n");
		return -EINVAL;
	}
	//call pmu interrupt handler
	return (*(axp_isr_node.handler))(axp_isr_node.arg);
}
