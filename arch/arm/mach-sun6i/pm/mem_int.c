#include "pm_types.h"
#include "pm_i.h"

/*
*********************************************************************************************************
*                                       mem gic save
*
* Description: mem gic save.
*
* Arguments  : struct gic_state *pgic_state.
*
* Returns    : 0/-1;
*********************************************************************************************************
*/
__s32 mem_int_save(struct gic_state *pgic_state)
{
	int i=0;
	//distributor
	struct gic_distributor_disc *GICD_REG = (struct gic_distributor_disc *)IO_ADDRESS(AW_GIC_DIST_BASE);
	struct gic_distributor_state *l_distributor = (struct gic_distributor_state *)(&(pgic_state->m_distributor));
	//interface
	struct gic_cpu_interface_disc *GICC_REG = (struct gic_cpu_interface_disc *)IO_ADDRESS(AW_GIC_CPU_BASE);
	struct gic_cpu_interface_state *l_interface = (struct gic_cpu_interface_state *)(&(pgic_state->m_interface));
	
	/* save GIC dispatch registers */	
	l_distributor->GICD_CTLR						=	GICD_REG->GICD_CTLR;
	l_distributor->GICD_IGROUPRn					=	GICD_REG->GICD_IGROUPRn;
	
	for(i = 0; i < (0x180/4); i++){ 
		l_distributor->GICD_ISENABLERn[i]		=	GICD_REG->GICD_ISENABLERn[i];			 
	}
	for(i = 0; i < (0x180/4); i++){ 
		l_distributor->GICD_ICENABLERn[i]		=	GICD_REG->GICD_ICENABLERn[i];
	}
	for(i = 0; i < (0x180/4); i++){ 			 
		l_distributor->GICD_ISPENDRn[i]			=	GICD_REG->GICD_ISPENDRn[i]; 	 
	}
	for(i = 0; i < (0x180/4); i++){ 			 
		l_distributor->GICD_ICPENDRn[i]			=	GICD_REG->GICD_ICPENDRn[i]; 	 
	}
	for(i = 0; i < (0x180/4); i++){ 
		l_distributor->GICD_ISACTIVERn[i]		=	GICD_REG->GICD_ISACTIVERn[i];			 
	}
	for(i = 0; i < (0x180/4); i++){ 
		l_distributor->GICD_ICACTIVERn[i]		=	GICD_REG->GICD_ICACTIVERn[i];			 
	}
	for(i = 0; i < (0x180/4); i++){ 
		l_distributor->GICD_IPRIORITYRn[i] 		=	GICD_REG->GICD_IPRIORITYRn[i];			 
	}
	
	for(i = 0; i < ((0x400-0x20)/4); i++){	
		l_distributor->GICD_ITARGETSRn[i]		=	GICD_REG->GICD_ITARGETSRn[i];	 
	}		  
	
	for(i = 0; i < (0x100/4); i++){ 
		l_distributor->GICD_ICFGRn[i]			=	GICD_REG->GICD_ICFGRn[i]; 			 
	}
	for(i = 0; i < (0x100/4); i++){ 
		l_distributor->GICD_NSACRn[i]			=	GICD_REG->GICD_NSACRn[i]; 			 
	}
	
	for(i = 0; i < (0x10/4); i++){	
		l_distributor->GICD_CPENDSGIRn[i] 		=	GICD_REG->GICD_CPENDSGIRn[i];		 
	}
	for(i = 0; i < (0x10/4); i++){	
		l_distributor->GICD_SPENDSGIRn[i] 		=	GICD_REG->GICD_SPENDSGIRn[i];		 
	}

	/* save CPU interface registers */
	for(i = 0; i < (0xc/4); i++){	
		l_interface->GICC_CTLR_PMR_BPR[i]		=       GICC_REG->GICC_CTLR_PMR_BPR[i]; 
	}   
	
	l_interface->GICC_ABPR			       		=       GICC_REG->GICC_ABPR; 
	
	for(i = 0; i < (0x10/4); i++){	
		l_interface->GICC_APRn[i]			    =       GICC_REG->GICC_APRn[i]; 
	} 
	 
	for(i = 0; i < (0x10/4); i++){	
		l_interface->GICC_NSAPRn[i]			    =       GICC_REG->GICC_NSAPRn[i]; 
	} 
	
	return 0;
}


/*
*********************************************************************************************************
*                                       mem_int_restore
*
* Description: mem gic restore.
*
* Arguments  : struct gic_state *pgic_state.
*
* Returns    : 0/-1;
*********************************************************************************************************
*/
__s32 mem_int_restore(struct gic_state *pgic_state)
{
	int i=0;
	//distributor
	struct gic_distributor_disc *GICD_REG = (struct gic_distributor_disc *)IO_ADDRESS(AW_GIC_DIST_BASE);
	struct gic_distributor_state *l_distributor = (struct gic_distributor_state *)(&(pgic_state->m_distributor));
	//interface
	struct gic_cpu_interface_disc *GICC_REG = (struct gic_cpu_interface_disc *)IO_ADDRESS(AW_GIC_CPU_BASE);
	struct gic_cpu_interface_state *l_interface = (struct gic_cpu_interface_state *)(&(pgic_state->m_interface));
	
	/* restore GIC dispatch registers */
	GICD_REG->GICD_CTLR						=	l_distributor->GICD_CTLR;
	GICD_REG->GICD_IGROUPRn					=	l_distributor->GICD_IGROUPRn;
	                                                        
	for(i = 0; i < (0x180/4); i++){                         
		GICD_REG->GICD_ISENABLERn[i]		=	l_distributor->GICD_ISENABLERn[i];			 
	}                                                       
	for(i = 0; i < (0x180/4); i++){                         
		GICD_REG->GICD_ICENABLERn[i]		=	l_distributor->GICD_ICENABLERn[i];
	}                                                       
	for(i = 0; i < (0x180/4); i++){ 			
		GICD_REG->GICD_ISPENDRn[i]			=	l_distributor->GICD_ISPENDRn[i]; 	 
	}                                                       
	for(i = 0; i < (0x180/4); i++){ 			
		GICD_REG->GICD_ICPENDRn[i]			=	l_distributor->GICD_ICPENDRn[i]; 	 
	}                                                      
	for(i = 0; i < (0x180/4); i++){                         
		GICD_REG->GICD_ISACTIVERn[i]		=	l_distributor->GICD_ISACTIVERn[i];			 
	}                                                       
	for(i = 0; i < (0x180/4); i++){                         
		GICD_REG->GICD_ICACTIVERn[i]		=	l_distributor->GICD_ICACTIVERn[i];			 
	}                                                      
	for(i = 0; i < (0x180/4); i++){                         
		GICD_REG->GICD_IPRIORITYRn[i] 		=	l_distributor->GICD_IPRIORITYRn[i];			 
	}                                                      
	                                                        
	for(i = 0; i < ((0x400-0x20)/4); i++){	               
		GICD_REG->GICD_ITARGETSRn[i]		=	l_distributor->GICD_ITARGETSRn[i];	 
	}		                                        
	                                                        
	for(i = 0; i < (0x100/4); i++){                        
		GICD_REG->GICD_ICFGRn[i]			=	l_distributor->GICD_ICFGRn[i]; 			 
	}                                                       
	for(i = 0; i < (0x100/4); i++){                         
		GICD_REG->GICD_NSACRn[i]			=	l_distributor->GICD_NSACRn[i]; 			 
	}                                                       
	                                                       
	for(i = 0; i < (0x10/4); i++){	                        
		GICD_REG->GICD_CPENDSGIRn[i] 		=	l_distributor->GICD_CPENDSGIRn[i];		 
	}                                                       
	for(i = 0; i < (0x10/4); i++){	                        
		GICD_REG->GICD_SPENDSGIRn[i] 		=	l_distributor->GICD_SPENDSGIRn[i];		 
	}                                                       

	/* restore CPU interface registers */
	for(i = 0; i < (0xc/4); i++){	
		GICC_REG->GICC_CTLR_PMR_BPR[i]			=       l_interface->GICC_CTLR_PMR_BPR[i]; 
	}   
	
	GICC_REG->GICC_ABPR							=       l_interface->GICC_ABPR; 
	
	for(i = 0; i < (0x10/4); i++){	
		GICC_REG->GICC_APRn[i]					=       l_interface->GICC_APRn[i]; 
	} 
	 
	for(i = 0; i < (0x10/4); i++){	
		GICC_REG->GICC_NSAPRn[i]				=       l_interface->GICC_NSAPRn[i]; 
	} 

	return 0;
}
