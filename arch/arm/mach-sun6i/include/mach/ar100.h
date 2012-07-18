/*
 * arch/arm/mach-sun6i/include/mach/ar100.h
 *
 * Copyright 2012 (c) Allwinner.
 * sunny (sunny@allwinnertech.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#ifndef	__ASM_ARCH_A100_H
#define	__ASM_ARCH_A100_H


//the mode of ar100 dvfs
#define	AR100_DVFS_SYN	(1<<0)


//axp driver interfaces
#define AXP_TRANS_BYTE_MAX	(8)


typedef int (*ar100_cb_t)(void *arg);

//temp here, ?????
//the wakeup source of super-standby
#define	SUPER_STANDBY_WAKEUP_SRC_NMI	(1<<0)	//pmu wakeup source
#define	SUPER_STANDBY_WAKEUP_SRC_PIN	(1<<1)	//pin wakeup source
#define	SUPER_STANDBY_WAKEUP_SRC_CIR	(1<<2)	//cir wakeup source
#define	SUPER_STANDBY_WAKEUP_SRC_ALM0	(1<<3)	//alarm0 wakeup source
#define	SUPER_STANDBY_WAKEUP_SRC_ALM1	(1<<3)	//alarm1 wakeup source
typedef	struct super_standby_para
{
	unsigned long event;	//wakeup event types
	unsigned long time_off;	//the time of power-off
} super_standby_para_t;

typedef	struct normal_standby_para
{
	unsigned long event;	//wakeup event types
	unsigned long time_off;	//the time of power-off
} normal_standby_para_t;

/*
 * set target frequency.
 * freq:  target frequency to be set, based on HZ.
 * return: result, 0 - set frequency successed, !0 - set frequency failed;
 */
int ar100_dvfs_set_cpufreq(unsigned long freq, unsigned long mode);


/*
 * enter normal standby.
 * para:  parameter for enter normal standby.
 * return: result, 0 - normal standby successed, !0 - normal standby failed;
 */
int ar100_standby_normal(struct normal_standby_para *para);


/*
 * enter super standby.
 * para:  parameter for enter normal standby.
 * return: result, 0 - super standby successed, !0 - super standby failed;
 */
int ar100_standby_super(struct super_standby_para *para);

/*
 * query super-standby wakeup source.
 * para:  point of buffer to store wakeup event informations.
 * return: result, 0 - query successed, !0 - query failed;
 */
int ar100_query_wakeup_source(unsigned long *event);


/*
 * notify ar100 cpux restored.
 * para:  none.
 * return: result, 0 - notify successed, !0 - notify failed;
 */
int ar100_cpux_ready_notify(void);


/*
 * read axp register data.
 * addr: point of registers address;
 * data: point of registers data;
 * len : number of read registers;
 * return: result, 0 - read register successed, !0 - read register failed;
 */
int ar100_axp_read_reg(unsigned char *addr, unsigned char *data, unsigned long len);


/*
 * write axp register data.
 * addr: point of registers address;
 * data: point of registers data;
 * len : number of write registers;
 * return: result, 0 - write register successed, !0 - write register failed;
 */
int ar100_axp_write_reg(unsigned char *addr, unsigned char *data, unsigned long len);


/*
 * axp get battery paramter.
 * para:  battery parameter;
 * return: result, 0 - get battery successed, !0 - get battery failed;
 */
int ar100_axp_get_battery(void *para);


/*
 * axp set battery paramter.
 * para:  battery parameter;
 * return: result, 0 - set battery successed, !0 - set battery failed;
 */
int ar100_axp_set_battery(void *para);

/*
 * axp power off.
 * para:  none;
 * return: result, 0 - power off successed, !0 - power off failed;
 */
int ar100_axp_power_off(void);

/*
 * register call-back function, call-back function is for ar100 notify some event to ac327,
 * axp interrupt for ex.
 * func:  call-back function;
 * para:  parameter for call-back function;
 * return: result, 0 - register call-back function successed;
 *                !0 - register call-back function failed;
 * NOTE: the function is like "int callback(void *para)";
 */
int ar100_cb_register(ar100_cb_t func, void *para);


/*
 * unregister call-back function.
 * func:  call-back function which need be unregister;
 */
void ar100_cb_unregister(ar100_cb_t func);


#endif	//__ASM_ARCH_A100_H
