/*
 * drivers/char/gpio_test/sun6i_gpio_test.c
 * (C) Copyright 2010-2015
 * Reuuimlla Technology Co., Ltd. <www.reuuimllatech.com>
 * liugang <liugang@reuuimllatech.com>
 *
 * sun6i gpio test driver
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include "sun6i_gpio_test.h"

#define TEST_REQUEST_FREE	/* test for gpio_request/gpio_free */
#define TEST_RE_REQUEST_FREE	/* test for re-gpio_request/re-gpio_free, so get warning */
#define TEST_GPIOLIB_API	/* test the standard linux gpio api */
#define TEST_CONFIG_API		/* test gpio multi-function */
#define TEST_GPIO_EINT_API	/* test gpio external interrupt */
#define TEST_GPIO_SCRIPT_API	/* test gpio script api */

/*
 * cur test case
 */
static enum gpio_test_case_e g_cur_test_case = GTC_API;

/**
 * gpio_chip_match - match function to check if gpio is in the gpio_chip
 * @chip:	gpio_chip to match
 * @data:	data to match
 *
 * Returns 1 if match, 0 if not match
 */
static inline int gpio_chip_match(struct gpio_chip * chip, void * data)
{
	u32 	num = 0;

	num = *(u32 *)data;
	if(num >= chip->base && num < chip->base + chip->ngpio) {
		return 1;
	}

	return 0;
}

/**
 * gpio_irq_handle_demo - gpio irq handle demo.
 *
 * Returns 0 if sucess, the err line number if failed.
 */
u32 gpio_irq_handle_demo(void *para)
{
	printk("%s: para 0x%08x\n", __FUNCTION__, (u32)para);
	return 0;
}

void __test_script_api(void)
{
	char main_key[256] = {0}, sub_key[256] = {0}, str_cmp[256] = {0};
	int gpio_cnt_cmp, gpio_cnt_get;
	script_item_u item_cmp, item_get, *list_get = NULL;
	script_item_value_type_e type_cmp, type_get;
	struct gpio_config card0_gpio[] = {
		{GPIOF(0)},
		{GPIOF(1)},
		{GPIOF(2)},
		{GPIOF(3)},
		{GPIOF(4)},
		{GPIOF(5)},
	};
	struct gpio_config csi0_gpio[] = {
		{GPIOE(0)},
		{GPIOE(1)},
		{GPIOE(2)},
		{GPIOE(3)},
		{GPIOE(4)},
		{GPIOE(5)},
		{GPIOE(6)},
		{GPIOE(7)},
		{GPIOE(8)},
		{GPIOE(9)},
		{GPIOE(10)},
		{GPIOE(11)},
		{GPIOH(13)},
		{GPIOH(16)},
		{GPIOH(18)},
	};

	/*
	[card0_boot_para]
	card_ctrl 		= 0
	card_high_speed 	= 1
	card_line       	= 4
	sdc_d1      		= port:PF0<2><1><default><default>
	sdc_d0      		= port:PF1<2><1><default><default>
	sdc_clk     		= port:PF2<2><1><default><default>
	sdc_cmd     		= port:PF3<2><1><default><default>
	sdc_d3      		= port:PF4<2><1><default><default>
	sdc_d2      		= port:PF5<2><1><default><default>

	[product]
	version = "100"
	machine = "evb_v12"

	[csi0_para]
	csi_used		= 1
	csi_twi_id		= 1
	csi_twi_addr		= 0x42
	csi_pck 		= port:PE00<3><default><default><default>
	csi_ck	 		= port:PE01<3><default><default><default>
	csi_hsync 		= port:PE02<3><default><default><default>
	csi_vsync		= port:PE03<3><default><default><default>
	csi_d0	 		= port:PE04<3><default><default><default>
	csi_d1	 		= port:PE05<3><default><default><default>
	csi_d2  		= port:PE06<3><default><default><default>
	csi_d3  		= port:PE07<3><default><default><default>
	csi_d4  		= port:PE08<3><default><default><default>
	csi_d5	 		= port:PE09<3><default><default><default>
	csi_d6  		= port:PE10<3><default><default><default>
	csi_d7  		= port:PE11<3><default><default><default>
	csi_reset  		= port:PH13<1><default><default><0>
	csi_power_en 		= port:PH16<1><default><default><0>
	csi_stby	 	= port:PH18<1><default><default><0>

	[lcd0_para]
	lcd_power                = port:power1<1><0><default><1>
	*/
	printk("%s, line %d\n", __func__, __LINE__);
	//script_dump_mainkey(NULL);

	/* test script api */
	strcpy(main_key, "card0_boot_para");
	script_dump_mainkey(main_key);

	/* test for type int */
	strcpy(sub_key, "card_ctrl");
	item_cmp.val = 0;
	type_cmp = SCIRPT_ITEM_VALUE_TYPE_INT;
	type_get = script_get_item(main_key, sub_key, &item_get);
	PIOTEST_DBG_FUN_LINE;
	WARN(type_get != type_cmp, "%s err, line %d, %s->%s type should be %d, but get %d\n",
		__func__, __LINE__, main_key, sub_key, type_cmp, type_get);
	PIOTEST_DBG_FUN_LINE;
	WARN(item_cmp.val != item_get.val, "%s err, line %d, %s->%s value should be %d, but get %d\n",
		__func__, __LINE__, main_key, sub_key, item_cmp.val, item_get.val);

	/* test for type gpio */
	strcpy(sub_key, "sdc_d3");
	type_cmp = SCIRPT_ITEM_VALUE_TYPE_PIO;
	type_get = script_get_item(main_key, sub_key, &item_get);
	PIOTEST_DBG_FUN_LINE;
	WARN(type_get != type_cmp, "%s err, line %d, %s->%s type should be %d, but get %d\n",
		__func__, __LINE__, main_key, sub_key, type_cmp, type_get);
	PIOTEST_DBG_FUN_LINE;
	sw_gpio_dump_config(&item_get.gpio, 1);

	/* test for gpio list */
	gpio_cnt_cmp = 6;
	PIOTEST_DBG_FUN_LINE;
	gpio_cnt_get = script_get_pio_list(main_key, &list_get);
	PIOTEST_DBG_FUN_LINE;
	WARN(gpio_cnt_get != gpio_cnt_cmp, "%s err, line %d, %s gpio cnt should be %d, but get %d\n",
		__func__, __LINE__, main_key, gpio_cnt_cmp, gpio_cnt_get);
	PIOTEST_DBG_FUN_LINE;
	sw_gpio_dump_config((struct gpio_config *)list_get, gpio_cnt_get);

	/* test for str */
	strcpy(main_key, "product");
	strcpy(sub_key, "machine");
	strcpy(str_cmp, "evb_v12");
	script_dump_mainkey(main_key);
	type_cmp = SCIRPT_ITEM_VALUE_TYPE_STR;
	type_get = script_get_item(main_key, sub_key, &item_get);
	PIOTEST_DBG_FUN_LINE;
	WARN(type_get != type_cmp, "%s err, line %d, %s->%s type should be %d, but get %d\n",
		__func__, __LINE__, main_key, sub_key, type_cmp, type_get);
	PIOTEST_DBG_FUN_LINE;
	WARN(strcmp(str_cmp, item_get.str), "%s err, line %d, %s->%s value should be %s, but get %s\n",
		__func__, __LINE__, main_key, sub_key, str_cmp, item_get.str);
	PIOTEST_DBG_FUN_LINE;

	/* test for csi0_para */
	strcpy(main_key, "csi0_para");
	script_dump_mainkey(main_key);
	/* test for int */
	strcpy(sub_key, "csi_twi_addr");
	item_cmp.val = 0x42;
	type_cmp = SCIRPT_ITEM_VALUE_TYPE_INT;
	type_get = script_get_item(main_key, sub_key, &item_get);
	PIOTEST_DBG_FUN_LINE;
	WARN(type_get != type_cmp, "%s err, line %d, %s->%s type should be %d, but get %d\n",
		__func__, __LINE__, main_key, sub_key, type_cmp, type_get);
	PIOTEST_DBG_FUN_LINE;
	WARN(item_cmp.val != item_get.val, "%s err, line %d, %s->%s value should be %d, but get %d\n",
		__func__, __LINE__, main_key, sub_key, item_cmp.val, item_get.val);
	PIOTEST_DBG_FUN_LINE;
	/* test for gpio list */
	gpio_cnt_cmp = 15;
	gpio_cnt_get = script_get_pio_list(main_key, &list_get);
	PIOTEST_DBG_FUN_LINE;
	WARN(gpio_cnt_get != gpio_cnt_cmp, "%s err, line %d, %s->%s gpio cnt should be %d, but get %d\n",
		__func__, __LINE__, main_key, sub_key, gpio_cnt_cmp, gpio_cnt_get);
	PIOTEST_DBG_FUN_LINE;
	sw_gpio_dump_config((struct gpio_config *)list_get, gpio_cnt_get);
	PIOTEST_DBG_FUN_LINE;

	/* test for gpio config api */
	strcpy(main_key, "card0_boot_para");
	gpio_cnt_cmp = 6;
	gpio_cnt_get = script_get_pio_list(main_key, &list_get);
	PIOTEST_DBG_FUN_LINE;
	WARN_ON(gpio_cnt_get != gpio_cnt_cmp);
	PIOTEST_DBG_FUN_LINE;
	WARN_ON(0 != sw_gpio_setall_range(&list_get[0].gpio, gpio_cnt_get));
	PIOTEST_DBG_FUN_LINE;
	WARN_ON(0 != sw_gpio_getall_range((struct gpio_config *)card0_gpio, gpio_cnt_get));
	PIOTEST_DBG_FUN_LINE;
	sw_gpio_dump_config((struct gpio_config *)card0_gpio, gpio_cnt_get);
	PIOTEST_DBG_FUN_LINE;

	/* test for gpio config api */
	strcpy(main_key, "csi0_para");
	//script_dump_mainkey(main_key);
	gpio_cnt_cmp = 15;
	gpio_cnt_get = script_get_pio_list(main_key, &list_get);
	PIOTEST_DBG_FUN_LINE;
	WARN_ON(gpio_cnt_get != gpio_cnt_cmp);
	PIOTEST_DBG_FUN_LINE;
	WARN_ON(0 != sw_gpio_setall_range(&list_get[0].gpio, gpio_cnt_get));
	PIOTEST_DBG_FUN_LINE;
	WARN_ON(0 != sw_gpio_getall_range((struct gpio_config *)csi0_gpio, gpio_cnt_get));
	PIOTEST_DBG_FUN_LINE;
	sw_gpio_dump_config((struct gpio_config *)csi0_gpio, gpio_cnt_get);

	/* test for axp pin */
	strcpy(main_key, "lcd0_para");
	script_dump_mainkey(main_key);
	strcpy(sub_key, "lcd_power");
	type_cmp = SCIRPT_ITEM_VALUE_TYPE_PIO;
	PIOTEST_DBG_FUN_LINE;
	type_get = script_get_item(main_key, sub_key, &item_get);
	WARN(type_get != type_cmp, "%s err, line %d, %s->%s type should be %d, but get %d\n",
		__func__, __LINE__, main_key, sub_key, type_cmp, type_get);
	PIOTEST_DBG_FUN_LINE;
	sw_gpio_dump_config((struct gpio_config *)&item_get.gpio, 1);
	PIOTEST_DBG_FUN_LINE;
	/* NOTE: axp gpio can only use standard linux gpio api */
	if(1 == item_get.gpio.mul_sel) {
		if(0 != gpio_direction_output(item_get.gpio.gpio, item_get.gpio.data))
			printk("%s err, set axp gpio output failed\n", __func__);
		else {
			printk("%s, set axp gpio output success!\n", __func__);
			if(item_get.gpio.data != __gpio_get_value(item_get.gpio.gpio))
				printk("%s err, get axp gpio value NOT match!\n", __func__);
			else
				printk("%s ok, get axp gpio value match!\n", __func__);
		}
	} else if(0 == item_get.gpio.mul_sel) {
		if(0 != gpio_direction_input(item_get.gpio.gpio))
			printk("%s err, set axp gpio input failed\n", __func__);
		else {
			int val = __gpio_get_value(item_get.gpio.gpio);
			printk("%s, set axp gpio input success! get value %d\n", __func__, val);
		}
	} else
		printk("%s err, line %d\n", __func__, __LINE__);
	PIOTEST_DBG_FUN_LINE;
}

u32 __test_request_free(void)
{
	u32 uret = 0;

	PIOTEST_DBG_FUN_LINE;
	PIO_CHECK_RST(0 == gpio_request(GPIOA(0), "pa0"), uret, end);
	PIO_CHECK_RST(0 == gpio_request(GPIOA(PA_NR / 2), "pa_mid"), uret, end);
	PIO_CHECK_RST(0 == gpio_request(GPIOA(PA_NR - 1), "pa_end_sub_1"), uret, end);
#if 0	/* request un-exist gpio, err */
	PIO_CHECK_RST(0 == gpio_request(GPIOA(PA_NR), "pa_end"), uret, end); /* err */
	PIO_CHECK_RST(0 == gpio_request(GPIOA(PA_NR + 1), "pa_end_plus_1"), uret, end); /* err */
#endif
	gpio_free(GPIOA(0));
	gpio_free(GPIOA(PA_NR / 2));
	gpio_free(GPIOA(PA_NR - 1));

	PIO_CHECK_RST(0 == gpio_request(GPIOB(0), "pb0"), uret, end);
	PIO_CHECK_RST(0 == gpio_request(GPIOB(PB_NR / 2), "pb_mid"), uret, end);
	PIO_CHECK_RST(0 == gpio_request(GPIOB(PB_NR - 1), "pb_end_sub_1"), uret, end);

	gpio_free(GPIOB(0));
	gpio_free(GPIOB(PB_NR / 2));
	gpio_free(GPIOB(PB_NR - 1));
end:
	if(0 != uret)
		printk("%s err, line %d\n", __FUNCTION__, uret);
	return uret;
}

u32 __test_re_request_free(void)
{
	u32 uret = 0;

	PIOTEST_DBG_FUN_LINE;
#if 0	/* free an un-requested pio, err */
	gpio_free(GPIOA(PA_NR / 2));
	PIOTEST_DBG_FUN_LINE;
#endif

	PIO_CHECK_RST(0 == gpio_request(GPIOA(PA_NR / 2), "pa_mid"), uret, end);
	PIOTEST_DBG_FUN_LINE;
	gpio_free(GPIOA(PA_NR / 2));
	PIOTEST_DBG_FUN_LINE;
#if 0	/* re-free, err */
	gpio_free(GPIOA(PA_NR / 2));
	PIOTEST_DBG_FUN_LINE;
#endif

	PIO_CHECK_RST(0 == gpio_request(GPIOB(PB_NR / 2), "pb_mid"), uret, end);
	PIOTEST_DBG_FUN_LINE;
#if 0	/* re-request, err */
	PIO_CHECK_RST(0 == gpio_request(GPIOB(PB_NR / 2), "pb_mid"), uret, end);
	PIOTEST_DBG_FUN_LINE;
#endif
	gpio_free(GPIOB(PB_NR / 2));
	PIOTEST_DBG_FUN_LINE;

end:
	if(0 != uret)
		printk("%s err, line %d\n", __FUNCTION__, uret);
	return uret;
}

u32 __test_standard_api(void)
{
	u32 	uret = 0;
	u32	utemp = 0;
	u32	upio_index = 0;
	u32	offset = 0;
	struct gpio_chip *pchip = NULL;
	struct gpio gpio_arry[] = {
		{GPIOA(0), GPIOF_OUT_INIT_HIGH, "pa0"},
		{GPIOB(3), GPIOF_IN, "pb3"},
		{GPIOC(5), GPIOF_OUT_INIT_LOW, "pc5"},
		{GPIOH(2), GPIOF_IN, "ph2"},
	};
	struct gpio_config gpio_cfg_temp[4] = {
		{GPIOA(0)},
		{GPIOB(3)},
		{GPIOC(5)},
		{GPIOH(2)},
	};

	PIOTEST_DBG_FUN_LINE;
	/* test gpio_request_one */
	PIO_CHECK_RST(0 == gpio_request_one(GPIOC(1), GPIOF_OUT_INIT_HIGH, "pc_1"), uret, end);
	PIOTEST_DBG_FUN_LINE;
	PIO_CHECK_RST(1 == __gpio_get_value(GPIOC(1)), uret, end); /* check if data ok */
	gpio_free(GPIOC(1));

	PIO_CHECK_RST(0 == gpio_request_one(GPIOC(1), GPIOF_OUT_INIT_LOW, "pc_1"), uret, end);
	PIOTEST_DBG_FUN_LINE;
	PIO_CHECK_RST(0 == __gpio_get_value(GPIOC(1)), uret, end); /* check if data ok */
	gpio_free(GPIOC(1));

	/* test gpio_request_array */
	PIO_CHECK_RST(0 == gpio_request_array(gpio_arry, ARRAY_SIZE(gpio_arry)), uret, end);
	PIOTEST_DBG_FUN_LINE;
	/* test gpio_free_array */
	gpio_free_array(gpio_arry, ARRAY_SIZE(gpio_arry));
	PIOTEST_DBG_FUN_LINE;
	/* check if request array success */
	PIO_CHECK_RST(0 == sw_gpio_getall_range(gpio_cfg_temp, ARRAY_SIZE(gpio_cfg_temp)), uret, end);
	PIOTEST_DBG_FUN_LINE;
	sw_gpio_dump_config(gpio_cfg_temp, ARRAY_SIZE(gpio_cfg_temp));
	PIOTEST_DBG_FUN_LINE;

	/* test gpiochip_find */
	offset = 5;
	utemp = GPIOB(offset);
	PIO_CHECK_RST(0 == gpio_request(utemp, "pb5"), uret, end);
	PIOTEST_DBG_FUN_LINE;
	PIO_CHECK_RST(NULL != (pchip = gpiochip_find(&utemp, gpio_chip_match)), uret, end);
	PIOTEST_DBG_FUN_LINE;
	printk("%s: gpiochip_find success, 0x%08x\n", __FUNCTION__, (u32)pchip);

	/* test gpiochip_is_requested */
	PIO_CHECK_RST(NULL != gpiochip_is_requested(pchip, offset), uret, end);
	PIOTEST_DBG_FUN_LINE;
	gpio_free(utemp);
	PIOTEST_DBG_FUN_LINE;

	/* test gpio_direction_input/__gpio_get_value/gpio_get_value_cansleep */
	upio_index = GPIOE(16);
	PIO_CHECK_RST(0 == gpio_request(upio_index, "pe_16"), uret, end);
	PIOTEST_DBG_FUN_LINE;
	PIO_CHECK_RST(0 == gpio_direction_input(upio_index), uret, end); /* warn autorequest */
	PIOTEST_DBG_FUN_LINE;
	utemp = __gpio_get_value(upio_index); /* __gpio_get_value */
	printk("%s: __gpio_get_value pe16 value %d\n", __FUNCTION__, utemp);
	utemp = (u32)gpio_get_value_cansleep(upio_index); /* gpio_get_value_cansleep, success even can_sleep flag not set */
	printk("%s: gpio_get_value_cansleep pe16 value %d\n", __FUNCTION__, utemp);
	gpio_free(upio_index);

	/* test gpio_direction_output/__gpio_get_value/__gpio_set_value/gpio_set_value_cansleep */
	upio_index = GPIOB(5);
	PIO_CHECK_RST(0 == gpio_request(upio_index, "pb5"), uret, end);
	PIOTEST_DBG_FUN_LINE;
	PIO_CHECK_RST(0 == gpio_direction_output(upio_index, 1), uret, end); /* gpio_direction_output */
	PIOTEST_DBG_FUN_LINE;
	PIO_CHECK_RST(1 == __gpio_get_value(upio_index), uret, end); /* __gpio_get_value */
	PIOTEST_DBG_FUN_LINE;
	PIO_CHECK_RST(0 == gpio_direction_output(upio_index, 0), uret, end);
	PIOTEST_DBG_FUN_LINE;
	PIO_CHECK_RST(0 == __gpio_get_value(upio_index), uret, end);
	PIOTEST_DBG_FUN_LINE;
	__gpio_set_value(upio_index, 1); /* __gpio_set_value */
	PIOTEST_DBG_FUN_LINE;
	PIO_CHECK_RST(1 == __gpio_get_value(upio_index), uret, end);
	PIOTEST_DBG_FUN_LINE;
	gpio_set_value_cansleep(upio_index, 0); /* gpio_set_value_cansleep */
	PIOTEST_DBG_FUN_LINE;
	/* test gpio_set_debounce, gpio_chip->set_debounce not impletment, so err here */
	utemp = gpio_set_debounce(upio_index, 10);
	printk("%s: gpio_set_debounce %d return %d\n", __FUNCTION__, upio_index, utemp);
	PIOTEST_DBG_FUN_LINE;
	/* test __gpio_cansleep */
	utemp = (u32)__gpio_cansleep(upio_index);
	printk("%s: __gpio_cansleep %d return %d\n", __FUNCTION__, upio_index, utemp);
	/* test __gpio_to_irq */
	utemp = (u32)__gpio_to_irq(upio_index);
	printk("%s: __gpio_to_irq %d return %d\n", __FUNCTION__, upio_index, utemp);
	gpio_free(upio_index);
	PIOTEST_DBG_FUN_LINE;

end:
	if(0 != uret)
		printk("%s err, line %d\n", __FUNCTION__, uret);
	return uret;
}

u32 __test_mul_fun_api(void)
{
	u32 	uret = 0;
	u32	upio_index = 0;
	struct gpio_config gpio_cfg[] = {
		/* use default if you donot care the pull or driver level status */
		{GPIOE(10), 3, GPIO_PULL_DEFAULT, GPIO_DRVLVL_DEFAULT, 0},
		{GPIOA(13), 2, 1, 2, -1},
		{GPIOD(2),  1, 2, 1, 1},
		{GPIOG(8),  0, 1, 1, 0},
	};

	/* test sw_gpio_getcfg with gpio_direction_output */
	upio_index = GPIOA(1);
	PIO_CHECK_RST(0 == gpio_request(upio_index, "pa1"), uret, end);
	PIOTEST_DBG_FUN_LINE;
	PIO_CHECK_RST(0 == gpio_direction_output(upio_index, 0), uret, end);
	PIOTEST_DBG_FUN_LINE;
	gpio_free(upio_index);
	PIOTEST_DBG_FUN_LINE;
	PIO_CHECK_RST(1 == sw_gpio_getcfg(upio_index), uret, end); /* sw_gpio_getcfg output */
	PIOTEST_DBG_FUN_LINE;

	/* test sw_gpio_getcfg with gpio_direction_input */
	PIO_CHECK_RST(0 == gpio_request(upio_index, "pa1"), uret, end);
	PIOTEST_DBG_FUN_LINE;
	PIO_CHECK_RST(0 == gpio_direction_input(upio_index), uret, end);
	gpio_free(upio_index);
	PIOTEST_DBG_FUN_LINE;
	PIO_CHECK_RST(0 == sw_gpio_getcfg(upio_index), uret, end); /* sw_gpio_getcfg input */
	PIOTEST_DBG_FUN_LINE;

	/* test sw_gpio_setcfg/sw_gpio_getcfg  */
	PIO_CHECK_RST(0 == sw_gpio_setcfg(upio_index, 3), uret, end); /* sw_gpio_setcfg(PA1->LCD1_D1) */
	PIOTEST_DBG_FUN_LINE;
	PIO_CHECK_RST(3 == sw_gpio_getcfg(upio_index), uret, end); /* sw_gpio_getcfg */
	PIOTEST_DBG_FUN_LINE;

	/* test sw_gpio_setpull/sw_gpio_getpull  */
	PIO_CHECK_RST(0 == sw_gpio_setpull(upio_index, 2), uret, end); /* sw_gpio_setpull down */
	PIOTEST_DBG_FUN_LINE;
	PIO_CHECK_RST(2 == sw_gpio_getpull(upio_index), uret, end); /* sw_gpio_getpull */
	PIOTEST_DBG_FUN_LINE;

	/* test sw_gpio_setdrvlevel/sw_gpio_getdrvlevel  */
	PIO_CHECK_RST(0 == sw_gpio_setdrvlevel(upio_index, 2), uret, end); /* sw_gpio_setdrvlevel level2 */
	PIOTEST_DBG_FUN_LINE;
	PIO_CHECK_RST(2 == sw_gpio_getdrvlevel(upio_index), uret, end); /* sw_gpio_getdrvlevel */
	PIOTEST_DBG_FUN_LINE;

	/* test sw_gpio_setall_range/sw_gpio_getall_range/sw_gpio_dump_config  */
	PIO_CHECK_RST(0 == sw_gpio_setall_range(gpio_cfg, ARRAY_SIZE(gpio_cfg)), uret, end); /* sw_gpio_setall_range */
	PIOTEST_DBG_FUN_LINE;
	PIO_CHECK_RST(0 == sw_gpio_getall_range(gpio_cfg, ARRAY_SIZE(gpio_cfg)), uret, end); /* sw_gpio_getall_range */
	PIOTEST_DBG_FUN_LINE;
	sw_gpio_dump_config(gpio_cfg, ARRAY_SIZE(gpio_cfg)); /* sw_gpio_dump_config */

end:
	if(0 != uret)
		printk("%s err, line %d\n", __FUNCTION__, uret);
	return uret;
}

u32 __test_eint_api(void)
{
	u32 	uret = 0;
	u32	utemp = 0;
	struct gpio_config_eint_all cfg_eint[] = {
		{GPIOA(0) , GPIO_PULL_DEFAULT, GPIO_DRVLVL_DEFAULT, true , 0, TRIG_EDGE_POSITIVE},
		{GPIOA(26), 1                , 2                  , true , 0, TRIG_EDGE_DOUBLE  },
		{GPIOB(3) , GPIO_PULL_DEFAULT, 1                  , false, 0, TRIG_EDGE_NEGATIVE},
		{GPIOE(14), 3                , 0                  , true , 0, TRIG_LEVL_LOW     },

		/* __para_check err in sw_gpio_eint_setall_range, because rpl3 cannot be eint */
		//{GPIOL(3) , 1                , GPIO_DRVLVL_DEFAULT, false, 0, TRIG_LEVL_HIGH  },

		{GPIOL(8) , 2                , GPIO_DRVLVL_DEFAULT, false, 0, TRIG_LEVL_HIGH    },
		{GPIOM(4) , 1                , 2                  , true,  0, TRIG_LEVL_LOW     },
	};
	struct gpio_config_eint_all cfg_eint_temp[] = {
		{GPIOA(0) },
		{GPIOA(26)},
		{GPIOB(3) },
		{GPIOE(14)},
		{GPIOL(8) },
		{GPIOM(4) },
	};
	u32 	handle_temp[5] = {0};

	/* test sw_gpio_eint_setall_range/sw_gpio_eint_getall_range api */
	sw_gpio_eint_dumpall_range(cfg_eint, ARRAY_SIZE(cfg_eint)); /* dump the orginal struct */
	PIOTEST_DBG_FUN_LINE;
	PIO_CHECK_RST(0 == sw_gpio_eint_setall_range(cfg_eint, ARRAY_SIZE(cfg_eint)), uret, end); /* sw_gpio_eint_setall_range */
	PIOTEST_DBG_FUN_LINE;
	PIO_CHECK_RST(0 == sw_gpio_eint_getall_range(cfg_eint_temp, ARRAY_SIZE(cfg_eint_temp)), uret, end); /* sw_gpio_eint_getall_range */
	PIOTEST_DBG_FUN_LINE;
	sw_gpio_eint_dumpall_range(cfg_eint_temp, ARRAY_SIZE(cfg_eint_temp)); /* dump the struct get from hw */
	PIOTEST_DBG_FUN_LINE;

	/* test sw_gpio_irq_request/sw_gpio_irq_free api */
	utemp = GPIOA(0);
	handle_temp[0] = sw_gpio_irq_request(utemp, TRIG_EDGE_POSITIVE,
		(peint_handle)gpio_irq_handle_demo, (void *)utemp);
	printk("%s: handle_temp[0] 0x%08x\n", __FUNCTION__, handle_temp[0]);

	utemp = GPIOA(1);
	handle_temp[1] = sw_gpio_irq_request(utemp, TRIG_EDGE_NEGATIVE,
		(peint_handle)gpio_irq_handle_demo, (void *)utemp);
	printk("%s: handle_temp[1] 0x%08x\n", __FUNCTION__, handle_temp[1]);

	utemp = GPIOC(0);
	handle_temp[2] = sw_gpio_irq_request(utemp, TRIG_EDGE_DOUBLE,
		(peint_handle)gpio_irq_handle_demo, (void *)utemp);
	printk("%s: handle_temp[2] 0x%08x\n", __FUNCTION__, handle_temp[2]);

	utemp = GPIOE(2);
	handle_temp[3] = sw_gpio_irq_request(utemp, TRIG_LEVL_LOW,
		(peint_handle)gpio_irq_handle_demo, (void *)utemp);
	printk("%s: handle_temp[3] 0x%08x\n", __FUNCTION__, handle_temp[3]);

	if(0 != handle_temp[0])
		sw_gpio_irq_free(handle_temp[0]);
	PIOTEST_DBG_FUN_LINE;
	if(0 != handle_temp[1])
		sw_gpio_irq_free(handle_temp[1]);
	PIOTEST_DBG_FUN_LINE;
	if(0 != handle_temp[2])
		sw_gpio_irq_free(handle_temp[2]);
	PIOTEST_DBG_FUN_LINE;
	if(0 != handle_temp[3])
		sw_gpio_irq_free(handle_temp[3]);
	PIOTEST_DBG_FUN_LINE;
	//gpio_request(GPIOA(0), NULL);
	//sw_gpio_irq_free(handle_temp[0]); /* WARNING: Trying to free already-free IRQ 43 */
	//gpio_free(GPIOA(0));

end:
	if(0 != uret)
		printk("%s err, line %d\n", __FUNCTION__, uret);
	return uret;
}

/**
 * __gtc_api - gpio test case, for api
 *
 * Returns 0 if sucess, the err line number if failed.
 */
u32 __gtc_api(void)
{
	u32 	uret = 0;

	/* wait for request and free */
	msleep(4000);

	/*
	 * test for request and free
	 */
#ifdef TEST_REQUEST_FREE
	if(0 != __test_request_free())
		printk("%s: __test_request_free failed\n", __func__);
	else
		printk("%s: __test_request_free success\n", __func__);
#endif /* TEST_REQUEST_FREE */

	/*
	 * test for re-request and re-free the same pio
	 */
#ifdef TEST_RE_REQUEST_FREE
	if(0 != __test_re_request_free())
		printk("%s: __test_re_request_free failed\n", __func__);
	else
		printk("%s: __test_re_request_free success\n", __func__);
#endif /* TEST_RE_REQUEST_FREE */

	/*
	 * test for gpiolib api
	 */
#ifdef TEST_GPIOLIB_API
	if(0 != __test_standard_api())
		printk("%s: __test_standard_api failed\n", __func__);
	else
		printk("%s: __test_standard_api success\n", __func__);
#endif /* TEST_GPIOLIB_API */

	/*
	 * test for cfg api
	 */
#ifdef TEST_CONFIG_API
	if(0 != __test_mul_fun_api())
		printk("%s: __test_mul_fun_api failed\n", __func__);
	else
		printk("%s: __test_mul_fun_api success\n", __func__);
#endif /* TEST_CONFIG_API */

	/*
	 * test for gpio external interrupt api
	 */
#ifdef TEST_GPIO_EINT_API
	if(0 != __test_eint_api())
		printk("%s: __test_eint_api failed\n", __func__);
	else
		printk("%s: __test_eint_api success\n", __func__);
#endif /* TEST_GPIO_EINT_API */

	/*
	 * test for script gpio api
	 */
#ifdef TEST_GPIO_SCRIPT_API
	__test_script_api();
#endif /* TEST_GPIO_SCRIPT_API */

end:
	if(0 == uret)
		printk("%s: test success!\n", __FUNCTION__);
	else
		printk("%s: test failed! line %d\n", __FUNCTION__, uret);
	return uret;
}

/**
 * __gpio_test_thread - gpio test main thread
 * @arg:	thread arg, not used
 *
 * Returns 0 if success, the err line number if failed.
 */
static int __gpio_test_thread(void * arg)
{
	u32 	uResult = 0;

	switch(g_cur_test_case) {
	case GTC_API:
		uResult = __gtc_api();
		break;
	default:
		uResult = __LINE__;
		break;
	}

	if(0 == uResult)
		printk("%s: test success!\n", __FUNCTION__);
	else
		printk("%s: test failed!\n", __FUNCTION__);

	return uResult;
}

/**
 * sw_gpio_test_init - enter the gpio test module
 */
static int __init sw_gpio_test_init(void)
{
	printk("%s enter\n", __FUNCTION__);

	/*
	 * create the test thread
	 */
	kernel_thread(__gpio_test_thread, NULL, CLONE_FS | CLONE_SIGHAND);
	return 0;
}

/**
 * sw_gpio_test_exit - exit the gpio test module
 */
static void __exit sw_gpio_test_exit(void)
{
	printk("sw_gpio_test_exit: enter\n");
}

#ifdef MODULE
module_init(sw_gpio_test_init);
module_exit(sw_gpio_test_exit);
MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("liugang");
MODULE_DESCRIPTION ("sun6i gpio Test driver code");
#else
__initcall(sw_gpio_test_init);
#endif /* MODULE */
