
#ifndef __DPHY_REG_H__
#define __DPHY_REG_H__

#include "dphy.h"

extern void dphy_reg_map(unsigned int addr_base);
extern void dphy_enable(void);
extern void dphy_disable(void);
extern void dphy_set_data_lane(unsigned char lane_num);                   		
extern void dphy_rx_enable(unsigned char lane_num);
extern void dphy_rx_disable(unsigned char lane_num);
extern void dphy_rx_dbc_enable(void);
extern void dphy_rx_dbc_disable(void);
extern void dphy_rx_hs_clk_miss_cnt_enable(void);
extern void dphy_rx_hs_clk_miss_cnt_disable(void);
extern void dphy_rx_hs_sync_cnt_enable(void);
extern void dphy_rx_hs_sync_cnt_disable(void);
extern void dphy_rx_lp_to_cnt_enable(void);
extern void dphy_rx_lp_to_cnt_disable(void);
extern void dphy_rx_freq_cnt_enable(void);
extern void dphy_rx_freq_cnt_disable(void);
extern void dphy_rx_set_hs_clk_miss(unsigned char cnt);
extern void dphy_rx_set_hs_sync_to(unsigned char cnt);
extern void dphy_rx_set_lp_to(unsigned char cnt);
extern void dphy_rx_set_rx_dly(unsigned short cnt);
extern void dphy_rx_set_lprst_dly(unsigned char cnt);
extern void dphy_rx_set_lp_ulps_wp(unsigned int cnt);
extern void dphy_int_enable(enum dphy_int dphy_int);
extern void dphy_int_disable(enum dphy_int dphy_int);
extern void dphy_get_int_status(enum dphy_int dphy_int);
extern void dphy_clear_int_status(enum dphy_int dphy_int);

extern unsigned char 	dphy_get_data_lane(void); 
extern unsigned char 	dphy_rx_get_hs_clk_miss(void);
extern unsigned char 	dphy_rx_get_hs_sync_to(void);
extern unsigned char 	dphy_rx_get_lp_to(void);
extern unsigned short dphy_rx_get_rx_dly(void);
extern unsigned char 	dphy_rx_get_lprst_dly(void);
extern unsigned int  	dphy_rx_get_lp_ulps_wp(void);
extern unsigned short	dphy_rx_get_freq_cnt(void);
extern unsigned char 	dphy_get_hs_data(enum dphy_lane lane);
extern enum dphy_lane_state dphy_get_lane_state(enum dphy_lane lane);
                                             
#endif
