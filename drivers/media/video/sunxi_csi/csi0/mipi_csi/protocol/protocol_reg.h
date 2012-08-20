
#ifndef __PROTOCOL_REG__H__
#define __PROTOCOL_REG__H__

#include "protocol.h"

extern void ptcl_reg_map(unsigned int addr_base);
extern void ptcl_enable(void);
extern void ptcl_disable(void);
extern void ptcl_set_data_lane(unsigned char lane_num);
extern void ptcl_set_pl_bit_order(enum bit_order pl_bit_ord);
extern void ptcl_set_ph_bit_order(enum bit_order ph_bit_ord);
extern void ptcl_set_ph_byte_order(enum byte_order ph_byte_order);
extern void ptcl_set_total_ch(unsigned char ch_num);
extern void ptcl_set_vc(unsigned char ch,unsigned char vc);
extern void ptcl_set_dt(unsigned char ch,enum pkt_fmt dt);
extern void ptcl_set_src_type(unsigned char ch, enum source_type src_type);
extern void ptcl_set_line_sync(unsigned char ch, enum line_sync ls_mode);
extern void ptcl_int_enable(unsigned char ch,enum protocol_int int_flag);
extern void ptcl_int_disable(unsigned char ch,enum protocol_int int_flag);
extern void ptcl_clear_int_status(unsigned char ch,enum protocol_int int_flag);
extern unsigned char 			ptcl_get_data_lane(void);
extern enum bit_order 		ptcl_get_pl_bit_order(void);
extern enum bit_order 		ptcl_get_ph_bit_order(void);
extern enum byte_order 		ptcl_get_ph_byte_order(void);
extern unsigned char 			ptcl_get_total_ch(void);
extern unsigned char 			ptcl_get_vc(unsigned char ch);
extern enum pkt_fmt 			ptcl_get_dt(unsigned char ch);
extern enum source_type 	ptcl_get_src_type(unsigned char ch);
extern enum line_sync 		ptcl_get_line_sync(unsigned char ch);
extern unsigned char 			ptcl_get_int_status(unsigned char ch,enum protocol_int int_flag);

#endif
