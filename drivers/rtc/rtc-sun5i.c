/*
 * drivers\rtc\rtc-sun5i.c
 * An I2C driver for the Philips PCF8563 RTC
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * huangxin <huangxin@allwinnertech.com>
 *
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */
 
#include <linux/i2c.h>
#include <linux/bcd.h>
#include <linux/rtc.h>
#include <linux/slab.h>

#define DRV_VERSION "0.4.3"

/*控制方式寄存器*/
#define PCF8563_REG_ST1		0x00 /* status */
#define PCF8563_REG_ST2		0x01

/*秒~年时间寄存器*/
#define PCF8563_REG_SC		0x02 /* datetime */
#define PCF8563_REG_MN		0x03
#define PCF8563_REG_HR		0x04
#define PCF8563_REG_DM		0x05
#define PCF8563_REG_DW		0x06
#define PCF8563_REG_MO		0x07
#define PCF8563_REG_YR		0x08

/*报警功能寄存器*/
#define PCF8563_REG_AMN		0x09 /* alarm */
#define PCF8563_REG_AHR		0x0A
#define PCF8563_REG_ADM		0x0B
#define PCF8563_REG_ADW		0x0C

/*时钟输出寄存器*/
#define PCF8563_REG_CLKO	0x0D /* clock out */

/*定时器功能寄存器*/
#define PCF8563_REG_TMRC	0x0E /* timer control */
#define PCF8563_REG_TMR		0x0F /* timer */

#define PCF8563_SC_LV		0x80 /* low voltage */
#define PCF8563_MO_C		0x80 /* century */

static struct i2c_driver pcf8563_driver;
//static int sun5i_rtc_alarmno = NO_IRQ;
struct pcf8563 {
	struct rtc_device *rtc;
	/*
	 * The meaning of MO_C bit varies by the chip type.
	 * From PCF8563 datasheet: this bit is toggled when the years
	 * register overflows from 99 to 00
	 *   0 indicates the century is 20xx
	 *   1 indicates the century is 19xx
	 * From RTC8564 datasheet: this bit indicates change of
	 * century. When the year digit data overflows from 99 to 00,
	 * this bit is set. By presetting it to 0 while still in the
	 * 20th century, it will be set in year 2000, ...
	 * There seems no reliable way to know how the system use this
	 * bit.  So let's do it heuristically, assuming we are live in
	 * 1970...2069.
	 */
	int c_polarity;	/* 0: MO_C=1 means 19xx, otherwise MO_C=1 means 20xx */
};
#if 0
/* IRQ Handlers, irq no. is shared with timer2 */
static irqreturn_t sun5ii_rtc_alarmirq(int irq, void *id)
{
//	struct rtc_device *rdev = id;
//	u32 val;
//
//    /*judge the int is whether ours*/
//    val = readl(sunxi_rtc_base + SUNXI_ALARM_INT_STATUS_REG)&(RTC_ENABLE_WK_IRQ | RTC_ENABLE_CNT_IRQ);
//    if (val) {
//		/*Clear pending count alarm*/
//		val = readl(sunxi_rtc_base + SUNXI_ALARM_INT_STATUS_REG);//0x11c
//		val |= (RTC_ENABLE_CNT_IRQ);	//0x00000001
//		writel(val, sunxi_rtc_base + SUNXI_ALARM_INT_STATUS_REG);
//		
//		rtc_update_irq(rdev, 1, RTC_AF | RTC_IRQF);
//		return IRQ_HANDLED;
//    } else {
//        return IRQ_NONE;
//    }
	return IRQ_NONE;
}
#endif
/*
 * In the routines that deal directly with the pcf8563 hardware, we use
 * rtc_time -- month 0-11, hour 0-23, yr = calendar year-epoch.
 * 读时钟步骤：
 *		step1:取器件地址
 *		step2:读取时间的首字节地址(从秒开始读)
 *		step3:读七个时间信息
 *		step4:读取时间并放入接收缓冲区中
 */
static int pcf8563_get_datetime(struct i2c_client *client, struct rtc_time *tm)
{
	struct pcf8563 *pcf8563 = i2c_get_clientdata(client);
	unsigned char buf[13] = { PCF8563_REG_ST1 };

	struct i2c_msg msgs[] = {
		{ client->addr, 0, 1, buf },	/* setup read ptr */
		{ client->addr, I2C_M_RD, 13, buf },	/* read status + date */
	};

	/* read registers */
	if ((i2c_transfer(client->adapter, msgs, 2)) != 2) {
		dev_err(&client->dev, "%s: read error\n", __func__);
		return -EIO;
	}

	if (buf[PCF8563_REG_SC] & PCF8563_SC_LV)
		dev_info(&client->dev,
			"low voltage detected, date/time is not reliable.\n");

	dev_dbg(&client->dev,
		"%s: raw data is st1=%02x, st2=%02x, sec=%02x, min=%02x, hr=%02x, "
		"mday=%02x, wday=%02x, mon=%02x, year=%02x\n",
		__func__,
		buf[0], buf[1], buf[2], buf[3],
		buf[4], buf[5], buf[6], buf[7],
		buf[8]);


	tm->tm_sec = bcd2bin(buf[PCF8563_REG_SC] & 0x7F);
	tm->tm_min = bcd2bin(buf[PCF8563_REG_MN] & 0x7F);
	tm->tm_hour = bcd2bin(buf[PCF8563_REG_HR] & 0x3F); /* rtc hr 0-23 */
	tm->tm_mday = bcd2bin(buf[PCF8563_REG_DM] & 0x3F);
	tm->tm_wday = buf[PCF8563_REG_DW] & 0x07;
	tm->tm_mon = bcd2bin(buf[PCF8563_REG_MO] & 0x1F) - 1; /* rtc mn 1-12 */
	tm->tm_year = bcd2bin(buf[PCF8563_REG_YR]);
	if (tm->tm_year < 70)
		tm->tm_year += 100;	/* assume we are in 1970...2069 */
	/* detect the polarity heuristically. see note above. */
	pcf8563->c_polarity = (buf[PCF8563_REG_MO] & PCF8563_MO_C) ?
		(tm->tm_year >= 100) : (tm->tm_year < 100);

	dev_dbg(&client->dev, "%s: tm is secs=%d, mins=%d, hours=%d, "
		"mday=%d, mon=%d, year=%d, wday=%d\n",
		__func__,
		tm->tm_sec, tm->tm_min, tm->tm_hour,
		tm->tm_mday, tm->tm_mon, tm->tm_year, tm->tm_wday);

	/* the clock can give out invalid datetime, but we cannot return
	 * -EINVAL otherwise hwclock will refuse to set the time on bootup.
	 */
	if (rtc_valid_tm(tm) < 0)
		dev_err(&client->dev, "retrieved date/time is not valid.\n");

	return 0;
}

/*
*写时钟步骤：
*	step1:将时间装入发送缓冲区(首地址为50H)中
*	step2:取器件地址
*	step3:取写入寄存器的首地址(从00H开始写)
*	step4:写7个时间信息和2个控制命令
*	step5:写时间
*/
static int pcf8563_set_datetime(struct i2c_client *client, struct rtc_time *tm)
{
	struct pcf8563 *pcf8563 = i2c_get_clientdata(client);
	int i, err;
	unsigned char buf[9];

	dev_dbg(&client->dev, "%s: secs=%d, mins=%d, hours=%d, "
		"mday=%d, mon=%d, year=%d, wday=%d\n",
		__func__,
		tm->tm_sec, tm->tm_min, tm->tm_hour,
		tm->tm_mday, tm->tm_mon, tm->tm_year, tm->tm_wday);

	/* hours, minutes and seconds */
	buf[PCF8563_REG_SC] = bin2bcd(tm->tm_sec);
	buf[PCF8563_REG_MN] = bin2bcd(tm->tm_min);
	buf[PCF8563_REG_HR] = bin2bcd(tm->tm_hour);

	buf[PCF8563_REG_DM] = bin2bcd(tm->tm_mday);

	/* month, 1 - 12 */
	buf[PCF8563_REG_MO] = bin2bcd(tm->tm_mon + 1);

	/* year and century */
	buf[PCF8563_REG_YR] = bin2bcd(tm->tm_year % 100);
	if (pcf8563->c_polarity ? (tm->tm_year >= 100) : (tm->tm_year < 100))
		buf[PCF8563_REG_MO] |= PCF8563_MO_C;

	buf[PCF8563_REG_DW] = tm->tm_wday & 0x07;

	/* write register's data */
	for (i = 0; i < 7; i++) {
		unsigned char data[2] = { PCF8563_REG_SC + i,
						buf[PCF8563_REG_SC + i] };

		err = i2c_master_send(client, data, sizeof(data));
		if (err != sizeof(data)) {
			dev_err(&client->dev,
				"%s: err=%d addr=%02x, data=%02x\n",
				__func__, err, data[0], data[1]);
			return -EIO;
		}
	};

	return 0;
}
#if 0
static int pcf8563_get_alarmtime(struct i2c_client *client, struct rtc_time *tm)
{
	return 0;
}

static int pcf8563_set_alarmtime(struct i2c_client *client, struct rtc_time *tm)
{
	struct rtc_time tm_now;
	unsigned long time_now = 0;
	unsigned long time_set = 0;
	unsigned long time_gap = 0;
	unsigned long time_gap_day = 0;
	unsigned long time_gap_hour = 0;
	unsigned long time_gap_minute = 0;
	unsigned long time_gap_second = 0;   
	int ret = 0;	
	
#ifdef RTC_ALARM_DEBUG    
    printk("*****************************\n\n");
    printk("line:%d,%s the alarm time: year:%d, month:%d, day:%d. hour:%d.minute:%d.second:%d\n",\
    __LINE__, __func__, tm->tm_year, tm->tm_mon,\
    	 tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);	  
   	printk("*****************************\n\n");
#endif

    ret = pcf8563_get_datetime(client, &tm_now);
    
    ret = rtc_tm_to_time(tm, &time_set);
    ret = rtc_tm_to_time(&tm_now, &time_now);
    if(time_set <= time_now){
    	printk("The time or date can`t set, The day has pass!!!\n");
    	return -EINVAL;
    }
    time_gap = time_set - time_now;
    time_gap_day = time_gap/(3600*24);//day
    time_gap_hour = (time_gap - time_gap_day*24)/3600;//hour
    time_gap_minute = (time_gap - time_gap_day*24*60 - time_gap_hour*60)/60;//minute
    time_gap_second = time_gap - time_gap_day*24*60*60 - time_gap_hour*60*60-time_gap_minute*60;//second
    if(time_gap_day > 255) {
    	printk("The time or date can`t set, The day range of 0 to 255\n");
    	return -EINVAL;
    }

#ifdef RTC_ALARM_DEBUG  		  
   	printk("line:%d,%s year:%d, month:%d, day:%ld. hour:%ld.minute:%ld.second:%ld\n",\
    __LINE__, __func__, tm->tm_year, tm->tm_mon,\
    	 time_gap_day, time_gap_hour, time_gap_minute, time_gap_second);
    printk("*****************************\n\n");	
#endif
	return 0;
}
#endif
static int pcf8563_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
	return pcf8563_get_datetime(to_i2c_client(dev), tm);
}

static int pcf8563_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	return pcf8563_set_datetime(to_i2c_client(dev), tm);
}
#if 0
static int pcf8563_rtc_read_alarm(struct device *dev, struct rtc_time *tm)
{
	return pcf8563_get_alarmtime(to_i2c_client(dev), tm);
}

static int pcf8563_rtc_set_alarm(struct device *dev, struct rtc_time *tm)
{
	return pcf8563_set_alarmtime(to_i2c_client(dev), tm);
}
#endif
static const struct rtc_class_ops pcf8563_rtc_ops = {
	.read_time	= pcf8563_rtc_read_time,
	.set_time	= pcf8563_rtc_set_time,
//	.set_alarm	= pcf8563_rtc_set_alarm,
//	.read_alarm = pcf8563_rtc_read_alarm,
};

static int pcf8563_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct pcf8563 *pcf8563;	
	int err = 0;
//	sun5i_rtc_alarmno = SW_INT_IRQNO_ALARM;
	dev_dbg(&client->dev, "%s\n", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		return -ENODEV;

	pcf8563 = kzalloc(sizeof(struct pcf8563), GFP_KERNEL);
	if (!pcf8563)
		return -ENOMEM;

	dev_info(&client->dev, "chip found, driver version " DRV_VERSION "\n");

	i2c_set_clientdata(client, pcf8563);

	pcf8563->rtc = rtc_device_register(pcf8563_driver.driver.name,
				&client->dev, &pcf8563_rtc_ops, THIS_MODULE);

	if (IS_ERR(pcf8563->rtc)) {
		err = PTR_ERR(pcf8563->rtc);
		goto exit_kfree;
	}
//	err = request_irq(sun5i_rtc_alarmno, sun5ii_rtc_alarmirq,
//			  IRQF_DISABLED,  "sun5i-rtc alarm", pcf8563->rtc);
//	if (err) {
//		printk("IRQ%d error %d\n", sun5i_rtc_alarmno, err);
//		return err;
//	}
	return 0;

exit_kfree:
	kfree(pcf8563);

	return err;
}

static int pcf8563_remove(struct i2c_client *client)
{
	struct pcf8563 *pcf8563 = i2c_get_clientdata(client);

	if (pcf8563->rtc)
		rtc_device_unregister(pcf8563->rtc);

	kfree(pcf8563);

	return 0;
}

static const struct i2c_device_id pcf8563_id[] = {
	{ "pcf8563", 0 },
	{ "rtc8564", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, pcf8563_id);

static struct i2c_driver pcf8563_driver = {
	.driver		= {
		.name	= "rtc-pcf8563",
	},
	.probe		= pcf8563_probe,
	.remove		= pcf8563_remove,
	.id_table	= pcf8563_id,
};

static int __init pcf8563_init(void)
{
	return i2c_add_driver(&pcf8563_driver);
}

static void __exit pcf8563_exit(void)
{
	i2c_del_driver(&pcf8563_driver);
}

MODULE_AUTHOR("huangxin");
MODULE_DESCRIPTION("allwinner RTC driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);

module_init(pcf8563_init);
module_exit(pcf8563_exit);
