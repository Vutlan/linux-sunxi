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

#ifndef __FPGA_DEBUG__
static char* _sysClkName[AW_CCU_CLK_CNT] =
{
    "none"            ,
    "sys_losc"         ,
    "sys_hosc"         ,
    "sys_pll1"         ,
    "sys_pll2"         ,
    "sys_pll3"         ,
    "sys_pll4"         ,
    "sys_pll5"         ,
    "sys_pll6"         ,
    "sys_pll7"         ,
    "sys_pll8"         ,
    "sys_pll9"         ,
    "sys_pll10"        ,
    "sys_pll2X8"       ,
    "sys_pll3X2"       ,
    "sys_pll6X2"       ,
    "sys_pll7X2"       ,
    "sys_mipi_pll"     ,
    "sys_ac327"        ,
    "sys_ar100"        ,
    "sys_axi"          ,
    "sys_ahb0"         ,
    "sys_ahb1"         ,
    "sys_apb0"         ,
    "sys_apb1"         ,
    "sys_apb2"         ,
    "none"             ,
    "mod_nand0"        ,
    "mod_nand1"        ,
    "mod_sdc0"         ,
    "mod_sdc1"         ,
    "mod_sdc2"         ,
    "mod_sdc3"         ,
    "mod_ts"           ,
    "mod_ss"           ,
    "mod_spi0"         ,
    "mod_spi1"         ,
    "mod_spi2"         ,
    "mod_spi3"         ,
    "mod_i2s0"         ,
    "mod_i2s1"         ,
    "mod_spdif"        ,
    "mod_usbphy0"      ,
    "mod_usbphy1"      ,
    "mod_usbphy2"      ,
    "mod_usbehci0"     ,
    "mod_usbehci1"     ,
    "mod_usbohci0"     ,
    "mod_usbohci1"     ,
    "mod_usbohci2"     ,
    "mod_usbotg"       ,
    "mod_mdfs"         ,
    "mod_debe0"        ,
    "mod_debe1"        ,
    "mod_defe0"        ,
    "mod_defe1"        ,
    "mod_demp"         ,
    "mod_lcd0ch0"      ,
    "mod_lcd0ch1"      ,
    "mod_lcd1ch0"      ,
    "mod_lcd1ch1"      ,
    "mod_csi0s"        ,
    "mod_csi0m"        ,
    "mod_csi1s"        ,
    "mod_csi1m"        ,
    "mod_ve"           ,
    "mod_adda"         ,
    "mod_avs"          ,
    "mod_hdmi"         ,
    "mod_ps"           ,
    "mod_mtcacc"       ,
    "mod_mbus0"        ,
    "mod_mbus1"        ,
    "mod_dram"         ,
    "mod_mipidsis"     ,
    "mod_mipidsip"     ,
    "mod_mipicsis"     ,
    "mod_mipicsip"     ,
    "mod_iepdrc0"      ,
    "mod_iepdrc1"      ,
    "mod_iepdeu0"      ,
    "mod_iepdeu1"      ,
    "mod_gpucore"      ,
    "mod_gpumem"       ,
    "mod_gpuhyd"       ,
    "mod_twi0"         ,
    "mod_twi1"         ,
    "mod_twi2"         ,
    "mod_twi3"         ,
    "mod_uart0"        ,
    "mod_uart1"        ,
    "mod_uart2"        ,
    "mod_uart3"        ,
    "mod_uart4"        ,
    "mod_uart5"        ,
    "mod_gmac"         ,
    "mod_dma"          ,
    "mod_hstmr"        ,
    "mod_msgbox"       ,
    "mod_spinlock"     ,
    "mod_lvds"         ,
    "smp_twd"          ,
    "axi_dram"         ,
    "ahb_mipicsi"      ,
    "ahb_mipidsi"      ,
    "ahb_ss"           ,
    "ahb_dma"          ,
    "ahb_sdmmc0"       ,
    "ahb_sdmmc1"       ,
    "ahb_sdmmc2"       ,
    "ahb_sdmmc3"       ,
    "ahb_nand1"        ,
    "ahb_nand0"        ,
    "ahb_sdram"        ,
    "ahb_gmac"         ,
    "ahb_ts"           ,
    "ahb_hstmr"        ,
    "ahb_spi0"         ,
    "ahb_spi1"         ,
    "ahb_spi2"         ,
    "ahb_spi3"         ,
    "ahb_otg"          ,
    "ahb_ehci0"        ,
    "ahb_ehci1"        ,
    "ahb_ohci0"        ,
    "ahb_ohci1"        ,
    "ahb_ohci2"        ,
    "ahb_ve"           ,
    "ahb_lcd0"         ,
    "ahb_lcd1"         ,
    "ahb_csi0"         ,
    "ahb_csi1"         ,
    "ahb_hdmid"        ,
    "ahb_debe0"        ,
    "ahb_debe1"        ,
    "ahb_defe0"        ,
    "ahb_defe1"        ,
    "ahb_mp"           ,
    "ahb_gpu"          ,
    "ahb_msgbox"       ,
    "ahb_spinlock"     ,
    "ahb_deu0"         ,
    "ahb_deu1"         ,
    "ahb_drc0"         ,
    "ahb_drc1"         ,
    "ahb_mtcacc"       ,
    "apb_adda"         ,
    "apb_spdif"        ,
    "apb_pio"          ,
    "apb_i2s0"         ,
    "apb_i2s1"         ,
    "apb_twi0"         ,
    "apb_twi1"         ,
    "apb_twi2"         ,
    "apb_twi3"         ,
    "apb_uart0"        ,
    "apb_uart1"        ,
    "apb_uart2"        ,
    "apb_uart3"        ,
    "apb_uart4"        ,
    "apb_uart5"        ,
    "dram_ve"          ,
    "dram_csi0"        ,
    "dram_csi1"        ,
    "dram_ts"          ,
    "dram_drc0"        ,
    "dram_drc1"        ,
    "dram_deu0"        ,
    "dram_deu1"        ,
    "dram_defe0"       ,
    "dram_defe1"       ,
    "dram_debe0"       ,
    "dram_debe1"       ,
    "dram_mp"          ,
    "none"             ,
};

static char* _modClkName[AW_CCU_CLK_CNT] =
{
    "none"            ,
    "sys_losc"         ,
    "sys_hosc"         ,
    "sys_pll1"         ,
    "sys_pll2"         ,
    "sys_pll3"         ,
    "sys_pll4"         ,
    "sys_pll5"         ,
    "sys_pll6"         ,
    "sys_pll7"         ,
    "sys_pll8"         ,
    "sys_pll9"         ,
    "sys_pll10"        ,
    "sys_pll2X8"       ,
    "sys_pll3X2"       ,
    "sys_pll6X2"       ,
    "sys_pll7X2"       ,
    "sys_mipi_pll"     ,
    "sys_ac327"        ,
    "sys_ar100"        ,
    "sys_axi"          ,
    "sys_ahb0"         ,
    "sys_ahb1"         ,
    "sys_apb0"         ,
    "sys_apb1"         ,
    "sys_apb2"         ,
    "none"             ,
    "mod_nand0"        ,
    "mod_nand1"        ,
    "mod_sdc0"         ,
    "mod_sdc1"         ,
    "mod_sdc2"         ,
    "mod_sdc3"         ,
    "mod_ts"           ,
    "mod_ss"           ,
    "mod_spi0"         ,
    "mod_spi1"         ,
    "mod_spi2"         ,
    "mod_spi3"         ,
    "mod_i2s0"         ,
    "mod_i2s1"         ,
    "mod_spdif"        ,
    "mod_usbphy0"      ,
    "mod_usbphy1"      ,
    "mod_usbphy2"      ,
    "mod_usbehci0"     ,
    "mod_usbehci1"     ,
    "mod_usbohci0"     ,
    "mod_usbohci1"     ,
    "mod_usbohci2"     ,
    "mod_usbotg"       ,
    "mod_mdfs"         ,
    "mod_debe0"        ,
    "mod_debe1"        ,
    "mod_defe0"        ,
    "mod_defe1"        ,
    "mod_demp"         ,
    "mod_lcd0ch0"      ,
    "mod_lcd0ch1"      ,
    "mod_lcd1ch0"      ,
    "mod_lcd1ch1"      ,
    "mod_csi0s"        ,
    "mod_csi0m"        ,
    "mod_csi1s"        ,
    "mod_csi1m"        ,
    "mod_ve"           ,
    "mod_adda"         ,
    "mod_avs"          ,
    "mod_hdmi"         ,
    "mod_ps"           ,
    "mod_mtcacc"       ,
    "mod_mbus0"        ,
    "mod_mbus1"        ,
    "mod_dram"         ,
    "mod_mipidsis"     ,
    "mod_mipidsip"     ,
    "mod_mipicsis"     ,
    "mod_mipicsip"     ,
    "mod_iepdrc0"      ,
    "mod_iepdrc1"      ,
    "mod_iepdeu0"      ,
    "mod_iepdeu1"      ,
    "mod_gpucore"      ,
    "mod_gpumem"       ,
    "mod_gpuhyd"       ,
    "mod_twi0"         ,
    "mod_twi1"         ,
    "mod_twi2"         ,
    "mod_twi3"         ,
    "mod_uart0"        ,
    "mod_uart1"        ,
    "mod_uart2"        ,
    "mod_uart3"        ,
    "mod_uart4"        ,
    "mod_uart5"        ,
    "mod_gmac"         ,
    "mod_dma"          ,
    "mod_hstmr"        ,
    "mod_msgbox"       ,
    "mod_spinlock"     ,
    "mod_lvds"         ,
    "smp_twd"          ,
    "axi_dram"         ,
    "ahb_mipicsi"      ,
    "ahb_mipidsi"      ,
    "ahb_ss"           ,
    "ahb_dma"          ,
    "ahb_sdmmc0"       ,
    "ahb_sdmmc1"       ,
    "ahb_sdmmc2"       ,
    "ahb_sdmmc3"       ,
    "ahb_nand1"        ,
    "ahb_nand0"        ,
    "ahb_sdram"        ,
    "ahb_gmac"         ,
    "ahb_ts"           ,
    "ahb_hstmr"        ,
    "ahb_spi0"         ,
    "ahb_spi1"         ,
    "ahb_spi2"         ,
    "ahb_spi3"         ,
    "ahb_otg"          ,
    "ahb_ehci0"        ,
    "ahb_ehci1"        ,
    "ahb_ohci0"        ,
    "ahb_ohci1"        ,
    "ahb_ohci2"        ,
    "ahb_ve"           ,
    "ahb_lcd0"         ,
    "ahb_lcd1"         ,
    "ahb_csi0"         ,
    "ahb_csi1"         ,
    "ahb_hdmid"        ,
    "ahb_debe0"        ,
    "ahb_debe1"        ,
    "ahb_defe0"        ,
    "ahb_defe1"        ,
    "ahb_mp"           ,
    "ahb_gpu"          ,
    "ahb_msgbox"       ,
    "ahb_spinlock"     ,
    "ahb_deu0"         ,
    "ahb_deu1"         ,
    "ahb_drc0"         ,
    "ahb_drc1"         ,
    "ahb_mtcacc"       ,
    "apb_adda"         ,
    "apb_spdif"        ,
    "apb_pio"          ,
    "apb_i2s0"         ,
    "apb_i2s1"         ,
    "apb_twi0"         ,
    "apb_twi1"         ,
    "apb_twi2"         ,
    "apb_twi3"         ,
    "apb_uart0"        ,
    "apb_uart1"        ,
    "apb_uart2"        ,
    "apb_uart3"        ,
    "apb_uart4"        ,
    "apb_uart5"        ,
    "dram_ve"          ,
    "dram_csi0"        ,
    "dram_csi1"        ,
    "dram_ts"          ,
    "dram_drc0"        ,
    "dram_drc1"        ,
    "dram_deu0"        ,
    "dram_deu1"        ,
    "dram_defe0"       ,
    "dram_defe1"       ,
    "dram_debe0"       ,
    "dram_debe1"       ,
    "dram_mp"          ,
    "none"             ,
};
__s32 OSAL_CCMU_SetSrcFreq( __u32 nSclkNo, __u32 nFreq )
{
    struct clk* hSysClk = NULL;
    s32 retCode = -1;

    __inf("OSAL_CCMU_SetSrcFreq,  _sysClkName[%d]=%s\n", nSclkNo,_sysClkName[nSclkNo]);
    
    hSysClk = clk_get(NULL, _sysClkName[nSclkNo]);

    __inf("OSAL_CCMU_SetSrcFreq<%s,%d>\n",hSysClk->aw_clk->name, nFreq);

    if(NULL == hSysClk){
        __wrn("Fail to get handle for system clock [%d].\n", nSclkNo);
        return -1;
    }
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

    __inf("OSAL_CCMU_GetSrcFreq,  _sysClkName[%d]=%s\n", nSclkNo,_sysClkName[nSclkNo]);
    
    hSysClk = clk_get(NULL, _sysClkName[nSclkNo]);
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

    __inf("OSAL_CCMU_OpenMclk,  _modClkName[%d]=%s\n", nMclkNo,_modClkName[nMclkNo]);
    hModClk = clk_get(NULL, _modClkName[nMclkNo]);
    __inf("hModClk=0x%08x\n", hModClk);

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

    __inf("OSAL_CCMU_SetMclkSrc, Mclk=%s, _sysClkName[%d]=%s\n", hModClk->aw_clk->name, nSclkNo,_sysClkName[nSclkNo]);

    hSysClk = clk_get(NULL, _sysClkName[nSclkNo]);

    __inf("OSAL_CCMU_SetMclkSrc<%s,%s>\n",hModClk->aw_clk->name,hSysClk->aw_clk->name);

    if(NULL == hSysClk){
        __wrn("Fail to get handle for system clock [%d].\n", nSclkNo);
        return -1;
    }
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

    __inf("OSAL_CCMU_MclkOnOff<%s,%d>\n",hModClk->aw_clk->name,bOnOff);

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

