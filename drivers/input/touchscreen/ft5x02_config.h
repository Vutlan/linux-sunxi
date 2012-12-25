#ifndef __FT5X02_CONFIG_H__ 
#define __FT5X02_CONFIG_H__ 
/*FT5X02 config*/

/*此参数兼容TP: 飞触、中触、德怡、滕耀发，wj*/
#define FT5X02_FIRMWARE_ID			14
#define FT5X02_OTP_PARAM_ID			0
#define FT5X02_CUSTOMER_ID			121
#define FT5X02_CHIPER_ID			2
#define FT5X02_RESOLUTION_X			480
#define FT5X02_RESOLUTION_Y			800
#define FT5X02_LEMDA_X			30
#define FT5X02_LEMDA_Y			50
#define FT5X02_KX			200
#define FT5X02_KY			215
#define FT5X02_DIRECTION			1
#define FT5X02_KX_LR			340
#define FT5X02_KY_UD			419
#define FT5X02_POINTS_SUPPORTED			5
#define FT5X02_THGROUP			110 //触摸的灵敏度，越小越灵敏
#define FT5X02_THPEAK			60
#define FT5X02_THDIFF			2560
#define FT5X02_MAX_TOUCH_VALUE			1200
#define FT5X02_DRAW_LINE_TH			250
#define FT5X02_PWMODE_CTRL			1
#define FT5X02_PERIOD_ACTIVE			16
#define FT5X02_TIME_ENTER_MONITOR			10
#define FT5X02_PERIOD_MONITOR			40
#define FT5X02_FILTER_FRAME_NOISE			2
#define FT5X02_POWERNOISE_FILTER_TH			0
#define FT5X02_DIFFDATA_HADDLE_VALUE			-100
#define FT5X02_FACE_DETECT_MODE			0
#define FT5X02_FACE_DETECT_STATISTICS_TX_NUM			3
#define FT5X02_FACE_DETECT_PRE_VALUE			20
#define FT5X02_FACE_DETECT_NUM			10
#define FT5X02_FACE_DETECT_LAST_TIME			1000
#define FT5X02_BIGAREA_PEAK_VALUE_MIN			255
#define FT5X02_BIGAREA_DIFF_VALUE_OVER_NUM			30
#define FT5X02_BIGAREA_POINT_AUTO_CLEAR_TIME			3000
#define FT5X02_ABNORMAL_DIFF_VALUE			60
#define FT5X02_ABNORMAL_DIFF_NUM			10
#define FT5X02_ABNORMAL_DIFF_LAST_FRAME			30
#define FT5X02_START_RX			0
#define FT5X02_ADC_TARGET			8500
#define FT5X02_ESD_FILTER_FRAME			0
#define FT5X02_MOVSTH_I			3
#define FT5X02_MOVSTH_N			2
#define FT5X02_MODE			1
#define FT5X02_PMODE			0
#define FT5X02_ERR			0
#define FT5X02_AUTO_CLB_MODE			255
#define FT5X02_STATE			1
#define FT5X02_HIGH_SPEED_TH			64
#define FT5X02_MID_SPEED_TH			85
#define FT5X02_STATIC_TH			243
#define FT5X02_THFALSE_TOUCH_PEAK			193


                           
            
unsigned char g_ft5x02_gain= 10; //增益  越小越灵敏
unsigned char g_ft5x02_voltage= 2;
unsigned char g_ft5x02_scanselect= 3; //扫面频率
unsigned char g_ft5x02_tx_offset= 1;

unsigned char g_ft5x02_tx_num = 15;
unsigned char g_ft5x02_rx_num = 10;
//unsigned char g_ft5x02_tx_order[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14};//DEYI
unsigned char g_ft5x02_tx_order[40] = {14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};//WJ
unsigned char g_ft5x02_tx_cap[40] = {30,36,36,37,37,38,38,39,39,40,40,41,41,42,45};//电容补偿
//unsigned char g_ft5x02_rx_order[] = {0,1,2,3,4,5,6,7,8,9};
unsigned char g_ft5x02_rx_order[30] = {9,8,7,6,5,4,3,2,1,0};
unsigned char g_ft5x02_rx_offset[15] = {153,169,154,153,152};
unsigned char g_ft5x02_rx_cap[30] = {70,75,70,70,70,70,70,70,70,70};//电容补偿

#endifdetect_mode                                 
#define FT5X02_FACE_DETECT_STATISTICS_TX_NUM		  ft5x02_face_detect_statustice_tx_num                    
#define FT5X02_FACE_DETECT_PRE_VALUE						  ft5x02_face_detect_pre_value                            
#define FT5X02_FACE_DETECT_NUM									  ft5x02_face_defect_num                                  
#define FT5X02_FACE_DETECT_LAST_TIME						  ft5x02_face_detect_last_time                            
#define FT5X02_BIGAREA_PEAK_VALUE_MIN						  ft5x02_bigarea_peak_value_min                           
#define FT5X02_BIGAREA_DIFF_VALUE_OVER_NUM			  ft5x02_bigarea_diff_value_over_num                      
#define FT5X02_BIGAREA_POINT_AUTO_CLEAR_TIME		  ft5x02_bigarea_point_auto_clear_time                    
#define FT5X02_ABNORMAL_DIFF_VALUE							  ft5x02_abnormal_diff_value                              
#define FT5X02_ABNORMAL_DIFF_NUM			      		  ft5x02_abnormal_diff_num                                
#define FT5X02_ABNORMAL_DIFF_LAST_FRAME					  ft5x02_abnormal_diff_last_frame                         
#define FT5X02_START_RX													  ft5x02_start_rx                                         
#define FT5X02_ADC_TARGET												  ft5x02_adc_target                                       
#define FT5X02_ESD_FILTER_FRAME									  ft5x02_esd_filter_frame                                 
#define FT5X02_MOVSTH_I													  ft5x02_movsth_i                                         
#define FT5X02_MOVSTH_N													  ft5x02_movsth_n                                         
#define FT5X02_MODE															  ft5x02_mode                                             
#define FT5X02_PMODE														  ft5x02_pmode                                            
#define FT5X02_ERR															  ft5x02_err                                              
#define FT5X02_AUTO_CLB_MODE										  ft5x02_auto_clb_mode                                    
#define FT5X02_STATE														  ft5x02_state                                            
#define FT5X02_HIGH_SPEED_TH			                ft5x02_high_speed_th                                    
#define FT5X02_MID_SPEED_TH											  ft5x02_mid_speed_th                                     
#define FT5X02_STATIC_TH												  ft5x02_static_th                                        
#define FT5X02_THFALSE_TOUCH_PEAK								  ft5x02_thfalse_touch_peak                               
#endif

static int ft5x02_firmware_id=14;                                      
static int ft5x02_opt_param_id=0;                                    
static int ft5x02_customer_id=121 ;                                     
static int ft5x02_chiper_id=2;                                       
static int ft5x02_resolution_x=480;                                    
static int ft5x02_resolution_y=800;                                     
static int ft5x02_lemda_x=30;                                       
static int ft5x02_lemda_y=50;                                          
static int ft5x02_kx=200;                                              
static int ft5x02_ky=215;                                              
static int ft5x02_direction=1;                                        
static int ft5x02_kx_lr=340;                                            
static int ft5x02_ky_ud=419;                                            
static int ft5x02_points_supported=5;                                 
static int ft5x02_thgroup=110;      //触摸的灵敏度，越小越灵敏        
static int ft5x02_thpeak=60;                                           
static int ft5x02_thdiff=2560;                                           
static int ft5x02_max_touch_value=1200;                                  
static int ft5x02_draw_line_th=250;                                     
static int ft5x02_pwmode_ctrl=1;                                      
static int ft5x02_period_active=16;                                    
static int ft5x02_time_enter_monitor=10;                               
static int ft5x02_period_monitor=40;                                   
static int ft5x02_filter_frame_noise=2;                               
static int ft5x02_powernoise_filter_th=0;                             
static int ft5x02_diffdata_haddle_value=-100;                            
static int ft5x02_face_detect_mode=0;                                 
static int ft5x02_face_detect_statustice_tx_num=3;                    
static int ft5x02_face_detect_pre_value=20;                            
static int ft5x02_face_defect_num=10;                                  
static int ft5x02_face_detect_last_time=1000;                            
static int ft5x02_bigarea_peak_value_min=255;                           
static int ft5x02_bigarea_diff_value_over_num=30;                      
static int ft5x02_bigarea_point_auto_clear_time=3000;                    
static int ft5x02_abnormal_diff_value=60;                              
static int ft5x02_abnormal_diff_num=10;                                
static int ft5x02_abnormal_diff_last_frame=30;                         
static int ft5x02_start_rx=0;                                         
static int ft5x02_adc_target=8500;                                       
static int ft5x02_esd_filter_frame=0;                                 
static int ft5x02_movsth_i=3;                                        
static int ft5x02_movsth_n=2;                                         
static int ft5x02_mode=1;                                             
static int ft5x02_pmode=0;                                            
static int ft5x02_err =0;                                             
static int ft5x02_auto_clb_mode=255;                                    
static int ft5x02_state=1;                                            
static int ft5x02_high_speed_th=64;                                    
static int ft5x02_mid_speed_th=85;                                     
static int ft5x02_static_th=243;                                        
static int ft5x02_thfalse_touch_peak=193;                               
            
unsigned char g_ft5x02_gain= 10; //增益  越小越灵敏
unsigned char g_ft5x02_voltage= 2;
unsigned char g_ft5x02_scanselect= 3; //扫面频率
unsigned char g_ft5x02_tx_offset= 1;

unsigned char g_ft5x02_tx_num = 15;
unsigned char g_ft5x02_rx_num = 10;
//unsigned char g_ft5x02_tx_order[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14};//DEYI
unsigned char g_ft5x02_tx_order[40] = {14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};//WJ
unsigned char g_ft5x02_tx_cap[40] = {30,36,36,37,37,38,38,39,39,40,40,41,41,42,45};//电容补偿
//unsigned char g_ft5x02_rx_order[] = {0,1,2,3,4,5,6,7,8,9};
unsigned char g_ft5x02_rx_order[30] = {9,8,7,6,5,4,3,2,1,0};
unsigned char g_ft5x02_rx_offset[15] = {153,169,154,153,152};
unsigned char g_ft5x02_rx_cap[30] = {70,75,70,70,70,70,70,70,70,70};//电容补偿

#endif