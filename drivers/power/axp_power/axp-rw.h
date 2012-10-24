#ifndef _LINUX_AXP_RW_H_
#define _LINUX_AXP_RW_H_

#include <linux/mfd/axp-mfd.h>
#include <mach/ar100.h>

static uint8_t axp_reg_addr = 0;

struct i2c_client *axp;
EXPORT_SYMBOL_GPL(axp);

static inline int __axp_read(int reg, uint8_t *val)
{
	int ret;
	unsigned char addr = (u8)reg;
	unsigned char data;
	ret = ar100_axp_read_reg(&addr , &data, 1);
	if (ret != 0) {
		printk("failed reading at 0x%02x\n", reg);
		return ret;
	}
	*val = data;
	return 0;
}

static inline int __axp_reads(int reg, int len, uint8_t *val)
{
	int ret;
	if ((reg < 0) || (len < 0) || (len > AXP_TRANS_BYTE_MAX)) {
		printk("pmu read reg para error\n");
		return -EINVAL;
	}
	unsigned char addr[AXP_TRANS_BYTE_MAX] ;
	int i = 0;
	for(i = 0;i < len;i++){
		addr[i] = (u8)(reg+i);
	}
	ret = ar100_axp_read_reg(addr,val,len);
	if (ret != 0) {
		printk("failed reading from 0x%02x\n", reg);
		return ret;
	}
	return 0;
}

static inline int __axp_write(int reg, uint8_t val)
{
	int ret;
	u8  addr = (u8)reg;
	
	ret = ar100_axp_write_reg(addr, val, 1);
	if (ret != 0) {
		printk("failed writing 0x%02x to 0x%02x\n",val, reg);
		return ret;
	}
	return 0;
}


static inline int __axp_writes(int reg, int len, uint8_t *val)
{
	int ret,i;
	len = (len + 1)/2;                
	if ((reg < 0) || (len < 0) || (len > AXP_TRANS_BYTE_MAX)) {
		printk("pmu write reg para error\n");
		return -EINVAL;
	}
	u8  addr[AXP_TRANS_BYTE_MAX];
	addr[0]= (u8)reg;
	for(i = 0;i < len;i++){
		addr[i] = (u8)(reg+i);
	}
	//u8  val[AXP_TRANS_BYTE_MAX];
	
	ret = ar100_axp_write_reg(reg, val, len);
	if (ret != 0) {
		printk("failed writings to 0x%02x\n", reg);
		return ret;
	}
	return 0;
}

int axp_register_notifier(struct device *dev, struct notifier_block *nb,
				uint64_t irqs)
{
	struct axp_mfd_chip *chip = dev_get_drvdata(dev);

	chip->ops->enable_irqs(chip, irqs);
	if(NULL != nb) {
	    return blocking_notifier_chain_register(&chip->notifier_list, nb);
    }

    return 0;
}
EXPORT_SYMBOL_GPL(axp_register_notifier);

int axp_unregister_notifier(struct device *dev, struct notifier_block *nb,
				uint64_t irqs)
{
	struct axp_mfd_chip *chip = dev_get_drvdata(dev);

	chip->ops->disable_irqs(chip, irqs);
	if(NULL != nb) {
	    return blocking_notifier_chain_unregister(&chip->notifier_list, nb);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(axp_unregister_notifier);

int axp_write(int reg, uint8_t val)
{
	return __axp_write(reg, val);
}
EXPORT_SYMBOL_GPL(axp_write);

int axp_writes(int reg, int len, uint8_t *val)
{
	return  __axp_writes(reg, len, val);
}
EXPORT_SYMBOL_GPL(axp_writes);

int axp_read(int reg, uint8_t *val)
{
	return __axp_read(reg, val);
}
EXPORT_SYMBOL_GPL(axp_read);

int axp_reads(int reg, int len, uint8_t *val)
{
	return __axp_reads(reg, len, val);
}
EXPORT_SYMBOL_GPL(axp_reads);

int axp_set_bits(int reg, uint8_t bit_mask)
{
	uint8_t reg_val;
	int ret = 0;

	ret = __axp_read(reg, &reg_val);
	if (ret)
		goto out;

	if ((reg_val & bit_mask) != bit_mask) {
		reg_val |= bit_mask;
		ret = __axp_write(reg, reg_val);
	}
out:
	return ret;
}
EXPORT_SYMBOL_GPL(axp_set_bits);

int axp_clr_bits(int reg, uint8_t bit_mask)
{
	uint8_t reg_val;
	int ret = 0;

	ret = __axp_read(reg, &reg_val);
	if (ret)
		goto out;

	if (reg_val & bit_mask) {
		reg_val &= ~bit_mask;
		ret = __axp_write(reg, reg_val);
	}
out:
	return ret;
}
EXPORT_SYMBOL_GPL(axp_clr_bits);

int axp_update(int reg, uint8_t val, uint8_t mask)
{
	uint8_t reg_val;
	int ret = 0;

	ret = __axp_read(reg, &reg_val);
	if (ret)
		goto out;

	if ((reg_val & mask) != val) {
		reg_val = (reg_val & ~mask) | val;
		ret = __axp_write(reg, reg_val);
	}
out:
	return ret;
}
EXPORT_SYMBOL_GPL(axp_update);

struct device *axp_get_dev(void)
{
	return &axp->dev;
}
EXPORT_SYMBOL_GPL(axp_get_dev);

#endif