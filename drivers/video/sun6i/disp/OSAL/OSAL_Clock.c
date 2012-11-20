/*
********************************************************************************
*                                                   OSAL
*
*                                     (c) Copyright 2008-2009, Kevin China
*                                             				All Rights Reserved
*
* File    : OSAL_Clock.c
* By      : Sam.Wu
* Version : V1.00
* Date    : 2011/3/25 20:25 
* Description :  
* Update   :  date      author      version     notes    
********************************************************************************
*/
#include "OSAL.h"
#include "OSAL_Clock.h"

#ifndef __OSAL_CLOCK_MASK__

#define disp_clk_inf(clk_id, clk_name)   {.id = clk_id, .name = clk_name}

__disp_clk_t disp_clk_tbl[] =
{
    disp_clk_inf(SYS_CLK_PLL3,        "sys_pll3"      ),
    disp_clk_inf(SYS_CLK_PLL7,        "sys_pll7"      ),
    disp_clk_inf(SYS_CLK_PLL9,        "sys_pll9"      ),
    disp_clk_inf(SYS_CLK_PLL10,       "sys_pll10"     ),
    disp_clk_inf(SYS_CLK_PLL3X2,      "sys_pll3x2"    ),
    disp_clk_inf(SYS_CLK_PLL6,        "sys_pll6"      ),
    disp_clk_inf(SYS_CLK_PLL6x2,      "sys_pll6x2"    ),
    disp_clk_inf(SYS_CLK_PLL7X2,      "sys_pll7x2"    ),
    disp_clk_inf(SYS_CLK_MIPIPLL,     "sys_mipi-pll"  ),

    disp_clk_inf(MOD_CLK_DEBE0,       "mod_debe0"     ),
    disp_clk_inf(MOD_CLK_DEBE1,       "mod_debe1"     ),
    disp_clk_inf(MOD_CLK_DEFE0,       "mod_defe0"     ),
    disp_clk_inf(MOD_CLK_DEFE1,       "mod_defe1"     ),
    disp_clk_inf(MOD_CLK_DEMIX,       "mod_demp"      ),
    disp_clk_inf(MOD_CLK_LCD0CH0,     "mod_lcd0ch0"   ),
    disp_clk_inf(MOD_CLK_LCD0CH1,     "mod_lcd0ch1"   ),
    disp_clk_inf(MOD_CLK_LCD1CH0,     "mod_lcd1ch0"   ),
    disp_clk_inf(MOD_CLK_LCD1CH1,     "mod_lcd1ch1"   ),
    disp_clk_inf(MOD_CLK_HDMI,        "mod_hdmi"      ),
    disp_clk_inf(MOD_CLK_MIPIDSIS,    "mod_mipidsis"  ),
    disp_clk_inf(MOD_CLK_MIPIDSIP,    "mod_mipidsip"  ),
    disp_clk_inf(MOD_CLK_IEPDRC0,     "mod_iepdrc0"   ),
    disp_clk_inf(MOD_CLK_IEPDRC1,     "mod_iepdrc1"   ),
    disp_clk_inf(MOD_CLK_IEPDEU0,     "mod_iepdeu0"   ),
    disp_clk_inf(MOD_CLK_IEPDEU1,     "mod_iepdeu1"   ),
    disp_clk_inf(MOD_CLK_LVDS,        "mod_lvds"      ),
    
    disp_clk_inf(AHB_CLK_MIPIDSI,     "ahb_mipidsi"   ),
    disp_clk_inf(AHB_CLK_LCD0,        "ahb_lcd0"      ),
    disp_clk_inf(AHB_CLK_LCD1,        "ahb_lcd1"      ),
    disp_clk_inf(AHB_CLK_CSI0,        "ahb_csi0"      ),
    disp_clk_inf(AHB_CLK_CSI1,        "ahb_csi1"      ),
    disp_clk_inf(AHB_CLK_HDMID,       "ahb_hdmid"     ),
    disp_clk_inf(AHB_CLK_DEBE0,       "ahb_debe0"     ),
    disp_clk_inf(AHB_CLK_DEBE1,       "ahb_debe1"     ),
    disp_clk_inf(AHB_CLK_DEFE0,       "ahb_defe0"     ),
    disp_clk_inf(AHB_CLK_DEFE1,       "ahb_defe1"     ),
    disp_clk_inf(AHB_CLK_DEU0,        "ahb_deu0"      ),
    disp_clk_inf(AHB_CLK_DEU1,        "ahb_deu1"      ),
    disp_clk_inf(AHB_CLK_DRC0,        "ahb_drc0"      ),
    disp_clk_inf(AHB_CLK_DRC1,        "ahb_drc1"      ),
    
    disp_clk_inf(DRAM_CLK_DRC0,       "dram_drc0"     ),
    disp_clk_inf(DRAM_CLK_DRC1,       "dram_drc1"     ),
    disp_clk_inf(DRAM_CLK_DEU0,       "dram_deu0"     ),
    disp_clk_inf(DRAM_CLK_DEU1,       "dram_deu1"     ),
    disp_clk_inf(DRAM_CLK_DEFE0,      "dram_defe0"    ),
    disp_clk_inf(DRAM_CLK_DEFE1,      "dram_defe1"    ),
    disp_clk_inf(DRAM_CLK_DEBE0,      "dram_debe0"    ),
    disp_clk_inf(DRAM_CLK_DEBE1,      "dram_debe1"    ),
};

__s32 osal_ccmu_get_clk_name(__disp_clk_id_t clk_no, char *clk_name)
{
    __u32 i;
    __u32 count;

    count = sizeof(disp_clk_tbl)/sizeof(__disp_clk_t);
    __inf("osal_ccmu_get_clk_name, count=%d\n",count);

    for(i=0;i<count;i++)
    {
        if(disp_clk_tbl[i].id == clk_no)
        {
            memcpy(clk_name, disp_clk_tbl[i].name, strlen(disp_clk_tbl[i].name)+1);
            return 0;
        }
    }

    return -1;
}

__s32 OSAL_CCMU_SetSrcFreq( __u32 nSclkNo, __u32 nFreq )
{
    struct clk* hSysClk = NULL;
    s32 retCode = -1;
    char clk_name[20];


    if(osal_ccmu_get_clk_name(nSclkNo, clk_name) != 0)
    {
        __wrn("Fail to get clk name from clk id [%d].\n", nSclkNo);
        return -1;
    }
    __inf("OSAL_CCMU_SetSrcFreq,  _sysClkName[%d]=%s\n", nSclkNo,clk_name);
    
    hSysClk = clk_get(NULL, clk_name);
    
    if(NULL == hSysClk){
        __wrn("Fail to get handle for system clock [%d].\n", nSclkNo);
        return -1;
    }
    
    __inf("OSAL_CCMU_SetSrcFreq<%s,%d>\n",hSysClk->aw_clk->name, nFreq);

    if(nFreq == clk_get_rate(hSysClk)){
        __inf("Sys clk[%d] freq is alreay %d, not need to set.\n", nSclkNo, nFreq);
        clk_put(hSysClk);
        return 0;
    }
    retCode = clk_set_rate(hSysClk, nFreq);
    if(-1 == retCode){
        __wrn("Fail to set nFreq[%d] for sys clk[%d].\n", nFreq, nSclkNo);
        clk_put(hSysClk);
        return retCode;
    }
    clk_put(hSysClk);
    hSysClk = NULL;

    return retCode;
}

__u32 OSAL_CCMU_GetSrcFreq( __u32 nSclkNo )
{
    struct clk* hSysClk = NULL;
    u32 nFreq = 0;

    char clk_name[20];

    if(osal_ccmu_get_clk_name(nSclkNo, clk_name) != 0)
    {
        __wrn("Fail to get clk name from clk id [%d].\n", nSclkNo);
        return -1;
    }
    __inf("OSAL_CCMU_SetSrcFreq,  _sysClkName[%d]=%s\n", nSclkNo,clk_name);
    
    hSysClk = clk_get(NULL, clk_name);
    
    if(NULL == hSysClk){
        __wrn("Fail to get handle for system clock [%d].\n", nSclkNo);
        return -1;
    }
    nFreq = clk_get_rate(hSysClk);
    clk_put(hSysClk);
    hSysClk = NULL;

    return nFreq;
}

__hdle OSAL_CCMU_OpenMclk( __s32 nMclkNo )
{
    struct clk* hModClk = NULL;
    char clk_name[20];

    if(osal_ccmu_get_clk_name(nMclkNo, clk_name) != 0)
    {
        __wrn("Fail to get clk name from clk id [%d].\n", nMclkNo);
        return -1;
    }
    __inf("OSAL_CCMU_SetSrcFreq,  _sysClkName[%d]=%s\n", nMclkNo,clk_name);
    
    hModClk = clk_get(NULL, clk_name);
    
    return (__hdle)hModClk;
}

__s32 OSAL_CCMU_CloseMclk( __hdle hMclk )
{
    struct clk* hModClk = (struct clk*)hMclk;

    clk_put(hModClk);

    return 0;
}

__s32 OSAL_CCMU_SetMclkSrc( __hdle hMclk, __u32 nSclkNo )
{
    struct clk* hSysClk = NULL;
    struct clk* hModClk = (struct clk*)hMclk;
    s32 retCode = -1;

    char clk_name[20];

    if(osal_ccmu_get_clk_name(nSclkNo, clk_name) != 0)
    {
        __wrn("Fail to get clk name from clk id [%d].\n", nSclkNo);
        return -1;
    }
    __inf("OSAL_CCMU_SetSrcFreq,  _sysClkName[%d]=%s\n", nSclkNo,clk_name);
    
    hSysClk = clk_get(NULL, clk_name);

    if(NULL == hSysClk){
        __wrn("Fail to get handle for system clock [%d].\n", nSclkNo);
        return -1;
    }

    __inf("OSAL_CCMU_SetMclkSrc<%s,%s>\n",hModClk->aw_clk->name,hSysClk->aw_clk->name);
    
    if(clk_get_parent(hModClk) == hSysClk){
        __inf("Parent is alreay %d, not need to set.\n", nSclkNo);
        clk_put(hSysClk);
        return 0;
    }
    retCode = clk_set_parent(hModClk, hSysClk);
    if(-1 == retCode){
        __wrn("Fail to set parent for clk.\n");
        clk_put(hSysClk);
        return -1;
    }
    
    clk_put(hSysClk);

    return retCode;
}

__s32 OSAL_CCMU_GetMclkSrc( __hdle hMclk )
{
    int sysClkNo = 0;
#if 0
    struct clk* hModClk = (struct clk*)hMclk;
    struct clk* hParentClk = clk_get_parent(hModClk);
    const int TOTAL_SYS_CLK = sizeof(_sysClkName)/sizeof(char*);

    for (; sysClkNo <  TOTAL_SYS_CLK; sysClkNo++)
    {
        struct clk* tmpSysClk = clk_get(NULL, _sysClkName[sysClkNo]);
        
        if(tmpSysClk == NULL)
        	continue;

        if(hParentClk == tmpSysClk){
            clk_put(tmpSysClk);
            break;
        }
        clk_put(tmpSysClk);
    }

    if(sysClkNo >= TOTAL_SYS_CLK){
        __wrn("Failed to get parent clk.\n");
        return -1;
    }
#endif
    return sysClkNo;
}

__s32 OSAL_CCMU_SetMclkDiv( __hdle hMclk, __s32 nDiv )
{
    struct clk* hModClk     = (struct clk*)hMclk;
    struct clk* hParentClk  = clk_get_parent(hModClk);
    u32         srcRate     = clk_get_rate(hParentClk);

    __inf("OSAL_CCMU_SetMclkDiv<p:%s,m:%s,%d>\n", hParentClk->aw_clk->name, hModClk->aw_clk->name, nDiv);

    if(nDiv == 0){
    	return -1;
    }
    
    return clk_set_rate(hModClk, srcRate/nDiv);
}

__u32 OSAL_CCMU_GetMclkDiv( __hdle hMclk )
{
    struct clk* hModClk = (struct clk*)hMclk;
    struct clk* hParentClk = clk_get_parent(hModClk);
    u32 mod_freq = clk_get_rate(hModClk);
    
    if(mod_freq == 0){
    	return 0;	
    }

    return clk_get_rate(hParentClk)/mod_freq;
}

__s32 OSAL_CCMU_MclkOnOff( __hdle hMclk, __s32 bOnOff )
{
    struct clk* hModClk = (struct clk*)hMclk;
    __s32 ret = 0;

    //__inf("OSAL_CCMU_MclkOnOff<%s,%d>\n",hModClk->aw_clk->name,bOnOff);

    if(bOnOff)
    {
        if(!hModClk->enable)
        {
            ret = clk_enable(hModClk);
        }
    }
    else
    {
        while(hModClk->enable)
        {
            clk_disable(hModClk);
        }
    }
    return ret;
}

__s32 OSAL_CCMU_MclkReset(__hdle hMclk, __s32 bReset)
{
    struct clk* hModClk = (struct clk*)hMclk;

    __inf("OSAL_CCMU_MclkReset<%s,%d>\n",hModClk->aw_clk->name,bReset);

    return clk_reset(hModClk, bReset);
}
#else

typedef __u32 CSP_CCM_sysClkNo_t;


__s32 OSAL_CCMU_SetSrcFreq( CSP_CCM_sysClkNo_t nSclkNo, __u32 nFreq )
{
    return 0;
}

__u32 OSAL_CCMU_GetSrcFreq( CSP_CCM_sysClkNo_t nSclkNo )
{
    return 0;
}

__hdle OSAL_CCMU_OpenMclk( __s32 nMclkNo )
{
    return 0;
}

__s32 OSAL_CCMU_CloseMclk( __hdle hMclk )
{
    return 0;
}

__s32 OSAL_CCMU_SetMclkSrc( __hdle hMclk, CSP_CCM_sysClkNo_t nSclkNo )
{
    return 0;
}

__s32 OSAL_CCMU_GetMclkSrc( __hdle hMclk )
{
    return 0;
}

__s32 OSAL_CCMU_SetMclkDiv( __hdle hMclk, __s32 nDiv )
{
    return 0;
}

__u32 OSAL_CCMU_GetMclkDiv( __hdle hMclk )
{
    return 0;
}

__s32 OSAL_CCMU_MclkOnOff( __hdle hMclk, __s32 bOnOff )
{
    return 0;
}

__s32 OSAL_CCMU_MclkReset(__hdle hMclk, __s32 bReset)
{
    return 0;
}
#endif

