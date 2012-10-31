/*
*********************************************************************************************************
*                                                    LINUX-KERNEL
*                                        AllWinner Linux Platform Develop Kits
*                                                   Kernel Module
*
*                                    (c) Copyright 2006-2011, kevin.z China
*                                             All Rights Reserved
*
* File    : standby_int.h
* By      : kevin.z
* Version : v1.0
* Date    : 2011-5-30 19:50
* Descript: intterupt bsp for platform standby.
* Update  : date                auther      ver     notes
*********************************************************************************************************
*/
#ifndef __STANDBY_INT_H__
#define __STANDBY_INT_H__

#include "standby_cfg.h"
#include "../pm_config.h"

#define GIC_400_ENABLE_LEN (0x40) //unit is byte. so in 32bit unit, the reg offset is 0-0x3c

/* define interrupt source */
enum interrupt_source_e{
#ifdef SUN6I_FPGA_SIM
	INT_SOURCE_TOUCHPNL = 24 + 32,
	INT_SOURCE_LRADC    = 24 + 32,
	INT_SOURCE_MSG_BOX  = 11 + 32,
#else
	INT_SOURCE_TOUCHPNL = 60,
	INT_SOURCE_LRADC    = 62,
	INT_SOURCE_MSG_BOX  = 81,
#endif
};


/* define register for interrupt controller */
struct gic_distributor_state{
	//distributor
	volatile __u32 GICD_CTLR;					//offset 0x00, distributor Contrl reg
	volatile __u32 GICD_IGROUPRn;					//offset 0x80, Interrupt Grp Reg ?           
	volatile __u32 GICD_ISENABLERn[0x180/4];			//offset 0x100-0x17c, Interrupt Set-Enable Reg       
	volatile __u32 GICD_ICENABLERn[0x180/4];			//offset 0x180-0x1fc, Iterrupt Clear-Enable Reg      
	volatile __u32 GICD_ISPENDRn[0x180/4];				//offset 0x200-0x27c, Iterrupt Set-Pending Reg       
	volatile __u32 GICD_ICPENDRn[0x180/4];				//offset 0x280-0x2fc, Iterrupt Clear-Pending Reg     
	volatile __u32 GICD_ISACTIVERn[0x180/4]; 			//offset 0x300-0x37c, GICv2 Iterrupt Set-Active Reg        
	volatile __u32 GICD_ICACTIVERn[0x180/4]; 			//offset 0x380-0x3fc, Iterrupt Clear-Active Reg        
	volatile __u32 GICD_IPRIORITYRn[0x180/4]; 			//offset 0x400-0x7F8, Iterrupt Priority Reg          
	volatile __u32 GICD_ITARGETSRn[(0x400-0x20)/4];			//offset 0x820-0xbf8, Iterrupt Processor Targets Reg         
	volatile __u32 GICD_ICFGRn[0x100/4];				//offset 0xc00-0xcfc, Iterrupt Config Reg            
	volatile __u32 GICD_NSACRn[0x100/4];				//offset 0xE00-0xEfc, non-secure Access Ctrl Reg ?   
	volatile __u32 GICD_CPENDSGIRn[0x10/4];				//offset 0xf10-0xf1c, SGI Clear-Pending Reg          
	volatile __u32 GICD_SPENDSGIRn[0x10/4];				//offset 0xf20-0xf2c, SGI Set-Pending Reg       	
};

struct gic_cpu_interface_state{
	//cpu interface reg
	volatile __u32 GICC_CTLR_PMR_BPR[0xc/4];			//offset 0x00-0x08, cpu interface Ctrl Reg	+ 	Interrupt Priority Mask Reg	 + 	  Binary Point Reg	
	volatile __u32 GICC_ABPR;				//offset 0x1c, Aliased Binary Point Reg	
	volatile __u32 GICC_APRn[0x10/4];			//offset 0xd0-0xdc, Active Priorities Reg 		  
	volatile __u32 GICC_NSAPRn[0x10/4];			//offset 0xe0-0xec, Non-secure Active Priorities Reg
};

struct gic_distributor_disc{
	//distributor
	volatile __u32 GICD_CTLR;					//offset 0x00, distributor Contrl reg
	volatile __u32 GICD_TYPER;					//offset 0x04, distributor Contrl Type reg
	volatile __u32 GICD_IIDR;					//offset 0x08, distributor Implementer Identification reg	
	volatile __u32 reserved0[0x74/4];				//0ffset 0x0c-0x7c
	volatile __u32 GICD_IGROUPRn[0x40/4];				//offset 0x80-0xBc, Interrupt Grp Reg ?           
	volatile __u32 reserved1[0x40/4];				//0ffset 0xc0-0xfc
	volatile __u32 GICD_ISENABLERn[0x40/4];				//offset 0x100-0x13c, Interrupt Set-Enable Reg       
	volatile __u32 reservedx1[0x40/4];				//0ffset 0x140-0x17c
	volatile __u32 GICD_ICENABLERn[0x40/4];				//offset 0x180-0x1bc, Iterrupt Clear-Enable Reg 
	volatile __u32 reservedx2[0x40/4];				//0ffset 0x1c0-0x1fc
	volatile __u32 GICD_ISPENDRn[0x40/4];				//offset 0x200-0x23c, Iterrupt Set-Pending Reg    
	volatile __u32 reservedx3[0x40/4];				//0ffset 0x240-0x27c
	volatile __u32 GICD_ICPENDRn[0x40/4];				//offset 0x280-0x2bc, Iterrupt Clear-Pending Reg     
	volatile __u32 reservedx4[0x40/4];				//0ffset 0x2c0-0x2fc
	volatile __u32 GICD_ISACTIVERn[0x80/4]; 			//offset 0x300-0x37c, GICv2 Iterrupt Set-Active Reg        
	volatile __u32 GICD_ICACTIVERn[0x80/4]; 			//offset 0x380-0x3fc, Iterrupt Clear-Active Reg        
	volatile __u32 GICD_IPRIORITYRn[0x400/4]; 			//offset 0x400-0x5Fc, Iterrupt Priority Reg          
	volatile __u32 reserved2[0x24/4];				//0ffset 0x7fc-0x81c
	volatile __u32 GICD_ITARGETSRn[(0x400-0x20)/4];			//offset 0x820-0xbf8, Iterrupt Processor Targets Reg         
	volatile __u32 reserved3;					//0ffset 0xbfc
	volatile __u32 GICD_ICFGRn[0x100/4];				//offset 0xc00-0xcfc, Iterrupt Config Reg            
	volatile __u32 reserved4[0x100/4];				//0ffset 0xd00-0xdfc
	volatile __u32 GICD_NSACRn[0x100/4];				//offset 0xE00-0xEfc, non-secure Access Ctrl Reg ?   
	volatile __u32 reserved5[0x10/4];				//0ffset 0xf00-0xf0c
	volatile __u32 GICD_CPENDSGIRn[0x10/4];				//offset 0xf10-0xf1c, SGI Clear-Pending Reg          
	volatile __u32 GICD_SPENDSGIRn[0x10/4];				//offset 0xf20-0xf2c, SGI Set-Pending Reg       
	volatile __u32 reserved6[0xd0/4];				//0ffset 0xf30-0xffc         
	
};

struct gic_cpu_interface_disc{
	//cpu interface reg
	volatile __u32 GICC_CTLR_PMR_BPR[0xc/4];			//offset 0x00-0x08, cpu interface Ctrl Reg	+ 	Interrupt Priority Mask Reg	 + 	  Binary Point Reg	
	volatile __u32 reserved7[0x10/4];			//0ffset 0xC-0x18, readonly or writeonly      
	volatile __u32 GICC_ABPR;				//offset 0x1c, Aliased Binary Point Reg	
	volatile __u32 reserved8[0xb0/4];			//0ffset 0x20-0xcf, readonly or writeonly      
	volatile __u32 GICC_APRn[0x10/4];			//offset 0xd0-0xdc, Active Priorities Reg 		  
	volatile __u32 GICC_NSAPRn[0x10/4];			//offset 0xe0-0xec, Non-secure Active Priorities Reg
	volatile __u32 reserved9[0x10/4];			//0ffset 0xf0-0xfc, readonly or writeonly  
	volatile __u32 reserved10[0xf00/4];			//0ffset 0x100-0xffc, readonly or writeonly  
	volatile __u32 reserved11;				//0ffset 0x1000, readonly or writeonly   

};


#define GIC_CPU_CTRL			0x00
#define GIC_CPU_PRIMASK			0x04
#define GIC_CPU_BINPOINT		0x08
#define GIC_CPU_INTACK			0x0c
#define GIC_CPU_EOI			0x10
#define GIC_CPU_RUNNINGPRI		0x14
#define GIC_CPU_HIGHPRI			0x18

#define GIC_DIST_CTRL			0x000
#define GIC_DIST_CTR			0x004
#define GIC_DIST_ENABLE_SET		0x100
#define GIC_DIST_ENABLE_CLEAR		0x180
#define GIC_DIST_PENDING_SET		0x200
#define GIC_DIST_PENDING_CLEAR		0x280
#define GIC_DIST_ACTIVE_BIT		0x300
#define GIC_DIST_PRI			0x400
#define GIC_DIST_TARGET			0x800
#define GIC_DIST_CONFIG			0xc00
#define GIC_DIST_SOFTINT		0xf00


extern __s32 standby_int_init(void);
extern __s32 standby_int_exit(void);
extern __s32 standby_enable_int(enum interrupt_source_e src);
extern __s32 standby_query_int(enum interrupt_source_e src);


#endif  //__STANDBY_INT_H__

