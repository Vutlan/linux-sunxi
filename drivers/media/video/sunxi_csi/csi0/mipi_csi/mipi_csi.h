#ifndef __MIPI_CSI__H__
#define __MIPI_CSI__H__

#include "dphy/dphy.h"
#include "protocol/protocol.h"

void bsp_mipi_csi_set_base_addr(unsigned int addr_base);
void bsp_mipi_csi_dphy_enable(void);
void bsp_mipi_csi_dphy_disable(void);
void bsp_mipi_csi_protocol_enable(void);
void bsp_mipi_csi_protocol_disable(void);

void bsp_mipi_csi_det_mipi_clk(unsigned int *mipi_bps,unsigned int dphy_clk);
void bsp_mipi_csi_set_rx_dly(unsigned int mipi_bps,unsigned int dphy_clk);
void bsp_mipi_csi_set_lp_ulps_wp(unsigned int lp_ulps_wp_ms,unsigned int lp_clk);
void bsp_mipi_csi_set_dphy_timing(unsigned int *mipi_bps,unsigned int dphy_clk, unsigned int mode);
void bsp_mipi_csi_set_lane(unsigned char lane_num);
void bsp_mipi_csi_set_total_ch(unsigned char ch_num);
void bsp_mipi_csi_set_pkt_header(unsigned char ch,unsigned char vc,enum pkt_fmt mipi_pkt_fmt);
void bsp_mipi_csi_set_src_type(unsigned char ch,enum source_type src_type);

#endif