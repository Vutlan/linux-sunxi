/* linux/driver/input/misc/ltr.c
 * Copyright (C) 2010 Samsung Electronics. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/gpio.h>
#include <linux/wakelock.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/suspend.h>
#include <linux/regulator/consumer.h>
#include <mach/sys_config.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
    #include <linux/pm.h>
    #include <linux/earlysuspend.h>
#endif

#include <linux/gfp.h>

#include "ltr_501als.h" 


/* Note about power vs enable/disable:
 *  The chip has two functions, proximity and ambient light sensing.
 *  There is no separate power enablement to the two functions (unlike
 *  the Capella CM3602/3623).
 *  This module implements two drivers: /dev/proximity and /dev/light.
 *  When either driver is enabled (via sysfs attributes), we give power
 *  to the chip.  When both are disabled, we remove power from the chip.
 *  In suspend, we remove power if light is disabled but not if proximity is
 *  enabled (proximity is allowed to wakeup from suspend).
 *
 *  There are no ioctls for either driver interfaces.  Output is via
 *  input device framework and control via sysfs attributes.
 */


//#define debug(str, args...) pr_debug("%s: " str, __func__, ##args)

#define module_tag "sensor:"

//#define  SENSOR_DEBUG

#ifdef  SENSOR_DEBUG
#define debug(str, x...)     printk("%s:" str, module_tag, ##x)
#else
#define debug(str, x...)
#endif
      
#define info(str, x...)     printk("%s:" str, module_tag, ##x)

#define PS_DISTANCE  500 

enum {
	LIGHT_ENABLED = BIT(0),
	PROXIMITY_ENABLED = BIT(1),
};

static const int chip_id_value[] = {0x05,0};

struct sensor_config{
	int twi_id;
	int twi_addr;
	int int1;
	int int_mode;
	char *ldo;
};
static struct sensor_config sensor_config ;

extern long phone_actived;

/* driver data */
struct ltr_data {
	struct input_dev *proximity_input_dev;
	struct input_dev *light_input_dev;
	struct sensor_config *sensor_config;
	struct delayed_work ps_delay_work;
	struct work_struct irq_workqueue;
	int gpio_irq_handle;
	struct i2c_client *i2c_client;
	int irq;
	struct work_struct work_light;
	struct hrtimer timer;
	ktime_t light_poll_delay;
	//int adc_value_buf[ADC_BUFFER_NUM];
	//int adc_index_count;
	//bool adc_buf_initialized;
	bool on;
	u8 power_state;
	unsigned long ps_poll_delay;
	struct mutex power_lock;
	struct wake_lock prx_wake_lock;
	struct workqueue_struct *wq;
};

int ltr558_als_read(void)
{
	int alsval_ch0_lo, alsval_ch0_hi, alsval_ch0;
	int alsval_ch1_lo, alsval_ch1_hi, alsval_ch1;
	int luxdata_int;
	float ratio, luxdata_flt;
	int gainrange= ALS_RANGE2_64K;


	alsval_ch0_lo = ltr558_i2c_read_reg(LTR558_ALS_DATA_CH0_0);
	alsval_ch0_hi = ltr558_i2c_read_reg(LTR558_ALS_DATA_CH0_1);
	alsval_ch0 = (alsval_ch0_hi * 256) + alsval_ch0_lo;

	alsval_ch1_lo = ltr558_i2c_read_reg(LTR558_ALS_DATA_CH1_0);
	alsval_ch1_hi = ltr558_i2c_read_reg(LTR558_ALS_DATA_CH1_1);
	alsval_ch1 = (alsval_ch1_hi * 256) + alsval_ch1_lo;
#if 1
	if ( 0 == alsval_ch0 )//Division by zero in kernel.
		return -1;

	ratio = alsval_ch1 / alsval_ch0;

	if (ratio < 0.69){
		luxdata_flt = (1.3618 * alsval_ch0) - (1.5 * alsval_ch1);
	}
	else if ((ratio >= 0.69) && (ratio < 1.0)){
		luxdata_flt = (0.57 * alsval_ch0) - (0.345 * alsval_ch1);
	}
	else {
		luxdata_flt = 0.0;
	}

	// For Range1
	if (gainrange == ALS_RANGE1_320)
		luxdata_flt = luxdata_flt / 150;

#endif

	// convert float to integer;
	luxdata_int = luxdata_flt;
	if ((luxdata_flt - luxdata_int) > 0.5){
		luxdata_int = luxdata_int + 1;
	}
	else {
		luxdata_int = luxdata_flt;
	}

	//final_lux_val = luxdata_int;
	return luxdata_int;
}
int ltr558_ps_read(void)
{
	int psval_lo, psval_hi, psdata;

	psval_lo = ltr558_i2c_read_reg(LTR558_PS_DATA_0);
	if (psval_lo < 0){
		psdata = psval_lo;
		goto out;
	}
		
	psval_hi = ltr558_i2c_read_reg(LTR558_PS_DATA_1);
	if (psval_hi < 0){
		psdata = psval_hi;
		goto out;
	}
		
	psdata = ((psval_hi & 7)* 256) + psval_lo;


	out:
//	final_prox_val = psdata;
	return psdata;
}
#if 0
int ltr558_i2c_read_reg(u8 regnum)
{
	info("xdafsdf\n");
}
#endif
struct ltr558_data {
	struct i2c_client *client;
};
static struct ltr558_data the_data;



// I2C Read
int ltr558_i2c_read_reg(unsigned char regnum)
{
	int readdata;

	/*
	 * i2c_smbus_read_byte_data - SMBus "read byte" protocol
	 * @client: Handle to slave device
	 * @command: Byte interpreted by slave
	 *
	 * This executes the SMBus "read byte" protocol, returning negative errno
	 * else a data byte received from the device.
	 */
	readdata = i2c_smbus_read_byte_data(the_data.client, regnum);
	return readdata;
}


// I2C Write
int ltr558_i2c_write_reg(unsigned char regnum, unsigned char value)
{
	int writeerror;

	/*
	 * i2c_smbus_write_byte_data - SMBus "write byte" protocol
	 * @client: Handle to slave device
	 * @command: Byte interpreted by slave
	 * @value: Byte being written
	 *
	 * This executes the SMBus "write byte" protocol, returning negative errno
	 * else zero on success.
	 */

	writeerror = i2c_smbus_write_byte_data(the_data.client, regnum, value);

	if (writeerror < 0)
		return writeerror;
	else
		return 0;
}



static int ltr558_ps_enable(void)
{
	int error;
	int setgain;
	int gainrange = PS_RANGE8;

	switch (gainrange) {
		case PS_RANGE1:
			setgain = MODE_PS_ON_Gain1;
			break;

		case PS_RANGE2:
			setgain = MODE_PS_ON_Gain2;
			break;

		case PS_RANGE4:
			setgain = MODE_PS_ON_Gain4;
			break;

		case PS_RANGE8:
			setgain = MODE_PS_ON_Gain8;
			break;

		default:
			setgain = MODE_PS_ON_Gain1;
			break;
	}

	error = ltr558_i2c_write_reg(LTR558_PS_CONTR, setgain | (1<<1)); 
	mdelay(WAKEUP_DELAY);

	/* =============== 
	 * ** IMPORTANT **
	 * ===============
	 * Other settings like timing and threshold to be set here, if required.
 	 * Not set and kept as device default for now.
 	 */

	return error;
}


// Put PS into Standby mode
static int ltr558_ps_disable(void)
{
	int error;
	error = ltr558_i2c_write_reg(LTR558_PS_CONTR, MODE_PS_StdBy); 
	return error;
}

static int ltr558_als_enable(void)
{
	int error;
	int gainrange = 2;//lkj Dynamic Range 2 (2 lux to 64k lux) 

	if (gainrange == 1)
		error = ltr558_i2c_write_reg(LTR558_ALS_CONTR, MODE_ALS_ON_Range1);
	else if (gainrange == 2)
		error = ltr558_i2c_write_reg(LTR558_ALS_CONTR, MODE_ALS_ON_Range2);
	else
		error = -1;

	mdelay(WAKEUP_DELAY);

	/* =============== 
	 * ** IMPORTANT **
	 * ===============
	 * Other settings like timing and threshold to be set here, if required.
 	 * Not set and kept as device default for now.
 	 */

	return error;
}


// Put ALS into Standby mode
static int ltr558_als_disable(void)
{
	int error;
	error = ltr558_i2c_write_reg(LTR558_ALS_CONTR, MODE_ALS_StdBy); 
	return error;
}


int ltr558_als_power(bool enable)
{
	int ret;

	if (enable)
		ret=ltr558_als_enable();
	else 
		ret=ltr558_als_disable();

	return ret;
}

int ltr558_ps_power(bool enable)
{
	int ret;

	if (enable)
		ret = ltr558_ps_enable();
	else
		ret = ltr558_ps_disable();

	return ret;
}


int ltr558_devinit(void)
{
	int error;

	mdelay(PON_DELAY);

	error = ltr558_i2c_write_reg(LTR558_INTERRUPT, 0x1); // 0 active, ps interrupt, latched until read 
	debug("devinit write int ret=%d\n", error);

	// Enable PS to Gain1 at startup
	error = ltr558_ps_enable();
	if (error < 0)
		goto out;

	// Enable ALS to Full Range at startup
	error = ltr558_als_enable();
	if (error < 0)
		goto out;

	error = 0;
        error = ltr558_ps_read();
	debug("first read ps=%d\n", error);

	ltr558_i2c_write_reg(LTR558_PS_LED, 0x7f );
	ltr558_i2c_write_reg(LTR558_PS_N_PULSES, 0x08);
	ltr558_i2c_write_reg(LTR558_INTERRUPT_PERSIST, 0x70);
	//ltr558_i2c_write_reg(LTR558_PS_MEAS_RATE, 0x20);

	/* 0 ~ 500 */
	ltr558_i2c_write_reg(LTR558_PS_THRES_LOW_0, 0xf4);
	ltr558_i2c_write_reg(LTR558_PS_THRES_LOW_1, 0x1);
	ltr558_i2c_write_reg(LTR558_PS_THRES_UP_0, 0xff);
	ltr558_i2c_write_reg(LTR558_PS_THRES_UP_1, 0x07);

#if 1
//printf reg
	{
	unsigned char upper_low,upper_high;
	unsigned char lower_low,lower_high;

	upper_low = ltr558_i2c_read_reg(LTR558_PS_THRES_UP_0);
	upper_high = ltr558_i2c_read_reg(LTR558_PS_THRES_UP_1);

	lower_low = ltr558_i2c_read_reg(LTR558_PS_THRES_LOW_0);
	lower_high = ltr558_i2c_read_reg(LTR558_PS_THRES_LOW_1);

	debug(" read thres upper =%d\n", ((upper_high & 0x7) << 7) |  upper_low );
	debug(" read thres lower =%d\n", ((lower_high & 0x7) << 7) |  lower_low );


//	error = ltr558_i2c_write_reg(LTR558_PS_LED, 0x6b & 0xf8);¶ÁÓÐ±ä»¯¡¡

	error = ltr558_i2c_read_reg(LTR558_PS_CONTR);
	debug(" LTR558_PS_CONTR =0x%0x\n", error );

	error = ltr558_i2c_read_reg(LTR558_PS_LED);
	debug(" LTR558_PS_LED =0x%0x\n", error );

	error = ltr558_i2c_read_reg(LTR558_PS_N_PULSES);
	debug(" LTR558_PS_N_PULSES =0x%0x\n", error );

	error = ltr558_i2c_read_reg(LTR558_PS_MEAS_RATE);
	debug(" LTR558_PS_MEAS_RATE =0x%0x\n", error );

	error = ltr558_i2c_read_reg(LTR558_INTERRUPT);
	debug(" LTR558_INTERRUPT =0x%0x\n", error );

	error = ltr558_i2c_read_reg(LTR558_INTERRUPT_PERSIST);
	debug(" LTR558_INTERRUPT_PERSIST =0x%0x\n", error );
	}
#endif


	out:
	return error;
}

void ltr558_set_client(struct i2c_client *client)
{
	the_data.client = client;
}


static void ltr_ps_enable(struct ltr_data *ltr)
{
	info("starting ps work");
	queue_delayed_work(ltr->wq, &ltr->ps_delay_work, 0); 
}

static void ltr_ps_disable(struct ltr_data *ltr)
{
	info("cancelling ps work\n");
	cancel_delayed_work_sync(&ltr->ps_delay_work);
}

static void ltr_light_enable(struct ltr_data *ltr)
{
	info("starting poll timer, delay %lldns\n", ktime_to_ns(ltr->light_poll_delay));
	hrtimer_start(&ltr->timer, ltr->light_poll_delay, HRTIMER_MODE_REL);
}

static void ltr_light_disable(struct ltr_data *ltr)
{
	info("cancelling poll timer\n");
	hrtimer_cancel(&ltr->timer);
	cancel_work_sync(&ltr->work_light);
}

static ssize_t poll_delay_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct ltr_data *ltr = dev_get_drvdata(dev);
	return sprintf(buf, "%lld\n", ktime_to_ns(ltr->light_poll_delay));
}


static ssize_t poll_delay_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct ltr_data *ltr = dev_get_drvdata(dev);
	int64_t new_delay;
	int err;

	err = strict_strtoll(buf, 10, &new_delay);
	if (err < 0)
		return err;

	debug("new delay = %lldns, old delay = %lldns\n",
		    new_delay, ktime_to_ns(ltr->light_poll_delay));
	mutex_lock(&ltr->power_lock);
	if (new_delay != ktime_to_ns(ltr->light_poll_delay)) {
		ltr->light_poll_delay = ns_to_ktime(new_delay);
		if (ltr->power_state & LIGHT_ENABLED) {
			ltr_light_disable(ltr);
			ltr_light_enable(ltr);
		}
	}
	mutex_unlock(&ltr->power_lock);

	return size;
}

static ssize_t light_enable_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct ltr_data *ltr = dev_get_drvdata(dev);
	return sprintf(buf, "%d\n",
		       (ltr->power_state & LIGHT_ENABLED) ? 1 : 0);
}

static ssize_t proximity_enable_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct ltr_data *ltr = dev_get_drvdata(dev);
	return sprintf(buf, "%d\n",
		       (ltr->power_state & PROXIMITY_ENABLED) ? 1 : 0);
}

static ssize_t light_enable_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t size)
{
	struct ltr_data *ltr = dev_get_drvdata(dev);
	bool new_value;

	if (sysfs_streq(buf, "1"))
		new_value = true;
	else if (sysfs_streq(buf, "0"))
		new_value = false;
	else {
		info("%s: invalid value %d\n", __func__, *buf);
		return -EINVAL;
	}

	mutex_lock(&ltr->power_lock);

	debug("new_value = %d, old state = %d\n",
		    new_value, (ltr->power_state & LIGHT_ENABLED) ? 1 : 0);

	if (new_value && !(ltr->power_state & LIGHT_ENABLED)) {
		ltr558_als_power(true);
		ltr->power_state |= LIGHT_ENABLED;
		ltr_light_enable(ltr);
	} else if (!new_value && (ltr->power_state & LIGHT_ENABLED)) {
		ltr_light_disable(ltr);
		ltr->power_state &= ~LIGHT_ENABLED;
		ltr558_als_power(false);
	}

	mutex_unlock(&ltr->power_lock);

	return size;
}

static ssize_t proximity_enable_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t size)
{
	struct ltr_data *ltr = dev_get_drvdata(dev);
	bool new_value;

	if (sysfs_streq(buf, "1"))
		new_value = true;
	else if (sysfs_streq(buf, "0"))
		new_value = false;
	else {
		info("%s: invalid value %d\n", __func__, *buf);
		return -EINVAL;
	}

	mutex_lock(&ltr->power_lock);
	debug("new_value = %d, old state = %d\n",  new_value, (ltr->power_state & PROXIMITY_ENABLED) ? 1 : 0);
	if (new_value && !(ltr->power_state & PROXIMITY_ENABLED)) {
		ltr558_ps_power(true);
		//enable_irq(ltr->irq);
		ltr->power_state |= PROXIMITY_ENABLED;
		ltr_ps_enable(ltr);
	} else if (!new_value && (ltr->power_state & PROXIMITY_ENABLED)) {
		//disable_irq(ltr->irq);
		//disable_irq_nosync(ltr->irq);
		ltr_ps_disable(ltr);
		ltr558_ps_power(false);
		ltr->power_state &= ~PROXIMITY_ENABLED;
	}
	mutex_unlock(&ltr->power_lock);
	return size;
}

static DEVICE_ATTR(poll_delay, S_IRUGO | S_IWUSR | S_IWGRP,
		   poll_delay_show, poll_delay_store);

static struct device_attribute dev_attr_light_enable =
	__ATTR(enable, S_IRUGO | S_IWUSR | S_IWGRP,
	       light_enable_show, light_enable_store);

static struct device_attribute dev_attr_proximity_enable =
	__ATTR(enable, S_IRUGO | S_IWUGO,
	       proximity_enable_show, proximity_enable_store);

static struct attribute *light_sysfs_attrs[] = {
	&dev_attr_light_enable.attr,
	&dev_attr_poll_delay.attr,
	NULL
};

static struct attribute_group light_attribute_group = {
	.attrs = light_sysfs_attrs,
};

static struct attribute *proximity_sysfs_attrs[] = {
	&dev_attr_proximity_enable.attr,
	NULL
};

static struct attribute_group proximity_attribute_group = {
	.attrs = proximity_sysfs_attrs,
};

static void ltr_work_func_light(struct work_struct *work)
{
	struct ltr_data *ltr = container_of(work, struct ltr_data,
			work_light);

	int adc = ltr558_als_read();
	if (adc < 0)
	{
		debug("light val 0");
		adc = 0; // no light 
	}

	debug("light val=%d\n",  adc);
	input_report_abs(ltr->light_input_dev, ABS_MISC, adc);
	input_sync(ltr->light_input_dev);
}

/* This function is for light sensor.  It operates every a few seconds.
 * It asks for work to be done on a thread because i2c needs a thread
 * context (slow and blocking) and then reschedules the timer to run again.
 */
static enum hrtimer_restart ltr_timer_func(struct hrtimer *timer)
{
	struct ltr_data *ltr = container_of(timer, struct ltr_data, timer);
	queue_work(ltr->wq, &ltr->work_light);
	hrtimer_forward_now(&ltr->timer, ltr->light_poll_delay);
	return HRTIMER_RESTART;
}

#if 1
/* interrupt happened due to transition/change of near/far proximity state */
static u32 ltr_irq_handler(void *data)
{
//	struct ltr_data *ltr =(struct ltr_data *)data;
//	schedule_work(&ltr->irq_workqueue)
	debug("in irq\n");
	return 0;
}
#endif

static void ltr558_schedwork(struct work_struct *work)
{
	struct ltr_data *ltr = container_of((struct delayed_work *)work, struct ltr_data,ps_delay_work);
	int val=-1;
#if 0
	int als_ps_status;
	int interrupt, newdata, val=-1;

	als_ps_status = ltr558_i2c_read_reg(LTR558_ALS_PS_STATUS);

	interrupt = als_ps_status & 10;
	newdata = als_ps_status & 5;

	switch (interrupt){
		case 2:
			// PS interrupt
			if ((newdata == 1) | (newdata == 5)){
				val = ltr558_ps_read();
			}
			break;

		case 8:
			info("!!!!!!!!!impossible irq als insterrupt!!!!!!!!");
			// ALS interrupt
			if ((newdata == 4) | (newdata == 5)){
				; //als_data_changed = 1;
			}
			break;

		case 10:
			info("!!!!!!!!!impossible irq ps and als insterrupt!!!!!!!!");
			// Both interrupt
			if ((newdata == 1) | (newdata == 5)){
				val = ltr558_ps_read();
				; //ps_data_changed = 1;
			}
			if ((newdata == 4) | (newdata == 5)){
				;//als_data_changed = 1;
			}
			break;
	}
#endif
	val = ltr558_ps_read();
	debug(" ps val =%d\n", val);
	
	/* 0 is close, 1 is far */
	val = val >= PS_DISTANCE ? 0: 1;

	input_report_abs(ltr->proximity_input_dev, ABS_DISTANCE, val);
	input_sync(ltr->proximity_input_dev);

	queue_delayed_work(ltr->wq, &ltr->ps_delay_work, msecs_to_jiffies(ltr->ps_poll_delay));
	//wake_lock_timeout(&ip->prx_wake_lock, 3*HZ);
}

#if 1
static int ltr_setup_irq(struct ltr_data *ltr)
{
	int ret = -EIO;

	debug(" enter %s\n", __func__);

	if (ltr->sensor_config->int1 >= 0) {
		// TRIG_LEVL_LOW made interrupt too frequently, so use TRIG_EDGE_NEGATIVE
		ret = sw_gpio_irq_request(ltr->sensor_config->int1, TRIG_EDGE_NEGATIVE, ltr_irq_handler, ltr);//TRIG_LEVL_LOW
		if (!ret) {
			info("Failed to request gpio irq \n");
			ret = -EIO;
		} else {
			ltr->gpio_irq_handle = ret ;
		}
	}

	ret = 0;
	debug("ltr_setup_irq success\n");
	return ret;
}
#endif

static int ltr_i2c_probe(struct i2c_client *client,
			  const struct i2c_device_id *id)
{
	struct input_dev *input_dev;
	struct ltr_data *ltr;
	int ret = -ENODEV;

	/* Return 1 if adapter supports everything we need, 0 if not. */
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_WRITE_BYTE | I2C_FUNC_SMBUS_READ_BYTE_DATA))
	{
		printk("%s,LTR-558ALS functionality check failed.\n", __func__);
		return -EIO;
	}

	ltr = kzalloc(sizeof(struct ltr_data), GFP_KERNEL);
	if (!ltr) {
		info("%s: failed to alloc memory for module data\n", __func__);
		return -ENOMEM;
	}

//	ltr->pdata = pdata;
	ltr->i2c_client = client;
	i2c_set_clientdata(client, ltr);
	ltr->sensor_config = &sensor_config;
	ltr->irq =  __gpio_to_irq(sensor_config.int1); 
	ltr->ps_poll_delay = 100; //ms

	debug("get irq =%d\n", ltr->irq);


/* the timer just fires off a work queue request.  we need a thread
	   to read the i2c (can be slow and blocking). */
	ltr->wq = create_singlethread_workqueue("ltr_wq");
	if (!ltr->wq) {
		ret = -ENOMEM;
		info("%s: could not create workqueue\n", __func__);
		goto err_create_workqueue;
	}
	
//==================proximity  sensor======================
	/* wake lock init */
	//wake_lock_init(&ltr->prx_wake_lock, WAKE_LOCK_SUSPEND, "prx_wake_lock");
	mutex_init(&ltr->power_lock);
	//INIT_WORK(&ltr->irq_workqueue,ltr558_schedwork);
	INIT_DELAYED_WORK(&ltr->ps_delay_work, ltr558_schedwork);

	/* allocate proximity input_device */
	input_dev = input_allocate_device();
	if (!input_dev) {
		info("%s: could not allocate input device\n", __func__);
		goto err_input_allocate_device_proximity;
	}
	ltr->proximity_input_dev = input_dev;
	input_set_drvdata(input_dev, ltr);
	input_dev->name = "proximity";
	input_set_capability(input_dev, EV_ABS, ABS_DISTANCE);
	//0,close ,1 far 
	input_set_abs_params(input_dev, ABS_DISTANCE, 0, 1, 0, 0);

	info("registering proximity input device\n");
	ret = input_register_device(input_dev);
	if (ret < 0) {
		info("%s: could not register input device\n", __func__);
		input_free_device(input_dev);
		goto err_input_register_device_proximity;
	}
	ret = sysfs_create_group(&input_dev->dev.kobj,
				 &proximity_attribute_group);
	if (ret) {
		info("%s: could not create sysfs group\n", __func__);
		goto err_sysfs_create_group_proximity;
	}



//==================light sensor======================
	/* hrtimer settings.  we poll for light values using a timer. */
	hrtimer_init(&ltr->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	ltr->light_poll_delay = ns_to_ktime(200 * NSEC_PER_MSEC);
	ltr->timer.function = ltr_timer_func;

	/* this is the thread function we run on the work queue */
	INIT_WORK(&ltr->work_light, ltr_work_func_light);

	/* allocate lightsensor-level input_device */
	input_dev = input_allocate_device();
	if (!input_dev) {
		info("%s: could not allocate input device\n", __func__);
		ret = -ENOMEM;
		goto err_input_allocate_device_light;
	}
	input_set_drvdata(input_dev, ltr);
	input_dev->name = "lightsensor";
	input_set_capability(input_dev, EV_ABS, ABS_MISC);
	//max 16bit 
	input_set_abs_params(input_dev, ABS_MISC, 0, 65535, 0, 0);

	debug("registering lightsensor-level input device\n");
	ret = input_register_device(input_dev);
	if (ret < 0) {
		info("%s: could not register input device\n", __func__);
		input_free_device(input_dev);
		goto err_input_register_device_light;
	}
	ltr->light_input_dev = input_dev;
	ret = sysfs_create_group(&input_dev->dev.kobj,
				 &light_attribute_group);
	if (ret) {
		info("%s: could not create sysfs group\n", __func__);
		goto err_sysfs_create_group_light;
	}

	ltr558_set_client(ltr->i2c_client);
	ltr558_devinit();

#if 1
// PS IRQ triger
	ret = ltr_setup_irq(ltr);
	if (ret) {
		info("%s: could not setup irq\n", __func__);
		goto err_setup_irq;
	}
#endif
	goto done;

	/* error, unwind it all */
err_sysfs_create_group_light:
	input_unregister_device(ltr->light_input_dev);
err_input_register_device_light:
err_input_allocate_device_light:
	destroy_workqueue(ltr->wq);
err_create_workqueue:
	sysfs_remove_group(&ltr->proximity_input_dev->dev.kobj,
			   &proximity_attribute_group);
err_sysfs_create_group_proximity:
	input_unregister_device(ltr->proximity_input_dev);
err_input_register_device_proximity:
err_input_allocate_device_proximity:
	free_irq(ltr->irq, 0);
err_setup_irq:
	mutex_destroy(&ltr->power_lock);
	//wake_lock_destroy(&ltr->prx_wake_lock);
	kfree(ltr);
done:
	return ret;
}

#if 0
#ifdef CONFIG_HAS_EARLYSUSPEND

static void ltr_suspend(struct early_suspend *handler)
{
        info("==early suspend=\n");
              
}
static void ltr_resume(struct early_suspend *handler)
{
        info("==early resume=\n");
}
#else //CONFIG_HAS_EARLYSUSPEND
#endif
#endif

#ifdef CONFIG_PM
static int ltr_suspend(struct i2c_client *client, pm_message_t mesg)
{
        /* We disable power only if proximity is disabled.  If proximity
	   is enabled, we leave power on because proximity is allowed
	   to wake up device.  We remove power without changing
	   ltr->power_state because we use that state in resume.
	*/
	struct ltr_data *ltr = i2c_get_clientdata(client);

	if (ltr->power_state & LIGHT_ENABLED){
		ltr_light_disable(ltr);
		ltr558_als_power(false);
	}

	if (ltr->power_state & PROXIMITY_ENABLED){
		ltr_ps_disable(ltr);
		if (!phone_actived) {
			ltr558_ps_power(false);
		}
	}

	return 0;
}
static int ltr_resume(struct i2c_client *client)
{
	/* Turn power back on if we were before suspend. */
	struct ltr_data *ltr = i2c_get_clientdata(client);
	if (ltr->power_state & LIGHT_ENABLED){
		ltr_light_enable(ltr);
		ltr558_als_power(true);
	}
	if (ltr->power_state & PROXIMITY_ENABLED){
		if (!phone_actived) {
			ltr558_ps_power(true);
		}
		ltr_ps_enable(ltr);
	}
	return 0;
		
}
#endif

static int ltr_i2c_remove(struct i2c_client *client)
{
	struct ltr_data *ltr = i2c_get_clientdata(client);

	sysfs_remove_group(&ltr->light_input_dev->dev.kobj,
			   &light_attribute_group);
	input_unregister_device(ltr->light_input_dev);
	sysfs_remove_group(&ltr->proximity_input_dev->dev.kobj,
			   &proximity_attribute_group);
	input_unregister_device(ltr->proximity_input_dev);
	if (ltr->gpio_irq_handle)
		sw_gpio_irq_free(ltr->gpio_irq_handle);
	//free_irq(ltr->irq, NULL);
	if (ltr->power_state) {
		ltr->power_state = 0;
		if (ltr->power_state & LIGHT_ENABLED)
			ltr_light_disable(ltr);
		ltr558_als_power(false);
		ltr558_ps_power(false);
	}
	destroy_workqueue(ltr->wq);
	mutex_destroy(&ltr->power_lock);
	//wake_lock_destroy(&ltr->prx_wake_lock);
	kfree(ltr);
	return 0;
}


static const struct i2c_device_id ltr_device_id[] = {
	{"ltr", 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, ltr_device_id);

static const unsigned short normal_i2c[2] = {0x23,I2C_CLIENT_END};

static struct i2c_driver ltr_i2c_driver = {
	.class = I2C_CLASS_HWMON,
	.probe		= ltr_i2c_probe,
	.remove		= __devexit_p(ltr_i2c_remove),
	.id_table	= ltr_device_id,
	.address_list   = normal_i2c,
#if 0
#ifdef CONFIG_HAS_EARLYSUSPEND

#else
#endif
#endif
#ifdef CONFIG_PM
	.suspend  = ltr_suspend, 
	.resume   = ltr_resume,
#endif
	.driver = {
		.name = "ltr",
		.owner = THIS_MODULE,
	 },
};


static int sensor_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;
        int ret = 0, i = 0;

        debug("enter %s\n",__func__);
        
        if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
                return -ENODEV;
    
	if(sensor_config.twi_id == adapter->nr){
                msleep(200);
	        ret = i2c_smbus_read_byte_data(client,LTR558_MANUFACTURER_ID);
                debug("addr:0x%x,chip_id_value:0x%x\n",client->addr,ret);
                while(chip_id_value[i]){
                        if(ret == 0x5){
            	                strlcpy(info->type, "ltr", I2C_NAME_SIZE);
    		                return 0;
                        }
			i++;
                }
        	info("%s:I2C connection might be something wrong ! \n",__func__);
        	return -ENODEV;
	}else{
		return -ENODEV;
	}
}


static int gyr_fetch_sysconfig_para(void)
{
//	int ret = 0;
	int twi_id = 0;
	int twi_addr = 0;
	int sensor_int = 0;
	char* ldo_str = NULL;

	script_item_u	val;
	script_item_value_type_e  type;

	if(SCIRPT_ITEM_VALUE_TYPE_INT  != script_get_item("light_distance_sensor_para", "light_distance_sensor_used", &val)){
		info("%s: script_parser_fetch err. \n", __func__);
		goto script_parser_fetch_err;
	}
	if(1 == val.val){
		type = script_get_item("light_distance_sensor_para", "light_distance_sensor_twi_id", &val);	
		if(SCIRPT_ITEM_VALUE_TYPE_INT != type){
			info("%s: type err twi_id = %d. \n", __func__, val.val);
			goto script_parser_fetch_err;
		}
		twi_id = val.val;

		type = script_get_item("light_distance_sensor_para", "light_distance_sensor_twi_addr", &val);	
		if(SCIRPT_ITEM_VALUE_TYPE_INT != type){
			info("%s: type err twi_addr = %d. \n", __func__, val.val);
			goto script_parser_fetch_err;
		}
		twi_addr = val.val;

		type = script_get_item("light_distance_sensor_para", "light_distance_sensor_int1", &val);	
		if(SCIRPT_ITEM_VALUE_TYPE_PIO != type){
			info("%s: type err twi int1 = %d. \n", __func__, val.gpio.gpio);
			goto script_parser_fetch_err;
		}
		sensor_int = val.gpio.gpio;
		/*
		ret = gpio_request_one(sensor_int,GPIOF_OUT_INIT_HIGH,NULL);
		if(ret != 0){
			info("interrupt pin set to output hight function failure!\n");
			return -1;
		} 
		*/
		/* get ldo string */
		type = script_get_item("light_distance_sensor_para", "light_distance_sensor_ldo", &val);
		if (SCIRPT_ITEM_VALUE_TYPE_STR != type)
			info("%s: no ldo for light distance sensor, ignore it\n", __func__);
		else
			ldo_str = val.str;
	} else {
		info("%s: unused. \n",  __func__);
		return -1;
	}

	sensor_config.twi_id=twi_id;
	sensor_config.twi_addr = twi_addr;
	sensor_config.int1 = sensor_int;
	sensor_config.ldo = ldo_str;

	debug("twi id=0x%0x, addr=0x%x\n", sensor_config.twi_id, sensor_config.twi_addr);
	debug("twi gpio=%d, ldo: '%s'\n", sensor_config.int1, sensor_config.ldo);

        return 0;

script_parser_fetch_err:
	info("=========script_parser_fetch_err============\n");
	return -1;
}


static int ltr_init(void)
{
	struct regulator *ldo = NULL;
	debug("%s:  sysfs driver init\n", __func__ );

	if(gyr_fetch_sysconfig_para()){
		info("%s: err.\n", __func__);
		return -1;
	}

	/* enalbe ldo if it exist */
	if (sensor_config.ldo) {
		ldo = regulator_get(NULL, sensor_config.ldo);
		if (!ldo) {
			info("%s: could not get sensor ldo '%s' in probe, maybe config error,"
					"ignore firstly !!!!!!!\n", __func__, sensor_config.ldo);
		}
		regulator_set_voltage(ldo, 3000000, 3000000);
		regulator_enable(ldo);
		regulator_put(ldo);
		usleep_range(10000, 15000);
	}
	ltr_i2c_driver.detect = sensor_detect;
	return i2c_add_driver(&ltr_i2c_driver);
}

static void ltr_exit(void)
{
	struct regulator *ldo = NULL;
	i2c_del_driver(&ltr_i2c_driver);

	/* disable ldo if it exist */
	ldo = regulator_get(NULL, sensor_config.ldo);
	if (!ldo) {
		info("%s: could not get ldo '%s' in remove, something error ???, "
				"ignore it here !!!!!!!!!\n", __func__, sensor_config.ldo);
	} else {
		regulator_disable(ldo);
		regulator_put(ldo);
	}
}

module_init(ltr_init);
module_exit(ltr_exit);

MODULE_AUTHOR("nsdfsdf@sta.samsung.com");
MODULE_DESCRIPTION("Optical Sensor driver for ltrp002a00f");
MODULE_LICENSE("GPL");

