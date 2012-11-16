/*
 *  lis3dh.c - Linux kernel modules for 3-Axis Orientation/Motion
 *  Detection Sensor 
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/hwmon.h>
#include <linux/input-polldev.h>
#include <linux/device.h>
#include <linux/earlysuspend.h>

#include <mach/system.h>
#include <mach/hardware.h>
#include <mach/sys_config.h>

/*
 * Defines
 */
#define assert(expr)\
	if (!(expr)) {\
		printk(KERN_ERR "Assertion failed! %s,%d,%s,%s\n",\
			__FILE__, __LINE__, __func__, #expr);\
	}

#define LIS3DH_DRV_NAME	"lis3dh"
#define SENSOR_NAME 			LIS3DH_DRV_NAME


#define POLL_INTERVAL_MAX	500
#define POLL_INTERVAL		50
#define INPUT_FUZZ	2
#define INPUT_FLAT	2


#define WHO_AM_I		0x0F	/*	WhoAmI register		*/
#define WHOAMI_LIS3DH_ACC	0x33


#define MODE_CHANGE_DELAY_MS 100

static struct device *hwmon_dev;
static struct i2c_client *lis3dh_i2c_client;

struct lis3dh_data_s {
    struct i2c_client       *client;
    struct input_polled_dev *pollDev; 
    struct mutex interval_mutex; 
    
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
	volatile int suspend_indator;
#endif
} lis3dh_data;
static struct input_polled_dev *lis3dh_idev;

/* Addresses to scan */
static const unsigned short normal_i2c[2] = {0x1d,I2C_CLIENT_END};
static const unsigned short i2c_address[2] ={0x1c,0x1d};
static __u32 twi_id = 0;
static int i2c_num = 0;

#ifdef CONFIG_HAS_EARLYSUSPEND
static void lis3dh_early_suspend(struct early_suspend *h);
static void lis3dh_late_resume(struct early_suspend *h);
#endif


/**
 * gsensor_fetch_sysconfig_para - get config info from sysconfig.fex file.
 * return value:  
 *                    = 0; success;
 *                    < 0; err
 */
static int gsensor_fetch_sysconfig_para(void)
{
	int ret = -1;
	int device_used = -1;
		
	printk("========%s===================\n", __func__);
	 
	if(SCRIPT_PARSER_OK != (ret = script_parser_fetch("gsensor_para", "gsensor_used", &device_used, 1))){
	                pr_err("%s: script_parser_fetch err.ret = %d. \n", __func__, ret);
	                goto script_parser_fetch_err;
	}
	if(1 == device_used){
		if(SCRIPT_PARSER_OK != script_parser_fetch("gsensor_para", "gsensor_twi_id", &twi_id, 1)){
			pr_err("%s: script_parser_fetch err. \n", __func__);
			goto script_parser_fetch_err;
		}
		printk("%s: twi_id is %d. \n", __func__, twi_id);
		ret = 0;
		
	}else{
		pr_err("%s: gsensor_unused. \n",  __func__);
		ret = -1;
	}

	return ret;

script_parser_fetch_err:
	pr_notice("=========script_parser_fetch_err============\n");
	return ret;

}
//Function as i2c_master_send, and return 1 if operation is successful. 
static int i2c_write_bytes(struct i2c_client *client, uint8_t *data, uint16_t len)
{
	struct i2c_msg msg;
	int ret=-1;
	
	msg.flags = !I2C_M_RD;
	msg.addr = client->addr;
	msg.len = len;
	msg.buf = data;		
	
	ret=i2c_transfer(client->adapter, &msg,1);
	return ret;
}

//Function as i2c_master_receive, and return 2 if operation is successful.
static int i2c_read_bytes(struct i2c_client *client, uint8_t *buf, uint16_t len)
{
	struct i2c_msg msgs[2];
	int ret=-1;
	//发送写地址
	msgs[0].flags = !I2C_M_RD;
	msgs[0].addr = client->addr;
	msgs[0].len = 1;		//data address
	msgs[0].buf = buf;
	//接收数据
	msgs[1].flags = I2C_M_RD;//读消息
	msgs[1].addr = client->addr;
	msgs[1].len = len-1;
	msgs[1].buf = buf+1;
	
	ret=i2c_transfer(client->adapter, msgs, 2);
	return ret;
}
static bool gsensor_i2c_test(struct i2c_client * client)
{
	int ret, retry;
	uint8_t test_data[1] = { 0 };	//only write a data address.
	
	for(retry=0; retry < 2; retry++)
	{
		ret =i2c_write_bytes(client, test_data, 1);	//Test i2c.
		if (ret == 1)
			break;
		msleep(5);
	}
	
	return ret==1 ? true : false;
}

/**
 * gsensor_detect - Device detection callback for automatic device creation
 * return value:  
 *                    = 0; success;
 *                    < 0; err
 */
int gsensor_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;
	int ret;

        if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
                return -ENODEV;
    
	if(twi_id == adapter->nr){
	        while(i2c_num < 2) {
                        client->addr = i2c_address[i2c_num++];
                        pr_info("%s: addr= %x\n",__func__,client->addr);
                        ret = gsensor_i2c_test(client);
                        if(ret){   
                                pr_info("I2C connection sucess!\n");
                                strlcpy(info->type, SENSOR_NAME, I2C_NAME_SIZE);
                                return 0;	
                        }
 
                }
          pr_info("%s: lis3dh  equipment is not found!\n",__func__);
          return  -ENODEV;
              
	}else{
		return -ENODEV;
	}
}


static ssize_t lis3dh_enable_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;

	error = strict_strtoul(buf, 10, &data);
	
	if(error) {
		pr_err("%s strict_strtoul error\n", __FUNCTION__);
		goto exit;
	}

	if(data) {
          lis3dh_data.suspend_indator = 0;
	      error = i2c_smbus_write_byte_data(lis3dh_i2c_client, 0x20, 0x67);
		assert(error==0);
	} else {
           lis3dh_data.suspend_indator = 1;
	       error = i2c_smbus_write_byte_data(lis3dh_i2c_client, 0x20, 0x00);
		assert(error==0);
	}

	return count;

exit:
	return error;
}

static ssize_t lis3dh_delay_store(struct device *dev,struct device_attribute *attr,
		const char *buf, size_t count)
{
   unsigned long data;
	int error;

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	if (data > POLL_INTERVAL_MAX)
		data = POLL_INTERVAL_MAX;
	lis3dh_idev->poll_interval = data;

	return count;
		}

static DEVICE_ATTR(enable, 0666,NULL, lis3dh_enable_store);

static DEVICE_ATTR(delay, 0666,NULL, lis3dh_delay_store);

static struct attribute *lis3dh_attributes[] = {
	&dev_attr_enable.attr,
	&dev_attr_delay.attr,
	NULL
};

static struct attribute_group lis3dh_attribute_group = {
	.attrs = lis3dh_attributes
};


static int lis3dh_acc_hw_init(struct i2c_client *client)
{
	int err = -1;
	u8 buf[7];

	printk("hw init start\n");

	buf[0] = WHO_AM_I;
	err = i2c_read_bytes(client, buf, 2);
	if (err < 0) {
	printk("Error reading WHO_AM_I: is device available/working?\n");
		goto err_firstread;
	}
	if (buf[1] != WHOAMI_LIS3DH_ACC) {
	printk("device unknown. Expected: 0x%x, Replies: 0x%x\n", WHOAMI_LIS3DH_ACC, buf[1]);

	}

	buf[0] = 0x20;
	buf[1] = 0x07;
	err = i2c_write_bytes(client, buf, 2);
	if (err < 0)
		goto err_resume_state;

	buf[0] = 0x1F;
	buf[1] = 0x00;
	err = i2c_write_bytes(client, buf, 2);
	if (err < 0)
		goto err_resume_state;

	buf[0] = 0x2E;
	buf[1] = 0x00;
	err = i2c_write_bytes(client, buf, 2);
	if (err < 0)
		goto err_resume_state;

	buf[0] = (0x80 | 0x3a);
	buf[1] = 0x00;
	buf[2] = 0x00;
	buf[3] = 0x00;
	buf[4] = 0x00;
	err = i2c_write_bytes(client, buf, 5);
	if (err < 0)
		goto err_resume_state;
	buf[0] = 0x38;
	buf[1] = 0x00;
	err = i2c_write_bytes(client, buf, 2);
	if (err < 0)
		goto err_resume_state;

	buf[0] = (0x80 | 0x32);
	buf[1] = 0x00;
	buf[2] = 0x00;
	err = i2c_write_bytes(client, buf, 3);
	if (err < 0)
		goto err_resume_state;
	buf[0] = 0x30;
	buf[1] = 0x00;
	err = i2c_write_bytes(client, buf, 2);
	if (err < 0)
		goto err_resume_state;


	buf[0] = (0x80 | 0x21);
	buf[1] = 0x00;
	buf[2] = 0x00;
	buf[3] = 0x00;
	buf[4] = 0x00;
	buf[5] = 0x00;
	err = i2c_write_bytes(client, buf, 6);
	if (err < 0)
		goto err_resume_state;

	
	printk("hw init done\n");
	return 0;

err_firstread:


err_resume_state:
	printk("hw init error 0x%x,0x%x: %d\n", buf[0],buf[1], err);
	return err;
}

static void report_abs(void)
{
	int i;
	s8 xyz[7]; 
	s16 x, y, z;
	i = 0x80 | 0x28;
    //printk("=====%s:====",__func__);

    i2c_smbus_read_i2c_block_data(lis3dh_i2c_client, i, 6, xyz);
    //i2c_read_bytes(mma7660_i2c_client, xyz, sizeof(xyz));
//	for(i=0; i<7; i++)
//        printk("xyz[%d]:%d, ",i,xyz[i]);
//    printk("\n");

	/* convert signed 8bits to signed 16bits */
	x = (xyz[1] << 8) >> 8;
	y = (xyz[3] << 8) >> 8;
	z = (xyz[5] << 8) >> 8;
	//pr_info("xyz[0] = 0x%hx, xyz[1] = 0x%hx, xyz[2] = 0x%hx. \n", xyz[0], xyz[1], xyz[2]);
	//pr_info("x[0] = 0x%hx, y[1] = 0x%hx, z[2] = 0x%hx. \n", x, y, z);
	
	input_report_abs(lis3dh_idev->input, ABS_X, x);
	input_report_abs(lis3dh_idev->input, ABS_Y, y);
	input_report_abs(lis3dh_idev->input, ABS_Z, z);

	input_sync(lis3dh_idev->input);
}

static void lis3dh_dev_poll(struct input_polled_dev *dev)
{
#ifdef CONFIG_HAS_EARLYSUSPEND
	if(0 == lis3dh_data.suspend_indator){
		report_abs();
	}
#else
	report_abs();
#endif
} 

/*
 * I2C init/probing/exit functions
 */

static int __devinit lis3dh_probe(struct i2c_client *client,
				   const struct i2c_device_id *id)
{
	int result;
	struct input_dev *idev;
	struct i2c_adapter *adapter;
        struct lis3dh_data_s* data = &lis3dh_data;
 
       client->addr = i2c_address[i2c_num - 1];
	printk("lis3dh probe\n");
	lis3dh_i2c_client = client;
	adapter = to_i2c_adapter(client->dev.parent);
 	result = i2c_check_functionality(adapter,
 					 I2C_FUNC_SMBUS_BYTE |
 					 I2C_FUNC_SMBUS_BYTE_DATA);
	assert(result);

	/* Initialize the MMA7660 chip */
	result = lis3dh_acc_hw_init(client);
	assert(result==0);
	
	//result = 1; // debug by lchen
//	printk("<%s> mma7660_init_client result %d\n", __func__, result);
//	if(result != 0)
//	{
//		printk("<%s> init err !", __func__);
//		return result;
//	}
    result = i2c_smbus_write_byte_data(client, 0x20, 0x67);
		assert(result==0);
		
	hwmon_dev = hwmon_device_register(&client->dev);
	assert(!(IS_ERR(hwmon_dev)));

	dev_info(&client->dev, "build time %s %s\n", __DATE__, __TIME__);
  
	/*input poll device register */
	lis3dh_idev = input_allocate_polled_device();
	if (!lis3dh_idev) {
		dev_err(&client->dev, "alloc poll device failed!\n");
		result = -ENOMEM;
		return result;
	}
	lis3dh_idev->poll = lis3dh_dev_poll;
	lis3dh_idev->poll_interval = POLL_INTERVAL;
	lis3dh_idev->poll_interval_max = POLL_INTERVAL_MAX;
	idev = lis3dh_idev->input;
	idev->name = LIS3DH_DRV_NAME;
	idev->id.bustype = BUS_I2C;
	idev->evbit[0] = BIT_MASK(EV_ABS);
    mutex_init(&data->interval_mutex);

	input_set_abs_params(idev, ABS_X, -16000, 16000, 0, 0);
	input_set_abs_params(idev, ABS_Y, -16000, 16000, 0, 0);
	input_set_abs_params(idev, ABS_Z, -16000, 16000, 0, 0);
	
	result = input_register_polled_device(lis3dh_idev);
	if (result) {
		dev_err(&client->dev, "register poll device failed!\n");
		return result;
	}
	result = sysfs_create_group(&lis3dh_idev->input->dev.kobj, &lis3dh_attribute_group);
	//result = device_create_file(&mma7660_idev->input->dev, &dev_attr_enable);
	//result = device_create_file(&mma7660_idev->input->dev, &dev_attr_value);

	if(result) {
		dev_err(&client->dev, "create sys failed\n");
	}

    data->client  = client;
    data->pollDev = lis3dh_idev;
	i2c_set_clientdata(client, data);

#ifdef CONFIG_HAS_EARLYSUSPEND
	lis3dh_data.early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	lis3dh_data.early_suspend.suspend = lis3dh_early_suspend;
	lis3dh_data.early_suspend.resume = lis3dh_late_resume;
	register_early_suspend(&lis3dh_data.early_suspend);
	lis3dh_data.suspend_indator = 0;
#endif

	return result;
}

static int __devexit lis3dh_remove(struct i2c_client *client)
{
	int result= 0;

	hwmon_device_unregister(hwmon_dev);
	#ifdef CONFIG_HAS_EARLYSUSPEND	
	  unregister_early_suspend(&lis3dh_data.early_suspend);
	#endif
	input_unregister_polled_device(lis3dh_idev);
	input_free_polled_device(lis3dh_idev);
	i2c_set_clientdata(lis3dh_i2c_client, NULL);

	return result;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void lis3dh_early_suspend(struct early_suspend *h)
{
	int result;
	printk(KERN_INFO "lis3dh early suspend\n");
	lis3dh_data.suspend_indator = 1;
	result = i2c_smbus_write_byte_data(lis3dh_i2c_client, 0x20, 0x00);
//	result = i2c_smbus_write_byte_data(mma7660_i2c_client, 
//		MMA7660_MODE, MK_MMA7660_MODE(0, 0, 0, 0, 0, 0, 0));
	assert(result==0);
	return;
}

static void lis3dh_late_resume(struct early_suspend *h)
{
	int result;
	printk(KERN_INFO "mma7660 late resume\n");
	lis3dh_data.suspend_indator = 0;
	result = i2c_smbus_write_byte_data(lis3dh_i2c_client, 0x20, 0x67);
//	result = i2c_smbus_write_byte_data(mma7660_i2c_client, 
//		MMA7660_MODE, MK_MMA7660_MODE(0, 1, 0, 0, 0, 0, 1));
	assert(result==0);
	return;
}
#endif /* CONFIG_HAS_EARLYSUSPEND */

static const struct i2c_device_id lis3dh_id[] = {
	{ LIS3DH_DRV_NAME, 1 },
	{ }
};
MODULE_DEVICE_TABLE(i2c,lis3dh_id);

static struct i2c_driver lis3dh_driver = {
	.class = I2C_CLASS_HWMON,
	.driver = {
		.name	= LIS3DH_DRV_NAME,
		.owner	= THIS_MODULE,
	},
	.probe	= lis3dh_probe,
	.remove	= __devexit_p(lis3dh_remove),
	.id_table = lis3dh_id,
	.address_list	= normal_i2c,
};

static int __init lis3dh_init(void)
{
	int ret = -1;
	printk("======%s=========. \n", __func__);
	
	if(gsensor_fetch_sysconfig_para()){
		printk("%s: err.\n", __func__);
		return -1;
	}

	printk("%s: after fetch_sysconfig_para:  normal_i2c: 0x%hx. normal_i2c[1]: 0x%hx \n", \
	__func__, normal_i2c[0], normal_i2c[1]);

	lis3dh_driver.detect = gsensor_detect;


	ret = i2c_add_driver(&lis3dh_driver);
	if (ret < 0) {
		printk("add lis3dh i2c driver failed\n");
		return -ENODEV;
	}
	printk( "add lis3dhi2c driver\n");
	


	return ret;
}

static void __exit lis3dh_exit(void)
{
	printk("remove lis3dhi2c driver.\n");
	sysfs_remove_group(&lis3dh_idev->input->dev.kobj, &lis3dh_attribute_group);
	i2c_del_driver(&lis3dh_driver);
}

MODULE_AUTHOR("Emma yin");
MODULE_DESCRIPTION("lis3dh 3-Axis Orientation/Motion Detection Sensor driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.1");

module_init(lis3dh_init);
module_exit(lis3dh_exit);

