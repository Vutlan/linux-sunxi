#ifndef GPIO_AW_CLASS_H_INCLUDED
#define GPIO_AW_CLASS_H_INCLUDED

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/rwsem.h>
#include <linux/timer.h>
#include "gpio_aw.h"



extern struct rw_semaphore gpio_aw_list_lock;
extern struct list_head gpio_aw_list;


void mul_sel_set(struct gpio_aw_classdev *gpio_aw_cdev,
					int  mul_sel)
{
	printk("  gpio_aw_cdev->mul_sel = %d  \n ", gpio_aw_cdev->mul_sel );
	printk("  mul_sel = %d  \n ",mul_sel );
	if ( mul_sel > 1){
		printk(" mul_sel max number is 1 ! \n  ");
		return ;
		}
	else if ( mul_sel < 0){
		printk(" mul_sel min number is  0! \n ");
		return ;		
		}
	else {
		printk("  set mul_sel = %d now ! \n ",mul_sel );
		gpio_aw_cdev->mul_sel = mul_sel;
	}
	printk("  gpio_aw_cdev->mul_sel = %d now ! \n ",gpio_aw_cdev->mul_sel );
	if (!(gpio_aw_cdev->flags & AW_GPIO_SUSPENDED))
		gpio_aw_cdev->gpio_aw_cfg_set(gpio_aw_cdev, mul_sel);
}


void pull_set(struct gpio_aw_classdev *gpio_aw_cdev,
					int  pull)
{
	printk("  gpio_aw_cdev->pull = %d  \n ",gpio_aw_cdev->pull );
	printk("  pull = %d  \n ",pull );
	if ( pull > 2){
		printk(" pull max number is 2 ! \n  ");
		return ;		
		}
	else if ( pull < 0){
		printk(" pull min number is 0 ! \n  ");
		return ;		
		}
	else {
		printk("  set pull = %d now ! \n ",pull );
		gpio_aw_cdev->pull = pull;
	}
	printk("  gpio_aw_cdev->pull = %d now ! \n ",gpio_aw_cdev->pull );
	if (!(gpio_aw_cdev->flags & AW_GPIO_SUSPENDED))
		gpio_aw_cdev->gpio_aw_pull_set(gpio_aw_cdev, pull);

}



void drv_level_set(struct gpio_aw_classdev *gpio_aw_cdev,
					int  drv_level)
{
	printk("  gpio_aw_cdev->drv_level = %d \n ",gpio_aw_cdev->drv_level );
	printk("  drv_level = %d  \n ",drv_level );
	if ( drv_level > 3){
		printk(" drv_level max number is 2 ! \n  ");
		return ;		
		}
	else if ( drv_level < 0){
		printk(" drv_level min number is 0 ! \n  ");
		return ;		
		}
	else {
		printk("  set drv_level = %d now ! \n ",drv_level );
		gpio_aw_cdev->drv_level = drv_level;
	}
	printk("  gpio_aw_cdev->drv_level = %d now ! \n ",gpio_aw_cdev->drv_level );
	if (!(gpio_aw_cdev->flags & AW_GPIO_SUSPENDED))
		gpio_aw_cdev->gpio_aw_drv_level_set(gpio_aw_cdev, drv_level);
}

void data_set(struct gpio_aw_classdev *gpio_aw_cdev,
					int  data)
{
	printk("  gpio_aw_cdev->data = %d  \n ",gpio_aw_cdev->data );
	printk("  data = %d  \n ",data );
	if ( data > 1){
		printk(" data max number is 2 ! \n  ");
		return ;		
		}
	else if ( data < 0){
		printk(" data min number is 0 ! \n  ");
		return ;		
		}
	else {
		printk("  set data = %d now ! \n ",data );
		gpio_aw_cdev->data = data;
	}
	printk("  gpio_aw_cdev->data = %d now ! \n ",gpio_aw_cdev->data );
	if (!(gpio_aw_cdev->flags & AW_GPIO_SUSPENDED))
		gpio_aw_cdev->gpio_aw_data_set(gpio_aw_cdev, data);
}

#endif