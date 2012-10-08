#ifndef _PM_H
#define _PM_H

/*
 * Copyright (c) 2011-2015 yanggq.young@allwinnertech.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

//#include "pm_types.h" 
#include "pm_config.h"
#include "pm_errcode.h"
#include "pm_debug.h"
#include "mem_cpu.h"
#include "mem_serial.h"
#include "mem_printk.h"
#include "mach/platform.h"
#include "mem_misc.h"

#define PM_STANDBY_PRINT_STANDBY (1U << 0)
#define PM_STANDBY_PRINT_RESUME (1U << 1)
#define PM_STANDBY_PRINT_IO_STATUS (1U << 2)
#define PM_STANDBY_PRINT_CACHE_TLB_MISS (1U << 3)

#ifdef CONFIG_ARCH_SUN4I
#define INT_REG_LENGTH	((0x90+0x4)>>2)
#define GPIO_REG_LENGTH	((0x218+0x4)>>2)
#define SRAM_REG_LENGTH	((0x94+0x4)>>2)
#elif defined CONFIG_ARCH_SUN5I
#define INT_REG_LENGTH	((0x94+0x4)>>2)
#define GPIO_REG_LENGTH	((0x218+0x4)>>2)
#define SRAM_REG_LENGTH	((0x94+0x4)>>2)
#elif defined CONFIG_ARCH_SUN6I
#define GPIO_REG_LENGTH	((0x278+0x4)>>2)
#define SRAM_REG_LENGTH	((0x94+0x4)>>2)
#elif defined CONFIG_ARCH_SUN7I
#define GPIO_REG_LENGTH	((0x218+0x4)>>2)
#define SRAM_REG_LENGTH	((0x94+0x4)>>2)
#endif

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

typedef struct __MEM_TMR_REG
{
    // offset:0x00
    volatile __u32   IntCtl;
    volatile __u32   IntSta;
    volatile __u32   reserved0[2];
    // offset:0x10
    volatile __u32   Tmr0Ctl;
    volatile __u32   Tmr0IntVal;
    volatile __u32   Tmr0CntVal;
    volatile __u32   reserved1;
    // offset:0x20
    volatile __u32   Tmr1Ctl;
    volatile __u32   Tmr1IntVal;
    volatile __u32   Tmr1CntVal;
    volatile __u32   reserved2;
    // offset:0x30
    volatile __u32   Tmr2Ctl;
    volatile __u32   Tmr2IntVal;
    volatile __u32   Tmr2CntVal;
    volatile __u32   reserved3;
    // offset:0x40
    volatile __u32   Tmr3Ctl;
    volatile __u32   Tmr3IntVal;
    volatile __u32   reserved4[2];
    // offset:0x50
    volatile __u32   Tmr4Ctl;
    volatile __u32   Tmr4IntVal;
    volatile __u32   Tmr4CntVal;
    volatile __u32   reserved5;
    // offset:0x60
    volatile __u32   Tmr5Ctl;
    volatile __u32   Tmr5IntVal;
    volatile __u32   Tmr5CntVal;
    volatile __u32   reserved6[5];
    // offset:0x80
    volatile __u32   AvsCtl;
    volatile __u32   Avs0Cnt;
    volatile __u32   Avs1Cnt;
    volatile __u32   AvsDiv;
	// offset:0xa0
    volatile __u32   WDog1_Irq_En;
    volatile __u32   WDog1_Irq_Sta;
    volatile __u32   WDog1_Ctrl_Reg;
    volatile __u32   WDog1_Cfg_Reg;
    volatile __u32   WDog1_Mode_Reg;	
    volatile __u32   WDog1_Reset_Pwh_Reg;

} __mem_tmr_reg_t;


struct clk_div_t {
    __u32   cpu_div:4;      /* division of cpu clock, divide core_pll */
    __u32   axi_div:4;      /* division of axi clock, divide cpu clock*/
    __u32   ahb_div:4;      /* division of ahb clock, divide axi clock*/
    __u32   apb_div:4;      /* division of apb clock, divide ahb clock*/
    __u32   reserved:16;
};
struct pll_factor_t {
    __u8    FactorN;
    __u8    FactorK;
    __u8    FactorM;
    __u8    FactorP;
    __u32   Pll;
};

struct mmu_state {
	/* CR0 */
	__u32 cssr;	/* Cache Size Selection */
	/* CR1 */
	__u32 cr;		/* Control */
	__u32 cacr;	/* Coprocessor Access Control */
	/* CR2 */
	__u32  ttb_0r;	/* Translation Table Base 0 */
	__u32  ttb_1r;	/* Translation Table Base 1 */
	__u32  ttbcr;	/* Translation Talbe Base Control */
	
	/* CR3 */
	__u32 dacr;	/* Domain Access Control */

	/*cr10*/
	__u32 prrr;	/* Primary Region Remap Register */
	__u32 nrrr;	/* Normal Memory Remap Register */
};

/**
*@brief struct of super mem
*/
struct aw_mem_para{
	void **resume_pointer;
	volatile __u32 mem_flag;
	__u32 axp_event;
	__u32 sys_event;
	__u32 debug_mask;
	__u32 saved_runtime_context_svc[RUNTIME_CONTEXT_SIZE];
	struct mmu_state saved_mmu_state;
	struct saved_context saved_cpu_context;
};


static inline __u32 raw_lib_udiv(__u32 dividend, __u32 divisior)
{
    __u32   tmpDiv = (__u32)divisior;
    __u32   tmpQuot = 0;
    __s32   shift = 0;

    if(!divisior)
    {
        /* divide 0 error abort */
        return 0;
    }

    while(!(tmpDiv & ((__u32)1<<31)))
    {
        tmpDiv <<= 1;
        shift ++;
    }

    do
    {
        if(dividend >= tmpDiv)
        {
            dividend -= tmpDiv;
            tmpQuot = (tmpQuot << 1) | 1;
        }
        else
        {
            tmpQuot = (tmpQuot << 1) | 0;
        }
        tmpDiv >>= 1;
        shift --;
    } while(shift >= 0);

    return tmpQuot;
}

extern void __aeabi_idiv(void);
extern void __aeabi_idivmod(void);
extern void __aeabi_uidiv(void);
extern void __aeabi_uidivmod(void);

extern unsigned int save_sp_nommu(void);
extern unsigned int save_sp(void);
extern void clear_reg_context(void);
extern void restore_sp(unsigned int sp);

extern void enable_cache(void);
extern void enable_icache(void);
extern void disable_cache(void);
extern void disable_dcache(void);
extern void disable_l2cache(void);
extern void flush_icache(void);
extern void flush_dcache(void);
extern void invalidate_dcache(void);

extern void restore_mmu_state(struct mmu_state *saved_mmu_state);
extern void mem_flush_tlb(void);
extern void mem_preload_tlb(void);

void disable_mmu(void);
void enable_mmu(void);
void set_ttbr0(void);

extern void disable_program_flow_prediction(void);
extern void invalidate_branch_predictor(void);
extern void enable_program_flow_prediction(void);

extern int jump_to_resume(void* pointer, __u32 *addr);
extern int jump_to_resume0(void* pointer);


#endif /*_PM_H*/
