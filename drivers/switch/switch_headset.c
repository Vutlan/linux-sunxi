/*
 *  drivers/switch/switch_gpio.c
 *
 * Copyright (C) 2008 Google, Inc.
 * Author: Mike Lockwood <lockwood@android.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
*/

#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/switch.h>
#include <linux/irq.h>
#include <linux/input.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <asm/irq.h>
#include <asm/io.h>

#include <linux/switch.h>
#include <mach/sys_config.h>
#include <mach/system.h>

#undef SWITCH_DBG
#if (0)
    #define SWITCH_DBG(format,args...)  printk("[SWITCH] "format,##args)
#else
    #define SWITCH_DBG(...)
#endif

/*---------------------------------------------------------------------------*/

#define TP_CTRL0 			(0x0)
#define TP_CTRL1 			(0x4)
#define TP_INT_FIFO_CTR		(0x10)
#define TP_INT_FIFO_STATUS	(0x14)
#define TP_DATA				(0x24)
#define TP_CTRL_CLK_PARA  	(0x00a6002f)

#define FUNCTION_NAME "h2w"


/* 耳机状态 */
#define HEADSET_NULL        0
#define HEADSET_UNKOWN      -1
#define HEADSET_PLUGOUT     HEADSET_NULL
#define HEADSET_PLUGIN_4    1
#define HEADSET_PLUGIN_3    2

/* 按键定义 */
#define KEY_VOLUME_UP			115
#define KEY_VOLUME_DOWN         114
#define KEY_MEDIA_PLAY_PAUSE    164
#define KEY_MEDIA_NEXT          163
#define KEY_MEDIA_PREVIOUS      165
#define KEY_HEADSETHOOK         226

/* 延时 */
#define TIMER_START_TIME        10      //启动延时, 为了去抖动, 单位s
#define PLUGIN_CIRCLE_TIME      200     //检测耳机插入的间隔, 单位ms
#define KEY_CIRCLE_TIME         20      //检测Hook键按下的间隔, 单位ms
#define PLUGIN_DEBOUNCE_TIME    600     //耳机插入去抖动, 单位ms
#define KEY_DEBOUNCE_TIME       200     //hook去抖动时间, 单位ms
#define KEY_LONG_TIME           1000    //长按键检测时间间隔, 单位ms

/*---------------------------------------------------------------------------*/

struct gpio_switch_data {
	struct switch_dev sdev;

    u32 earphone_valid;
    user_gpio_set_t earphone_set;
	int earphone_hdle;

	int state;
	int pre_state;

	struct work_struct work;
	struct timer_list timer;

	struct input_dev *key;
    spinlock_t lock;

    atomic_t count_state;
    atomic_t hook_down;
    atomic_t hook_down_long;
    atomic_t plugin_delay;
    u32 time_circle;
};

static int gpio_earphone_switch = 0;
static void __iomem *tpadc_base = NULL;

static int switch_used = 0;

/*---------------------------------------------------------------------------*/

static void modify_time_circle(struct gpio_switch_data *switch_data, u32 time)
{
	unsigned long flags = 0;

    spin_lock_irqsave(&switch_data->lock, flags);
    switch_data->time_circle = time;
    spin_unlock_irqrestore(&switch_data->lock, flags);

    return;
}

/* 除不尽,就加1 */
static u32 division_verge(u32 dividend, u32 divisor)
{
    u32 temp = 0;

    if(!divisor){
        return 0;
    }

    temp = dividend / divisor;
    if(dividend % divisor){
        temp++;
    }

    return temp;
}

static u32 plugin_debounce_circle(u32 time)
{
    return division_verge(PLUGIN_DEBOUNCE_TIME, time);
}

static u32 is_change(struct gpio_switch_data *switch_data)
{
    u32 temp = 0;

    temp = readl(tpadc_base + TP_INT_FIFO_STATUS);
	temp &= (1<<16);

    return temp;
}

static void headset_earphone(struct gpio_switch_data *switch_data, u32 on)
{
    u32 on_off = 0;

    if(!switch_data->earphone_hdle){
        return;
    }

    SWITCH_DBG("headset earphone is %s\n", (on ? "on" : "off"));

    if(switch_data->earphone_set.data == 0){
        on_off = on ? 1 : 0;
    }else{
        on_off = on ? 0 : 1;
    }

    gpio_write_one_pin_value(switch_data->earphone_hdle, on_off, "audio_earphone_ctrl");
}

static void clear_change_status(struct gpio_switch_data *switch_data)
{
    u32 temp = 0;
    unsigned long flags = 0;

    spin_lock_irqsave(&switch_data->lock, flags);
    temp = readl(tpadc_base + TP_INT_FIFO_STATUS);
    temp |= (1<<16);
    writel(temp, tpadc_base + TP_INT_FIFO_STATUS);
    spin_unlock_irqrestore(&switch_data->lock, flags);

    return;
}

static u32 get_ave_count(struct gpio_switch_data *switch_data)
{
	u32 fifo_val[4];
	u32 ave_count = 0;
    unsigned long flags = 0;

    spin_lock_irqsave(&switch_data->lock, flags);
	fifo_val[0] = readl(tpadc_base + TP_DATA);
	fifo_val[1] = readl(tpadc_base + TP_DATA);
	fifo_val[2] = readl(tpadc_base + TP_DATA);
	fifo_val[3] = readl(tpadc_base + TP_DATA);

    ave_count = (fifo_val[0] + fifo_val[1] + fifo_val[2] + fifo_val[3])/4;
    spin_unlock_irqrestore(&switch_data->lock, flags);

    return ave_count;
}

static int do_headset_null(struct gpio_switch_data	*switch_data)
{
	u32 ave_count = get_ave_count(switch_data);

	SWITCH_DBG("do_headset_null: ave_count = %d\n", ave_count);

    /*如果x2线端采样值大于2900，代表耳机拔出*/
    if (ave_count > 2900) {
//        SWITCH_DBG("do_headset_null: null\n");

        atomic_set(&switch_data->plugin_delay, 0);
        switch_data->state = HEADSET_NULL;
        modify_time_circle(switch_data, PLUGIN_CIRCLE_TIME);
    /*如果x2线端采样值在0~500之间，代表3段耳机插入*/
    } else if (ave_count < 500) {
        /* 去抖动 */
        atomic_inc(&switch_data->plugin_delay);
        if(atomic_read(&switch_data->plugin_delay) < plugin_debounce_circle(switch_data->time_circle)){
			modify_time_circle(switch_data, PLUGIN_CIRCLE_TIME);
            goto end;
        }
        atomic_set(&switch_data->plugin_delay, 0);

        SWITCH_DBG("do_headset_null: plugin_3\n");

        switch_data->state = HEADSET_PLUGIN_3;
        modify_time_circle(switch_data, PLUGIN_CIRCLE_TIME);

        headset_earphone(switch_data, 0);
    /*如果x2线端采样值大于600,小于2600,代表4段耳机插入*/
    } else if (ave_count >= 600 && ave_count < 2600) {
        /* 去抖动 */
        atomic_inc(&switch_data->plugin_delay);
        if(atomic_read(&switch_data->plugin_delay) < plugin_debounce_circle(switch_data->time_circle)){
			modify_time_circle(switch_data, PLUGIN_CIRCLE_TIME);
            goto end;
        }
        atomic_set(&switch_data->plugin_delay, 0);

        SWITCH_DBG("do_headset_null: plugin_4\n");

        switch_data->state = HEADSET_PLUGIN_4;
        modify_time_circle(switch_data, PLUGIN_CIRCLE_TIME);

        headset_earphone(switch_data, 1);
    }

end:
    return switch_data->state;
}

static int do_headset_plugin_3(struct gpio_switch_data	*switch_data)
{
	u32 ave_count = get_ave_count(switch_data);

    modify_time_circle(switch_data, PLUGIN_CIRCLE_TIME);

    if (ave_count > 2900) {
        /*如果x2线端采样值大于2900，代表耳机拔出*/
        SWITCH_DBG("do_headset_plugin_3: plugout\n");

        switch_data->state = HEADSET_NULL;
    }

    return switch_data->state;
}

#if 0

static u32 long_key_circle(u32 time)
{
    return division_verge(KEY_LONG_TIME, time);
}

static int do_headset_plugin_4(struct gpio_switch_data	*switch_data)
{
	u32 ave_count = get_ave_count(switch_data);

    if (ave_count > 2900) {
        /*如果x2线端采样值大于2900，代表耳机拔出*/
        SWITCH_DBG("do_headset_plugin_4: plugout\n");

        switch_data->state = HEADSET_NULL;
        modify_time_circle(switch_data, PLUGIN_CIRCLE_TIME);
        headset_earphone(switch_data, 0);

        goto end;
    }else if(ave_count <= 410){  //Hook down
//        SWITCH_DBG("\n\ndo_headset_plugin_4: hook down %d\n\n", ave_count);

        /* 按键抖动时间大约为 150ms，因此每隔200ms发送一次按键消息 */
        atomic_inc(&switch_data->count_state);
        if(atomic_read(&switch_data->count_state) > long_key_circle(switch_data->time_circle)){
            SWITCH_DBG("Hook long: %d, %d\n",
                       atomic_read(&switch_data->count_state),
                       long_key_circle(switch_data->time_circle));

            input_report_key(switch_data->key, KEY_MEDIA_PREVIOUS, 1);
            input_sync(switch_data->key);
            input_report_key(switch_data->key, KEY_MEDIA_PREVIOUS, 0);
            input_sync(switch_data->key);

            atomic_set(&switch_data->hook_down_long, 1);
            atomic_set(&switch_data->count_state, 0);
        }
        atomic_set(&switch_data->hook_down, 1);
    }else if(ave_count > 410 && ave_count <= 2900){ //Hook up
//        SWITCH_DBG("\n\ndo_headset_plugin_4: hook up %d\n\n", ave_count);

        /* 如果有长按键, 那么就忽略抬起键 */
        if(atomic_read(&switch_data->hook_down_long)){
            atomic_set(&switch_data->hook_down_long, 0);
            atomic_set(&switch_data->hook_down, 0);
            atomic_set(&switch_data->count_state, 0);

            goto end;
        }

        /* 在抬起时，一起发送按下和抬起 */
        if(atomic_read(&switch_data->hook_down)){
            SWITCH_DBG("send Hook up & down, %d\n", atomic_read(&switch_data->count_state));

            atomic_set(&switch_data->hook_down, 0);
            atomic_set(&switch_data->count_state, 0);

            input_report_key(switch_data->key, KEY_HEADSETHOOK, 1);
            input_sync(switch_data->key);
            input_report_key(switch_data->key, KEY_HEADSETHOOK, 0);
            input_sync(switch_data->key);
        }
    }else{
        SWITCH_DBG("do_headset_plugin_4: unkown ave_count=%d\n", ave_count);
    }

	modify_time_circle(switch_data, KEY_CIRCLE_TIME);

end:
    return switch_data->state;
}

#else

static u32 key_debounce_circle(u32 time)
{
    return division_verge(KEY_DEBOUNCE_TIME, time);
}

static int do_headset_plugin_4(struct gpio_switch_data	*switch_data)
{
	u32 ave_count = get_ave_count(switch_data);

    if (ave_count > 2900) {
        /*如果x2线端采样值大于2900，代表耳机拔出*/
        SWITCH_DBG("do_headset_plugin_4: plugout\n");

        switch_data->state = HEADSET_NULL;
        modify_time_circle(switch_data, PLUGIN_CIRCLE_TIME);
        headset_earphone(switch_data, 0);

        goto end;
    }else if(ave_count <= 410){  //Hook down
//        SWITCH_DBG("\n\ndo_headset_plugin_4: hook down %d\n\n", ave_count);

        /* 按键抖动时间大约为 150ms，因此每隔200ms发送一次按键消息 */
        atomic_inc(&switch_data->count_state);
        if(atomic_read(&switch_data->count_state) > key_debounce_circle(switch_data->time_circle)){
            /* 只发送一次down */
            if(!atomic_read(&switch_data->hook_down_long)){
                SWITCH_DBG("Hook up: %d, %d\n",
                           atomic_read(&switch_data->count_state),
                           key_debounce_circle(switch_data->time_circle));

                input_report_key(switch_data->key, KEY_HEADSETHOOK, 1);
                input_sync(switch_data->key);

                atomic_set(&switch_data->hook_down_long, 1);
            }
        }

        atomic_set(&switch_data->hook_down, 1);
    }else if(ave_count > 410 && ave_count <= 2900){ //Hook up
//        SWITCH_DBG("\n\ndo_headset_plugin_4: hook up %d\n\n", ave_count);

        if(atomic_read(&switch_data->hook_down)){
            /* 如果debouce期间，都未发送按下消息, 则发送按下消息 */
            if(atomic_read(&switch_data->count_state) <= key_debounce_circle(switch_data->time_circle)){
                SWITCH_DBG("send Hook up & down, %d\n", atomic_read(&switch_data->count_state));
                input_report_key(switch_data->key, KEY_HEADSETHOOK, 1);
                input_sync(switch_data->key);
            }else{
                SWITCH_DBG("send Hook down, %d\n", atomic_read(&switch_data->count_state));
            }

            input_report_key(switch_data->key, KEY_HEADSETHOOK, 0);
            input_sync(switch_data->key);

            atomic_set(&switch_data->hook_down, 0);
            atomic_set(&switch_data->count_state, 0);
            atomic_set(&switch_data->hook_down_long, 0);
        }
    }else{
        SWITCH_DBG("do_headset_plugin_4: unkown ave_count=%d\n", ave_count);
    }

	modify_time_circle(switch_data, KEY_CIRCLE_TIME);

end:
    return switch_data->state;
}

#endif

static void earphone_handle(unsigned long data)
{
	struct gpio_switch_data	*switch_data = (struct gpio_switch_data *)data;

    if(is_change(switch_data)){
        switch(switch_data->state){
            case HEADSET_NULL:
            //case HEADSET_PLUGOUT:
                switch_data->state = do_headset_null(switch_data);
            break;

            case HEADSET_PLUGIN_3:
                switch_data->state = do_headset_plugin_3(switch_data);
            break;

            case HEADSET_PLUGIN_4:
                switch_data->state = do_headset_plugin_4(switch_data);
            break;

            default:
                switch_data->state = HEADSET_NULL;
                printk("err: earphone_hook_handle, unkown state(%d)\n", switch_data->state);
        }

        clear_change_status(switch_data);

        if (switch_data->pre_state != switch_data->state) {
            SWITCH_DBG("sentmsg: pre_state=%d, state=%d\n", switch_data->pre_state, switch_data->state);

            switch_data->pre_state = switch_data->state;
            switch_set_state(&switch_data->sdev, switch_data->state);
        }
    }

	mod_timer(&switch_data->timer, jiffies + msecs_to_jiffies(switch_data->time_circle));

	return;
}

static ssize_t switch_gpio_print_state(struct switch_dev *sdev, char *buf)
{
	struct gpio_switch_data	*switch_data =
		container_of(sdev, struct gpio_switch_data, sdev);

	return sprintf(buf, "%d\n", switch_data->state);
}

static ssize_t print_headset_name(struct switch_dev *sdev, char *buf)
{
	struct gpio_switch_data	*switch_data =
		container_of(sdev, struct gpio_switch_data, sdev);

	return sprintf(buf, "%s\n", switch_data->sdev.name);
}

static int gpio_switch_probe(struct platform_device *pdev)
{
	struct gpio_switch_platform_data *pdata = pdev->dev.platform_data;
	struct gpio_switch_data *switch_data;
	int ret = 0;
	int temp;
    unsigned long flags = 0;

	if (!pdata) {
	    printk("err: gpio_switch_probe, pdata is null\n");
		return -EBUSY;
	}

	switch_data = kzalloc(sizeof(struct gpio_switch_data), GFP_KERNEL);
	if (!switch_data) {
	    printk("err: gpio_switch_probe, kzalloc failed\n");
		return -ENOMEM;
	}

	memset(switch_data, 0, sizeof(struct gpio_switch_data));
	atomic_set(&switch_data->count_state, 0);
	atomic_set(&switch_data->hook_down, 0);
	atomic_set(&switch_data->hook_down_long, 0);
	atomic_set(&switch_data->plugin_delay, 0);
	switch_data->time_circle = PLUGIN_CIRCLE_TIME;

    /* earphone */
    ret = script_parser_fetch("audio_para", "audio_earphone_ctrl", (int *)&switch_data->earphone_set, 64);
    if(ret != 0){
        printk("err: get audio_earphone_ctrl failed\n");
        goto err_gpio_request;
    }

    if(switch_data->earphone_set.port){
	    switch_data->earphone_valid = 1;
	}else{
	    printk("wrning: audio_earphone_ctrl is invalid\n");
	    switch_data->earphone_valid = 0;
	}

    if(switch_data->earphone_valid){
        gpio_earphone_switch = gpio_request_ex("audio_para", "audio_earphone_ctrl");
        if(!gpio_earphone_switch) {
            printk("earphone request gpio fail!\n");
            ret = -1;
            goto err_gpio_request;
        }

        headset_earphone(switch_data, 0);
    }

    /* initial */
    spin_lock_init (&switch_data->lock);
    spin_lock_irqsave(&switch_data->lock, flags);

	tpadc_base = (void __iomem *)SW_VA_TP_IO_BASE;
	writel(TP_CTRL_CLK_PARA, tpadc_base + TP_CTRL0);
	temp = readl(tpadc_base + TP_CTRL1);
	temp |= ((1<<3) | (0<<4)); //tp mode function disable and select ADC module.
	temp |=0x1;//X2 channel
	writel(temp, tpadc_base + TP_CTRL1);

	temp = readl(tpadc_base + TP_INT_FIFO_CTR);
	temp |= (1<<16); //TP FIFO Data available IRQ Enable
	temp |= (0x3<<8); //4次采样数据的平均值 (3+1)
	writel(temp, tpadc_base + TP_INT_FIFO_CTR);

	temp = readl(tpadc_base + TP_CTRL1);
	temp |= (1<<4);
	writel(temp, tpadc_base + TP_CTRL1);

    spin_unlock_irqrestore(&switch_data->lock, flags);

    /* create switch device */
	switch_data->sdev.state = HEADSET_NULL;
	switch_data->pre_state = HEADSET_UNKOWN;
	switch_data->sdev.name = pdata->name;
	switch_data->earphone_hdle = gpio_earphone_switch;
	switch_data->sdev.print_name = print_headset_name;
	switch_data->sdev.print_state = switch_gpio_print_state;

    ret = switch_dev_register(&switch_data->sdev);
	if (ret < 0) {
		goto err_switch_dev_register;
	}

	switch_set_state(&switch_data->sdev, HEADSET_NULL);

    /* create input device */
    switch_data->key = input_allocate_device();
    if (!switch_data->key) {
        printk(KERN_ERR "gpio_switch_probe: not enough memory for input device\n");
        ret = -ENOMEM;
        goto err_input_allocate_device;
    }

    switch_data->key->name          = "sun4i-headset";
    switch_data->key->phys          = "headset/input0";
    switch_data->key->id.bustype    = BUS_HOST;
    switch_data->key->id.vendor     = 0x0001;
    switch_data->key->id.product    = 0xffff;
    switch_data->key->id.version    = 0x0100;

    switch_data->key->evbit[0] = BIT_MASK(EV_KEY);

    set_bit(KEY_VOLUME_UP, switch_data->key->keybit);
    set_bit(KEY_VOLUME_DOWN, switch_data->key->keybit);
    set_bit(KEY_MEDIA_PLAY_PAUSE, switch_data->key->keybit);
    set_bit(KEY_MEDIA_NEXT, switch_data->key->keybit);
    set_bit(KEY_MEDIA_PREVIOUS, switch_data->key->keybit);
    set_bit(KEY_HEADSETHOOK, switch_data->key->keybit);

    ret = input_register_device(switch_data->key);
    if (ret) {
        printk(KERN_ERR "gpio_switch_probe: input_register_device failed\n");
        goto err_input_register_device;
    }

    SWITCH_DBG("TIMER_START_TIME = %d\n", TIMER_START_TIME);

	init_timer(&switch_data->timer);
	switch_data->timer.expires = jiffies + TIMER_START_TIME * HZ;
	switch_data->timer.function = &earphone_handle;
	switch_data->timer.data = (unsigned long)switch_data;
	add_timer(&switch_data->timer);

	return 0;

err_input_register_device:
    if(switch_data->key){
        input_free_device(switch_data->key);
    }

err_input_allocate_device:
    switch_dev_unregister(&switch_data->sdev);

err_switch_dev_register:
    if(switch_data->earphone_hdle){
	    gpio_release(switch_data->earphone_hdle, 1);
	}

err_gpio_request:
    if(switch_data){
	    kfree(switch_data);
    }

	return ret;
}

static int __devexit gpio_switch_remove(struct platform_device *pdev)
{
	struct gpio_switch_data *switch_data = platform_get_drvdata(pdev);

    if(switch_data->earphone_hdle){
        gpio_release(switch_data->earphone_hdle, 1);
    }

    if(switch_data->key){
        input_unregister_device(switch_data->key);
        input_free_device(switch_data->key);
    }

	del_timer(&switch_data->timer);
    switch_dev_unregister(&switch_data->sdev);
	kfree(switch_data);

	return 0;
}

static struct platform_driver gpio_switch_driver = {
	.probe		= gpio_switch_probe,
	.remove		= __devexit_p(gpio_switch_remove),
	.driver		= {
		.name	= "switch-gpio",
		.owner	= THIS_MODULE,
	},
};

static struct gpio_switch_platform_data headset_switch_data = {
    .name = "h2w",
};

static struct platform_device gpio_switch_device = {
    .name = "switch-gpio",
    .dev = {
            .platform_data = &headset_switch_data,
    }
};

static int __init gpio_switch_init(void)
{
	int ret = 0;

	ret = script_parser_fetch("switch_para","switch_used", &switch_used, sizeof(int));
	if (ret) {
        printk("[switch]switch_headset init fetch para using configuration failed\n");
        return -1;
    }

    if (switch_used) {
		ret = platform_device_register(&gpio_switch_device);
		if (ret == 0) {
			ret = platform_driver_register(&gpio_switch_driver);
		}
	} else {
		printk("[switch]switch headset cannot find any using configuration for controllers, return directly!\n");
		return 0;
	}

	return ret;
}

static void __exit gpio_switch_exit(void)
{

	if (switch_used) {
		switch_used = 0;
		platform_driver_unregister(&gpio_switch_driver);
		platform_device_unregister(&gpio_switch_device);
	}

	return;
}
module_init(gpio_switch_init);
module_exit(gpio_switch_exit);

MODULE_AUTHOR("Mike Lockwood <lockwood@android.com>");
MODULE_DESCRIPTION("GPIO Switch driver");
MODULE_LICENSE("GPL");
