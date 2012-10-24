/*
 * aw1636-gpio.c  --  gpiolib support for X-powers &axp PMICs
 *
 * Copyright 2011 X-powers Microelectronics PLC.
 *
 * Author: Kyle Cheung <kylecheung@x-powers.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/mfd/core.h>
#include <linux/platform_device.h>
#include <linux/seq_file.h>
#include <linux/i2c.h>
#include <linux/mfd/axp-mfd.h>

#include "axp-gpio.h"

struct virtual_gpio_data {
	struct mutex lock;
	int gpio;				//gpio number : 0/1
	int io;                 //0: input      1: output
	int value;				//0: low        1: high
};

int axp_gpio_set_io(int gpio, int io_mode)
{
	if(io_mode == 1){
		switch(gpio)
		{
			case 0: return axp_clr_bits(AW1636_GPIO0_CFG, 0x06);
			case 1: return axp_clr_bits(AW1636_GPIO1_CFG, 0x06);
			default:return -ENXIO;
		}
	}
	else if(io_mode == 0){
		switch(gpio)
		{
			case 0: axp_clr_bits(AW1636_GPIO0_CFG,0x05);
					return axp_set_bits(AW1636_GPIO0_CFG,0x02);
			case 1: axp_clr_bits(AW1636_GPIO1_CFG,0x05);
					return axp_set_bits(AW1636_GPIO1_CFG,0x02);
			default:return -ENXIO;
		}
	}
	return -EINVAL;
}
EXPORT_SYMBOL_GPL(axp_gpio_set_io);


int axp_gpio_get_io(int gpio, int *io_mode)
{
	uint8_t val;
	switch(gpio)
	{
		case 0: axp_read(AW1636_GPIO0_CFG,&val);val &= 0x07;
				if(val < 0x02)
					*io_mode = 1;
				else if (val == 0x02)
					*io_mode = 0;
				else
					return -EIO;
				break;
		case 1: axp_read(AW1636_GPIO1_CFG,&val);val &= 0x07;
				if(val < 0x02)
					*io_mode = 1;
				else if (val == 0x02)
					*io_mode = 0;
				else
					return -EIO;
				break;
		default:return -ENXIO;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(axp_gpio_get_io);


int axp_gpio_set_value(int gpio, int value)
{
	int io_mode,ret;
	ret = axp_gpio_get_io(gpio,&io_mode);
	if(ret)
		return ret;
	if(io_mode){
		if(value){
			switch(gpio)
			{
				case 0: axp_clr_bits(AW1636_GPIO0_CFG,0x06);
						return axp_set_bits(AW1636_GPIO0_CFG,0x01);
				case 1: axp_clr_bits(AW1636_GPIO1_CFG,0x06);
						return axp_set_bits(AW1636_GPIO1_CFG,0x01);
				default:break;
			}
		}
		else{
			switch(gpio)
			{
				case 0: return axp_clr_bits(AW1636_GPIO0_CFG,0x03);
				case 1: return axp_clr_bits(AW1636_GPIO1_CFG,0x03);
				default:break;
			}
		}
		return -ENXIO;
	}
	return -ENXIO;
}
EXPORT_SYMBOL_GPL(axp_gpio_set_value);


int axp_gpio_get_value(int gpio, int *value)
{
	int io_mode;
	int ret;
	uint8_t val;
	ret = axp_gpio_get_io(gpio,&io_mode);
	if(ret)
		return ret;
	if(io_mode){
		switch(gpio)
		{
			case 0:ret = axp_read(AW1636_GPIO0_CFG,&val);*value = val & 0x01;break;
			case 1:ret = axp_read(AW1636_GPIO1_CFG,&val);*value = val & 0x01;break;
			default:return -ENXIO;
		}
	}
	else{
		switch(gpio)
		{
			case 0:ret = axp_read(AW1636_GPIO01_STATE,&val);break;
			case 1:ret = axp_read(AW1636_GPIO01_STATE,&val);*value = val>>1;break;
			default:return -ENXIO;
		}
	}
	return ret;
}
EXPORT_SYMBOL_GPL(axp_gpio_get_value);

static ssize_t show_gpio(struct device *dev,
			   struct device_attribute *attr, char *buf)
{
	struct virtual_gpio_data *data = dev_get_drvdata(dev);
	return sprintf(buf, "%d\n", data->gpio);
}

static ssize_t set_gpio(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	struct virtual_gpio_data *data = dev_get_drvdata(dev);
	long val;

	if (strict_strtol(buf, 10, &val) != 0)
		return count;

	data->gpio = val;

	return count;
}

static ssize_t show_io(struct device *dev,
			   struct device_attribute *attr, char *buf)
{
	struct virtual_gpio_data *data = dev_get_drvdata(dev);
	int ret;
	mutex_lock(&data->lock);

	ret = axp_gpio_get_io(data->gpio,&data->io);

	mutex_unlock(&data->lock);

	if(ret)
		return ret;

	return sprintf(buf, "%d\n", data->io);
}

static ssize_t set_io(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	struct virtual_gpio_data *data = dev_get_drvdata(dev);
	long val;
	int ret;

	if (strict_strtol(buf, 10, &val) != 0)
		return count;

	mutex_lock(&data->lock);

	data->io = val;
	ret = axp_gpio_set_io(data->gpio,data->io);

	mutex_unlock(&data->lock);
	if(ret)
		return ret;
	return count;
}


static ssize_t show_value(struct device *dev,
			   struct device_attribute *attr, char *buf)
{
	struct virtual_gpio_data *data = dev_get_drvdata(dev);
	int ret;

	mutex_lock(&data->lock);

	ret = axp_gpio_get_value(data->gpio,&data->value);

	mutex_unlock(&data->lock);

	if(ret)
		return ret;

	return sprintf(buf, "%d\n", data->value);
}

static ssize_t set_value(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	struct virtual_gpio_data *data = dev_get_drvdata(dev);
	long val;
	int ret;

	if (strict_strtol(buf, 10, &val) != 0)
		return count;

	mutex_lock(&data->lock);

	data->value = val;
	ret = axp_gpio_set_value(data->gpio,data->value);

	mutex_unlock(&data->lock);

	if(ret){
		return ret;
	}

	return count;
}


static DEVICE_ATTR(gpio,0664, show_gpio, set_gpio);
static DEVICE_ATTR(io, 0664, show_io, set_io);
static DEVICE_ATTR(value, 0664, show_value, set_value);

struct device_attribute *attributes[] = {
	&dev_attr_gpio,
	&dev_attr_io,
	&dev_attr_value,
};


static int __devinit axp_gpio_probe(struct platform_device *pdev)
{
	//struct axp_mfd_chip *axp_chip = dev_get_drvdata(pdev->dev.parent);
	struct virtual_gpio_data *drvdata;
	int ret, i;

	drvdata = kzalloc(sizeof(struct virtual_gpio_data), GFP_KERNEL);
	if (drvdata == NULL) {
		ret = -ENOMEM;
		goto err;
	}

	mutex_init(&drvdata->lock);

	for (i = 0; i < ARRAY_SIZE(attributes); i++) {
		ret = device_create_file(&pdev->dev, attributes[i]);
		if (ret != 0)
			goto err;
	}

	platform_set_drvdata(pdev, drvdata);

	return 0;

err:
	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		device_remove_file(&pdev->dev, attributes[i]);
	kfree(drvdata);
	return ret;

return 0;
}

static int __devexit axp_gpio_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver axp_gpio_driver = {
	.driver.name	= "AW1636-gpio",
	.driver.owner	= THIS_MODULE,
	.probe		= axp_gpio_probe,
	.remove		= __devexit_p(axp_gpio_remove),
};

static int __init axp_gpio_init(void)
{
	return platform_driver_register(&axp_gpio_driver);
}
subsys_initcall(axp_gpio_init);

static void __exit axp_gpio_exit(void)
{
	platform_driver_unregister(&axp_gpio_driver);
}
module_exit(axp_gpio_exit);

MODULE_AUTHOR("Kyle Cheung");
MODULE_DESCRIPTION("GPIO interface for AXP PMICs");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:axp-gpio");
