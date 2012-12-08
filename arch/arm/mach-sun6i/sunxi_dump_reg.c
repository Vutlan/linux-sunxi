/*
 * arch/arm/mach-sun6i/clock/sunxi_dump_reg.c
 * (C) Copyright 2010-2015
 * Reuuimlla Technology Co., Ltd. <www.reuuimllatech.com>
 * liugang <liugang@reuuimllatech.com>
 *
 * sunxi dump sysfs driver
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/kdev_t.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/slab.h>

#include <mach/platform.h>
#include <mach/hardware.h>
#include <mach/sunxi_dump_reg.h>

typedef struct __dump_struct {
	u32 	st_addr;	/* start reg physical addr */
	u32 	ed_addr;	/* end reg physical addr */
}dump_struct;

static dump_struct dump_para;
struct compare_group *cmp_group = NULL;
struct write_group *wt_group = NULL;

/**
 * addr_valid - check if the addr is valid
 * @addr: addr to judge
 * 
 * return true if the addr is register addr, false if not.
 */
bool addr_valid(u32 addr)
{
	if(addr >= AW_IO_PHYS_BASE && addr < AW_IO_PHYS_BASE + AW_IO_SIZE)
		return true;
	if(addr >= AW_SRAM_A1_BASE && addr < AW_SRAM_A1_BASE + AW_SRAM_A1_SIZE)
		return true;
	if(addr >= AW_SRAM_A2_BASE && addr < AW_SRAM_A2_BASE + AW_SRAM_A2_SIZE)
		return true;
	return false;
}

/**
 * first_str_to_int - convert substring of pstr to int, the substring is
 * 		from hed_addrd of pstr to the first occurance of ch in pstr
 * @pstr: the string to convert
 * @ch: a char in pstr
 * @pout: store the convert result
 *
 * return the first occurance of ch in pstr on success, NULL if failed.
 */
char * first_str_to_u32(char *pstr, char ch, u32 *pout)
{
	char 	*pret = NULL;
	char 	str_tmp[260] = {0};

	pret = strchr(pstr, ch);
	if(NULL != pret) {
		memcpy(str_tmp, pstr, pret - pstr);
		if(strict_strtoul(str_tmp, 16, (long unsigned int *)pout)) {
			printk(KERN_ERR "%s err, line %d\n", __func__, __LINE__);
			return NULL;
		}
	} else
		*pout = 0;

	return pret;
}

/**
 * parse_dump_str - parse the input string for dump attri.
 * @buf:     the input string, eg: "0x01c20000,0x01c20300".
 * @size:    buf size.
 * @start:   store the start reg's addr parsed from buf, eg 0x01c20000.
 * @end:     store the end reg's addr parsed from buf, eg 0x01c20300.
 *
 * return 0 if success, otherwise failed.
 */
int parse_dump_str(const char *buf, size_t size, u32 *start, u32 *end)
{
	char 	*ptr = (char *)buf;

	if(NULL == strchr(ptr, ',')) { /* only one reg to dump */
		if(strict_strtoul(ptr, 16, (long unsigned int *)start))
			return -EINVAL;
		*end = *start;
		return 0;
	}

	ptr = first_str_to_u32(ptr, ',', start);
	if(NULL == ptr)
		return -EINVAL;

	ptr += 1;
	if(strict_strtoul(ptr, 16, (long unsigned int *)end))
		return -EINVAL;

	return 0;
}

/**
 * sunxi_dump_regs_ex - dump a range of registers' value, copy to buf.
 * @start_reg:   physcal address of start reg.
 * @end_reg:     physcal address of end reg.
 * @buf:         store the dump info.
 * 
 * return bytes written to buf, <=0 indicate err
 */
ssize_t sunxi_dump_regs_ex(u32 start_reg, u32 end_reg, char *buf)
{
	int 	i;
	ssize_t cnt = 0;
	u32 	first_addr = 0, end_addr = 0;

	if(!addr_valid(start_reg) || !addr_valid(end_reg) || NULL == buf) {
		printk(KERN_ERR "%s err, invalid para, start 0x%08x, end 0x%08x, buf 0x%08x\n", __func__, start_reg, end_reg, (u32)buf);
		return -EIO;
	}
	/* only one to dump */
	if(start_reg == end_reg)
		return sprintf(buf, "0x%08x: 0x%08x\n", start_reg, readl(IO_ADDRESS(start_reg)));

	first_addr = start_reg & (~0xf);
	end_addr   = (end_reg   & (~0xf)) + 0xf;
	cnt += sprintf(buf, "0x%08x: ", first_addr);
	for(i = first_addr; i < end_addr; i += 4) {
		if(i < start_reg || i > end_reg)
			cnt += sprintf(buf + cnt, "           "); /* "0x12345678 ", 11 space*/
		else
			cnt += sprintf(buf + cnt, "0x%08x ", readl(IO_ADDRESS(i)));

		if((i & 0xc) == 0xc) {
			cnt += sprintf(buf + cnt, "\n");
			if(i + 4 < end_addr) /* avoid the last blank line */
				cnt += sprintf(buf + cnt, "0x%08x: ", i + 4);
		}
	}
	printk(KERN_INFO "%s, start 0x%08x, end 0x%08x, return %d\n", __func__, start_reg, end_reg, cnt);
	return cnt;
}

/**
 * dump_show - show func of dump attribute.
 * @dev:     class ptr.
 * @attr:    attribute ptr.
 * @buf:     the input buf which contain the start and end reg. eg: "0x01c20000,0x01c20100\n"
 * 
 * return size written to the buf, otherwise failed
 */
ssize_t dump_show(struct class *class, struct class_attribute *attr, char *buf)
{
	return sunxi_dump_regs_ex(dump_para.st_addr, dump_para.ed_addr, buf);
}

/**
 * dump_store - store func of dump attribute.
 * @class:   class ptr.
 * @attr:    attribute ptr.
 * @buf:     the input buf which contain the start and end reg. eg: "0x01c20000,0x01c20100\n"
 * @size:    buf size.
 * 
 * return size if success, otherwise failed
 */
ssize_t dump_store(struct class *class, struct class_attribute *attr,
			const char *buf, size_t size)
{
	u32 	start_reg = 0, end_reg = 0;

	if(0 != parse_dump_str((char *)buf, size, &start_reg, &end_reg)) {
		printk(KERN_ERR "%s err, invalid para, parse_dump_str failed\n", __func__);
		goto err;
	}
	//printk(KERN_INFO "%s: get start_reg 0x%08x, end_reg 0x%08x\n", __func__, start_reg, end_reg);
	if(!addr_valid(start_reg) || !addr_valid(end_reg)) {
		printk(KERN_ERR "%s err, invalid para, the addr is not reg\n", __func__);
		goto err;
	}

	dump_para.st_addr = start_reg;
	dump_para.ed_addr = end_reg;
	return size;
err:
	dump_para.st_addr = dump_para.ed_addr = 0;
	return -EINVAL;
}

/**
 * parse_compare_str - parse the input string for compare attri.
 * @str:     string to be parsed, eg: "0x01c20000 0x80000011 0x00000011".
 * @reg_addr:   store the reg address. eg: 0x01c20000.
 * @val_expect: store the expect value. eg: 0x80000011.
 * @val_mask:   store the mask value. eg: 0x00000011.
 * 
 * return 0 if success, otherwise failed.
 */
int parse_compare_str(char *str, u32 *reg_addr,
		u32 *val_expect, u32 *val_mask)
{
	char *ptr = str;

	ptr = first_str_to_u32(ptr, ' ', reg_addr);
	if(NULL == ptr)
		return -EINVAL;

	ptr += 1;
	ptr = first_str_to_u32(ptr, ' ', val_expect);
	if(NULL == ptr)
		return -EINVAL;

	ptr += 1;
	if(strict_strtoul(ptr, 16, (long unsigned int *)val_mask))
		return -EINVAL;

	return 0;
}

/**
 * compare_item_init - init for compare attri. parse input string, and construct compare struct.
 * @buf:     the input string, eg: "0x01c20000 0x80000011 0x00000011,0x01c20004 0x0000c0a4 0x0000c0a0...".
 * @size:    buf size.
 * @ppgroup: store the struct allocated, the struct contains items parsed from input buf.
 * 
 * return 0 if success, otherwise failed.
 */
int compare_item_init(const char *buf, size_t size, struct compare_group **ppgroup)
{
	int 	i = 0;
	char 	str_temp[256] = {0};
	char 	*ptr = NULL, *ptr2 = NULL;
	u32 	reg_addr = 0, val_expect = 0, val_mask = 0;
	struct compare_group *pgroup = NULL;

	/* alloc item buffer */
	pgroup = kmalloc(sizeof(struct compare_group), GFP_KERNEL);
	if(NULL == pgroup)
		return -EINVAL;
	pgroup->pitem = kmalloc(sizeof(struct compare_item) * MAX_COMPARE_ITEM, GFP_KERNEL);
	if(NULL == pgroup->pitem) {
		kfree(pgroup);
		return -EINVAL;
	}

	pgroup->num = 0;

	/* get item from buf */
	ptr = (char *)buf;
	while((ptr2 = strchr(ptr, ',')) != NULL) {
		i = ptr2 - ptr;
		memcpy(str_temp, ptr, i);
		str_temp[i] = 0;
		if(0 != parse_compare_str(str_temp, &reg_addr, &val_expect, &val_mask))
			printk(KERN_ERR "%s err, line %d, str_temp %s\n", __func__, __LINE__, str_temp);
		else {
			//printk(KERN_DEBUG "%s: reg_addr 0x%08x, val_expect 0x%08x, val_mask 0x%08x\n",
			//	__func__, reg_addr, val_expect, val_mask);
			if(pgroup->num < MAX_COMPARE_ITEM) {
				pgroup->pitem[pgroup->num].reg_addr = reg_addr;
				pgroup->pitem[pgroup->num].val_expect = val_expect;
				pgroup->pitem[pgroup->num].val_mask = val_mask;
				pgroup->num++;
			} else {
				printk(KERN_ERR "%s err, line %d, pgroup->num %d exceed %d\n",
					__func__, __LINE__, pgroup->num, MAX_COMPARE_ITEM);
				break;
			}
		}

		ptr = ptr2 + 1;
	}

	/* the last item */
	if(0 != parse_compare_str(ptr, &reg_addr, &val_expect, &val_mask))
		printk(KERN_ERR "%s err, line %d, ptr %s\n", __func__, __LINE__, ptr);
	else {
		//printk(KERN_DEBUG "%s: line %d, reg_addr 0x%08x, val_expect 0x%08x, val_mask 0x%08x\n",
		//	__func__, __LINE__, reg_addr, val_expect, val_mask);
		if(pgroup->num < MAX_COMPARE_ITEM) {
			pgroup->pitem[pgroup->num].reg_addr = reg_addr;
			pgroup->pitem[pgroup->num].val_expect = val_expect;
			pgroup->pitem[pgroup->num].val_mask = val_mask;
			pgroup->num++;
		}
	}

	/* free buffer if no valid item */
	if(0 == pgroup->num) {
		kfree(pgroup->pitem);
		kfree(pgroup);
		return -EINVAL;
	}

	*ppgroup = pgroup;
	return 0;
}

/**
 * compare_item_deinit - reled_addrse memory that cred_addrted by compare_item_init.
 * @pgroup: the compare struct allocated in compare_item_init.
 */
void compare_item_deinit(struct compare_group *pgroup)
{
	if(NULL != pgroup) {
		if(NULL != pgroup->pitem)
			kfree(pgroup->pitem);
		kfree(pgroup);
	}
}

ssize_t sunxi_compare_regs_ex(struct compare_group *pgroup, char *buf)
{
	int 	i = 0;
	ssize_t cnt = 0;
	u32 	reg = 0, expect = 0, actual = 0, mask = 0;

	if(NULL == pgroup) {
		printk(KERN_ERR "%s err, line %d, pgroup is NULL\n", __func__, __LINE__);
		goto end;
	}
	cnt += sprintf(buf, "reg         expect      actual      mask        result\n");
	for(i = 0; i < pgroup->num; i++) {
		reg    = pgroup->pitem[i].reg_addr;
		expect = pgroup->pitem[i].val_expect;
		actual = readl(IO_ADDRESS(reg));
		mask   = pgroup->pitem[i].val_mask;
		if((actual & mask) == (expect & mask))
			cnt += sprintf(buf + cnt, "0x%08x  0x%08x  0x%08x  0x%08x  OK\n", reg, expect, actual, mask);
		else
			cnt += sprintf(buf + cnt, "0x%08x  0x%08x  0x%08x  0x%08x  ERR\n", reg, expect, actual, mask);
	}
end:
	return cnt;
}

ssize_t compare_show(struct class *class, struct class_attribute *attr, char *buf)
{
	/* dump the items */
	return sunxi_compare_regs_ex(cmp_group, buf);
}

/**
 * compare_store - store func of compare attribute.
 * @class:   class ptr.
 * @attr:    attribute ptr.
 * @buf:     the input buf which contain the items to compared.
 * 		eg: "0x01c20000 0x01c20100\n"
 * @size:    buf size.
 */
ssize_t compare_store(struct class *class, struct class_attribute *attr,
			const char *buf, size_t size)
{
	/* free if struct not null */
	if(NULL != cmp_group) {
		compare_item_deinit(cmp_group);
		cmp_group = NULL;
	}
	/* parse input buf for items that will be dumped */
	if(compare_item_init(buf, size, &cmp_group) < 0)
		return -EINVAL;
	return size;
}

/**
 * parse_write_str - parse the input string for write attri.
 * @str:     string to be parsed, eg: "0x01c20818 0x55555555".
 * @reg_addr:   store the reg address. eg: 0x01c20818.
 * @val: store the expect value. eg: 0x55555555.
 * 
 * return 0 if success, otherwise failed.
 */
int parse_write_str(char *str, u32 *reg_addr, u32 *val)
{
	char *ptr = str;

	ptr = first_str_to_u32(ptr, ' ', reg_addr);
	if(NULL == ptr)
		return -EINVAL;

	ptr += 1;
	if(strict_strtoul(ptr, 16, (long unsigned int *)val))
		return -EINVAL;

	return 0;
}

/**
 * write_item_init - init for write attri. parse input string, and construct write struct.
 * @buf:     the input string, eg: "0x01c20800 0x00000031,0x01c20818 0x55555555,...".
 * @size:    buf size.
 * @ppgroup: store the struct allocated, the struct contains items parsed from input buf.
 * 
 * return 0 if success, otherwise failed.
 */
int write_item_init(const char *buf, size_t size, struct write_group **ppgroup)
{
	int 	i = 0;
	char 	str_temp[256] = {0};
	char 	*ptr = NULL, *ptr2 = NULL;
	u32 	reg_addr = 0, val;
	struct write_group *pgroup = NULL;

	/* alloc item buffer */
	pgroup = kmalloc(sizeof(struct write_group), GFP_KERNEL);
	if(NULL == pgroup)
		return -EINVAL;
	pgroup->pitem = kmalloc(sizeof(struct write_item) * MAX_WRITE_ITEM, GFP_KERNEL);
	if(NULL == pgroup->pitem) {
		kfree(pgroup);
		return -EINVAL;
	}

	pgroup->num = 0;

	/* get item from buf */
	ptr = (char *)buf;
	while((ptr2 = strchr(ptr, ',')) != NULL) {
		i = ptr2 - ptr;
		memcpy(str_temp, ptr, i);
		str_temp[i] = 0;
		if(0 != parse_write_str(str_temp, &reg_addr, &val))
			printk(KERN_ERR "%s err, line %d, str_temp %s\n", __func__, __LINE__, str_temp);
		else {
			//printk(KERN_DEBUG "%s: reg_addr 0x%08x, val 0x%08x\n", __func__, reg_addr, val);
			if(pgroup->num < MAX_WRITE_ITEM) {
				pgroup->pitem[pgroup->num].reg_addr = reg_addr;
				pgroup->pitem[pgroup->num].val = val;
				pgroup->num++;
			} else {
				printk(KERN_ERR "%s err, line %d, pgroup->num %d exceed %d\n",
					__func__, __LINE__, pgroup->num, MAX_WRITE_ITEM);
				break;
			}
		}

		ptr = ptr2 + 1;
	}

	/* the last item */
	if(0 != parse_write_str(ptr, &reg_addr, &val))
		printk(KERN_ERR "%s err, line %d, ptr %s\n", __func__, __LINE__, ptr);
	else {
		//printk(KERN_DEBUG "%s: line %d, reg_addr 0x%08x, val 0x%08x\n", __func__, __LINE__, reg_addr, val);
		if(pgroup->num < MAX_WRITE_ITEM) {
			pgroup->pitem[pgroup->num].reg_addr = reg_addr;
			pgroup->pitem[pgroup->num].val = val;
			pgroup->num++;
		}
	}

	/* free buffer if no valid item */
	if(0 == pgroup->num) {
		kfree(pgroup->pitem);
		kfree(pgroup);
		return -EINVAL;
	}

	*ppgroup = pgroup;
	return 0;
}

/**
 * write_item_deinit - reled_addrse memory that cred_addrted by write_item_init.
 * @pgroup: the write struct allocated in write_item_init.
 */
void write_item_deinit(struct write_group *pgroup)
{
	if(NULL != pgroup) {
		if(NULL != pgroup->pitem)
			kfree(pgroup->pitem);
		kfree(pgroup);
	}
}

ssize_t sunxi_write_regs_ex(struct write_group *pgroup, char *buf)
{
	int 	i = 0;
	ssize_t cnt = 0;
	u32 	reg = 0, val = 0, red_addrdback = 0;

	if(NULL == pgroup) {
		printk(KERN_ERR "%s err, line %d, pgroup is NULL\n", __func__, __LINE__);
		goto end;
	}
	cnt += sprintf(buf, "reg         to_write    after_write \n");
	for(i = 0; i < pgroup->num; i++) {
		reg    	= pgroup->pitem[i].reg_addr;
		val 	= pgroup->pitem[i].val;
		writel(val, IO_ADDRESS(reg));
		red_addrdback = readl(IO_ADDRESS(reg));
		cnt += sprintf(buf + cnt, "0x%08x  0x%08x  0x%08x\n", reg, val, red_addrdback);
	}
end:
	return cnt;
}

ssize_t write_show(struct class *class, struct class_attribute *attr, char *buf)
{
	/* write the items */
	return sunxi_write_regs_ex(wt_group, buf);
}

/**
 * write_store - store func of dump attribute.
 * @class:   class ptr.
 * @attr:    attribute ptr.
 * @buf:     the input buf which contain reg&val to write.
 * 		eg: "0x01c20800 0x00000031,0x01c20818 0x55555555,...\n"
 * @size:    buf size.
 */
ssize_t write_store(struct class *class, struct class_attribute *attr,
			const char *buf, size_t size)
{
	/* free if not NULL */
	if(NULL != wt_group) {
		write_item_deinit(wt_group);
		wt_group = NULL;
	}
	/* parse input buf for items that will be dumped */
	if(write_item_init(buf, size, &wt_group) < 0)
		return -EINVAL;

	return size;
}

static struct class_attribute dump_class_attrs[] = {
	__ATTR(dump, 	0644, dump_show, dump_store),
	__ATTR(compare,	0644, compare_show, compare_store),
	__ATTR(write,	0644, write_show, write_store),
	__ATTR_NULL,
};

static struct class dump_class = {
	.name		= "sunxi_dump",
	.owner		= THIS_MODULE,
	.class_attrs	= dump_class_attrs,
};

static int __init sunxi_dump_init(void)
{
	int	status;

	status = class_register(&dump_class);
	if(status < 0)
		printk(KERN_ERR "%s err, status %d\n", __func__, status);
	else
		printk(KERN_DEBUG "%s success\n", __func__);

	return status;
}
postcore_initcall(sunxi_dump_init);

/**
 * sunxi_write_regs - write a group of regs' value.
 * @pgroup: the write struct which contain items that will be write.
 */
void sunxi_write_regs(struct write_group *pgroup)
{
	int 	i = 0;
	u32 	reg = 0, val = 0, red_addrdback = 0;

	printk("reg         to_write    after_write \n");
	for(i = 0; i < pgroup->num; i++) {
		reg    	= pgroup->pitem[i].reg_addr;
		val 	= pgroup->pitem[i].val;
		writel(val, IO_ADDRESS(reg));
		red_addrdback = readl(IO_ADDRESS(reg));
		printk("0x%08x  0x%08x  0x%08x\n", reg, val, red_addrdback);
	}
}
EXPORT_SYMBOL(sunxi_write_regs);

/**
 * sunxi_compare_regs - dump values for compare items.
 * @pgroup: the compare struct which contain items that will be dumped.
 */
void sunxi_compare_regs(struct compare_group *pgroup)
{
	int 	i = 0;
	u32 	reg = 0, expect = 0, actual = 0, mask = 0;

	printk("reg         expect      actual      mask        result\n");
	for(i = 0; i < pgroup->num; i++) {
		reg    = pgroup->pitem[i].reg_addr;
		expect = pgroup->pitem[i].val_expect;
		actual = readl(IO_ADDRESS(reg));
		mask   = pgroup->pitem[i].val_mask;
		if((actual & mask) == (expect & mask))
			printk("0x%08x  0x%08x  0x%08x  0x%08x  OK\n", reg, expect, actual, mask);
		else
			printk("0x%08x  0x%08x  0x%08x  0x%08x  ERR\n", reg, expect, actual, mask);
	}
}
EXPORT_SYMBOL(sunxi_compare_regs);

/**
 * sunxi_dump_regs - dump a range of registers' value.
 * @start_reg:   physcal address of start reg.
 * @end_reg:     physcal address of end reg.
 */
void sunxi_dump_regs(u32 start_reg, u32 end_reg)
{
	int 	i;
	u32 	first_addr = 0, end_addr = 0;

	if(start_reg == end_reg) { /* only one to dump */
		printk("0x%08x: 0x%08x\n", start_reg, readl(IO_ADDRESS(start_reg)));
		return;
	}

	first_addr = start_reg & (~0xf);
	end_addr   = end_reg   & (~0xf);

	printk("0x%08x: ", first_addr);

	for(i = first_addr; i < end_addr + 0xf; i += 4) {
		if(i < start_reg || i > end_reg)
			printk("           "); /* "0x12345678 ", 11 space*/
		else
			printk("0x%08x ", readl(IO_ADDRESS(i)));

		if((i & 0xc) == 0xc) {
			printk("\n");
			if(i + 4 < end_addr + 0xf) /* avoid the last blank line */
				printk("0x%08x: ", i + 4);
		}
	}
}
EXPORT_SYMBOL(sunxi_dump_regs);

