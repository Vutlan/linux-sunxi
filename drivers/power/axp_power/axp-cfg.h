#ifndef __LINUX_AXP_CFG_H_
#define __LINUX_AXP_CFG_H_

#define AW1636_ADDR			0x68 >> 1

#define	AW1636_I2CBUS		0
#define BATRDC				200 //initial rdc
#define AW1636_IRQNO         0


#define	LDO1SET				0  //0: LDO1SET connect AGND, 1: LDO1SET connect AIPS, for axp189 LDOSET bonding to AGND
#define	DC2SET				1  //0: DC2SET connect GND, 1: DC2SET connect IPSOUT, for axp189 DC2SET bonding to IPSOUT
#define	DC3SET				1  //0:DC3SET connect GND, 1:DC3SET connect IPSOUT ,for axp189 DC3SET to pin

#define AW1636LDO1          1800

#if !LDO1SET
	#define LDO1MIN			1250
	#define LDO1MAX			1250
#else
	#define LDO1MIN			3300
	#define LDO1MAX			3300
#endif

#if DC2SET
	#define DCDC2MIN		800
	#define DCDC2MAX		1400
#else
	#define DCDC2MIN		1400
	#define DCDC2MAX		2000
#endif

#if DC3SET
	#define DCDC3MIN		2000
	#define DCDC3MAX		2700
	#define  LDO3MIN		1600
	#define  LDO3MAX		1900
#else
	#define DCDC3MIN		1300
	#define DCDC3MAX		1900
	#define  LDO3MIN		2300
	#define  LDO3MAX		2600
#endif

/*check by spec*/
#define AW1636_VOL_MAX		12 // capability buffer length
#define AW1636_TIME_MAX		20
#define AW1636_AVER_MAX		10
#define AW1636_RDC_COUNT	10

#define ABS(x)				((x) >0 ? (x) : -(x) )

#define END_VOLTAGE_APS		3350

#define BAT_AVER_VOL		3820	//Aver Vol:3.82V

#define FUELGUAGE_LOW_VOL	3400	//<3.4v,2%
#define FUELGUAGE_VOL1		3500    //<3.5v,3%
#define FUELGUAGE_VOL2		3600
#define FUELGUAGE_VOL3		3700
#define FUELGUAGE_VOL4		3800
#define FUELGUAGE_VOL5		3900
#define FUELGUAGE_VOL6		4000
#define FUELGUAGE_VOL7		4100
#define FUELGUAGE_TOP_VOL	4160	//>4.16v,100%

#define FUELGUAGE_LOW_LEVEL	2		//<3.4v,2%
#define FUELGUAGE_LEVEL1	3		//<3.5v,3%
#define FUELGUAGE_LEVEL2	5
#define FUELGUAGE_LEVEL3	16
#define FUELGUAGE_LEVEL4	46
#define FUELGUAGE_LEVEL5	66
#define FUELGUAGE_LEVEL6	83
#define FUELGUAGE_LEVEL7	93
#define FUELGUAGE_TOP_LEVEL	100     //>4.16v,100%

#define INTLDO4					2800000								//initial ldo4 voltage
#define INIT_RDC				200										//initial rdc
#define TIMER 					20										//axp19 renew capability time
#define BATTERYCAP      2600									// battery capability
#define RENEW_TIME      10										//axp20 renew capability time
#define INTCHGCUR				300000								//set initial charging current limite
#define SUSCHGCUR				1000000								//set suspend charging current limite
#define RESCHGCUR				INTCHGCUR							//set resume charging current limite
#define CLSCHGCUR				SUSCHGCUR							//set shutdown charging current limite
#define INTCHGVOL				4200000								//set initial charing target voltage
#define INTCHGENDRATE		10										//set initial charing end current	rate
#define INTCHGENABLED		1										  //set initial charing enabled
#define INTADCFREQ			25										//set initial adc frequency
#define INTADCFREQC			100										//set initial coulomb adc coufrequency
#define INTCHGPRETIME		50										//set initial pre-charging time
#define INTCHGCSTTIME		480										//set initial pre-charging time
#define BATMAXVOL				4200000								//set battery max design volatge
#define BATMINVOL				3500000								//set battery min design volatge

#define OCVREG0			    0x00									//3.0976
#define OCVREG1			    0x00									//3.2032
#define OCVREG2			    0x00									//3.3088
#define OCVREG3			    0x01									//3.4144
#define OCVREG4			    0x01									//3.4848
#define OCVREG5			    0x02									//3.5200
#define OCVREG6			    0x03									//3.5552
#define OCVREG7			    0x04									//3.5728
#define OCVREG8			    0x05									//3.5904
#define OCVREG9			    0x06									//3.6080
#define OCVREGA			    0x07									//3.6256
#define OCVREGB			    0x0A									//3.6432
#define OCVREGC			    0x0D									//3.6608
#define OCVREGD			    0x10									//3.6960
#define OCVREGE			    0x1A									//3.7312
#define OCVREGF			    0x24									//3.7664
#define OCVREG10			0x29									//3.7840
#define OCVREG11			0x2E									//3.8016
#define OCVREG12			0x32									//3.8192
#define OCVREG13			0x35									//3.8368
#define OCVREG14			0x39									//3.8544
#define OCVREG15			0x3D									//3.8720
#define OCVREG16			0x43									//3.9072
#define OCVREG17			0x49									//3.9424
#define OCVREG18			0x4D									//3.9776
#define OCVREG19			0x54								    //4.0128
#define OCVREG1A			0x58									//4.0656
#define OCVREG1B			0x5C									//4.0832
#define OCVREG1C			0x5E									//4.1008
#define OCVREG1D			0x60									//4.1184
#define OCVREG1E			0x62									//4.1360
#define OCVREG1F			0x63									//4.1536

extern int pmu_used;
extern int pmu_twi_id;
extern int pmu_irq_id;
extern int pmu_twi_addr;
extern int pmu_battery_rdc;
extern int pmu_battery_cap;
extern int pmu_init_chgcur;
extern int pmu_suspend_chgcur;
extern int pmu_resume_chgcur;
extern int pmu_shutdown_chgcur;
extern int pmu_init_chgvol;
extern int pmu_init_chgend_rate;
extern int pmu_init_chg_enabled;
extern int pmu_init_adc_freq;
extern int pmu_init_adc_freqc;
extern int pmu_init_chg_pretime;
extern int pmu_init_chg_csttime;

extern int pmu_bat_para1;
extern int pmu_bat_para2;
extern int pmu_bat_para3;
extern int pmu_bat_para4;
extern int pmu_bat_para5;
extern int pmu_bat_para6;
extern int pmu_bat_para7;
extern int pmu_bat_para8;
extern int pmu_bat_para9;
extern int pmu_bat_para10;
extern int pmu_bat_para11;
extern int pmu_bat_para12;
extern int pmu_bat_para13;
extern int pmu_bat_para14;
extern int pmu_bat_para15;
extern int pmu_bat_para16;
extern int pmu_bat_para17;
extern int pmu_bat_para18;
extern int pmu_bat_para19;
extern int pmu_bat_para20;
extern int pmu_bat_para21;
extern int pmu_bat_para22;
extern int pmu_bat_para23;
extern int pmu_bat_para24;
extern int pmu_bat_para25;
extern int pmu_bat_para26;
extern int pmu_bat_para27;
extern int pmu_bat_para28;
extern int pmu_bat_para29;
extern int pmu_bat_para30;
extern int pmu_bat_para31;
extern int pmu_bat_para32;


extern int pmu_usbvol_limit;
extern int pmu_usbvol;
extern int pmu_usbcur_limit;
extern int pmu_usbcur;

extern int pmu_pwroff_vol;
extern int pmu_pwron_vol;

extern int dcdc1_vol;
extern int dcdc2_vol;
extern int dcdc3_vol;
extern int dcdc4_vol;
extern int dcdc5_vol;

extern int ldo1_vol;
extern int ldo2_vol;
extern int ldo3_vol;
extern int ldo4_vol;
extern int ldo5_vol;
extern int ldo6_vol;
extern int ldo7_vol;
extern int ldo8_vol;
extern int ldo9_vol;
extern int ldo10_vol;
extern int ldo11_vol;
extern int ldo12_vol;
extern int ldoio0_vol;
extern int ldoio1_vol;

extern int pmu_pokoff_time;
extern int pmu_pokoff_en;
extern int pmu_poklong_time;
extern int pmu_pokon_time;
extern int pmu_pwrok_time;
extern int pmu_pwrnoe_time;
extern int pmu_intotp_en;


#endif
