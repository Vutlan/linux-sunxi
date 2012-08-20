
#ifndef __PROTOCOL__H__
#define __PROTOCOL__H__

enum bit_order
{
	LSB_FST,
	MSB_FST,
};

enum byte_order
{
	WCh_WCl_DI,
	DI_WCh_WCl,
	WCl_WCh,DI,
	DI_WCl_WCh,
};

enum source_type
{
	PROGRESSIVE,
	INTERLACED,
};

enum line_sync
{
	TGL_PKT,
	TGL_LSLE,
};

enum pkt_fmt
{
	FS						=	0X00, //short packet      
	FE						=	0X01,
	LS						=	0X02,
	LE						=	0X03,
	SDAT0					=	0X08,
	SDAT1					=	0X09,
	SDAT2					=	0X0A,
	SDAT3					=	0X0B,
	SDAT4					=	0X0C,
	SDAT5					=	0X0D,
	SDAT6					=	0X0E,
	SDAT7					=	0X0F,      				
//	NULL					=	0X10,	//long packet      
	BLK						=	0X11,
	EMBD					=	0X12,
	YUV420				=	0X18,
	YUV420_10 		= 0X19,
	YUV420_CSP		=	0X1C,
	YUV420_CSP_10 =	0X1D,
	YUV422				=	0X1E,
	YUV422_10 		= 0X1F,
	RGB565				=	0X22,
	RGB888				=	0X24,
	RAW8					=	0X2A,
	RAW10					=	0X2B,
	RAW12					=	0X2C,
	USR_DAT0			=	0X30,
	USR_DAT1			=	0X31,
	USR_DAT2			=	0X32,
	USR_DAT3			=	0X33,
	USR_DAT4			=	0X34,
	USR_DAT5			=	0X35,
	USR_DAT6			=	0X36,
	USR_DAT7			=	0X37,
};

enum protocol_int
{
	FIFO_OVER_INT       	,
	FRAME_END_SYNC_INT    ,
	FRAME_START_SYNC_INT  ,
	LINE_END_SYNC_INT     ,
	LINE_START_SYNC_INT   ,
	PH_UPDATE_INT         ,
	PF_INT                ,
	EMB_DATA_INT          ,
	FRAME_SYNC_ERR_INT    ,
	LINE_SYNC_ERR_INT     ,
	ECC_ERR_INT           ,
	ECC_WRN_INT           ,
	CHKSUM_ERR_INT        ,
	EOT_ERR_INT           ,
};

#endif
