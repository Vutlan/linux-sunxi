/*
 *  arch/arm/mach-sun6i/ar100/ar100_dvfs.c
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

#include "..//ar100_i.h"

/*
 * set target frequency.
 * freq:  target frequency to be set, based on KHZ.
 * return: result, 0 - set frequency successed, !0 - set frequency failed;
 */
int ar100_dvfs_set_cpufreq(unsigned long freq, unsigned long mode)
{
	unsigned int          msg_attr;
	struct ar100_message *pmessage;
	
	msg_attr = 0;
	if (mode & AR100_DVFS_SYN) {
		msg_attr |= AR100_MESSAGE_ATTR_SYN;
	}
	//allocate a message frame
	pmessage = ar100_message_allocate(msg_attr);
	if (pmessage == NULL) {
		AR100_WRN("allocate message failed\n");
		return -ENOMEM;
	}
	//initialize message
	pmessage->type     = AR100_CPUX_DVFS_REQ;
	pmessage->attr     = (unsigned char)msg_attr;
	pmessage->paras[0] = freq;
	pmessage->state    = AR100_MESSAGE_INITIALIZED;
	ar100_hwmsgbox_send_message(pmessage, AR100_SEND_MSG_TIMEOUT);
	
	//dvfs mode : syn or not.
	if (mode & AR100_DVFS_SYN) {
		ar100_message_free(pmessage);
	}
	
	return 0;
}
EXPORT_SYMBOL(ar100_dvfs_set_cpufreq);
