/*
 *  arch/arm/mach-sun6i/include/mach/memory.h
 *
 * Copyright (c) Allwinner.  All rights reserved.
 * Benn Huang (benn@allwinnertech.com)
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
#ifndef __ASM_ARCH_MEMORY_H
#define __ASM_ARCH_MEMORY_H

#define PLAT_PHYS_OFFSET	UL(0x40000000)
#define PLAT_MEM_SIZE		SZ_1G

#define SUPER_STANDBY_MEM_BASE	(PLAT_PHYS_OFFSET + SZ_64M + SZ_32M)
#define SUPER_STANDBY_MEM_SIZE	(SZ_1K)

#define SYS_CONFIG_MEMBASE      (PLAT_PHYS_OFFSET + SZ_32M + SZ_16M)
#define SYS_CONFIG_MEMSIZE      (SZ_64K)

#define VE_MEM_SIZE             (SZ_128M + SZ_64M + SZ_8M)
#define VE_MEM_BASE             (PLAT_PHYS_OFFSET + PLAT_MEM_SIZE - VE_MEM_SIZE)

/* g2d memory reserve, same as a1x */
#define G2D_MEM_BASE            (PLAT_PHYS_OFFSET + SZ_512M - SZ_64M - SZ_32M)
#define G2D_MEM_SIZE            (SZ_16M)

#endif
