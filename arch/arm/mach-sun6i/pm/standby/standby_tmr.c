/*
*********************************************************************************************************
*                                                    eMOD
*                                   the Easy Portable/Player Operation System
*                                            power manager sub-system
*
*                                     (c) Copyright 2008-2009, kevin.z China
*                                              All Rights Reserved
*
* File   : standby_tmr.c
* Version: V1.0
* By     : kevin.z
* Date   : 2009-7-22 18:31
*********************************************************************************************************
*/
#include "standby_i.h"

static __u32 *TmrReg;
static __u32 WatchDog1_Config_Reg_Bak, WatchDog1_Mod_Reg_Bak;
/*
*********************************************************************************************************
*                                     TIMER INIT
*
* Description: initialise timer for standby.
*
* Arguments  : none
*
* Returns    : EPDK_TRUE/EPDK_FALSE;
*********************************************************************************************************
*/
__s32 standby_tmr_init(void)
{
	/* set timer register base */
	TmrReg = (__u32 *)(IO_ADDRESS(AW_TIMER_BASE));
	WatchDog1_Config_Reg_Bak = *(volatile __u32 *)(TmrReg + WatchDog1_Config_Offset);
	WatchDog1_Mod_Reg_Bak = *(volatile __u32 *)(TmrReg + WatchDog1_Mod_Offset);

	return 0;
}


/*
*********************************************************************************************************
*                                     TIMER EXIT
*
* Description: exit timer for standby.
*
* Arguments  : none
*
* Returns    : EPDK_TRUE/EPDK_FALSE;
*********************************************************************************************************
*/
__s32 standby_tmr_exit(void)
{
	*(volatile __u32 *)(TmrReg + WatchDog1_Config_Offset) = WatchDog1_Config_Reg_Bak;
	*(volatile __u32 *)(TmrReg + WatchDog1_Mod_Offset) = WatchDog1_Mod_Reg_Bak;

	return 0;
}


/*
*********************************************************************************************************
*                           standby_tmr_enable_watchdog
*
*Description: enable watch-dog.
*
*Arguments  : none.
*
*Return     : none;
*
*Notes      :
*
*********************************************************************************************************
*/
void standby_tmr_enable_watchdog(void)
{
	/* set watch-dog reset to whole system*/ 
	*(volatile __u32 *)(TmrReg + WatchDog1_Config_Offset) &= ~(0x3);
	*(volatile __u32 *)(TmrReg + WatchDog1_Config_Offset) |= 0x1;
	/*  timeout is 2 seconds */
	*(volatile __u32 *)(TmrReg + WatchDog1_Mod_Offset) = (2<<4);

	/* enable watch-dog */
	*(volatile __u32 *)(TmrReg + WatchDog1_Mod_Offset) |= (1<<0);

	return;
}


/*
*********************************************************************************************************
*                           standby_tmr_disable_watchdog
*
*Description: disable watch-dog.
*
*Arguments  : none.
*
*Return     : none;
*
*Notes      :
*
*********************************************************************************************************
*/
void standby_tmr_disable_watchdog(void)
{
    /* disable watch-dog reset: only intterupt */
    *(volatile __u32 *)(TmrReg + WatchDog1_Config_Offset) &= ~(0x3);
    *(volatile __u32 *)(TmrReg + WatchDog1_Config_Offset) |= 0x2;
    /* disable watch-dog */
    *(volatile __u32 *)(TmrReg + WatchDog1_Mod_Offset) &= ~(1<<0);
}


