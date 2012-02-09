/*
 * panlong <panlong@allwinnertech.com>
 */

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/rwsem.h>



DECLARE_RWSEM(gpio_aw_list_lock);
EXPORT_SYMBOL_GPL(gpio_aw_list_lock);

LIST_HEAD(gpio_aw_list);
EXPORT_SYMBOL_GPL(gpio_aw_list);
