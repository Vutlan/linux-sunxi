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

unsigned long ar100_sram_a2_vbase;

int ar100_init(void)
{
	AR100_INF("ar100 initialize\n");
	
	//remap sram_a2 space
	ar100_sram_a2_vbase = ioremap(AW_SRAM_A2_BASE, AW_SRAM_A2_SIZE);
	if (ar100_sram_a2_vbase == NULL) {
		AR100_LOG("ioremap sram a2 space failed\n");
		return -ENOMEM;
	}
	AR100_LOG("ioremap sram a2 space to vaddr(%x)\n", ar100_sram_a2_vbase);
	
	//load ar100 system binary file to sram_a2
	//...
	
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
	if (__ar100_wait_ready(50)) {
		AR100_LOG("ar100 startup failed\n");
	}
	
	AR100_INF("ar100 startup succeeded\n");
	
	//enable message-queue 0 receiver interrupt.
	ar100_hwmsgbox_enable_receiver_int(0, AW_HWMSG_QUEUE_USER1);
	
	return 0;
}
device_initcall(ar100_init);

static int __ar100_wait_ready(unsigned int timeout)
{
	//wait ar100 startup ready.
	while (1) {
		//linux cpu interrupt is disable now, 
		//we should query message by hand.
		struct ar100_message *pmessage = ar100_hwmsgbox_query_message(timeout);
		if (pmessage == NULL) {
			//maybe timeout;
			return -EBUSY;
		}
		if (pmessage->type == AR100_STARTUP_NOTIFY) {
			//received ar100 startup ready message.
			AR100_INF("ar100 startup ready\n");
			if (pmessage->attr & AR100_MESSAGE_ATTR_SYN) {
				//synchronous message, just feedback it.
				ar100_hwmsgbox_send_message(pmessage, AR100_SEND_MSG_TIMEOUT);
			} else {
				//asyn message, free message directly.
				ar100_message_free(pmessage);
			}
			break;
		}
		//invalid message detected, ignore it.
		//by sunny at 2012-7-6 18:34:38.
		AR100_WRN("ar100 startup waiting ignore message\n");
		if (pmessage->attr & AR100_MESSAGE_ATTR_SYN) {
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
