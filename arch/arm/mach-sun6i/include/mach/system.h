/*
 *  arch/arm/mach-sun6i/include/mach/system.h
 *
 *  Copyright (C) 2012-2016 Allwinner Limited
 *  Benn Huang (benn@allwinnertech.com)
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
#ifndef __ASM_ARCH_SYSTEM_H
#define __ASM_ARCH_SYSTEM_H
#include <linux/io.h>
#include <asm/proc-fns.h>
#include <mach/hardware.h>
#include <mach/platform.h>
#include <asm/delay.h>

static inline void arch_idle(void)
{
	/*
	 * This should do all the clock switching
	 * and wait for interrupt tricks
	 */
	cpu_do_idle();
}

static inline void arch_reset(char mode, const char *cmd)
{
    /* use watch-dog to reset system */
/*
    #define WATCH_DOG_CTRL_REG  (SW_VA_TIMERC_IO_BASE + 0x0094)
    *(volatile unsigned int *)WATCH_DOG_CTRL_REG = 0;
    __delay(100000);
    *(volatile unsigned int *)WATCH_DOG_CTRL_REG |= 2;
    while(1) {
        __delay(100);
        *(volatile unsigned int *)WATCH_DOG_CTRL_REG |= 1;
    }
*/
}

/*
 * define chip version
 */
enum sw_ic_ver {
	MAGIC_VER_NULL      = 0,        /* invalid value        */
	MAGIC_VER_UNKNOWN   = 1,        /* unknown version      */
	MAGIC_VER_A31       = 0xA310,   /* chip version a31     */
	MAGIC_VER_A31S      = 0xA315,   /* chip version a31s    */
	MAGIC_VER_A3XP      = 0xA388,   /* chip versiion a3x    */
};
enum sw_ic_ver sw_get_ic_ver(void);

#endif
