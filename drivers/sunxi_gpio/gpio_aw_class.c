#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/sysdev.h>
#include <linux/timer.h>
#include <linux/err.h>
#include <linux/ctype.h>
#include <mach/sys_config.h>
#include "gpio_aw.h"
#include "gpio_aw_class.h"
static struct class *gpio_aw_class;

static ssize_t mul_sel_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct gpio_aw_classdev *gpio_aw_cdev = dev_get_drvdata(dev);

	return sprintf(buf, "%u\n", gpio_aw_cdev->mul_sel);
}

static ssize_t pull_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct gpio_aw_classdev *gpio_aw_cdev = dev_get_drvdata(dev);

	return sprintf(buf, "%u\n", gpio_aw_cdev->pull);
}

static ssize_t drv_level_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct gpio_aw_classdev *gpio_aw_cdev = dev_get_drvdata(dev);

	return sprintf(buf, "%u\n", gpio_aw_cdev->drv_level);
}

static ssize_t data_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct gpio_aw_classdev *gpio_aw_cdev = dev_get_drvdata(dev);

	return sprintf(buf, "%u\n", gpio_aw_cdev->data);
}

static ssize_t mul_sel_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct gpio_aw_classdev *gpio_aw_cdev = dev_get_drvdata(dev);
	ssize_t ret = -EINVAL;
	char *after;
	int in_out = simple_strtoul(buf, &after, 10);
	size_t count = after - buf;

	if (isspace(*after))
		count++;

	if (count == size) {
		ret = count;
		
		mul_sel_set(gpio_aw_cdev, in_out);
	}

	return ret;
}


static ssize_t pull_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct gpio_aw_classdev *gpio_aw_cdev = dev_get_drvdata(dev);
	ssize_t ret = -EINVAL;
	char *after;
	int pull = simple_strtoul(buf, &after, 10);
	size_t count = after - buf;

	if (isspace(*after))
		count++;

	if (count == size) {
		ret = count;
		
		pull_set(gpio_aw_cdev, pull);
	}

	return ret;
}

static ssize_t drv_level_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct gpio_aw_classdev *gpio_aw_cdev = dev_get_drvdata(dev);
	ssize_t ret = -EINVAL;
	char *after;
	int drv_level = simple_strtoul(buf, &after, 10);
	size_t count = after - buf;

	if (isspace(*after))
		count++;

	if (count == size) {
		ret = count;
		
		drv_level_set(gpio_aw_cdev, drv_level);
	}

	return ret;
}


static ssize_t data_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct gpio_aw_classdev *gpio_aw_cdev = dev_get_drvdata(dev);
	ssize_t ret = -EINVAL;
	char *after;
	int data = simple_strtoul(buf, &after, 10);
	size_t count = after - buf;

	if (isspace(*after))
		count++;

	if (count == size) {
		ret = count;
		
		data_set(gpio_aw_cdev, data);
	}

	return ret;
}

void gpio_aw_classdev_suspend(struct gpio_aw_classdev *gpio_aw_cdev)
{
	gpio_aw_cdev->flags |= AW_GPIO_SUSPENDED;
	printk("gpio_aw_classdev_suspend OK!");
}
EXPORT_SYMBOL_GPL(gpio_aw_classdev_suspend);

void gpio_aw_classdev_resume(struct gpio_aw_classdev *gpio_aw_cdev)
{
	printk("gpio_aw_classdev_resume OK!");
	gpio_aw_cdev->flags &= ~AW_GPIO_SUSPENDED;
}
EXPORT_SYMBOL_GPL(gpio_aw_classdev_resume);

static int gpio_aw_suspend(struct device *dev, pm_message_t state)
{
	struct gpio_aw_classdev *gpio_aw_cdev = dev_get_drvdata(dev);

	if (gpio_aw_cdev->flags & AW_GPIO_CORE_SUSPENDED)
		gpio_aw_classdev_suspend(gpio_aw_cdev);

	return 0;
}

static int gpio_aw_resume(struct device *dev)
{
	struct gpio_aw_classdev *gpio_aw_cdev = dev_get_drvdata(dev);

	if (gpio_aw_cdev->flags & AW_GPIO_CORE_SUSPENDED)
		gpio_aw_classdev_resume(gpio_aw_cdev);

	return 0;
}



static struct device_attribute gpio_aw_class_attrs[] = {
	__ATTR(mul_sel, 0644, mul_sel_show, mul_sel_store),
	__ATTR(pull, 0644, pull_show, pull_store),
	__ATTR(drv_level, 0644, drv_level_show, drv_level_store),
	__ATTR(data, 0644, data_show, data_store),
	__ATTR_NULL,
};



int gpio_aw_classdev_register(struct device *parent, struct gpio_aw_classdev *gpio_aw_cdev)
{
	gpio_aw_cdev->dev = device_create(gpio_aw_class, parent, 0, gpio_aw_cdev,
				      "%s", gpio_aw_cdev->name);
	if (IS_ERR(gpio_aw_cdev->dev))
		return PTR_ERR(gpio_aw_cdev->dev);
	down_write(&gpio_aw_list_lock);
	list_add_tail(&gpio_aw_cdev->node, &gpio_aw_list);
	up_write(&gpio_aw_list_lock);
	printk(KERN_DEBUG "Registered gpio_aw device: %s\n",
			gpio_aw_cdev->name);

	return 0;
}
EXPORT_SYMBOL_GPL(gpio_aw_classdev_register);

void gpio_aw_classdev_unregister(struct gpio_aw_classdev *gpio_aw_cdev)
{


	device_unregister(gpio_aw_cdev->dev);
	down_write(&gpio_aw_list_lock);
	list_del(&gpio_aw_cdev->node);
	up_write(&gpio_aw_list_lock);
}
EXPORT_SYMBOL_GPL(gpio_aw_classdev_unregister);

static int __init gpio_aw_init(void)
{
	gpio_aw_class = class_create(THIS_MODULE, "gpio_aw");
	if (IS_ERR(gpio_aw_class))
		return PTR_ERR(gpio_aw_class);
	gpio_aw_class->suspend = gpio_aw_suspend;
	gpio_aw_class->resume = gpio_aw_resume;
	gpio_aw_class->dev_attrs = gpio_aw_class_attrs;
	return 0;
}

static void __exit gpio_aw_exit(void)
{
	class_destroy(gpio_aw_class);
}

subsys_initcall(gpio_aw_init);
module_exit(gpio_aw_exit);

MODULE_AUTHOR("panlong <panlong@allwinnertech.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("aw_gpio Class Interface");







