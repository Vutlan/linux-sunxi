#ifndef ADC_FILTER_VT100V1_H
#define ADC_FILTER_VT100V1_H

#ifdef __cplusplus
extern "C"  {
#endif

#include <linux/types.h>

// Median filter
#define ADC_FILTER_VT100V1_SZ 	(7)				// buffer size (odd)
#define ADC_FILTER_VT100V1_DC 	(4)				// integral length
#define ADC_FILTER_VT100V1_AC 	(1)				// ac fraction
#define ADC_FILTER_VT100V1_DELTA 	((1<<12)*0.05)	// minimal delta (absolute) for accelerate
#define ADC_FILTER_VT100V1_SURGE	(1<<31)			// surge flag


/*
	SKY25 custom filter
*/
struct filter_sky25 {
	u16 value[ADC_FILTER_VT100V1_SZ];
	u8 index;
	u32 integral;
	u8 wdc;
	u8 wac;
	u16 delta;
	u8 surge;
};

void inline sky25_filter_create(struct filter_sky25 *f, u8 wdc, u8 wac, u16 delta)
{
	f->index = ADC_FILTER_VT100V1_SZ-1;
	f->integral = 0;
	f->wdc = wdc;
	f->wac = wac;
	f->delta = delta;
}

void inline sky25_filter_put(struct filter_sky25 *f, u16 val)
{
	f->index = (f->index+1)%(ADC_FILTER_VT100V1_SZ);
	f->value[f->index] = val;
	f->integral -= ((f->integral) >>(f->wdc+f->wac));
	f->integral += (val>>(f->wac));
}

u32 inline sky25_filter_get(struct filter_sky25 *f)
{
	u8 i,j,pos,cnt;
	u32 value;
	u32 dc=0;

	value = f->value[0];

	for(i=0;i<ADC_FILTER_VT100V1_SZ;i++)
	{
		// median search
		cnt = 0;

		pos = (i+1) % ADC_FILTER_VT100V1_SZ;

		for(j=0;j<ADC_FILTER_VT100V1_SZ-1;j++)
		{
			if(f->value[i] < f->value[pos])
				cnt++;

			pos = (pos+1) % ADC_FILTER_VT100V1_SZ;
		}

		// its median?
		if(cnt == ((ADC_FILTER_VT100V1_SZ >> 1)+1))
		{
			value = f->value[i];
			goto exit;
		}
	}

exit:
	// signal dc
	dc = ((f->integral) >> (f->wdc));

	if( abs(dc - value ) < (f->delta) )
	{// smooth
		value = dc;
		f->surge = 0;
	}
	else
	{// accelerate
		f->integral = (value << (f->wdc));
		f->surge = 1;
	}

	if(f->surge)
		value |= ADC_FILTER_VT100V1_SURGE;

	return value;
}


#ifdef __cplusplus
}
#endif

#endif

