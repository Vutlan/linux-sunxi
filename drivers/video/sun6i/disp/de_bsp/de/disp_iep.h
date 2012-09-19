#ifndef __DISP_IEP_H_
#define __DISP_IEP_H_

#include "disp_display_i.h"
#include "disp_display.h"


#define DRC_USED 						0x04000000
#define DRC_USED_MASK 					(~(DRC_USED))
#define CMU_SCREEN_EN                   0x10000000
#define CMU_SCREEN_EN_MASK              (~(CMU_SCREEN_EN))
#define CMU_LAYER_EN                    0x20000000
#define CMU_LAYER_EN_MASK               (~(CMU_LAYER_EN))

 


 

//DRC:
#if 0
extern __s32 IEP_Drc_Init(__u32 sel);
extern __s32 IEP_Drc_Exit(__u32 sel);
extern __s32 IEP_Drc_Enable(__u32 sel, __u32 en);
extern __s32 IEP_Drc_Set_Mode(__u32 sel, __u32 mode);
extern __s32 IEP_Drc_Set_Imgsize(__u32 sel, __u32 width, __u32 height);
extern __s32 IEP_Drc_Operation_In_Vblanking(__u32 sel, __u32 mode); 
extern __s32 IEP_Drc_Set_Reg_Base(__u32 sel, __u32 base);
extern __s32 IEP_Drc_Set_Winodw(__u32 sel, __disp_rect_t *window);//full screen for default
extern __s32 IEP_Drc_Get_Winodw(__u32 sel, __disp_rect_t *window);//full screen for default
extern __s32 IEP_Drc_Get_Bright_Diming(__u32 sel);
extern __s32 IEP_Drc_Early_Suspend(__u32 sel);//close clk
extern __s32 IEP_Drc_suspend(__u32 sel);//save register
extern __s32 IEP_Drc_Resume (__u32 sel);//restore register
extern __s32 IEP_Drc_Late_Resume(__u32 sel);//open clk
#endif

__s32 Disp_drc_start_video_mode(__u32 sel);
__s32 Disp_drc_start_ui_mode(__u32 sel);
__s32 disp_deu_set_frame_info(__u32 sel, __u32 hid);
__s32 disp_cmu_layer_clear(__u32 sel);
__s32 disp_deu_clear(__u32 sel, __u32 hid);




#endif
