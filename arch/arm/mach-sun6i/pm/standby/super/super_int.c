/*
*********************************************************************************************************
*                                                    LINUX-KERNEL
*                                        AllWinner Linux Platform Develop Kits
*                                                   Kernel Module
*
*                                    (c) Copyright 2006-2011, kevin.z China
*                                             All Rights Reserved
*
* File    : super_int.c
* By      : kevin.z
* Version : v1.0
* Date    : 2011-5-30 20:13
* Descript: interrupt for platform mem
* Update  : date                auther      ver     notes
*********************************************************************************************************
*/
#include "super_i.h"

static __u32    IrqEnReg[3], IrqMaskReg[3], IrqSelReg[3];
static struct mem_int_reg_t  *IntcReg;


