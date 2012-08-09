/*
*********************************************************************************************************
*                                                    LINUX-KERNEL
*                                        AllWinner Linux Platform Develop Kits
*                                                   Kernel Module
*
*                                    (c) Copyright 2006-2011, kevin.z China
*                                             All Rights Reserved
*
* File    : standby.c
* By      : kevin.z
* Version : v1.0
* Date    : 2011-5-30 18:34
* Descript: platform standby fucntion.
* Update  : date                auther      ver     notes
*********************************************************************************************************
*/
#include "standby_i.h"

extern unsigned int save_sp(void);
extern void restore_sp(unsigned int sp);
extern void standby_flush_tlb(void);
extern void standby_preload_tlb(void);
static void restore_ccu(void);
static void backup_ccu(void);

extern char *__bss_start;
extern char *__bss_end;
extern char *__standby_start;
extern char *__standby_end;

static __u32 sp_backup;
static void standby(void);

/* parameter for standby, it will be transfered from sys_pwm module */
struct aw_pm_info  pm_info;
struct normal_standby_para normal_standby_para_info;

/*
*********************************************************************************************************
*                                   STANDBY MAIN PROCESS ENTRY
*
* Description: standby main process entry.
*
* Arguments  : arg  pointer to the parameter that transfered from sys_pwm module.
*
* Returns    : none
*
* Note       :
*********************************************************************************************************
*/
int main(struct aw_pm_info *arg)
{
	char    *tmpPtr = (char *)&__bss_start;
	int i = 0;

	/* save stack pointer registger, switch stack to sram */
	sp_backup = save_sp();

	serial_init();
	if(!arg){
		/* standby parameter is invalid */
		return -1;
	}

	/* flush data and instruction tlb, there is 32 items of data tlb and 32 items of instruction tlb,
	The TLB is normally allocated on a rotating basis. The oldest entry is always the next allocated */
	standby_flush_tlb();
	/* preload tlb for standby */
	standby_preload_tlb();

	/* clear bss segment */
	do{*tmpPtr ++ = 0;}while(tmpPtr <= (char *)&__bss_end);

	/* copy standby parameter from dram */
	standby_memcpy(&pm_info, arg, sizeof(pm_info));
	/* copy standby code & data to load tlb */
	//standby_memcpy((char *)&__standby_end, (char *)&__standby_start, (char *)&__bss_end - (char *)&__bss_start);

	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
	/* init module before dram enter selfrefresh */
	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
	
	/* initialise standby modules */
	standby_ar100_init();
	standby_clk_init();
	standby_int_init();
	standby_tmr_init();
	/* init some system wake source */
	if(pm_info.standby_para.event & CPU0_WAKEUP_MSGBOX){
		i = INT_SOURCE_MSG_BOX;
		printk("INT_SOURCE_MSG_BOX = 0x%x. \n", i);
		standby_enable_int(INT_SOURCE_MSG_BOX);
	}
	if(pm_info.standby_para.event & CPU0_WAKEUP_KEY){
		standby_key_init();
		i = INT_SOURCE_LRADC;
		printk("INT_SOURCE_LRADC = 0x%x. \n", i);
		standby_enable_int(INT_SOURCE_LRADC);
	}

	/* process standby */

	for (i = 4; i < (0x40); i += 4){
		printk("enable bit = 0x%x. \n",	*(volatile __u32 *)(IO_ADDRESS(AW_GIC_DIST_BASE) + GIC_DIST_ENABLE_SET + i));

	}
	
	for (i = 4; i < (0x40); i += 4){
		printk("pending bit = 0x%x. \n", *(volatile __u32 *)(IO_ADDRESS(AW_GIC_DIST_BASE) + GIC_DIST_PENDING_SET + i));

	}

	standby();

	
	for (i = 4; i < (0x40); i += 4){
		printk("enable bit = 0x%x. \n",	*(volatile __u32 *)(IO_ADDRESS(AW_GIC_DIST_BASE) + GIC_DIST_ENABLE_SET + i));

	}
	
	for (i = 4; i < (0x40); i += 4){
		printk("pending bit = 0x%x. \n", *(volatile __u32 *)(IO_ADDRESS(AW_GIC_DIST_BASE) + GIC_DIST_PENDING_SET + i));

	}

	/* check system wakeup event */
	pm_info.standby_para.event = 0;
	pm_info.standby_para.event |= standby_query_int(INT_SOURCE_MSG_BOX)? 0:CPU0_WAKEUP_MSGBOX;
	pm_info.standby_para.event |= standby_query_int(INT_SOURCE_LRADC)? 0:CPU0_WAKEUP_KEY;

	/* exit standby module */
	if(pm_info.standby_para.event & CPU0_WAKEUP_KEY){
		standby_key_exit();
	}

	standby_int_exit();
	
	/*check completion status: only after restore completion, access dram is allowed. */
	while(standby_ar100_check_restore_status())
		;

	/* disable watch-dog    */
	standby_tmr_disable_watchdog();
	standby_tmr_exit();

	/* restore stack pointer register, switch stack back to dram */
	restore_sp(sp_backup);
	
	/* report which wake source wakeup system */
	arg->standby_para.event = pm_info.standby_para.event;
	arg->standby_para.axp_event = pm_info.standby_para.axp_event;
	
	return 0;
}


/*
*********************************************************************************************************
*                                     SYSTEM PWM ENTER STANDBY MODE
*
* Description: enter standby mode.
*
* Arguments  : none
*
* Returns    : none;
*********************************************************************************************************
*/
static void standby(void)
{
	/*backup clk freq and voltage*/
	backup_ccu();
	
	/*notify ar100 enter normal standby*/
	normal_standby_para_info.event = pm_info.standby_para.axp_event;
	normal_standby_para_info.timeout = pm_info.standby_para.timeout;
	
	standby_ar100_standby_normal((&normal_standby_para_info));

	/* cpu enter sleep, wait wakeup by interrupt */
	asm("WFI");

	/*restore cpu0 ccu: enable hosc and change to 24M. */
	restore_ccu();
	
	printk("standby: after restore ccu. \n");
	/*query wakeup src*/
	standby_ar100_query_wakeup_src((unsigned long *)&(pm_info.standby_para.axp_event));
	printk("after query wakeup src. \n");
	
	/* enable watch-dog to prevent in case dram training failed */
	standby_tmr_enable_watchdog();

	/* notify for cpus to: restore cpus freq and volt, restore dram */
	standby_ar100_notify_restore(STANDBY_AR100_ASYNC);	

	printk("gic iar == 0x%x. \n", *(volatile __u32   *)(IO_ADDRESS(AW_GIC_CPU_BASE)+0x0c));
	
	return;
}

static void backup_ccu(void)
{
	return;
}

/*change clk src to hosc*/
static void restore_ccu(void)
{
	
#if(ALLOW_DISABLE_HOSC)
		/* enable LDO, enable HOSC */
		standby_clk_ldoenable();
		/* delay 1ms for power be stable */
		//3ms
		standby_delay_cycle(1);
		standby_clk_hoscenable();
		//3ms
		standby_delay_cycle(1);
#endif
	
		return;

}
