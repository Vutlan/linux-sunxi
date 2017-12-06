/*
 * A i.MX25 ADC block driver for kernel 2.6.38.
 *
 * (C) 2011 SkyControl
 * (C) 2011 SLPotapenko <slpotapenko at gmail dot com>>
 * Redistributable under the terms of the GNU GPL.
 */
#ifndef ADC_VT100V1_H
#define ADC_VT100V1_H

#define ADC_VT100V1_HWR_NUM		(4)		/* num hardware channels */
#define ADC_VT100V1_MUX_NUM		(16)	/* num analog channels */
#define ADC_VT100V1_BLF_NUM		(2)		/* num board life channels */

#define ADC_VT100V1_DEV_NAME		"/dev/adc0"
//#define ADC_VT100V1_VREF			(3.0f)
//#define ADC_VT100V1_VONE			(ADC_VT100V1_VREF/((float)(1<<12)))
//#define ADC_VT100V1_VOLTAGE(x)		(((float)(x))*ADC_VT100V1_VONE)

//#define ADC_VT100V1_FACTOR_V12		(5)

#define ADC_VT100V1_FILTER_DELTA(f) 	((1<<12)*f)	// part of the full scope to delta

#define ADC_VT100V1_FILTER_SURGE		(1<<31)	/* surge flag */


/* Hardware channels numbers */
enum {
	ADC_VT100V1_TEMP=0,
	ADC_VT100V1_V12=1,
	ADC_VT100V1_ADETECT=2,
	ADC_VT100V1_ASIGNAL=3,
};

/*
 * output data format block
 */
struct sky25_adc_data {
	u32 count;
	u32 analog[ADC_VT100V1_MUX_NUM];
	u32 brdlife[ADC_VT100V1_BLF_NUM];
};

/*
 * For ioctl support.
 */
#include <asm/ioctl.h>

/* magic number for SKY25 IOCTL operations */
#define ADC_VT100V1_MAGIC 's'

#define ADC_VT100V1_PRMGET  _IOR(ADC_VT100V1_MAGIC, 1 , char *) // get filter param
#define ADC_VT100V1_PRMSET  _IOW(ADC_VT100V1_MAGIC, 2 , char *) // set filter param

struct sky25_adc_param {
	u8 num;
	u8 wdc;
	u8 wac;
	u16 delta;
};

#endif
