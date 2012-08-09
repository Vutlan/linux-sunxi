/*
 *  arch/arm/mach-sun6i/ar100/ar100.c
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

#include "ar100_i.h"

//local functions
static int __ar100_wait_ready(unsigned int timeout);

unsigned long ar100_sram_a2_vbase = (unsigned long)IO_ADDRESS(AW_SRAM_A2_BASE);

extern char *ar100_binary_start;
extern char *ar100_binary_end;

int ar100_init(void)
{
	int binary_len;
	
	AR100_INF("ar100 initialize\n");
	
	//remap sram_a2 space
	//ar100_sram_a2_vbase = (unsigned long)ioremap(AW_SRAM_A2_BASE, AW_SRAM_A2_SIZE);
	if (ar100_sram_a2_vbase == 0) {
		AR100_ERR("ioremap sram a2 space failed\n");
		return -ENOMEM;
	}
	AR100_INF("ioremap sram a2 space to vaddr(%x)\n", (unsigned int)ar100_sram_a2_vbase);
	
	//load ar100 system binary data to sram_a2
	binary_len = (int)(&ar100_binary_end) - (int)(&ar100_binary_start);
	memcpy((void *)ar100_sram_a2_vbase, (void *)(&ar100_binary_start), binary_len);
	AR100_INF("move ar100 binary data [addr = %x, len = %x] to sram_a2 finished\n", 
	         (unsigned int)(&ar100_binary_start), (unsigned int)binary_len);
	
	//initialize hwspinlock
	AR100_INF("hwspinlock initialize\n");
	ar100_hwspinlock_init();
	
	//initialize hwmsgbox
	AR100_INF("hwmsgbox initialize\n");
	ar100_hwmsgbox_init();
	
	//initialize message manager
	AR100_INF("message manager initialize\n");
	ar100_message_manager_init();
	
	//set ar100 cpu reset to de-assert state
	AR100_INF("set ar100 reset to de-assert state\n");
	{
		volatile unsigned long value;
		value = readl((IO_ADDRESS(AW_R_CPUCFG_BASE) + 0x0));
		value |= 1;
		writel(value, (IO_ADDRESS(AW_R_CPUCFG_BASE) + 0x0));
	}
	
	//wait ar100 ready
	AR100_INF("wait ar100 ready....\n");
	if (__ar100_wait_ready(500)) {
		AR100_LOG("ar100 startup failed\n");
	}
	
	AR100_INF("ar100 startup succeeded\n");
	
	//enable ar100 asyn tx interrupt.
	ar100_hwmsgbox_enable_receiver_int(AR100_HWMSGBOX_AR100_ASYN_TX_CH, AW_HWMSG_QUEUE_USER_AC327);
	
	//enable ar100 syn tx interrupt.
	ar100_hwmsgbox_enable_receiver_int(AR100_HWMSGBOX_AR100_SYN_TX_CH, AW_HWMSG_QUEUE_USER_AC327);
	
	return 0;
}
device_initcall(ar100_init);

static int __ar100_wait_ready(unsigned int timeout)
{
	unsigned long          expire;
	
	expire = msecs_to_jiffies(timeout) + jiffies;
	
	//wait ar100 startup ready.
	while (1) {
		//linux cpu interrupt is disable now, 
		//we should query message by hand.
		struct ar100_message *pmessage = ar100_hwmsgbox_query_message();
		if (pmessage == NULL) {
			if (time_is_before_eq_jiffies(expire)) {
				return -ETIMEDOUT;
			}
			//try to query again
			continue;
		}
		//query valid message
		if (pmessage->type == AR100_STARTUP_NOTIFY) {
			//received ar100 startup ready message.
			AR100_INF("ar100 startup ready\n");
			if ((pmessage->attr & AR100_MESSAGE_ATTR_SOFTSYN) ||
				(pmessage->attr & AR100_MESSAGE_ATTR_HARDSYN)) {
				//synchronous message, just feedback it.
				AR100_INF("ar100 startup notify message feedback\n");
				ar100_hwmsgbox_feedback_message(pmessage, AR100_SEND_MSG_TIMEOUT);
			} else {
				//asyn message, free message directly.
				AR100_INF("ar100 startup notify message free directly\n");
				ar100_message_free(pmessage);
			}
			break;
		}
		//invalid message detected, ignore it.
		//by sunny at 2012-7-6 18:34:38.
		AR100_WRN("ar100 startup waiting ignore message\n");
		if ((pmessage->attr & AR100_MESSAGE_ATTR_SOFTSYN) ||
			(pmessage->attr & AR100_MESSAGE_ATTR_HARDSYN)) {
			//synchronous message, just feedback it.
			ar100_hwmsgbox_send_message(pmessage, AR100_SEND_MSG_TIMEOUT);
		} else {
			//asyn message, free message directly.
			ar100_message_free(pmessage);
		}
		//we need waiting continue.
	}
	return 0;
}
