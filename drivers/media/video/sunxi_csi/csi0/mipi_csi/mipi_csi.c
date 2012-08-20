#include <linux/delay.h>
#include "dphy/dphy_reg.h"
#include "protocol/protocol_reg.h"

void bsp_mipi_csi_set_base_addr(unsigned int addr_base)
{
	ptcl_reg_map(addr_base+0x1000);
	dphy_reg_map(addr_base+0x2000);
}

void bsp_mipi_csi_dphy_enable(void)
{
	dphy_enable();
}

void bsp_mipi_csi_dphy_disable(void)
{
	dphy_disable();
}

void bsp_mipi_csi_protocol_enable(void)
{
	ptcl_enable();
}

void bsp_mipi_csi_protocol_disable(void)
{
	ptcl_disable();
}

void bsp_mipi_csi_det_mipi_clk(unsigned int *mipi_bps,unsigned int dphy_clk)
{
	unsigned int freq_cnt;
	
	dphy_rx_freq_cnt_enable();
	mdelay(1);
	freq_cnt = dphy_rx_get_freq_cnt();
	if(freq_cnt == 0)
		return; 
	
	*mipi_bps = 1000 * dphy_clk * 8 / freq_cnt;
}

void bsp_mipi_csi_set_rx_dly(unsigned int mipi_bps,unsigned int dphy_clk)
{
	unsigned short rx_dly_cnt;
	unsigned int mipi_byte_clk;
	
	if(mipi_bps == 0)
		return;
	
	mipi_byte_clk = mipi_bps / 8;
	rx_dly_cnt = (8 * (dphy_clk + mipi_byte_clk / 16)) / mipi_byte_clk;
	dphy_rx_set_rx_dly(rx_dly_cnt);
}

void bsp_mipi_csi_set_lp_ulps_wp(unsigned int lp_ulps_wp_ms,unsigned int lp_clk)
{
	unsigned int lp_ulps_wp_cnt;

	lp_ulps_wp_cnt = lp_ulps_wp_ms * lp_clk / 1000;
	dphy_rx_set_lp_ulps_wp(lp_ulps_wp_cnt);
}

void bsp_mipi_csi_set_dphy_timing(unsigned int *mipi_bps,unsigned int dphy_clk, unsigned int mode)
{
	if(mode == 1)
		bsp_mipi_csi_det_mipi_clk(mipi_bps,dphy_clk);
	
	dphy_rx_dbc_enable();
	bsp_mipi_csi_set_rx_dly(*mipi_bps,dphy_clk);
	dphy_rx_hs_clk_miss_cnt_disable();
	dphy_rx_hs_sync_cnt_disable();
	dphy_rx_lp_to_cnt_disable();
	//dphy_rx_set_lp_ulps_wp(0xff);
}

void bsp_mipi_csi_set_lane(unsigned char lane_num)
{
	dphy_rx_disable(4);
	dphy_set_data_lane(lane_num);
	dphy_rx_enable(lane_num);
	ptcl_set_data_lane(lane_num);
}

void bsp_mipi_csi_set_total_ch(unsigned char ch_num)
{
	ptcl_set_total_ch(ch_num);
}

void bsp_mipi_csi_set_pkt_header(unsigned char ch,unsigned char vc,enum pkt_fmt mipi_pkt_fmt)
{
	ptcl_set_vc(ch,vc);
	ptcl_set_dt(ch,mipi_pkt_fmt);
}

void bsp_mipi_csi_set_src_type(unsigned char ch,enum source_type src_type)
{
	ptcl_set_src_type(ch,src_type);
}