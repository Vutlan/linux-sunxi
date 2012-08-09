#include <mach/platform.h>

/**stack point address in sram*/
#define SP_IN_SRAM	0xf0007ffc

    .text
    .globl save_sp
save_sp:
    mov r0, r13
    ldr  r13, =SP_IN_SRAM
    mov pc,lr

    .text
    .globl restore_sp
restore_sp:
    mov r13, r0
    mov pc,lr


    .text
    .globl standby_flush_tlb
standby_flush_tlb:
    push    {r0-r3}
    /* invalid un-locked data and instruction tlb */
    mcr p15, 0, r0, c8, c7, 0
    pop     {r0-r3}
    mov pc,lr

    .text
    .globl standby_preload_tlb
standby_preload_tlb:
    mov pc,lr

