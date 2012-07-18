/*
 *  arch/arm/mach-sun6i/ar100/include/ar100_hwmsgbox.h
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

#ifndef	__AR100_HWMSGBOX_H__
#define	__AR100_HWMSGBOX_H__

/*
*********************************************************************************************************
*                                       	INITIALIZE HWMSGBOX
*
* Description: 	initialize hwmsgbox.
*
* Arguments  : 	none.
*
* Returns    : 	OK if initialize hwmsgbox succeeded, others if failed.
*********************************************************************************************************
*/
int ar100_hwmsgbox_init(void);

/*
*********************************************************************************************************
*                                       	EXIT HWMSGBOX
*
* Description: 	exit hwmsgbox.
*
* Arguments  : 	none.
*
* Returns    : 	OK if exit hwmsgbox succeeded, others if failed.
*********************************************************************************************************
*/
int ar100_hwmsgbox_exit(void);

/*
*********************************************************************************************************
*                                       SEND MESSAGE BY HWMSGBOX
*
* Description: 	send one message to another processor by hwmsgbox.
*
* Arguments  : 	pmessage 	: the pointer of sended message frame.
*				timeout		: the wait time limit when message fifo is full,
*							  it is valid only when parameter mode = SEND_MESSAGE_WAIT_TIMEOUT.
*
* Returns    : 	OK if send message succeeded, other if failed.
*********************************************************************************************************
*/
int ar100_hwmsgbox_send_message(struct ar100_message *pmessage, unsigned int timeout);

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
struct ar100_message *ar100_hwmsgbox_query_message(u32 timeout);

int ar100_hwmsgbox_enable_receiver_int(int queue, int user);

#endif	//__AR100_HWMSGBOX_H__
