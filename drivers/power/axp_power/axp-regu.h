#ifndef _LINUX_AXP_REGU_H_
#define _LINUX_AXP_REGU_H_

#include <linux/mfd/axp-mfd.h>

#include "axp-cfg.h"

/* AW1636 Regulator Registers */
#define AW1636_LDO1		AW1636_STATUS
#define AW1636_LDO5	    AW1636_DLDO1OUT_VOL
#define AW1636_LDO6	    AW1636_DLDO2OUT_VOL
#define AW1636_LDO7	    AW1636_DLDO3OUT_VOL
#define AW1636_LDO8	    AW1636_DLDO4OUT_VOL
#define AW1636_LDO9		AW1636_ELDO1OUT_VOL
#define AW1636_LDO10		AW1636_ELDO2OUT_VOL
#define AW1636_LDO11		AW1636_ELDO3OUT_VOL
#define AW1636_LDO12       AW1636_DC5LDOOUT_VOL 
#define AW1636_DCDC1	    AW1636_DC1OUT_VOL
#define AW1636_DCDC2	    AW1636_DC2OUT_VOL
#define AW1636_DCDC3	    AW1636_DC3OUT_VOL
#define AW1636_DCDC4	    AW1636_DC4OUT_VOL
#define AW1636_DCDC5	    AW1636_DC5OUT_VOL

#define AW1636_LDOIO0	    AW1636_GPIO0LDOOUT_VOL
#define AW1636_LDOIO1	    AW1636_GPIO1LDOOUT_VOL
#define AW1636_LDO2	    AW1636_ALDO1OUT_VOL
#define AW1636_LDO3	    AW1636_ALDO2OUT_VOL
#define AW1636_LDO4	    AW1636_ALDO3OUT_VOL

#define AW1636_LDO1EN		AW1636_STATUS
#define AW1636_LDO2EN		AW1636_LDO_DC_EN1
#define AW1636_LDO3EN		AW1636_LDO_DC_EN1
#define AW1636_LDO4EN		AW1636_LDO_DC_EN3
#define AW1636_LDO5EN		AW1636_LDO_DC_EN2
#define AW1636_LDO6EN		AW1636_LDO_DC_EN2
#define AW1636_LDO7EN		AW1636_LDO_DC_EN2
#define AW1636_LDO8EN		AW1636_LDO_DC_EN2
#define AW1636_LDO9EN		AW1636_LDO_DC_EN2
#define AW1636_LDO10EN		AW1636_LDO_DC_EN2
#define AW1636_LDO11EN		AW1636_LDO_DC_EN2
#define AW1636_LDO12EN		AW1636_LDO_DC_EN1
#define AW1636_DCDC1EN		AW1636_LDO_DC_EN1
#define AW1636_DCDC2EN		AW1636_LDO_DC_EN1
#define AW1636_DCDC3EN		AW1636_LDO_DC_EN1
#define AW1636_DCDC4EN		AW1636_LDO_DC_EN1
#define AW1636_DCDC5EN		AW1636_LDO_DC_EN1
#define AW1636_LDOIO0EN		AW1636_LDO_DC_EN3
#define AW1636_LDOIO1EN		AW1636_LDO_DC_EN1
#define AW1636_DC1SW1EN		AW1636_LDO_DC_EN2

#define AW1636_BUCKMODE     AW1636_DCDC_MODESET
#define AW1636_BUCKFREQ     AW1636_DCDC_FREQSET


#define AXP_LDO(_pmic, _id, min, max, step, vreg, shift, nbits, ereg, ebit)	\
{									\
	.desc	= {							\
		.name	= #_pmic"_LDO" #_id,					\
		.type	= REGULATOR_VOLTAGE,				\
		.id	= _pmic##_ID_LDO##_id,				\
		.n_voltages = (step) ? ((max - min) / step + 1) : 1,	\
		.owner	= THIS_MODULE,					\
	},								\
	.min_uV		= (min) * 1000,					\
	.max_uV		= (max) * 1000,					\
	.step_uV	= (step) * 1000,				\
	.vol_reg	= _pmic##_##vreg,				\
	.vol_shift	= (shift),					\
	.vol_nbits	= (nbits),					\
	.enable_reg	= _pmic##_##ereg,				\
	.enable_bit	= (ebit),					\
}

#define AXP_BUCK(_pmic, _id, min, max, step, vreg, shift, nbits, ereg, ebit)	\
{									\
	.desc	= {							\
		.name	= #_pmic"_BUCK" #_id,					\
		.type	= REGULATOR_VOLTAGE,				\
		.id	= _pmic##_ID_BUCK##_id,				\
		.n_voltages = (step) ? ((max - min) / step + 1) : 1,	\
		.owner	= THIS_MODULE,					\
	},								\
	.min_uV		= (min) * 1000,					\
	.max_uV		= (max) * 1000,					\
	.step_uV	= (step) * 1000,				\
	.vol_reg	= _pmic##_##vreg,				\
	.vol_shift	= (shift),					\
	.vol_nbits	= (nbits),					\
	.enable_reg	= _pmic##_##ereg,				\
	.enable_bit	= (ebit),					\
}

#define AXP_DCDC(_pmic, _id, min, max, step, vreg, shift, nbits, ereg, ebit)	\
{									\
	.desc	= {							\
		.name	= #_pmic"_DCDC" #_id,					\
		.type	= REGULATOR_VOLTAGE,				\
		.id	= _pmic##_ID_DCDC##_id,				\
		.n_voltages = (step) ? ((max - min) / step + 1) : 1,	\
		.owner	= THIS_MODULE,					\
	},								\
	.min_uV		= (min) * 1000,					\
	.max_uV		= (max) * 1000,					\
	.step_uV	= (step) * 1000,				\
	.vol_reg	= _pmic##_##vreg,				\
	.vol_shift	= (shift),					\
	.vol_nbits	= (nbits),					\
	.enable_reg	= _pmic##_##ereg,				\
	.enable_bit	= (ebit),					\
}

#define AXP_SW(_pmic, _id, min, max, step, vreg, shift, nbits, ereg, ebit)	\
{									\
	.desc	= {							\
		.name	= #_pmic"_SW" #_id,					\
		.type	= REGULATOR_VOLTAGE,				\
		.id	= _pmic##_ID_SW##_id,				\
		.n_voltages = (step) ? ((max - min) / step + 1) : 1,	\
		.owner	= THIS_MODULE,					\
	},								\
	.min_uV		= (min) * 1000,					\
	.max_uV		= (max) * 1000,					\
	.step_uV	= (step) * 1000,				\
	.vol_reg	= _pmic##_##vreg,				\
	.vol_shift	= (shift),					\
	.vol_nbits	= (nbits),					\
	.enable_reg	= _pmic##_##ereg,				\
	.enable_bit	= (ebit),					\
}

#define AXP_REGU_ATTR(_name)					\
{									\
	.attr = { .name = #_name,.mode = 0644 },					\
	.show =  _name##_show,				\
	.store = _name##_store, \
}

struct axp_regulator_info {
	struct regulator_desc desc;

	int	min_uV;
	int	max_uV;
	int	step_uV;
	int	vol_reg;
	int	vol_shift;
	int	vol_nbits;
	int	enable_reg;
	int	enable_bit;
};

#endif
