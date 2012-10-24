#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/regulator/machine.h>
#include <linux/i2c.h>
#include <mach/irqs.h>
#include <linux/power_supply.h>
#include <linux/apm_bios.h>
#include <linux/apm-emulation.h>
#include <linux/mfd/axp-mfd.h>
#include <linux/module.h>
//#include <linux/archarm/mach-sun7i/include/mach/irqs-sun7i.h>

#include "axp-cfg.h"
#include <mach/sys_config.h>


int pmu_used;
int pmu_twi_id;
int pmu_irq_id;
int pmu_twi_addr;
int pmu_battery_rdc;
int pmu_battery_cap;
int pmu_init_chgcur;
int pmu_suspend_chgcur;
int pmu_resume_chgcur;
int pmu_shutdown_chgcur;
int pmu_init_chgvol;
int pmu_init_chgend_rate;
int pmu_init_chg_enabled;
int pmu_init_adc_freq;
int pmu_init_adc_freqc;
int pmu_init_chg_pretime;
int pmu_init_chg_csttime;

int pmu_bat_para1;
int pmu_bat_para2;
int pmu_bat_para3;
int pmu_bat_para4;
int pmu_bat_para5;
int pmu_bat_para6;
int pmu_bat_para7;
int pmu_bat_para8;
int pmu_bat_para9;
int pmu_bat_para10;
int pmu_bat_para11;
int pmu_bat_para12;
int pmu_bat_para13;
int pmu_bat_para14;
int pmu_bat_para15;
int pmu_bat_para16;
int pmu_bat_para17;
int pmu_bat_para18;
int pmu_bat_para19;
int pmu_bat_para20;
int pmu_bat_para21;
int pmu_bat_para22;
int pmu_bat_para23;
int pmu_bat_para24;
int pmu_bat_para25;
int pmu_bat_para26;
int pmu_bat_para27;
int pmu_bat_para28;
int pmu_bat_para29;
int pmu_bat_para30;
int pmu_bat_para31;
int pmu_bat_para32;


int pmu_usbvol_limit;
int pmu_usbvol;
int pmu_usbcur_limit;
int pmu_usbcur;

int pmu_pwroff_vol;
int pmu_pwron_vol;

int dcdc1_vol;
int dcdc2_vol;
int dcdc3_vol;
int dcdc4_vol;
int dcdc5_vol;

int ldo1_vol;
int ldo2_vol;
int ldo3_vol;
int ldo4_vol;
int ldo5_vol;
int ldo6_vol;
int ldo7_vol;
int ldo8_vol;
int ldo9_vol;
int ldo10_vol;
int ldo11_vol;
int ldo12_vol;
int ldoio0_vol;
int ldoio1_vol;

int pmu_pokoff_time;
int pmu_pokoff_en;
int pmu_poklong_time;
int pmu_pokon_time;
int pmu_pwrok_time;
int pmu_pwrnoe_time;
int pmu_intotp_en;

/* Reverse engineered partly from Platformx drivers */
enum axp_regls{

	vcc_ldo1,
	vcc_ldo2,
	vcc_ldo3,
	vcc_ldo4,
	vcc_ldo5,
	vcc_ldo6,
	vcc_ldo7,
	vcc_ldo8,
	vcc_ldo9,
	vcc_ldo10,
	vcc_ldo11,
	vcc_ldo12,
	
	vcc_DCDC1,
	vcc_DCDC2,
	vcc_DCDC3,
	vcc_DCDC4,
	vcc_DCDC5,
	vcc_ldoio0,
	vcc_ldoio1,
};

/* The values of the various regulator constraints are obviously dependent
 * on exactly what is wired to each ldo.  Unfortunately this information is
 * not generally available.  More information has been requested from Xbow
 * but as of yet they haven't been forthcoming.
 *
 * Some of these are clearly Stargate 2 related (no way of plugging
 * in an lcd on the IM2 for example!).
 */

static struct regulator_consumer_supply ldo1_data[] = {
		{
			.supply = "AW1636_ldo1",
		},
	};


static struct regulator_consumer_supply ldo2_data[] = {
		{
			.supply = "AW1636_ldo2",
		},
	};

static struct regulator_consumer_supply ldo3_data[] = {
		{
			.supply = "AW1636_ldo3",
		},
	};

static struct regulator_consumer_supply ldo4_data[] = {
		{
			.supply = "AW1636_ldo4",
		},
	};

static struct regulator_consumer_supply ldo5_data[] = {
		{
			.supply = "AW1636_ldo5",
		},
	};


static struct regulator_consumer_supply ldo6_data[] = {
		{
			.supply = "AW1636_ldo6",
		},
	};

static struct regulator_consumer_supply ldo7_data[] = {
		{
			.supply = "AW1636_ldo7",
		},
	};

static struct regulator_consumer_supply ldo8_data[] = {
		{
			.supply = "AW1636_ldo8",
		},
	};

static struct regulator_consumer_supply ldo9_data[] = {
		{
			.supply = "AW1636_ldo9",
		},
	};


static struct regulator_consumer_supply ldo10_data[] = {
		{
			.supply = "AW1636_ldo10",
		},
	};

static struct regulator_consumer_supply ldo11_data[] = {
		{
			.supply = "AW1636_ldo11",
		},
	};

static struct regulator_consumer_supply ldo12_data[] = {
		{
			.supply = "AW1636_ldo12",
		},
	};
	
static struct regulator_consumer_supply ldoio0_data[] = {
		{
			.supply = "AW1636_ldoio0",
		},
	};

static struct regulator_consumer_supply ldoio1_data[] = {
		{
			.supply = "AW1636_ldoio1",
		},
	};

static struct regulator_consumer_supply DCDC1_data[] = {
		{
			.supply = "AW1636_io",
		},
	};

static struct regulator_consumer_supply DCDC2_data[] = {
		{
			.supply = "AW1636_cpu",
		},
	};

static struct regulator_consumer_supply DCDC3_data[] = {
		{
			.supply = "AW1636_gpu",
		},
	};

static struct regulator_consumer_supply DCDC4_data[] = {
		{
			.supply = "AW1636_core",
		},
	};

static struct regulator_consumer_supply DCDC5_data[] = {
		{
			.supply = "AW1636_ddr",
		},
	};


static struct regulator_init_data axp_regl_init_data[] = {
	[vcc_ldo1] = {
		.constraints = { 
			.name = "AW1636_ldo1",
			.min_uV =  AW1636LDO1 * 1000,
			.max_uV =  AW1636LDO1 * 1000,
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo1_data),
		.consumer_supplies = ldo1_data,
	},
	[vcc_ldo2] = {
		.constraints = { 
			.name = "AW1636_ldo2",
			.min_uV = 700000,
			.max_uV = 3300000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				//.uV = ldo2_vol * 1000,
				//.enabled = 1,
			}
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo2_data),
		.consumer_supplies = ldo2_data,
	},
	[vcc_ldo3] = {
		.constraints = {
			.name = "AW1636_ldo3",
			.min_uV =  700000,
			.max_uV =  3300000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				//.uV = ldo3_vol * 1000,
				//.enabled = 1,
			}
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo3_data),
		.consumer_supplies = ldo3_data,
	},
	[vcc_ldo4] = {
		.constraints = {
			.name = "AW1636_ldo4",
			.min_uV = 700000,
			.max_uV = 3300000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				//.uV = ldo4_vol * 1000,
				//.enabled = 1,
			}
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo4_data),
		.consumer_supplies = ldo4_data,
	},
	[vcc_ldo5] = {
		.constraints = { 
			.name = "AW1636_ldo5",
			.min_uV = 700000,
			.max_uV = 3300000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				//.uV = ldo5_vol * 1000,
				//.enabled = 1,
			}
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo5_data),
		.consumer_supplies = ldo5_data,
	},
	[vcc_ldo6] = {
		.constraints = { 
			.name = "AW1636_ldo6",
			.min_uV = 700000,
			.max_uV = 3300000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				//.uV = ldo6_vol * 1000,
				//.enabled = 1,
			}
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo6_data),
		.consumer_supplies = ldo6_data,
	},
	[vcc_ldo7] = {
		.constraints = {
			.name = "AW1636_ldo7",
			.min_uV =  700000,
			.max_uV =  3300000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				//.uV = ldo7_vol * 1000,
				//.enabled = 1,
			}
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo7_data),
		.consumer_supplies = ldo7_data,
	},
	[vcc_ldo8] = {
		.constraints = {
			.name = "AW1636_ldo8",
			.min_uV = 700000,
			.max_uV = 3300000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				//.uV = ldo8_vol * 1000,
				//.enabled = 1,
			}
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo8_data),
		.consumer_supplies = ldo8_data,
	},
	[vcc_ldo9] = {
		.constraints = { 
			.name = "AW1636_ldo9",
			.min_uV = 700000,
			.max_uV = 3300000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				//.uV = ldo9_vol * 1000,
				//.enabled = 1,
			}
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo9_data),
		.consumer_supplies = ldo9_data,
	},
	[vcc_ldo10] = {
		.constraints = {
			.name = "AW1636_ldo10",
			.min_uV = 700000,
			.max_uV = 3300000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				//.uV = ldo10_vol * 1000,
				//.enabled = 1,
			}
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo10_data),
		.consumer_supplies = ldo10_data,
	},
	[vcc_ldo11] = {
		.constraints = {
			.name = "AW1636_ldo11",
			.min_uV =  700000,
			.max_uV =  3300000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				//.uV = ldo11_vol * 1000,
				//.enabled = 1,
			}
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo11_data),
		.consumer_supplies = ldo11_data,
	},
	[vcc_ldo12] = {
		.constraints = {
			.name = "AW1636_ldo12",
			.min_uV = 700000,
			.max_uV = 1400000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				//.uV = ldo12_vol * 1000,
				//.enabled = 1,
			}
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo12_data),
		.consumer_supplies = ldo12_data,
	},
	[vcc_DCDC1] = {
		.constraints = {
			.name = "AW1636_DCDC1",
			.min_uV = 1600000,
			.max_uV = 3400000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				//.uV = dcdc1_vol * 1000,
				//.enabled = 1,
			}
		},
		.num_consumer_supplies = ARRAY_SIZE(DCDC2_data),
		.consumer_supplies = DCDC2_data,
	},
	[vcc_DCDC2] = {
		.constraints = {
			.name = "AW1636_DCDC3",
			.min_uV = 600000,
			.max_uV = 1540000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				//.uV = dcdc3_vol * 1000,
				//.enabled = 1,
			}
		},
		.num_consumer_supplies = ARRAY_SIZE(DCDC3_data),
		.consumer_supplies = DCDC3_data,
	},
	[vcc_DCDC3] = {
		.constraints = { 
			.name = "AW1636_DCDC3",
			.min_uV = 600000,
			.max_uV = 1860000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				//.uV = dcdc3_vol * 1000,
				//.enabled = 1,
			}
		},
		.num_consumer_supplies = ARRAY_SIZE(DCDC3_data),
		.consumer_supplies = DCDC3_data,
	},
	[vcc_DCDC4] = {
		.constraints = { 
			.name = "AW1636_DCDC4",
			.min_uV = 600000,
			.max_uV = 1540000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				//.uV = dcdc4_vol * 1000,
				//.enabled = 1,
			}
		},
		.num_consumer_supplies = ARRAY_SIZE(DCDC4_data),
		.consumer_supplies = DCDC4_data,
	},
	[vcc_DCDC5] = {
		.constraints = { 
			.name = "AW1636_DCDC4",
			.min_uV = 1000000,
			.max_uV = 2550000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.initial_state = PM_SUSPEND_STANDBY,
			.state_standby = {
				//.uV = dcdc5_vol * 1000,
				//.enabled = 1,
			}
		},
		.num_consumer_supplies = ARRAY_SIZE(DCDC5_data),
		.consumer_supplies = DCDC5_data,
	},
	[vcc_ldoio0] = {
		.constraints = {
			.name = "AW1636_ldoio0",
			.min_uV = 700000,
			.max_uV = 3300000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		},
		.num_consumer_supplies = ARRAY_SIZE(ldoio0_data),
		.consumer_supplies = ldoio0_data,
	},
	[vcc_ldoio1] = {
		.constraints = {
			.name = "AW1636_ldoio1",
			.min_uV = 700000,
			.max_uV = 3300000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		},
		.num_consumer_supplies = ARRAY_SIZE(ldoio1_data),
		.consumer_supplies = ldoio1_data,
	},
};

static struct axp_funcdev_info axp_regldevs[] = {
	{
		.name = "AW1636-regulator",
		.id = AW1636_ID_LDO1,
		.platform_data = &axp_regl_init_data[vcc_ldo1],
	}, {
		.name = "AW1636-regulator",
		.id = AW1636_ID_LDO2,
		.platform_data = &axp_regl_init_data[vcc_ldo2],
	}, {
		.name = "AW1636-regulator",
		.id = AW1636_ID_LDO3,
		.platform_data = &axp_regl_init_data[vcc_ldo3],
	}, {
		.name = "AW1636-regulator",
		.id = AW1636_ID_LDO4,
		.platform_data = &axp_regl_init_data[vcc_ldo4],
	}, {
		.name = "AW1636-regulator",
		.id = AW1636_ID_LDO5,
		.platform_data = &axp_regl_init_data[vcc_ldo5],
	}, {
		.name = "AW1636-regulator",
		.id = AW1636_ID_LDO6,
		.platform_data = &axp_regl_init_data[vcc_ldo6],
	}, {
		.name = "AW1636-regulator",
		.id = AW1636_ID_LDO7,
		.platform_data = &axp_regl_init_data[vcc_ldo7],
	}, {
		.name = "AW1636-regulator",
		.id = AW1636_ID_LDO8,
		.platform_data = &axp_regl_init_data[vcc_ldo8],
	}, {
		.name = "AW1636-regulator",
		.id = AW1636_ID_LDO9,
		.platform_data = &axp_regl_init_data[vcc_ldo9],
	}, {
		.name = "AW1636-regulator",
		.id = AW1636_ID_LDO10,
		.platform_data = &axp_regl_init_data[vcc_ldo10],
	}, {
		.name = "AW1636-regulator",
		.id = AW1636_ID_LDO11,
		.platform_data = &axp_regl_init_data[vcc_ldo11],
	}, {
		.name = "AW1636-regulator",
		.id = AW1636_ID_LDO12,
		.platform_data = &axp_regl_init_data[vcc_ldo12],
	}, {
		.name = "AW1636-regulator",
		.id = AW1636_ID_DCDC1,
		.platform_data = &axp_regl_init_data[vcc_DCDC1],
	}, {
		.name = "AW1636-regulator",
		.id = AW1636_ID_DCDC2,
		.platform_data = &axp_regl_init_data[vcc_DCDC2],
	}, {
		.name = "AW1636-regulator",
		.id = AW1636_ID_DCDC3,
		.platform_data = &axp_regl_init_data[vcc_DCDC3],
	}, {
		.name = "AW1636-regulator",
		.id = AW1636_ID_DCDC4,
		.platform_data = &axp_regl_init_data[vcc_DCDC4],
	}, {
		.name = "AW1636-regulator",
		.id = AW1636_ID_DCDC5,
		.platform_data = &axp_regl_init_data[vcc_DCDC5],
	}, {
		.name = "AW1636-regulator",
		.id = AW1636_ID_LDOIO0,
		.platform_data = &axp_regl_init_data[vcc_ldoio0],
	}, {
		.name = "AW1636-regulator",
		.id = AW1636_ID_LDOIO1,
		.platform_data = &axp_regl_init_data[vcc_ldoio1],
	},
};

static struct power_supply_info battery_data ={
		.name ="PTI PL336078",
		.technology = POWER_SUPPLY_TECHNOLOGY_LiFe,
		//.voltage_max_design = pmu_init_chgvol,
		//.voltage_min_design = pmu_pwroff_vol,
		//.energy_full_design = pmu_battery_cap,
		.use_for_apm = 1,
};


static struct axp_supply_init_data axp_sply_init_data = {
	.battery_info = &battery_data,
	//.chgcur = pmu_init_chgcur,
	//.chgvol = pmu_init_chgvol,
	//.chgend = pmu_init_chgend_rate,
	//.chgen = pmu_init_chg_enabled,
	//.sample_time = pmu_init_adc_freq,
	//.chgpretime = pmu_init_chg_pretime,
	//.chgcsttime = pmu_init_chg_csttime,
};

static struct axp_funcdev_info axp_splydev[]={
   	{
   		.name = "AW1636-supplyer",
		.id = AW1636_ID_SUPPLY,
      .platform_data = &axp_sply_init_data,
    },
};

static struct axp_funcdev_info axp_gpiodev[]={
   	{  
		.name = "AW1636-gpio",
   		.id = AW1636_ID_GPIO,
    },
};

static struct axp_platform_data axp_pdata = {
	.num_regl_devs = ARRAY_SIZE(axp_regldevs),
	.num_sply_devs = ARRAY_SIZE(axp_splydev),
	.num_gpio_devs = ARRAY_SIZE(axp_gpiodev),
	.regl_devs = axp_regldevs,
	.sply_devs = axp_splydev,
	.gpio_devs = axp_gpiodev,
	.gpio_base = 0,
};

static struct i2c_board_info __initdata axp_mfd_i2c_board_info[] = {
	{
		.type = "aw1636_mfd",
		//.addr = pmu_twi_addr,
		.platform_data = &axp_pdata,
		//.irq = pmu_irq_id,
	},
};

static int __init axp_board_init(void)
{
		int ret;
    /*ret = script_parser_fetch("pmu_para", "pmu_used", &pmu_used, sizeof(int));
    if (ret)
    {
        printk("axp driver uning configuration failed(%d)\n", __LINE__);
        return -1;
    }
    if (pmu_used)
    {
        ret = script_parser_fetch("pmu_para", "pmu_twi_id", &pmu_twi_id, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_twi_id = AW1636_I2CBUS;
        }
        ret = script_parser_fetch("pmu_para", "pmu_irq_id", &pmu_irq_id, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_irq_id = AW1636_IRQNO;
        }
        ret = script_parser_fetch("pmu_para", "pmu_twi_addr", &pmu_twi_addr, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_twi_addr = AW1636_ADDR;
        }
        ret = script_parser_fetch("pmu_para", "pmu_battery_rdc", &pmu_battery_rdc, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_battery_rdc = BATRDC;
        }
        ret = script_parser_fetch("pmu_para", "pmu_battery_cap", &pmu_battery_cap, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_battery_cap = BATTERYCAP;
        }
        ret = script_parser_fetch("pmu_para", "pmu_init_chgcur", &pmu_init_chgcur, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_init_chgcur = INTCHGCUR / 1000;
        }
        pmu_init_chgcur = pmu_init_chgcur * 1000;
        ret = script_parser_fetch("pmu_para", "pmu_suspend_chgcur", &pmu_suspend_chgcur, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_suspend_chgcur = SUSCHGCUR / 1000;
        }
        pmu_suspend_chgcur = pmu_suspend_chgcur * 1000;
        ret = script_parser_fetch("pmu_para", "pmu_resume_chgcur", &pmu_resume_chgcur, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_resume_chgcur = RESCHGCUR / 1000;
        }
        pmu_resume_chgcur = pmu_resume_chgcur * 1000;
        ret = script_parser_fetch("pmu_para", "pmu_shutdown_chgcur", &pmu_shutdown_chgcur, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_shutdown_chgcur = CLSCHGCUR / 1000;
        }
        pmu_shutdown_chgcur = pmu_shutdown_chgcur * 1000;
        ret = script_parser_fetch("pmu_para", "pmu_init_chgvol", &pmu_init_chgvol, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_init_chgvol = INTCHGVOL / 1000;
        }
        pmu_init_chgvol = pmu_init_chgvol * 1000;
        ret = script_parser_fetch("pmu_para", "pmu_init_chgend_rate", &pmu_init_chgend_rate, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_init_chgend_rate = INTCHGENDRATE;
        }
        ret = script_parser_fetch("pmu_para", "pmu_init_chg_enabled", &pmu_init_chg_enabled, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_init_chg_enabled = INTCHGENABLED;
        }
        ret = script_parser_fetch("pmu_para", "pmu_init_adc_freq", &pmu_init_adc_freq, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_init_adc_freq = INTADCFREQ;
        }
        ret = script_parser_fetch("pmu_para", "pmu_init_adc_freqc", &pmu_init_adc_freqc, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_init_adc_freq = INTADCFREQC;
        }
        ret = script_parser_fetch("pmu_para", "pmu_init_chg_pretime", &pmu_init_chg_pretime, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_init_chg_pretime = INTCHGPRETIME;
        }
        ret = script_parser_fetch("pmu_para", "pmu_init_chg_csttime", &pmu_init_chg_csttime, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_init_chg_csttime = INTCHGCSTTIME;
        }

        ret = script_parser_fetch("pmu_para", "pmu_bat_para1", &pmu_bat_para1, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para1 = OCVREG0;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para2", &pmu_bat_para2, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para2 = OCVREG1;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para3", &pmu_bat_para3, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para3 = OCVREG2;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para4", &pmu_bat_para4, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para4 = OCVREG3;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para5", &pmu_bat_para5, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para5 = OCVREG4;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para6", &pmu_bat_para6, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para6 = OCVREG5;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para7", &pmu_bat_para7, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para7 = OCVREG6;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para8", &pmu_bat_para8, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para8 = OCVREG7;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para9", &pmu_bat_para9, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para9 = OCVREG8;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para10", &pmu_bat_para10, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para10 = OCVREG9;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para11", &pmu_bat_para11, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para11 = OCVREGA;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para12", &pmu_bat_para12, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para12 = OCVREGB;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para13", &pmu_bat_para13, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para13 = OCVREGC;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para14", &pmu_bat_para14, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para14 = OCVREGD;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para15", &pmu_bat_para15, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para15 = OCVREGE;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para16", &pmu_bat_para16, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para16 = OCVREGF;
        }
		ret = script_parser_fetch("pmu_para", "pmu_bat_para17", &pmu_bat_para17, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para1 = OCVREG0;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para18", &pmu_bat_para18, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para2 = OCVREG1;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para3", &pmu_bat_para3, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para3 = OCVREG2;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para4", &pmu_bat_para4, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para4 = OCVREG3;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para5", &pmu_bat_para5, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para5 = OCVREG4;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para6", &pmu_bat_para6, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para6 = OCVREG5;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para7", &pmu_bat_para7, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para7 = OCVREG6;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para8", &pmu_bat_para8, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para8 = OCVREG7;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para9", &pmu_bat_para9, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para9 = OCVREG8;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para10", &pmu_bat_para10, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para10 = OCVREG9;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para11", &pmu_bat_para11, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para11 = OCVREGA;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para12", &pmu_bat_para12, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para12 = OCVREGB;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para13", &pmu_bat_para13, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para13 = OCVREGC;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para14", &pmu_bat_para14, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para14 = OCVREGD;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para15", &pmu_bat_para15, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para15 = OCVREGE;
        }
        ret = script_parser_fetch("pmu_para", "pmu_bat_para16", &pmu_bat_para16, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_bat_para16 = OCVREGF;
        }
        ret = script_parser_fetch("pmu_para", "pmu_usbvol_limit", &pmu_usbvol_limit, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_usbvol_limit = 1;
        }
        ret = script_parser_fetch("pmu_para", "pmu_usbvol", &pmu_usbvol, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_usbvol = 4400;
        }
        ret = script_parser_fetch("pmu_para", "pmu_usbcur_limit", &pmu_usbcur_limit, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_usbcur_limit = 1;
        }
        ret = script_parser_fetch("pmu_para", "pmu_usbcur", &pmu_usbcur, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_usbcur = 0;
        }
        ret = script_parser_fetch("pmu_para", "pmu_pwroff_vol", &pmu_pwroff_vol, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_pwroff_vol = 3300;
        }
        ret = script_parser_fetch("pmu_para", "pmu_pwron_vol", &pmu_pwron_vol, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_pwron_vol = 2900;
        }
		ret = script_parser_fetch("target", "dcdc1_vol", &dcdc1_vol, sizeof(int));
		if (ret)
		{
			printk("axp driver uning configuration failed(%d)\n", __LINE__);
			dcdc1_vol = 1400;
		}
		ret = script_parser_fetch("target", "dcdc2_vol", &dcdc2_vol, sizeof(int));
		if (ret)
		{
			printk("axp driver uning configuration failed(%d)\n", __LINE__);
			dcdc2_vol = 1250;
			}
        ret = script_parser_fetch("target", "dcdc3_vol", &dcdc3_vol, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            dcdc3_vol = 1400;
        }
        ret = script_parser_fetch("target", "dcdc4_vol", &dcdc4_vol, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            dcdc4_vol = 1250;
        }
		ret = script_parser_fetch("target", "dcdc5_vol", &dcdc5_vol, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            dcdc5_vol = 1250;
        }
        ret = script_parser_fetch("target", "ldo2_vol", &ldo2_vol, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            ldo2_vol = 3000;
        }
        ret = script_parser_fetch("target", "ldo3_vol", &ldo3_vol, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            ldo3_vol = 2800;
        }
        ret = script_parser_fetch("target", "ldo4_vol", &ldo4_vol, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            ldo4_vol = 2800;
        }
		ret = script_parser_fetch("target", "ldo5_vol", &ldo5_vol, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            ldo5_vol = 3000;
        }
        ret = script_parser_fetch("target", "ldo6_vol", &ldo6_vol, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            ldo6_vol = 2800;
        }
        ret = script_parser_fetch("target", "ldo7_vol", &ldo7_vol, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            ldo7_vol = 2800;
        }
		ret = script_parser_fetch("target", "ldo8_vol", &ldo8_vol, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            ldo8_vol = 3000;
        }
        ret = script_parser_fetch("target", "ldo9_vol", &ldo9_vol, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            ldo9_vol = 2800;
        }
        ret = script_parser_fetch("target", "ldo10_vol", &ldo10_vol, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            ldo10_vol = 2800;
        }
		ret = script_parser_fetch("target", "ldo11_vol", &ldo11_vol, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            ldo11_vol = 3000;
        }
        ret = script_parser_fetch("target", "ldo12vol", &ldo12_vol, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            ldo12_vol = 2800;
        }
        ret = script_parser_fetch("target", "ldoio0_vol", &ldoio0_vol, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            ldoio0_vol = 2800;
        }
		ret = script_parser_fetch("target", "ldoio1_vol", &ldoio1_vol, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            ldoio1_vol = 2800;
        }
		ret = script_parser_fetch("pmu_para", "pmu_pokoff_time", &pmu_pokoff_time, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_pokoff_time = 6000;
        }
        ret = script_parser_fetch("pmu_para", "pmu_pokoff_en", &pmu_pokoff_en, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_pokoff_en   = 1;
        }
        ret = script_parser_fetch("pmu_para", "pmu_poklong_time", &pmu_peklong_time, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_poklong_time = 1500;
        }
        ret = script_parser_fetch("pmu_para", "pmu_pwrok_time", &pmu_pwrok_time, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
           pmu_pwrok_time    = 64;
        }
        ret = script_parser_fetch("pmu_para", "pmu_pwrnoe_time", &pmu_pwrnoe_time, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_pwrnoe_time = 2000;
        }
        ret = script_parser_fetch("pmu_para", "pmu_intotp_en", &pmu_intotp_en, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_intotp_en = 1;
        }   
        ret = script_parser_fetch("pmu_para", "pmu_pokon_time", &pmu_pokon_time, sizeof(int));
        if (ret)
        {
            printk("axp driver uning configuration failed(%d)\n", __LINE__);
            pmu_pokon_time = 1000;
        }           
*/
        int pmu_used                 = 1;
		int pmu_twi_addr             = 0x34;
		int pmu_twi_id               = 1;
		int pmu_irq_id               = 32;/*For FPGA irq_no is 32,For Real AW1633 irq_no is 64*/
		int pmu_battery_rdc          = 100;
		int pmu_battery_cap          = 2600;
		int pmu_init_chgcur          = 300;
		int pmu_earlysuspend_chgcur  = 600;
		int pmu_suspend_chgcur       = 1000;
		int pmu_resume_chgcur        = 300;
		int pmu_shutdown_chgcur      = 1000;
		int pmu_init_chgvol          = 4200;
		int pmu_init_chgend_rate     = 15;
		int pmu_init_chg_enabled     = 1;
		int pmu_init_adc_freq        = 100;
		int pmu_init_adc_freqc       = 100;
		int pmu_init_chg_pretime     = 50;
		int pmu_init_chg_csttime     = 720;

		int pmu_bat_para1            = 0;
		int pmu_bat_para2            = 0;
		int pmu_bat_para3            = 0;
		int pmu_bat_para4            = 1;
		int pmu_bat_para5            = 1;
		int pmu_bat_para6            = 2;
		int pmu_bat_para7            = 3;
		int pmu_bat_para8            = 4;
		int pmu_bat_para9            = 5;
		int pmu_bat_para10           = 6;
		int pmu_bat_para11           = 7;
		int pmu_bat_para12           = 10;
		int pmu_bat_para13           = 13;
		int pmu_bat_para14           = 16;
		int pmu_bat_para15           = 26;
		int pmu_bat_para16           = 36;
		int pmu_bat_para17           = 41;
		int pmu_bat_para18           = 46;
		int pmu_bat_para19           = 50;
		int pmu_bat_para20           = 53;
		int pmu_bat_para21           = 57;
		int pmu_bat_para22           = 61;
		int pmu_bat_para23           = 67;
		int pmu_bat_para24           = 73;
		int pmu_bat_para25           = 78;
		int pmu_bat_para26           = 84;
		int pmu_bat_para27           = 88;
		int pmu_bat_para28           = 92;
		int pmu_bat_para29           = 94;
		int pmu_bat_para30           = 96;
		int pmu_bat_para31           = 98;
		int pmu_bat_para32           = 99;
		int pmu_usbvol_limit         = 1;
		int pmu_usbcur_limit         = 0;
		int pmu_usbvol               = 4000;
		int pmu_usbcur               = 0;

		int pmu_usbvol_pc            = 4000;
		int pmu_usbcur_pc            = 0;

		int pmu_pwroff_vol           = 3300;
		int pmu_pwron_vol            = 2900;

		int dcdc1_vol                = 1400;
		int dcdc2_vol                = 1250;
		int dcdc3_vol                = 1400;
		int dcdc4_vol                = 1250;
		int dcdc5_vol                = 1250;
		
		int ldo2_vol                 = 3000;
		int ldo3_vol                 = 2800;
		int ldo4_vol                 = 2800;
		int ldo5_vol                 = 3000;
		int ldo6_vol                 = 2800;
		int ldo7_vol                 = 2800;
		int ldo8_vol                 = 3000;
		int ldo9_vol                 = 2800;
		int ldo10_vol                = 2800;
		int ldo11_vol                = 3000;
		int ldo12_vol                = 2800;
		int ldoio0_vol               = 2800;
		int ldoio1_vol               = 2800;

		int pmu_pekoff_time          = 6000;
		int pmu_pekoff_en            = 1;
		int pmu_peklong_time         = 1500;
		int pmu_pekon_time           = 1000;
		int pmu_pwrok_time           = 64;
		int pmu_pwrnoe_time          = 2000;
		int pmu_intotp_en            = 1;

		pmu_init_chgcur = pmu_init_chgcur * 1000;
		pmu_suspend_chgcur = pmu_suspend_chgcur * 1000;
		pmu_resume_chgcur = pmu_resume_chgcur * 1000;
		pmu_shutdown_chgcur = pmu_shutdown_chgcur * 1000;
		pmu_init_chgvol = pmu_init_chgvol * 1000;
        axp_regl_init_data[1].constraints.state_standby.uV = ldo2_vol * 1000;
        axp_regl_init_data[2].constraints.state_standby.uV = ldo3_vol * 1000;
        axp_regl_init_data[3].constraints.state_standby.uV = ldo4_vol * 1000;
		axp_regl_init_data[4].constraints.state_standby.uV = ldo5_vol * 1000;
        axp_regl_init_data[5].constraints.state_standby.uV = ldo6_vol * 1000;
        axp_regl_init_data[6].constraints.state_standby.uV = ldo7_vol * 1000;
		axp_regl_init_data[7].constraints.state_standby.uV = ldo8_vol * 1000;
        axp_regl_init_data[8].constraints.state_standby.uV = ldo9_vol * 1000;
        axp_regl_init_data[9].constraints.state_standby.uV = ldo10_vol * 1000;
		axp_regl_init_data[10].constraints.state_standby.uV = ldo11_vol * 1000;
        axp_regl_init_data[11].constraints.state_standby.uV = ldo12_vol * 1000;
		axp_regl_init_data[12].constraints.state_standby.uV = dcdc1_vol * 1000;
        axp_regl_init_data[13].constraints.state_standby.uV = dcdc2_vol * 1000;
        axp_regl_init_data[14].constraints.state_standby.uV = dcdc3_vol * 1000;
        axp_regl_init_data[15].constraints.state_standby.uV = dcdc4_vol * 1000;
		axp_regl_init_data[16].constraints.state_standby.uV = dcdc5_vol * 1000;
		axp_regl_init_data[17].constraints.state_standby.uV = ldoio0_vol * 1000;
        axp_regl_init_data[18].constraints.state_standby.uV = ldoio1_vol * 1000;
        axp_regl_init_data[1].constraints.state_standby.enabled = (ldo2_vol)?1:0;
        axp_regl_init_data[1].constraints.state_standby.disabled = (ldo2_vol)?0:1;
        axp_regl_init_data[2].constraints.state_standby.enabled = (ldo3_vol)?1:0;
        axp_regl_init_data[2].constraints.state_standby.disabled = (ldo3_vol)?0:1;
        axp_regl_init_data[3].constraints.state_standby.enabled = (ldo4_vol)?1:0;
        axp_regl_init_data[3].constraints.state_standby.disabled = (ldo4_vol)?0:1;
		axp_regl_init_data[4].constraints.state_standby.enabled = (ldo5_vol)?1:0;
        axp_regl_init_data[4].constraints.state_standby.disabled = (ldo5_vol)?0:1;
        axp_regl_init_data[5].constraints.state_standby.enabled = (ldo6_vol)?1:0;
        axp_regl_init_data[5].constraints.state_standby.disabled = (ldo6_vol)?0:1;
        axp_regl_init_data[6].constraints.state_standby.enabled = (ldo7_vol)?1:0;
        axp_regl_init_data[6].constraints.state_standby.disabled = (ldo7_vol)?0:1;
		axp_regl_init_data[7].constraints.state_standby.enabled = (ldo8_vol)?1:0;
        axp_regl_init_data[7].constraints.state_standby.disabled = (ldo8_vol)?0:1;
        axp_regl_init_data[8].constraints.state_standby.enabled = (ldo9_vol)?1:0;
        axp_regl_init_data[8].constraints.state_standby.disabled = (ldo9_vol)?0:1;
        axp_regl_init_data[9].constraints.state_standby.enabled = (ldo10_vol)?1:0;
        axp_regl_init_data[9].constraints.state_standby.disabled = (ldo10_vol)?0:1;
		axp_regl_init_data[10].constraints.state_standby.enabled = (ldo11_vol)?1:0;
        axp_regl_init_data[10].constraints.state_standby.disabled = (ldo11_vol)?0:1;
        axp_regl_init_data[11].constraints.state_standby.enabled = (ldo12_vol)?1:0;
        axp_regl_init_data[11].constraints.state_standby.disabled = (ldo12_vol)?0:1;		
        axp_regl_init_data[12].constraints.state_standby.enabled = (dcdc1_vol)?1:0;
        axp_regl_init_data[12].constraints.state_standby.disabled = (dcdc1_vol)?0:1;
		axp_regl_init_data[13].constraints.state_standby.enabled = (dcdc2_vol)?1:0;
        axp_regl_init_data[13].constraints.state_standby.disabled = (dcdc2_vol)?0:1;
        axp_regl_init_data[14].constraints.state_standby.enabled = (dcdc3_vol)?1:0;
        axp_regl_init_data[14].constraints.state_standby.disabled = (dcdc3_vol)?0:1;
		axp_regl_init_data[15].constraints.state_standby.enabled = (dcdc4_vol)?1:0;
        axp_regl_init_data[15].constraints.state_standby.disabled = (dcdc4_vol)?0:1;
        axp_regl_init_data[16].constraints.state_standby.enabled = (dcdc5_vol)?1:0;
        axp_regl_init_data[16].constraints.state_standby.disabled = (dcdc5_vol)?0:1;
		axp_regl_init_data[17].constraints.state_standby.enabled = (ldoio0_vol)?1:0;
        axp_regl_init_data[17].constraints.state_standby.disabled = (ldoio0_vol)?0:1;
        axp_regl_init_data[18].constraints.state_standby.enabled = (ldoio1_vol)?1:0;
        axp_regl_init_data[18].constraints.state_standby.disabled = (ldoio1_vol)?0:1;
        battery_data.voltage_max_design = pmu_init_chgvol;
        battery_data.voltage_min_design = pmu_pwroff_vol;
        battery_data.energy_full_design = pmu_battery_cap;
        axp_sply_init_data.chgcur = pmu_init_chgcur;
        axp_sply_init_data.chgvol = pmu_init_chgvol;
        axp_sply_init_data.chgend = pmu_init_chgend_rate;
        axp_sply_init_data.chgen = pmu_init_chg_enabled;
        axp_sply_init_data.sample_time = pmu_init_adc_freq;
        axp_sply_init_data.chgpretime = pmu_init_chg_pretime;
        axp_sply_init_data.chgcsttime = pmu_init_chg_csttime;
        axp_mfd_i2c_board_info[0].addr = pmu_twi_addr;
        //axp_mfd_i2c_board_info[0].irq = pmu_irq_id;

        return i2c_register_board_info(pmu_twi_id, axp_mfd_i2c_board_info, ARRAY_SIZE(axp_mfd_i2c_board_info));
}

fs_initcall(axp_board_init);

MODULE_DESCRIPTION("X-powers axp board");
MODULE_AUTHOR("Kyle Cheung");
MODULE_LICENSE("GPL");

