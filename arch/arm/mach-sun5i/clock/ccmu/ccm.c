/*
*********************************************************************************************************
*                                                    LINUX-KERNEL
*                                        AllWinner Linux Platform Develop Kits
*                                                   Kernel Module
*
*                                    (c) Copyright 2006-2011, kevin.z China
*                                             All Rights Reserved
*
* File    : ccm.c
* By      : kevin.z
* Version : v1.0
* Date    : 2011-5-13 18:42
* Descript:
* Update  : date                auther      ver     notes
*********************************************************************************************************
*/
#include <mach/platform.h>
#include <mach/clock.h>
#include "ccm_i.h"

#define __reg_value(x)          (*(volatile __u32 *)(x))
unsigned int timer_clk_version;

__ccmu_reg_list_t   *aw_ccu_reg;


/*
*********************************************************************************************************
*                           aw_ccu_init
*
*Description: initialise clock mangement unit;
*
*Arguments  : none
*
*Return     : result,
*               AW_CCMU_OK,     initialise ccu successed;
*               AW_CCMU_FAIL,   initialise ccu failed;
*
*Notes      :
*
*********************************************************************************************************
*/
__s32 aw_ccu_init(void)
{
    /* initialise the CCU io base */
    aw_ccu_reg = (__ccmu_reg_list_t *)SW_VA_CCM_IO_BASE;

    /* config the CCU to default status */

    /* get timer clock version */
    __reg_value(0xf1c20060) |= 0x01<<5;
    __reg_value(0xf1c2009c)  = 0x80000000;
    timer_clk_version = (__reg_value(0xf1c15000) >>16)&0x7;
    __reg_value(0xf1c2009c)  = 0x00000000;
    __reg_value(0xf1c20060) &= ~(0x01<<5);

    #if(USE_PLL6M_REPLACE_PLL4)
    /* switch pll4 output to pll6 */
    aw_ccu_reg->Pll4Ctl.PllSwitch = 2;
    #else
    aw_ccu_reg->Pll4Ctl.PllSwitch = 0;
    #endif

    return AW_CCU_ERR_NONE;
}


/*
*********************************************************************************************************
*                           aw_ccu_exit
*
*Description: exit clock managment unit;
*
*Arguments  : none
*
*Return     : result,
*               AW_CCMU_OK,     exit ccu successed;
*               AW_CCMU_FAIL,   exit ccu failed;
*
*Notes      :
*
*********************************************************************************************************
*/
__s32 aw_ccu_exit(void)
{
    return AW_CCU_ERR_NONE;
}


/*
*********************************************************************************************************
*                           fix timer clock
*
*Description: fix timer clock with timer version;
*
*Arguments  : none
*
*Return     : none
*
*Notes      :
*
*********************************************************************************************************
*/
void fix_timer_clock(void)
{
    if(timer_clk_version == 1) {
        if(__reg_value(0xf1c20064)&(1<<4)) {
            __reg_value(0xf1c20064) = (__reg_value(0xf1c0c048) & 0x07ff) > 0x0321?     \
                            (__reg_value(0xf1c20064) & 0xffffafef) : __reg_value(0xf1c20064);
            __reg_value(0xf1c20064) = (__reg_value(0xf1c0c048) & 0x07ff0000) > 0x3210000?     \
                            (__reg_value(0xf1c20064) & 0xffffafef) : __reg_value(0xf1c20064);
        }
    }
}

