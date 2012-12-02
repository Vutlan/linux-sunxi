#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/regulator/machine.h>
#include <linux/i2c.h>
#include <mach/irqs.h>
#include <linux/power_supply.h>
#include <linux/mfd/axp-mfd.h>
#include <linux/module.h>

#include "axp-cfg.h"

static struct platform_device virt[]={
	{
			.name = "reg-22-cs-ldo1",
			.id = -1,
			.dev		= {
				.platform_data = "axp22_rtc",
			}
 	},{
			.name = "reg-22-cs-ldo2",
			.id = -1,
			.dev		= {
				.platform_data = "axp22_aldo1",
			}
 	},{
			.name = "reg-22-cs-ldo3",
			.id = -1,
			.dev		= {
				.platform_data = "axp22_aldo2",
			}
 	},{
			.name = "reg-22-cs-ldo4",
			.id = -1,
			.dev		= {
				.platform_data = "axp22_aldo3",
			}
	},{
			.name = "reg-22-cs-dcdc2",
			.id = -1,
			.dev		= {
				.platform_data = "axp22_dcdc2",
			}
	},{
			.name = "reg-22-cs-dcdc3",
			.id = -1,
			.dev		= {
				.platform_data = "axp22_dcdc3",
			}
	},{
			.name = "reg-22-cs-dcdc4",
			.id = -1,
			.dev		= {
				.platform_data = "axp22_dcdc4",
			}
 	},{
			.name = "reg-22-cs-dcdc5",
			.id = -1,
			.dev		= {
				.platform_data = "axp22_dcdc5",
			}
	},
};



 static int __init virtual_init(void)
{
	int j,ret;
	for (j = 0; j < ARRAY_SIZE(virt); j++){
 		ret =  platform_device_register(&virt[j]);
  		if (ret)
				goto creat_devices_failed;
	}

	return ret;

creat_devices_failed:
	while (j--)
		platform_device_register(&virt[j]);
	return ret;

}

module_init(virtual_init);

static void __exit virtual_exit(void)
{
	int j;
	for (j = ARRAY_SIZE(virt) - 1; j >= 0; j--){
		platform_device_unregister(&virt[j]);
	}
}
module_exit(virtual_exit);

MODULE_DESCRIPTION("Krosspower axp regulator test");
MODULE_AUTHOR("Donglu Zhang Krosspower");
MODULE_LICENSE("GPL");