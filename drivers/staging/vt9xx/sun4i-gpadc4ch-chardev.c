// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Allwinner sunxi gpadc driver
 */

#include <linux/err.h>
#include <linux/init.h>

#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include <linux/completion.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>

#include <linux/delay.h>

#include <linux/sysfs.h>
#include <linux/string.h>

#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h> /* for put_user */

#include "sunxi-gpadc4ch.h"



#define TP_CTRL0		0x00
#define TP_CTRL1		0x04
#define TP_CTRL2		0x08
#define TP_CTRL3		0x0c
#define TP_INT			0x10
#define TP_FIFOCS		0x14
#define TP_TPR			0x18
#define TP_CDAT			0x1c
#define TEMP_DATA		0x20
#define TP_DATA			0x24
#define TP_IO_CONFIG	0x28
#define TP_PORT_DATA	0x2c

/* TP_CTRL0 bits */
#define ADC_FIRST_DLY(x)	((x) << 24) /* 8 bits */
#define ADC_FIRST_DLY_MODE(x)	((x) << 23)
#define ADC_CLK_SEL(x)		((x) << 22)
#define ADC_CLK_DIV(x)		((x) << 20) /* 3 bits */
#define FS_DIV(x)		((x) << 16) /* 4 bits */
#define T_ACQ(x)		((x) << 0) /* 16 bits */

/* TP_CTRL1 bits */
#define SUN4_CHOP_TEMP_EN		(1<<7)
#define SUN4_TOUCH_PAN_CALI_EN	(1<<6)
#define SUN4_TP_CTRL_MODE_EN	(1<<4)
#define SUN4_TP_ADC_SELECT		(1<<3)
#define SUN4_ADC_CHAN_SELECT(N)	(N)
#define SUN8_CHOP_TEMP_EN		(1<<8)
#define SUN8_TOUCH_PAN_CALI_EN	(1<<7)
#define SUN8_TP_DUAL_EN			(1<<6)
#define SUN8_TP_CTRL_MODE_EN	(1<<5)
#define SUN8_TP_ADC_SELECT		(1<<4)
#define ADC_CHAN_SELECT(N)		(N)


/* TP_CTRL2 */
#define TP_SENSITIVE_ADJUST(l) (l<<28)
#define TP_FIFO_MODE_SELECT(m) (m<<26)
#define PRE_MEA_EN			   (1<<24)
#define PRE_MEA_THRE_CNT(cnt)  (cnt)

/* TP_CTRL3 */
#define FILTER_EN              (1<<2)
#define FILTER_4_2             (0)
#define FILTER_5_3             (1)
#define FILTER_8_4             (2)
#define FILTER_16_8            (3)

/* TP_INT */
#define TP_TEMP_IRQ_EN		   (1<<18)
#define TP_OVERRUN_IRQ_EN      (1<<17)
#define TP_DATA_IRQ_EN         (1<<16)
#define TP_FIFO_TRIG_LEVEL(L)  ((L-1)<<8)
#define TP_DATA_DRQ_EN         (1<<7)
#define TP_FIFO_FLUSH          (1<<4)
#define TP_UP_IRQ_EN		   (1<<1)
#define TP_DOWN_IRQ_EN		   (1<<0)

/* TP_INT_FIFOS irq and fifo status bits */
#define TEMP_DATA_PENDING		BIT(18)
#define FIFO_OVERRUN_PENDING	BIT(17)
#define FIFO_DATA_PENDING		BIT(16)
#define RXA_CNT_SHIFT			8
#define RXA_CNT_MASK			0x1f
#define TP_IDLE_FLG				BIT(2)
#define TP_UP_PENDING			BIT(1)
#define TP_DOWN_PENDING			BIT(0)

/* TP_TPR bits */
#define TEMP_ENABLE(x)			((x) << 16)
#define TEMP_PERIOD(x)			((x) << 0)
/* t = x * 256 * 16 / clkin */

#define SUNXI_GPADC4CH_TIMEOUT	(msecs_to_jiffies(1000))

#define ADC_VT900_DEV "adc"
#define SUCCESS 0
#define DEVICE_NAME "adcdev"
#define BUF_LEN 80


#define ARCH_SUN4I_A10 0
#define ARCH_SUN7I_A20 1
#define ARCH_SUN8I_R40 2
struct sun4i_gpadc4ch_data {
	struct device		*dev;
	void __iomem		*base;
	unsigned int		irq;
	u32					sunxi_arch;

	struct miscdevice mdev;
	int device_open;

	struct completion	completion;
	struct mutex		mutex;
	//atomic_t			ignore_fifo_data_irq;
	volatile bool		conversion_ok;
	volatile u16		adc_data[4];
};


/*
 * Prototypes
 */

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
//static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
//static long compat_device_ioctl(struct file *filp, u32 cmd, unsigned long arg);
static long device_ioctl(struct file *filp, u32 cmd, unsigned long arg);

static int sun4i_gpadc_read(struct sun4i_gpadc4ch_data *gpadc4ch);


static const struct file_operations fops = {
	.owner = THIS_MODULE,
	//.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release,
	.unlocked_ioctl = device_ioctl,
/*
#ifdef CONFIG_COMPAT
	.compat_ioctl = compat_device_ioctl,
#endif
*/
};

struct miscdevice sample_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &fops,
};

static inline struct sun4i_gpadc4ch_data *to_gpadc4ch_struct(struct file *file)
{
    struct miscdevice *miscdev = file->private_data;
    return container_of(miscdev, struct sun4i_gpadc4ch_data, mdev);
}

static int device_open(struct inode *inode, struct file *file)
{
	struct sun4i_gpadc4ch_data *gpadc4ch = to_gpadc4ch_struct(file);

	if (gpadc4ch->device_open)
		return -EBUSY;

	gpadc4ch->device_open =1;
	return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
	struct sun4i_gpadc4ch_data *gpadc4ch = to_gpadc4ch_struct(file);

    gpadc4ch->device_open =0;
    return 0;
}

static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct sun4i_gpadc4ch_data *gpadc4ch = to_gpadc4ch_struct(file);
	int ret = 0;

	switch (cmd)
	{
	case IOCTL_ONE_CONVERSION :
		mutex_lock(&gpadc4ch->mutex);

		//pr_warn("%s %d \n", __FUNCTION__, __LINE__);
		sun4i_gpadc_read(gpadc4ch);
		//pr_warn("%s %d \n", __FUNCTION__, __LINE__);

		if (gpadc4ch->conversion_ok) {
			if (copy_to_user((char *)arg, (const void *)&gpadc4ch->adc_data, sizeof(gpadc4ch->adc_data)))
				ret = -EFAULT;
		}
		else {
			pr_info("%s not ok \n", __FUNCTION__);
			ret = -EAGAIN;
		}

		mutex_unlock(&gpadc4ch->mutex);

		break;

	default:
		printk(KERN_WARNING "MYCHARDEV: Device ioctl %d\n", cmd);
		ret = -1;
	};   /* switch */
	return ret;
}

static ssize_t device_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
	printk(KERN_WARNING "Sorry, this operation isn't supported.\n");
	return -EINVAL;
}

static void fifo_flush(struct sun4i_gpadc4ch_data *gpadc4ch)
{
	u32 reg;

	reg = readl(gpadc4ch->base + TP_INT);
	writel(reg | TP_FIFO_FLUSH, gpadc4ch->base +TP_INT);
	udelay(1);
}

static void sun4i_enable_adc(struct sun4i_gpadc4ch_data *gpadc4ch)
{
	u32 reg;
	reg = readl(gpadc4ch->base + TP_CTRL1);
	if (gpadc4ch->sunxi_arch == ARCH_SUN8I_R40) {
		reg |= SUN8_TP_CTRL_MODE_EN; /* all channel */
	}
	else
		reg |= SUN4_TP_CTRL_MODE_EN;

	writel(reg, gpadc4ch->base +TP_CTRL1);
}
static void sun4i_disable_adc(struct sun4i_gpadc4ch_data *gpadc4ch)
{
	u32 reg;
	reg = readl(gpadc4ch->base + TP_CTRL1);
	if (gpadc4ch->sunxi_arch == ARCH_SUN8I_R40)
		reg &= ~SUN8_TP_CTRL_MODE_EN;
	else
		reg &= ~SUN4_TP_CTRL_MODE_EN;
	writel(reg, gpadc4ch->base + TP_CTRL1);
}

static void sun4i_enable_data_irq(struct sun4i_gpadc4ch_data *gpadc4ch)
{
	u32 reg;
	reg= readl(gpadc4ch->base + TP_INT);
	reg |= TP_DATA_IRQ_EN;
	writel(reg, gpadc4ch->base + TP_INT);
}
static void sun4i_disable_data_irq(struct sun4i_gpadc4ch_data *gpadc4ch)
{
	u32 reg;
	reg= readl(gpadc4ch->base + TP_INT);
	reg &= ~TP_DATA_IRQ_EN;
	writel(reg, gpadc4ch->base + TP_INT);
}
static void sun4i_prepare_for_irq(struct sun4i_gpadc4ch_data *gpadc4ch)
{
	u32 reg;

	gpadc4ch->conversion_ok = false;
	reinit_completion(&gpadc4ch->completion);

	fifo_flush(gpadc4ch);

	/* clear all status flags */
	reg = readl(gpadc4ch->base + TP_FIFOCS);
	writel(reg, gpadc4ch->base + TP_FIFOCS);

	sun4i_enable_adc(gpadc4ch);

	sun4i_enable_data_irq(gpadc4ch);
}

static int sun4i_gpadc_read(struct sun4i_gpadc4ch_data *gpadc4ch)
{
	int ret;

	sun4i_prepare_for_irq(gpadc4ch);

	if (!wait_for_completion_timeout(&gpadc4ch->completion,	SUNXI_GPADC4CH_TIMEOUT)) {
		ret = -ETIMEDOUT;
		pr_warn("%s %d completion timeout!\n", __FUNCTION__, __LINE__);
		goto err;
	}

	return 0;
err:
	sun4i_disable_adc(gpadc4ch);
	sun4i_disable_data_irq(gpadc4ch);

	return ret;
}

static ssize_t analog_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int len=0;
	int i;
	u32 val;
	struct sun4i_gpadc4ch_data *gpadc4ch = dev_get_drvdata(dev);

	mutex_lock(&gpadc4ch->mutex);

	//pr_warn("%s %d \n", __FUNCTION__, __LINE__);
	sun4i_gpadc_read(gpadc4ch);
	//pr_warn("%s %d \n", __FUNCTION__, __LINE__);

	if (gpadc4ch->conversion_ok) {
		for (i=0; i<4; i++) {
			val = gpadc4ch->adc_data[i];
			len+= scnprintf(buf+len, PAGE_SIZE-len, "%04x ", val);
		}
	}
	else
		pr_info("%s not ok \n", __FUNCTION__);

	mutex_unlock(&gpadc4ch->mutex);
	return len;
}
static DEVICE_ATTR(analog,  0444, analog_show,  /*analog_store*/NULL);



static irqreturn_t sun4i_gpadc4ch_irq(int irq, void *dev_id)
{
	struct sun4i_gpadc4ch_data *gpadc4ch = dev_id;
	u32 reg;
	u32 data_avail, i;

	reg  = readl(gpadc4ch->base + TP_FIFOCS);

	data_avail = (reg >> RXA_CNT_SHIFT) & RXA_CNT_MASK;

	if (data_avail < 4)
		pr_err("%s false interrupt!\n", __FUNCTION__);

	if ( (reg & FIFO_DATA_PENDING) && (data_avail >=4) ) {
		sun4i_disable_adc(gpadc4ch);
		sun4i_disable_data_irq(gpadc4ch);

		if ((data_avail % 4)!=0) {
			gpadc4ch->conversion_ok = false;
		}
		else {
			gpadc4ch->conversion_ok = true;
		}

		for (i=0; i<4; i++)
			gpadc4ch->adc_data[i] = readl(gpadc4ch->base + TP_DATA) & 0xffff;

		dsb();
		complete(&gpadc4ch->completion);
		//pr_warn("%s data_avail=%d i=%d\n", __FUNCTION__, data_avail, i);
	}

	writel(FIFO_DATA_PENDING, gpadc4ch->base + TP_FIFOCS);
	return IRQ_HANDLED;
}


static void adc_init(struct sun4i_gpadc4ch_data *gpadc4ch)
{
	u32 val;

	/* adc clock setup */
	writel(ADC_FIRST_DLY(0x1f) | ADC_FIRST_DLY_MODE(1) | ADC_CLK_SEL(0) |
			ADC_CLK_DIV(2) | FS_DIV(7) | T_ACQ(0xff),
			gpadc4ch->base + TP_CTRL0);

	if (gpadc4ch->sunxi_arch == ARCH_SUN8I_R40)
		val = SUN8_TP_ADC_SELECT | ADC_CHAN_SELECT(15);
	else
		val = SUN4_TP_ADC_SELECT | ADC_CHAN_SELECT(4);
	writel(val, gpadc4ch->base + TP_CTRL1);

	/* filter setup */
	writel(0*FILTER_EN | FILTER_5_3, gpadc4ch->base + TP_CTRL3);

	/* offset */
	writel(0x800, gpadc4ch->base + TP_CDAT);

	/*fifo setup */
	fifo_flush(gpadc4ch);
	writel(TP_FIFO_MODE_SELECT(2), gpadc4ch->base +  TP_CTRL2);
	writel(TP_FIFO_TRIG_LEVEL(4), gpadc4ch->base + TP_INT);

	/* disable temperature channel */
	writel(TEMP_ENABLE(0) | TEMP_PERIOD(0xffff), gpadc4ch->base + TP_TPR);

	/* Pad setup*/
	writel(0x2222, gpadc4ch->base + TP_IO_CONFIG);
	writel(0, gpadc4ch->base + TP_PORT_DATA);
}




static const struct of_device_id sun4i_gpadc4ch_of_match[] = {
	{ .compatible = "allwinner,sun4i-a10-gpadc4ch", .data = (void *)ARCH_SUN4I_A10 },
	{ .compatible = "allwinner,sun7i-a20-gpadc4ch", .data = (void *)ARCH_SUN7I_A20 },
	{ .compatible = "allwinner,sun8i-r40-gpadc4ch", .data = (void *)ARCH_SUN8I_R40 },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, sun4i_gpadc4ch_of_match);

static int sun4i_gpadc4ch_probe(struct platform_device *pdev)
{
	struct sun4i_gpadc4ch_data *gpadc4ch;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	const struct of_device_id *of_id;
	int ret;

	//pr_warn("%s\n", __FUNCTION__);

	gpadc4ch = devm_kzalloc(dev, sizeof(struct sun4i_gpadc4ch_data), GFP_KERNEL);
	if (!gpadc4ch)
		return -ENOMEM;
	gpadc4ch->dev = dev;

	of_id = of_match_node(sun4i_gpadc4ch_of_match, np);
	if (!of_id)
		return -EINVAL;

	gpadc4ch->sunxi_arch = (u32)of_id->data;
	dev_info(&pdev->dev, "sunxi_arch=%d\n", gpadc4ch->sunxi_arch);

	gpadc4ch->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(gpadc4ch->base))
		return PTR_ERR(gpadc4ch->base);

	gpadc4ch->irq = platform_get_irq(pdev, 0);
	ret = devm_request_irq(dev, gpadc4ch->irq, sun4i_gpadc4ch_irq, 0, "sun4i-gpadc4ch", gpadc4ch);
	if (ret)
		return ret;

	mutex_init(&gpadc4ch->mutex);
	init_completion(&gpadc4ch->completion);
	adc_init(gpadc4ch);

	/* sysfs interface  */
	ret = device_create_file(&pdev->dev, &dev_attr_analog);
	if (ret) {
		dev_err(&pdev->dev, "failed to create analog file in " ADC_VT900_DEV "\n");
		return ret;
	}

	platform_set_drvdata(pdev, gpadc4ch);

	gpadc4ch->mdev.minor  = MISC_DYNAMIC_MINOR;
	gpadc4ch->mdev.name   = DEVICE_NAME;
	gpadc4ch->mdev.fops   = &fops;
	gpadc4ch->mdev.parent = NULL;

	ret = misc_register(&gpadc4ch->mdev);
	if (ret) {
		dev_err(&pdev->dev, "Failed to register miscdev\n");
		return ret;
	}

	dev_info(&pdev->dev, "Registered\n");
	return 0;
}

static int sun4i_gpadc4ch_remove(struct platform_device *pdev)
{
	struct sun4i_gpadc4ch_data *gpadc4ch = platform_get_drvdata(pdev);

	sun4i_disable_adc(gpadc4ch);

	/* Deactivate all IRQs */
	writel(0, gpadc4ch->base + TP_INT);

	misc_deregister(&gpadc4ch->mdev);

	device_remove_file(&pdev->dev, &dev_attr_analog);
	mutex_destroy(&gpadc4ch->mutex);

	dev_info(&pdev->dev, "Removed\n");

	return 0;
}



static struct platform_driver sun4i_gpadc4ch_driver = {
	.driver = {
		.name	= "sun4i-gpadc4ch",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(sun4i_gpadc4ch_of_match),
	},
	.probe	= sun4i_gpadc4ch_probe,
	.remove	= sun4i_gpadc4ch_remove,
};

module_platform_driver(sun4i_gpadc4ch_driver);

MODULE_DESCRIPTION("Allwinner gpadc driver");
MODULE_AUTHOR("Mikhail Kofanov <c5r@yandex.ru>");
MODULE_LICENSE("GPL");
