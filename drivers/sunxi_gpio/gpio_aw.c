#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <mach/sys_config.h>
#include "gpio_aw.h"
#define mul_sel_1	1
#define pull_2 		2
#define drv_level_3	3
#define data_4		4
#define port_5		5
#define port_num_6	6


int get_gpio_member_value( u32 gpio_hd , char *name , int member_name )
{
	user_gpio_set_t  gpio_info[1];
	int  ret;
    
    ret = gpio_get_one_pin_status(gpio_hd, gpio_info, name, 1);
    if(ret < 0){
        printk("fetch gpio infomation fail \n");
	}
	else{
        printk("fetch gpio infomation ok \n");
	}
	switch ( member_name )
		{
		case mul_sel_1:
		ret = gpio_info->mul_sel;
		break;
		case pull_2:	
		ret = gpio_info->pull;
		break;
		case drv_level_3:		
		ret = gpio_info->drv_level;
		break;
		case data_4:
		ret = gpio_info->data;
		break;
		case port_5:
		ret = gpio_info->port;
		break;
		case port_num_6:
		ret = gpio_info->port_num;
		break;	
		}
		
return ret;

}


struct gpio_aw {
	struct gpio_aw_classdev		cdev;
	struct gpio_aw_platdata		*pdata;
};

static inline struct gpio_aw *pdev_to_gpio(struct platform_device *dev)
{
	return platform_get_drvdata(dev);
}

static inline struct gpio_aw *to_gpio(struct gpio_aw_classdev *gpio_aw_cdev)
{
	return container_of(gpio_aw_cdev, struct gpio_aw, cdev);
}

static int	gpio_aw_cfg_set(struct gpio_aw_classdev *gpio_aw_cdev,int  mul_sel)
{
	struct gpio_aw *gpio = to_gpio(gpio_aw_cdev);
	struct gpio_aw_platdata *pd = gpio->pdata;
	int ret ;

	printk("attending gpio_aw_cfg_set \n");
	printk("pio_hdle is %x \n",pd->pio_hdle);

	ret =   gpio_set_one_pin_io_status(pd->pio_hdle, mul_sel, pd->name);
	if ( !ret )
	gpio_aw_cdev->mul_sel=mul_sel;
	printk("left gpio_aw_cfg_set \n");				
	return ret ;
}

static int	gpio_aw_pull_set(struct gpio_aw_classdev *gpio_aw_cdev,int  pull)
{
	struct gpio_aw *gpio = to_gpio(gpio_aw_cdev);
	struct gpio_aw_platdata *pd = gpio->pdata;
	int ret ;

	printk("attending gpio_aw_pull_set \n");
	printk("pio_hdle is %x \n",pd->pio_hdle);

	ret =   gpio_set_one_pin_pull (pd->pio_hdle, pull, pd->name);
	if ( !ret )
	gpio_aw_cdev->pull=pull;
	printk("left gpio_aw_pull_set \n");				
	return ret ;
}

static int	gpio_aw_data_set(struct gpio_aw_classdev *gpio_aw_cdev,int  data)
{
	struct gpio_aw *gpio = to_gpio(gpio_aw_cdev);
	struct gpio_aw_platdata *pd = gpio->pdata;
	int ret ;

	printk("attending gpio_aw_data_set \n");
	printk("pio_hdle is %x \n",pd->pio_hdle);

	ret =  gpio_write_one_pin_value (pd->pio_hdle, data, pd->name);
	if ( !ret )
	gpio_aw_cdev->data=data;
	printk("left gpio_aw_data_set \n");				
	return ret ;
}

static int	gpio_aw_drv_level_set(struct gpio_aw_classdev *gpio_aw_cdev,int  drv_level)
{
	struct gpio_aw *gpio = to_gpio(gpio_aw_cdev);
	struct gpio_aw_platdata *pd = gpio->pdata;
	int ret ;

	printk("attending gpio_aw_drv_level_set \n");
	printk("pio_hdle is %x \n",pd->pio_hdle);

	ret =  gpio_set_one_pin_driver_level (pd->pio_hdle, drv_level, pd->name);
	if ( !ret )
	gpio_aw_cdev->drv_level=drv_level;
	printk("left gpio_aw_drv_level_set \n");				
	return ret ;
}

static int	gpio_aw_cfg_get(struct gpio_aw_classdev *gpio_aw_cdev)
{
	struct gpio_aw *gpio = to_gpio(gpio_aw_cdev);
	struct gpio_aw_platdata *pd = gpio->pdata;
	int ret;
	ret=get_gpio_member_value( pd->pio_hdle , pd->name , mul_sel_1 );
	return ret;
}


static int	gpio_aw_pull_get(struct gpio_aw_classdev *gpio_aw_cdev)
{
	struct gpio_aw *gpio = to_gpio(gpio_aw_cdev);
	struct gpio_aw_platdata *pd = gpio->pdata;
	int ret;
	ret=get_gpio_member_value( pd->pio_hdle , pd->name , pull_2 );
	return ret;
}

static int	gpio_aw_data_get(struct gpio_aw_classdev *gpio_aw_cdev)
{
	struct gpio_aw *gpio = to_gpio(gpio_aw_cdev);
	struct gpio_aw_platdata *pd = gpio->pdata;
	int ret;
	ret=get_gpio_member_value( pd->pio_hdle , pd->name , data_4 );
	return ret;
}

static int	gpio_aw_drv_level_get(struct gpio_aw_classdev *gpio_aw_cdev)
{
	struct gpio_aw *gpio = to_gpio(gpio_aw_cdev);
	struct gpio_aw_platdata *pd = gpio->pdata;
	int ret;
	ret=get_gpio_member_value( pd->pio_hdle , pd->name , drv_level_3 );
	return ret;
}







static int  gpio_aw_put_resource(struct gpio_aw *agpio){
	struct  gpio_aw_platdata *pd = agpio->pdata;
	 printk("attending gpio_aw_put_resource \n");
	 printk("pio_hdle is %x \n",pd->pio_hdle);
	gpio_release(pd->pio_hdle, 1);
	return 0;
}



static int gpio_aw_remove(struct platform_device *dev)
{
	struct gpio_aw *gpio = pdev_to_gpio(dev);
	struct gpio_aw_platdata *pdata = dev->dev.platform_data;
	printk("pio_hdle is %x \n",pdata->pio_hdle);
	gpio_aw_classdev_unregister(&gpio->cdev);
	printk("gpio_aw_classdev_unregister ok !\n");
	gpio_aw_put_resource(gpio);
	printk("gpio_aw_put_resource ok !\n");
	printk("pdata->name is %s \n",pdata->name);
	
	//kfree(pdata);
	pdata = NULL;
	printk("kfree  pdata ok !\n");
	kfree(gpio);
	gpio = NULL;
	printk("kfree ok !\n");
	

	return 0;
}


static int gpio_aw_probe(struct platform_device *dev)
{
	struct gpio_aw_platdata *pdata = dev->dev.platform_data;
	struct gpio_aw *gpio;
	
	int ret;

	gpio = kzalloc(sizeof(struct gpio_aw), GFP_KERNEL);
	printk("kzalloc ok !\n");
	gpio->pdata = kzalloc(sizeof(struct gpio_aw_platdata), GFP_KERNEL);
	if (gpio == NULL) {
		dev_err(&dev->dev, "No memory for device\n");
		return -ENOMEM;
	}
	
	platform_set_drvdata(dev, gpio);
	printk("platform_set_drvdata ok !\n");
	//aw_led_get_resource(led);

	pdata->pio_hdle = gpio_request_ex("gpio", pdata->name);
	gpio->cdev.port = get_gpio_member_value( pdata->pio_hdle , pdata->name , port_5 );
	gpio->cdev.port_num = get_gpio_member_value( pdata->pio_hdle , pdata->name , port_num_6 );
	gpio->cdev.mul_sel = get_gpio_member_value( pdata->pio_hdle , pdata->name , mul_sel_1 );
	gpio->cdev.pull = get_gpio_member_value( pdata->pio_hdle , pdata->name , pull_2 );
	gpio->cdev.drv_level = get_gpio_member_value( pdata->pio_hdle , pdata->name , drv_level_3 );
	gpio->cdev.data = get_gpio_member_value( pdata->pio_hdle , pdata->name , data_4 );
	
	gpio->cdev.gpio_aw_cfg_set = gpio_aw_cfg_set;
	gpio->cdev.gpio_aw_pull_set = gpio_aw_pull_set;
	gpio->cdev.gpio_aw_data_set = gpio_aw_data_set;
	gpio->cdev.gpio_aw_drv_level_set = gpio_aw_drv_level_set;
	
	gpio->cdev.gpio_aw_cfg_get = gpio_aw_cfg_get;
	gpio->cdev.gpio_aw_pull_get = gpio_aw_pull_get;
	gpio->cdev.gpio_aw_data_get = gpio_aw_data_get;
	gpio->cdev.gpio_aw_drv_level_get = gpio_aw_drv_level_get;
	
	gpio->cdev.name = pdata->name;
	gpio->cdev.flags |= pdata->flags;
	gpio->pdata = pdata;
	

	ret = gpio_aw_classdev_register(&dev->dev, &gpio->cdev);
	printk("gpio_aw_classdev_register ok !\n");
	if (ret < 0) {
		dev_err(&dev->dev, "gpio_aw_classdev_register failed\n");
		kfree(gpio);
		return ret;
	}
	printk("pio_hdle is %x \n",pdata->pio_hdle);
	printk("gpio_aw_classdev_register good !\n");
	return 0;
}
static void gpio_aw_release (struct device *dev)
{
}
static struct platform_driver gpio_aw_driver = {
	.probe		= gpio_aw_probe,
	.remove		= gpio_aw_remove,
	
	.driver		= {
		.name		= "gpio_aw",
		.owner		= THIS_MODULE,
	},
};
#if 1
static struct gpio_aw_platdata pdateaw[] = {
	[0]={
		.flags = AW_GPIO_CORE_SUSPENDED,
		.name = "gpio_aw_first",
	},	
	[1]={
		.flags = AW_GPIO_CORE_SUSPENDED,
		.name = "gpio_aw_second",
	},
	[2]={
		.flags = AW_GPIO_CORE_SUSPENDED,
		.name = "gpio_aw_third",
	},
	[3]={
		NULL,
	},
};
#endif
static struct platform_device gpio_aw_dev[] = {
	[0]={
		.name = "gpio_aw",
		.id = 0, 
		.dev = { 
			.platform_data = &pdateaw[0],	
			.release    = gpio_aw_release,
		}
	},
	[1]={
		.name = "gpio_aw",
		.id = 1, 
		.dev = { 
			.platform_data = &pdateaw[1],	
			.release    = gpio_aw_release,
		}
	},
	[2]={
		.name = "gpio_aw",
		.id = 2, 
		.dev = { 
			.platform_data = &pdateaw[2],	
			.release    = gpio_aw_release,
		}
	},
	[3]={
	},
};


static int __init gpio_aw_init(void)
{
	int i;
	for(i=0;i<2;i++)
	platform_device_register(&gpio_aw_dev[i]);	
	platform_driver_register(&gpio_aw_driver);
	
	
	return 0;
}

static void __exit gpio_aw_exit(void)
{
	int i;
	platform_driver_unregister(&gpio_aw_driver);
	for(i=0;i<2;i++)
	platform_device_unregister(&gpio_aw_dev[i]);
}

module_init(gpio_aw_init);
module_exit(gpio_aw_exit);

MODULE_AUTHOR("panlong <panlong@allwinnertech.com>");
MODULE_DESCRIPTION("AW GPIO driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:gpio_aw");
