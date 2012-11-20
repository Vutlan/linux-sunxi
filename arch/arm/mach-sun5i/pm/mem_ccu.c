#include "pm_types.h"
#include "pm_i.h"

static __ccmu_reg_list_t   CmuReg;
/*
*********************************************************************************************************
*                                       MEM CCU INITIALISE
*
* Description: mem interrupt initialise.
*
* Arguments  : none.
*
* Returns    : 0/-1;
*********************************************************************************************************
*/
__s32 mem_ccu_save(__ccmu_reg_list_t *pReg)
{
	CmuReg.Pll2Ctl      = pReg->Pll2Ctl;
	CmuReg.Pll2Tune     = pReg->Pll2Tune;
	CmuReg.Pll3Ctl      = pReg->Pll3Ctl;
	CmuReg.Pll4Ctl      = pReg->Pll4Ctl;
	CmuReg.Pll6Ctl      = pReg->Pll6Ctl;
	CmuReg.Pll7Ctl      = pReg->Pll7Ctl;
	CmuReg.HoscCtl      = pReg->HoscCtl;
	CmuReg.SysClkDiv    = pReg->SysClkDiv;
	CmuReg.Apb1ClkDiv   = pReg->Apb1ClkDiv;
	CmuReg.AxiGate      = pReg->AxiGate;
	CmuReg.AhbGate0     = pReg->AhbGate0;
	CmuReg.AhbGate1     = pReg->AhbGate1;
	CmuReg.Apb0Gate     = pReg->Apb0Gate;
	CmuReg.Apb1Gate     = pReg->Apb1Gate;
	CmuReg.NandClk      = pReg->NandClk;
	CmuReg.MsClk        = pReg->MsClk;
	CmuReg.SdMmc0Clk    = pReg->SdMmc0Clk;
	CmuReg.SdMmc1Clk    = pReg->SdMmc1Clk;
	CmuReg.SdMmc2Clk    = pReg->SdMmc2Clk;
	CmuReg.TsClk        = pReg->TsClk;
	CmuReg.SsClk        = pReg->SsClk;
	CmuReg.Spi0Clk      = pReg->Spi0Clk;
	CmuReg.Spi1Clk      = pReg->Spi1Clk;
	CmuReg.Spi2Clk      = pReg->Spi2Clk;
	CmuReg.Ir0Clk       = pReg->Ir0Clk;
	CmuReg.I2sClk       = pReg->I2sClk;
	CmuReg.SpdifClk     = pReg->SpdifClk;
	CmuReg.KeyPadClk    = pReg->KeyPadClk;
	CmuReg.UsbClk       = pReg->UsbClk;
	CmuReg.GpsClk       = pReg->GpsClk;
	CmuReg.DramGate     = pReg->DramGate;
	CmuReg.DeBe0Clk     = pReg->DeBe0Clk;
	CmuReg.DeFe0Clk     = pReg->DeFe0Clk;
	CmuReg.Lcd0Ch0Clk   = pReg->Lcd0Ch0Clk;
	CmuReg.Lcd0Ch1Clk   = pReg->Lcd0Ch1Clk;
	CmuReg.Csi0Clk      = pReg->Csi0Clk;
	CmuReg.VeClk        = pReg->VeClk;
	CmuReg.AddaClk      = pReg->AddaClk;
	CmuReg.AvsClk       = pReg->AvsClk;
	CmuReg.LvdsClk      = pReg->LvdsClk;
	CmuReg.HdmiClk      = pReg->HdmiClk;
	CmuReg.MaliClk      = pReg->MaliClk;
	
	CmuReg.MbusClk		= pReg->MbusClk;
	CmuReg.IepClk		= pReg->IepClk;

	return 0;
}

__s32 mem_ccu_restore(__ccmu_reg_list_t *pReg)
{
	//1. pll(pll1/pll5)
	pReg->Pll2Ctl       = CmuReg.Pll2Ctl;
	pReg->Pll2Tune      = CmuReg.Pll2Tune;
	pReg->Pll3Ctl       = CmuReg.Pll3Ctl;
	pReg->Pll4Ctl       = CmuReg.Pll4Ctl;
	pReg->Pll6Ctl       = CmuReg.Pll6Ctl;
	pReg->Pll7Ctl       = CmuReg.Pll7Ctl;

	//2. mod clk-src , div;
	pReg->HoscCtl       = CmuReg.HoscCtl;
	pReg->SysClkDiv     = CmuReg.SysClkDiv;
	pReg->Apb1ClkDiv    = CmuReg.Apb1ClkDiv;

	//3. mod gating;
	pReg->AxiGate       = CmuReg.AxiGate;
	pReg->AhbGate0      = CmuReg.AhbGate0;
	pReg->AhbGate1      = CmuReg.AhbGate1;
	pReg->Apb0Gate      = CmuReg.Apb0Gate;
	pReg->Apb1Gate      = CmuReg.Apb1Gate;

	//4. mod reset;
	pReg->NandClk       = CmuReg.NandClk;
	pReg->MsClk         = CmuReg.MsClk;
	pReg->SdMmc0Clk     = CmuReg.SdMmc0Clk;
	pReg->SdMmc1Clk     = CmuReg.SdMmc1Clk;
	pReg->SdMmc2Clk     = CmuReg.SdMmc2Clk;
	pReg->TsClk         = CmuReg.TsClk;
	pReg->SsClk         = CmuReg.SsClk;
	pReg->Spi0Clk       = CmuReg.Spi0Clk;
	pReg->Spi1Clk       = CmuReg.Spi1Clk;
	pReg->Spi2Clk       = CmuReg.Spi2Clk;
	pReg->Ir0Clk        = CmuReg.Ir0Clk;
	pReg->I2sClk        = CmuReg.I2sClk;
	pReg->SpdifClk      = CmuReg.SpdifClk;
	pReg->KeyPadClk     = CmuReg.KeyPadClk;
	pReg->UsbClk        = CmuReg.UsbClk;
	pReg->GpsClk        = CmuReg.GpsClk;
	pReg->DramGate		= CmuReg.DramGate;
	pReg->DeBe0Clk      = CmuReg.DeBe0Clk;
	pReg->DeFe0Clk      = CmuReg.DeFe0Clk;
	pReg->Lcd0Ch0Clk    = CmuReg.Lcd0Ch0Clk;
	pReg->Lcd0Ch1Clk    = CmuReg.Lcd0Ch1Clk;
	pReg->Csi0Clk       = CmuReg.Csi0Clk;
	pReg->VeClk         = CmuReg.VeClk;
	pReg->AddaClk       = CmuReg.AddaClk;
	pReg->AvsClk        = CmuReg.AvsClk;
	pReg->LvdsClk       = CmuReg.LvdsClk;
	pReg->HdmiClk       = CmuReg.HdmiClk;
	pReg->MaliClk       = CmuReg.MaliClk;

	pReg->MbusClk		= CmuReg.MbusClk;
	pReg->IepClk		= CmuReg.IepClk;

	return 0;
}

