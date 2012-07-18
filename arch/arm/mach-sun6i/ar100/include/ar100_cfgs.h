/*
 *  arch/arm/mach-sun6i/ar100/include/ar100_cfgs.h
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

#ifndef	__AR100_CFGS_H__
#define __AR100_CFGS_H__

//debugger system
#define	AR100_DEBUG_LEVEL			(3)		//debug level

//the max number of cached message frame
#define	AR100_MESSAGE_CACHED_MAX	(4)

//the start address of message pool
#define AR100_MESSAGE_POOL_START	(0x13C00)
#define AR100_MESSAGE_POOL_END		(0x14000)

//spinlock max timeout, base on ms
#define AR100_SPINLOCK_TIMEOUT		(10)

//send message max timeout, base on ms
#define AR100_SEND_MSG_TIMEOUT		(10)

#endif //__AR100_CFGS_H__
