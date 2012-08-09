/* these code will be removed to sram.
 * function: open the mmu, and jump to dram, for continuing resume*/
#include "./../super_i.h"

extern unsigned int save_sp(void);
extern int jump_to_resume(void* pointer, __u32 *addr);
extern void restore_mmu_state(struct mmu_state *saved_mmu_state);
extern void mem_flush_tlb(void);
extern void flush_icache(void);
extern void flush_dcache(void);
extern void invalidate_dcache(void);
extern void mem_preload_tlb_nommu(void);
extern void clear_reg_context(void);

extern void disable_cache(void);
extern void disable_program_flow_prediction(void);
extern void invalidate_branch_predictor(void);
extern void enable_cache(void);
extern void enable_program_flow_prediction(void);
extern void disable_dcache(void);
extern void disable_l2cache(void);

static struct aw_mem_para mem_para_info;

extern char *__bss_start;
extern char *__bss_end;
static __s32 dcdc2, dcdc3;
static __u32 sp_backup;
static char    *tmpPtr = (char *)&__bss_start;
static __u32 status = 0; 

#ifdef RETURN_FROM_RESUME0_WITH_MMU
#define MMU_OPENED
#undef POWER_OFF
#define FLUSH_TLB
#define FLUSH_ICACHE
#define INVALIDATE_DCACHE
#endif

#ifdef RETURN_FROM_RESUME0_WITH_NOMMU
#undef MMU_OPENED
#undef POWER_OFF
#define FLUSH_TLB
#define FLUSH_ICACHE
#define INVALIDATE_DCACHE
#endif

#if defined(ENTER_SUPER_STANDBY) || defined(ENTER_SUPER_STANDBY_WITH_NOMMU) || defined(WATCH_DOG_RESET)
#undef MMU_OPENED
#define POWER_OFF
#define FLUSH_TLB
#define SET_COPRO_REG
#define FLUSH_ICACHE
#define INVALIDATE_DCACHE
#endif

int resume1_c_part(void)
{
	serial_init_nommu();
	//busy_waiting();
	serial_puts_nommu("resume1: 0. \n", 13);
	/* clear bss segment */
	do{*tmpPtr ++ = 0;}while(tmpPtr <= (char *)&__bss_end);
	
#ifdef SET_COPRO_REG
	set_copro_default();
#endif

#ifdef MMU_OPENED
	save_mem_status(RESUME1_START |0x02);
	//serial_puts("resume1: 2. \n", 13);

	//move other storage to sram: saved_resume_pointer(virtual addr), saved_mmu_state
	mem_memcpy((void *)&mem_para_info, (void *)(DRAM_BACKUP_BASE_ADDR1), sizeof(mem_para_info));
#else
	mem_preload_tlb_nommu();
	/*switch stack*/
	save_mem_status_nommu(RESUME1_START |0x02);

	//move other storage to sram: saved_resume_pointer(virtual addr), saved_mmu_state
	mem_memcpy((void *)&mem_para_info, (void *)(DRAM_BACKUP_BASE_ADDR1_PA), sizeof(mem_para_info));
	/*restore mmu configuration*/
	save_mem_status_nommu(RESUME1_START |0x03);
	//save_mem_status(RESUME1_START |0x03);
	
	//busy_waiting();
	serial_puts_nommu("resume1: 2. \n", 13);
	
	restore_mmu_state(&(mem_para_info.saved_mmu_state));
	save_mem_status(RESUME1_START |0x13);

#endif

#ifdef POWER_OFF
	/* disable watch-dog: coresponding with  */
	mem_tmr_init();
	mem_tmr_disable_watchdog();
	//serial_puts("resume1: a. \n", 13);
#endif

//before jump to late_resume	
#ifdef FLUSH_TLB
	save_mem_status(RESUME1_START |0x9);
	mem_flush_tlb();
#endif

#ifdef FLUSH_ICACHE
	save_mem_status(RESUME1_START |0xa);
	flush_icache();
#endif

	//busy_waiting();
	jump_to_resume((void *)mem_para_info.resume_pointer, mem_para_info.saved_runtime_context_svc);

	serial_puts("resume1: e. \n", 13);
	return;
}


/*******************************************************************************
*函数名称: set_pll
*函数原型：void set_pll( void )
*函数功能: resume中用C语言编写的 调整CPU频率
*入口参数: void
*返 回 值: void
*备    注:
*******************************************************************************/
void set_pll( void )
{
	//cpus in charge this

	return ;
}


