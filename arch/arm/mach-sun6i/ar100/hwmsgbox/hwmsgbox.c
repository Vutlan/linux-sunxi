/*
 *  arch/arm/mach-sun6i/ar100/hwmsgbox/hwmsgbox.c
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

#include "hwmsgbox_i.h"

/*
*********************************************************************************************************
*                                       	INITIALIZE HWMSGBOX
*
* Description: 	initialize hwmsgbox.
*
* Arguments  : 	none.
*
* Returns    : 	0 if initialize hwmsgbox succeeded, others if failed.
*********************************************************************************************************
*/
int ar100_hwmsgbox_init(void)
{
	int ret;
	
	/* register dma interrupt */
	ret = request_irq(AW_IRQ_MBOX, ar100_hwmsgbox_int_handler, 
					  IRQF_DISABLED, "ar100_hwmsgbox_irq", NULL);
	if(ret) {
		printk("request_irq error, return %d\n", ret);
		return ret;
	}
	
	return 0;
}

/*
*********************************************************************************************************
*                                       	EXIT HWMSGBOX
*
* Description: 	exit hwmsgbox.
*
* Arguments  : 	none.
*
* Returns    : 	0 if exit hwmsgbox succeeded, others if failed.
*********************************************************************************************************
*/
int ar100_hwmsgbox_exit(void)
{
	return 0;
}

/*
*********************************************************************************************************
*                                       SEND MESSAGE BY HWMSGBOX
*
* Description: 	send one message to another processor by hwmsgbox.
*
* Arguments  : 	pmessage 	: the pointer of sended message frame.
*				timeout		: the wait time limit when message fifo is full,
*							  it is valid only when parameter mode = HWMSG_SEND_WAIT_TIMEOUT.
*
* Returns    : 	0 if send message succeeded, other if failed.
*********************************************************************************************************
*/
int ar100_hwmsgbox_send_message(struct ar100_message *pmessage, unsigned int timeout)
{
	volatile unsigned long value;
	unsigned long          expire;
	
	expire = msecs_to_jiffies(timeout) + jiffies;
	
	if (pmessage == NULL) {
		return -EINVAL;
	}
	
	//use message-queue 1 as cpu1 transmitter.
	while (readl(IO_ADDRESS(AW_MSGBOX_FIFO_STATUS_REG(1))) == 1) {
		//message-queue fifo is full
		if (time_is_before_eq_jiffies(expire)) {
			return -ETIMEDOUT;
		}
	}
	//write message to message-queue fifo,
	//cpu1 use message-queue 1 as transmitter.
	value = ((volatile unsigned long)pmessage) - ar100_sram_a2_vbase; 
	AR100_LOG("ac327 send message : %x\n", (unsigned int)value);
	writel(value, IO_ADDRESS(AW_MSGBOX_MSG_REG(1)));
	
	//syn messsage must wait message feedback
	if ((pmessage->attr & AR100_MESSAGE_ATTR_SYN) &&
		(pmessage->state == AR100_MESSAGE_INITIALIZED)) {
		ar100_hwmsgbox_wait_message_feedback(pmessage);
	}
	
	return 0;
}

/*
*********************************************************************************************************
*                                       	ENABLE RECEIVER INT
*
* Description: 	enbale the receiver interrupt of message-queue.
*
* Arguments  : 	queue 	: the number of message-queue which we want to enable interrupt.
*				user	: the user which we want to enable interrupt.
*
* Returns    : 	0 if enable interrupt succeeded, others if failed.
*********************************************************************************************************
*/
int ar100_hwmsgbox_enable_receiver_int(int queue, int user)
{
	volatile unsigned int value;
	
	value  =  readl(IO_ADDRESS(AW_MSGBOX_IRQ_EN_REG(user)));
	value &= ~(0x1 << (queue * 2));
	value |=  (0x1 << (queue * 2));
	writel(value, IO_ADDRESS(AW_MSGBOX_IRQ_EN_REG(user)));
	
	return 0;
}

/*
*********************************************************************************************************
*                                       	QUERY PENDING
*
* Description: 	query the receiver interrupt pending of message-queue.
*
* Arguments  : 	queue 	: the number of message-queue which we want to query.
*				user	: the user which we want to query.
*
* Returns    : 	0 if query pending succeeded, others if failed.
*********************************************************************************************************
*/
int ar100_hwmsgbox_query_receiver_pending(int queue, int user)
{
	volatile unsigned long value;
	
	value  =  readl(IO_ADDRESS((AW_MSGBOX_IRQ_STATUS_REG(user))));
	
	return value & (0x1 << (queue * 2));
}

/*
*********************************************************************************************************
*                                       	CLEAR PENDING
*
* Description: 	clear the receiver interrupt pending of message-queue.
*
* Arguments  : 	queue 	: the number of message-queue which we want to clear.
*				user	: the user which we want to clear.
*
* Returns    : 	0 if clear pending succeeded, others if failed.
*********************************************************************************************************
*/
int ar100_hwmsgbox_clear_receiver_pending(int queue, int user)
{
	writel((0x1 << (queue * 2)), IO_ADDRESS(AW_MSGBOX_IRQ_STATUS_REG(user)));
	
	return 0;
}

/*
*********************************************************************************************************
*                                       	INT HANDLER
*
* Description: 	the interrupt handler for message-queue 1 receiver.
*
* Arguments  : 	parg 	: the argument of this handler.
*
* Returns    : 	TRUE if handle interrupt succeeded, others if failed.
*********************************************************************************************************
*/
irqreturn_t ar100_hwmsgbox_int_handler(int irq, void *dev)
{
	AR100_LOG("ac327 msgbox interrupt handler...\n");
	
	//use message-queue 1 as cpu0 receiver
	//process the received messages
	while (readl(IO_ADDRESS(AW_MSGBOX_MSG_STATUS_REG(0)))) {
		volatile unsigned long value;
		struct ar100_message *pmessage;
		value = readl(IO_ADDRESS(AW_MSGBOX_MSG_REG(0))) + ar100_sram_a2_vbase;
		pmessage = (struct ar100_message *)value;
		if (ar100_message_valid(pmessage)) {
			//message state switch
			if (pmessage->state == AR100_MESSAGE_PROCESSED) {
				//AR100_MESSAGE_PROCESSED->AR100_MESSAGE_FEEDBACKED,
				//process feedback message.
				pmessage->state = AR100_MESSAGE_FEEDBACKED;
				ar100_hwmsgbox_message_feedback(pmessage);
			} else {
				//AR100_MESSAGE_INITIALIZED->AR100_MESSAGE_RECEIVED,
				//notify new message coming.
				pmessage->state = AR100_MESSAGE_RECEIVED;
				ar100_message_coming_notify(pmessage);
			}
		} else {
			AR100_ERR("invalid message received\n");
		}
	}
	//clear pending
	ar100_hwmsgbox_clear_receiver_pending(0, 1);
	
	return IRQ_HANDLED;
}

/*
*********************************************************************************************************
*                                        QUERY MESSAGE
*
* Description: 	query message of hwmsgbox by hand, mainly for.
*
* Arguments  : 	timeout	: the timeout which we want to wait.
*
* Returns    : 	the point of message, NULL if timeout.
*********************************************************************************************************
*/
struct ar100_message *ar100_hwmsgbox_query_message(u32 timeout)
{
	struct ar100_message *pmessage;
	unsigned long         expire;
	
	expire = msecs_to_jiffies(timeout) + jiffies;
	
	//use message-queue 1 as ar100 receiver
	//process the received messages
	while (1) {
		if (readl(IO_ADDRESS(AW_MSGBOX_MSG_STATUS_REG(0)))) {
			volatile unsigned long value;
			value = readl(IO_ADDRESS(AW_MSGBOX_MSG_REG(0)));
			pmessage = (struct ar100_message *)(value + ar100_sram_a2_vbase);
			if (ar100_message_valid(pmessage)) {
				//message state switch
				if (pmessage->state == AR100_MESSAGE_PROCESSED) {
					//AR100_MESSAGE_PROCESSED->AR100_MESSAGE_FEEDBACKED
					pmessage->state = AR100_MESSAGE_FEEDBACKED;
				} else {
					//AR100_MESSAGE_INITIALIZED->AR100_MESSAGE_RECEIVED
					pmessage->state = AR100_MESSAGE_RECEIVED;
				}
				break;
			} else {
				AR100_ERR("invalid message received\n");
				return NULL;
			}
		}
		//check time out or not
		if (time_is_before_eq_jiffies(expire)) {
			return NULL;
		}
	}
	//clear pending
	ar100_hwmsgbox_clear_receiver_pending(0, 1);
	
	return pmessage;
}

int ar100_hwmsgbox_wait_message_feedback(struct ar100_message *pmessage)
{
	//linux method: wait semaphore flag to set.
	down((struct semaphore *)(pmessage->private));
	
	AR100_INF("message : %x finished\n", (unsigned int)pmessage);
	return 0;
}

int ar100_hwmsgbox_message_feedback(struct ar100_message *pmessage)
{
	//linux method: wait semaphore flag to set.
	up((struct semaphore *)(pmessage->private));
	
	return 0;
}

int ar100_message_valid(struct ar100_message *pmessage)
{
	if ((((u32)pmessage) >= (AR100_MESSAGE_POOL_START + ar100_sram_a2_vbase)) && 
		(((u32)pmessage) <  (AR100_MESSAGE_POOL_END   + ar100_sram_a2_vbase))) {
		//valid message
		return 1;
	}
	return 0;
}
