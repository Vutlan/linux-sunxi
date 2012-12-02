/*
*********************************************************************************************************
*                                                    LINUX-KERNEL
*                                        AllWinner Linux Platform Develop Kits
*                                                   Kernel Module
*
*                                    (c) Copyright 2006-2011, kevin.z China
*                                             All Rights Reserved
*
* File    : standby_clock.c
* By      : kevin.z
* Version : v1.0
* Date    : 2011-5-31 13:40
* Descript: ccmu process for platform standby;
* Update  : date                auther      ver     notes
*********************************************************************************************************
*/

#include "standby_i.h"

#define readb(addr)		(*((volatile unsigned char  *)(addr)))
#define readw(addr)		(*((volatile unsigned short *)(addr)))
#define readl(addr)		(*((volatile unsigned long  *)(addr)))
#define writeb(v, addr)	(*((volatile unsigned char  *)(addr)) = (unsigned char)(v))
#define writew(v, addr)	(*((volatile unsigned short *)(addr)) = (unsigned short)(v))
#define writel(v, addr)	(*((volatile unsigned long  *)(addr)) = (unsigned long)(v))

static void *r_prcm;
static __ccmu_reg_list_t   *CmuReg;
__u32   cpu_ms_loopcnt;

//==============================================================================
// CLOCK SET FOR SYSTEM STANDBY
//==============================================================================




/*
*********************************************************************************************************
*                           standby_clk_init
*
*Description: ccu init for platform standby
*
*Arguments  : none
*
*Return     : result,
*
*Notes      :
*
*********************************************************************************************************
*/
__s32 standby_clk_init(void)
{
    r_prcm = (void *)IO_ADDRESS(AW_R_PRCM_BASE);
    CmuReg = (__ccmu_reg_list_t   *)IO_ADDRESS(AW_CCM_BASE);
    

    return 0;
}


/*
*********************************************************************************************************
*                           standby_clk_exit
*
*Description: ccu exit for platform standby
*
*Arguments  : none
*
*Return     : result,
*
*Notes      :
*
*********************************************************************************************************
*/
__s32 standby_clk_exit(void)
{

    return 0;
}


/*
*********************************************************************************************************
*                                     standby_clk_hoscenable
*
* Description: enable HOSC.
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/
__s32 standby_clk_hoscenable(void)
{
	//cpus power domain, offset 0x40, how to enable?
#if 1
	CmuReg->SysClkDiv.CpuClkSrc = 1;
#endif

	
	return 0;
}


/*
*********************************************************************************************************
*                                     standby_clk_ldoenable
*
* Description: enable LDO.
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/
__s32 standby_clk_ldoenable(void)
{
	//cpus power domain, offset 0x44, how to enable?
#if 0
	CmuReg->HoscCtl.KeyField = 0x538;
	CmuReg->HoscCtl.LDOEn = 1;
	CmuReg->Pll5Ctl.LDO2En = 1;
	CmuReg->HoscCtl.KeyField = 0x00;
#endif
	__u32 tmp;
	tmp = readl(r_prcm + PLL_CTRL_REG1_OFFSET );
	tmp &= ~(0xff000000);
	tmp |= (0xa7000000);
	writel(tmp, r_prcm + PLL_CTRL_REG1_OFFSET);

	//enalbe ldo, ldo1
	tmp = readl(r_prcm + PLL_CTRL_REG1_OFFSET );
	tmp &= ~(0x00000003);
	tmp |= (0x00000003);
	writel(tmp, r_prcm + PLL_CTRL_REG1_OFFSET);

	//enalbe crystal
	tmp = readl(r_prcm + PLL_CTRL_REG1_OFFSET );
	tmp &= ~(0x00000004);
	tmp |= (0x00000004);
	writel(tmp, r_prcm + PLL_CTRL_REG1_OFFSET);

	//disable change.
	tmp = readl(r_prcm + PLL_CTRL_REG1_OFFSET );
	tmp &= ~(0xff000000);
	writel(tmp, r_prcm + PLL_CTRL_REG1_OFFSET);

	return 0;
}

