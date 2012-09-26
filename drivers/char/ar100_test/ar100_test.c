 /*
 * drivers/char/ar100_test/ar100_test.c
 * (C) Copyright 2010-2015
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * sunny <sunny@allwinnertech.com>
 *
 * sun6i ar100 test driver
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include "ar100_test.h"

void __ar100_dvfs_test(void)
{
	unsigned int i;
	unsigned int freq_table[] = {
		120000000, 
		240000000, 
		300000000, 
		360000000, 
		300000000, 
		240000000, 
		120000000
	};
	for (i = 0; i < sizeof(freq_table) / sizeof(unsigned int); i++) {
		printk("dvfs request freq: %d\n", freq_table[i]);
		ar100_dvfs_set_cpufreq(freq_table[i], AR100_DVFS_SYN, 0);
	}
	/* test succeeded */
	printk("dvfs test succeeded\n");
}

#define AXP_TEST_BASE_REG_ADDR	(0x15)

static int __ar100_axp_cb(void *arg)
{
	printk("ar100 axp interrupt coming\n");
	return 0;
}


static void __ar100_axp_test(void)
{
	unsigned char addr_table[AXP_TRANS_BYTE_MAX];
	unsigned char data_table[AXP_TRANS_BYTE_MAX];
	unsigned int  len;
	int           ret;
	int           i;
	
	/* test write regs */
	printk("test axp write regs begin...\n");
	len = AXP_TRANS_BYTE_MAX;
	for (i = 0; i < AXP_TRANS_BYTE_MAX; i++) {
		addr_table[i] = AXP_TEST_BASE_REG_ADDR + i;
		data_table[i] = 0x8;
	}
	for (len = 1; len <= AXP_TRANS_BYTE_MAX; len++) {
		printk("write axp regs data:\n");
		for (i = 0; i < len; i++) {
			printk("addr%x : %x\n", (unsigned int)addr_table[i], 
									(unsigned int)data_table[i]);
		}
		ret = ar100_axp_write_reg(addr_table, data_table, len);
		if (ret) {
			printk("test axp write failed, len = %d, ret = %d\n", len, ret);
		}
		printk("write axp regs data [len = %d] succeeded\n", len);
	}
	printk("test axp write regs succeeded\n");
	
	/* test read regs */
	printk("test axp read regs begin...\n");
	len = AXP_TRANS_BYTE_MAX;
	for (i = 0; i < AXP_TRANS_BYTE_MAX; i++) {
		addr_table[i] = AXP_TEST_BASE_REG_ADDR + i;
	}
	for (len = 1; len <= AXP_TRANS_BYTE_MAX; len++) {
		ret = ar100_axp_read_reg(addr_table, data_table, len);
		if (ret) {
			printk("test axp read failed, len = %d, ret = %d\n", len, ret);
		}
		printk("read axp regs data:\n");
		for (i = 0; i < len; i++) {
			printk("addr%x : %x\n", (unsigned int)addr_table[i], 
									(unsigned int)data_table[i]);
		}
		printk("read axp regs data [len = %d] succeeded\n", len);
	}
	printk("test axp read regs succeeded\n");
	
	/* test axp set battery */
	printk("test axp set battery begin...\n");
	if(ar100_axp_set_battery(NULL)) {
		printk("test axp set battery failed\n");
	}
	printk("test axp set battery succeeded\n");
	
	/* test axp get battery */
	printk("test axp get battery begin...\n");
	if(ar100_axp_get_battery(NULL)) {
		printk("test axp get battery failed\n");
	}
	printk("test axp get battery succeeded\n");
	
	/* test axp interrupt call-back */
	printk("test axp call-back begin...\n");
	if(ar100_axp_cb_register(__ar100_axp_cb, NULL)) {
		printk("test axp reg cb failed\n");
	}
	printk("test axp call-back succeeded\n");
	
	/* test power off */
	printk("test axp power off....\n");
	ar100_axp_power_off();
	//if runing on SOC-CHIP, the system die already.
	
	printk("axp test succeeded\n");
}

/**
 * __ar100_test_thread - dma test main thread
 * @arg:	thread arg, not used
 *
 * Returns 0 if success, the err line number if failed.
 */
static int __ar100_test_thread(void * arg)
{
	printk("ar100 test thread...\n");
	
	printk("dvfs test begin....\n");
	__ar100_dvfs_test();
	printk("dvfs test end....\n");
	
	printk("axp test begin....\n");
	__ar100_axp_test();
	printk("axp test end....\n");
	
	
	return 0;
}

/**
 * sw_ar100_test_init - enter the dma test module
 */
static int __init sw_ar100_test_init(void)
{
	printk(">>>>ar100 test driver init enter<<<<<\n");
	
	/*
	 * create the test thread
	 */
	kernel_thread(__ar100_test_thread, NULL, CLONE_FS | CLONE_SIGHAND);
	
	printk("ar100 test driver test finished\n");
	return 0;
}
late_initcall(sw_ar100_test_init);

/**
 * sw_ar100_test_exit - exit the dma test module
 */
static void __exit sw_ar100_test_exit(void)
{
	printk("sw_ar100_test_exit: enter\n");
}

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("sunny");
MODULE_DESCRIPTION ("sun6i ar100 test driver code");
