

#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <asm/irq.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <mach/irqs.h>
#include <mach/system.h>
#include <mach/hardware.h>
#include <linux/workqueue.h>
#include <linux/vmalloc.h>
#include <linux/gpio.h>
#include <linux/mutex.h>

#include <linux/time.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/string.h>

#include "sky25filters.h"
#include "vt900-gpadc.h"

#define IRQ_TP                 29
#define TP_BASSADDRESS         0xf1c25000
#define TP_CTRL0               0x00
#define TP_CTRL1               0x04
#define TP_CTRL2               0x08
#define TP_CTRL3               0x0c
#define TP_INT_FIFOC           0x10
#define TP_INT_FIFOS           0x14
#define TP_TPR                 0x18
#define TP_CDAT                0x1c
#define TEMP_DATA              0x20
#define TP_DATA                0x24
#define TP_IO_CONFIG           0x28
#define TP_PORT_DATA           0x2c

/* TP_CTRL0 */
#define FIRST_DLY             7
#define ADC_FIRST_DLY         (FIRST_DLY<<24)
// clk/6
#define ADC_CLK_DIVIDER        (0x2<<20)
#define CLK                    7
#define FS_DIV                 (CLK<<16)
#define ACQ                    0xe8
#define T_ACQ                  ACQ

/* TP_CTRL1 */
#define CHOP_TEMP_EN		   (1<<7)
#define TOUCH_PAN_CALI_EN      (1<<6)
#define TP_CTRL_MODE_EN        (1<<4)
#define TP_ADC_SELECT		   (1<<3)
#define ADC_CHAN_SELECT(N)	   (N)

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

/* TP_FIFOCS */
#define TP_TEMP_IRQ_PENDING	   (1<<18)
#define FIFO_OVERRUN_PENDING   (1<<17)
#define FIFO_DATA_PENDING	   (1<<16)
#define RXA_CNT(r)			   (((1<<(12-8))-1)&(r>>8))
#define TP_UP_PENDING          (1<<1)
#define TP_DOWN_PENDING        (1<<0)

/*  */
#define TP_TPR_TEMP_ENABLE	(1<<16)
#define TP_TPR_TEMP_PERIOD	0x0fff



#define ADC_VT900_DEV "adc"

static int adc_mux_pin0 = -1;
module_param(adc_mux_pin0, int, 0664);
MODULE_PARM_DESC(adc_mux_pin0, "External multilpexor pin0 number");

static int adc_offset = 0;
module_param(adc_offset, int, 0664);
MODULE_PARM_DESC(adc_offset, "ADC offset correction (raw)");


/*
 * device.
 */
static struct sky25_adc_device {
	struct resource *res;
	void __iomem *base;

	int irq;
	//volatile u32 intstatus;

	//spinlock_t lock;
	struct mutex result_mutex;


	/* work for result processing */
	struct work_struct work_adc;

	unsigned long size;
	void *data;


	/* filters */
	struct filter_sky25 analog[ADC_VT100V1_MUX_NUM];
	struct filter_sky25 brdlife[ADC_VT100V1_BLF_NUM];

	struct timespec ts;
	u32 measure_all[4][ADC_VT100V1_MUX_NUM/2];

	u8 mux;
	u8 mux_prev;
} Device;


static u32 calc_adc_step(void) {
	u32 step;
	// adc step = 2500mV / adc_code
	step= 5000000ul/(sky25_filter_get(&Device.brdlife[0]) & ~ADC_FILTER_VT100V1_SURGE);
	return (step & 1)? (step>>1)+1 : step>>1;
}
static u32 adc_to_mv(u32 val, u32 step) {
	u32 tmp;
	tmp = (((val & ~ADC_FILTER_VT100V1_SURGE)*step)+500ul)/1000ul;
	return tmp | (val & ADC_FILTER_VT100V1_SURGE);
}

static ssize_t analog_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int len;
	int i;
	u32 step, val;

	mutex_lock(&Device.result_mutex);
	// timestamp: sec.nsec
	len = scnprintf(buf, PAGE_SIZE, "%ld.%ld\n", Device.ts.tv_sec, Device.ts.tv_nsec);

	step= calc_adc_step();
	// 8 channel: detect:value
	for (i=0; i<ADC_VT100V1_MUX_NUM/2; i++) {
		val = sky25_filter_get(&Device.analog[i*2+1]);
		len+= scnprintf(buf+len, PAGE_SIZE-len, "%u:%u\n",
				sky25_filter_get(&(Device.analog[i*2])),	// detect (raw)
				adc_to_mv(val, step) ); 					// value (mv)
	}
	len+= scnprintf(buf+len, PAGE_SIZE-len, "%u\n%u\n",
				sky25_filter_get(&Device.brdlife[0]),		// vref_raw
				adc_to_mv(sky25_filter_get(&Device.brdlife[1]), step));	// V12 (mv)
	mutex_unlock(&Device.result_mutex);
	return len;
}
/*
static ssize_t analog_store(struct device *dev, struct device_attribute *attr,
		char *buf, size_t count)
{
	sscanf(buf, "%du", &foo);
	return count;
}
 */
static DEVICE_ATTR(analog,  0444, analog_show,  /*analog_store*/NULL);



static int sky25_mux_init(void) {

	Device.mux = 0; // 0 channel
	Device.mux_prev = ~Device.mux;

	if(( gpio_request(adc_mux_pin0, "ASENS_MUX0")<0 )
			|| ( gpio_request(adc_mux_pin0+1, "ASENS_MUX1")<0 )
			|| ( gpio_request(adc_mux_pin0+2, "ASENS_MUX2")<0 ))
	{
		printk (KERN_INFO "%s request mux gpio failed\n", __FUNCTION__);
		return -1;
	}

	gpio_direction_output(adc_mux_pin0, 0);
	gpio_direction_output(adc_mux_pin0+1, 0);
	gpio_direction_output(adc_mux_pin0+2, 0);

	return 0;
}
static void sky25_mux_deinit(void) {
	gpio_free(adc_mux_pin0);
	gpio_free(adc_mux_pin0+1);
	gpio_free(adc_mux_pin0+2);
}

static inline void sky25_mux_switch(u8 ch) {
	u8 changed;
	changed = Device.mux_prev ^ ch;
	if (changed & 1)
		gpio_set_value(adc_mux_pin0, (ch>>0)&1);
	if (changed & 2)
		gpio_set_value(adc_mux_pin0+1, (ch>>1)&1);
	if (changed & 4)
		gpio_set_value(adc_mux_pin0+2, (ch>>2)&1);
	Device.mux_prev = ch;
}

static inline void adc_filter_create(void) {
	int i;

	for(i=0; i<ADC_VT100V1_MUX_NUM; i++) {
		sky25_filter_create(&(Device.analog[i]), ADC_FILTER_VT100V1_DC, ADC_FILTER_VT100V1_AC, ADC_FILTER_VT100V1_DELTA);
	}
	for(i=0; i<ADC_VT100V1_BLF_NUM; i++) {
		sky25_filter_create(&(Device.brdlife[i]), ADC_FILTER_VT100V1_DC, ADC_FILTER_VT100V1_AC, ADC_FILTER_VT100V1_DELTA);
	}
}

/*
static void fifo_flush(void)
{
	u32 reg;
#if 0
	reg = readl(Device.base+TP_INT_FIFOC);
	writel(reg | TP_FIFO_FLUSH, Device.base+TP_INT_FIFOC);
	msleep(2000);

	//while(mmio_readl(&io, TP_INT) & TP_FIFO_FLUSH) ;
	//while (mmio_readl(&io, TP_FIFOCS) & (31<<8)) ;

	reg = readl(TP_CTRL1);
	writel(reg & (~TP_CTRL_MODE_EN), Device.base+TP_CTRL1);
#else
	do
	{
		readl(Device.base+TP_DATA);
		reg=(readl(Device.base+TP_INT_FIFOS) >> 8) & 0x1f;
	}
	while(reg!=0);
#endif
}
*/

/*
 * ADC settings
 */
static void adc_init(void)
{
	u32 reg;

	//printk(KERN_ERR "%s\n", __FUNCTION__);

	// clock setup
	writel(ADC_CLK_DIVIDER | FS_DIV | T_ACQ, Device.base+TP_CTRL0);

	writel(0*CHOP_TEMP_EN | TP_ADC_SELECT | ADC_CHAN_SELECT(4), Device.base+TP_CTRL1);
	writel(0*FILTER_EN | FILTER_5_3, Device.base+TP_CTRL3);

	writel(0x800, Device.base+TP_CDAT);

	// fifo setup
	writel(TP_FIFO_MODE_SELECT(2), Device.base+TP_CTRL2);  //
	writel(0*TP_DATA_IRQ_EN | TP_FIFO_FLUSH | TP_FIFO_TRIG_LEVEL(4), Device.base+TP_INT_FIFOC); // прерывание после 4 отсчетов
	udelay(2000);

	// pad settings
	writel(0x2222, Device.base+TP_IO_CONFIG);
	writel(0x00, Device.base+TP_PORT_DATA);

	// clear all status flags
	reg=readl(Device.base+TP_INT_FIFOS);
	writel(reg, Device.base+TP_INT_FIFOS);

	//writel(TP_TPR_TEMP_ENABLE | 0x01ff, Device.base+TP_TPR) ;

	adc_filter_create();

	// enable interrupt
	writel(1*TP_DATA_IRQ_EN | 0*TP_FIFO_FLUSH | TP_FIFO_TRIG_LEVEL(4), Device.base+TP_INT_FIFOC); // прерывание после 4 отсчетов
	//printk(KERN_INFO "%s [1] FIFOS=0x%x\n", __FUNCTION__, readl(Device.base+TP_INT_FIFOS));
}


static void adc_start_one_conversion(void)
{
	u32 reg;

	//printk(KERN_INFO "%s [1] FIFOS=0x%x\n", __FUNCTION__, readl(Device.base+TP_INT_FIFOS));

	// flush fifo
	while((readl(Device.base+TP_INT_FIFOS) >> 8) & 0x1f)
		readl(Device.base+TP_DATA);

	reg = readl(Device.base+TP_INT_FIFOS);
	//printk(KERN_INFO "%s [3] FIFOS=0x%x\n", __FUNCTION__, reg);
	writel(reg & (TP_TEMP_IRQ_PENDING | FIFO_OVERRUN_PENDING |	FIFO_DATA_PENDING |
			TP_UP_PENDING | TP_DOWN_PENDING), Device.base+TP_INT_FIFOS);

	//printk(KERN_INFO "%s [3] FIFOS=0x%x\n", __FUNCTION__, readl(Device.base+TP_INT_FIFOS));
	// start conversion
	reg = readl(Device.base+TP_CTRL1);
	writel(reg | TP_CTRL_MODE_EN, Device.base+TP_CTRL1);

	// only one conversion
	reg = readl(Device.base+TP_CTRL1);
	writel(reg & ~TP_CTRL_MODE_EN, Device.base+TP_CTRL1);
}


static void adc_stop(void)
{
	u32 reg;

	// deactivate all IRQ
	writel(0, Device.base+TP_INT_FIFOC);
	reg = readl(Device.base+TP_CTRL1);
	writel(reg & (~TP_CTRL_MODE_EN), Device.base+TP_CTRL1);
}





/*
 * ADC interrupt handler
 */
static inline s32 adc_get(void) {
	s32 val;
	val = readl(Device.base + TP_DATA)+adc_offset;
	if (val < 0)
		val = 0;
	if (val >= (4096+256))
		val = 4096+256;
	return val;
}
static irqreturn_t vt900_gpadc_handle_irq(int irq, void *dev_id)
{
	struct platform_device *pdev = dev_id;
	struct sky25_adc_device *adc_data = (struct sky25_adc_device *)platform_get_drvdata(pdev);
	u32 intstatus;

	intstatus = readl(adc_data->base + TP_INT_FIFOS);

	if ((intstatus & FIFO_DATA_PENDING) && (RXA_CNT(intstatus)>=4)) {
		//writel(0*TP_DATA_IRQ_EN | 0*TP_FIFO_FLUSH | TP_FIFO_TRIG_LEVEL(4), Device.base+TP_INT_FIFOC); // прерывание после 4 отсчетов

		adc_data->measure_all[0][Device.mux] = adc_get();	// 0 - Vref
		adc_data->measure_all[1][Device.mux] = adc_get();	// 1 - detect[mux]
		adc_data->measure_all[2][Device.mux] = adc_get();	// 2 - Vaux
		adc_data->measure_all[3][Device.mux] = adc_get();	// 3 - value[mux]

		Device.mux = (Device.mux+1)%(ADC_VT100V1_MUX_NUM>>1);
		sky25_mux_switch(Device.mux);
		//printk(KERN_ERR "new_mux=%d\tFIFOS=0x%x\n", Device.mux, readl(adc_data->base + TP_INT_FIFOS));

		writel(intstatus & FIFO_DATA_PENDING, adc_data->base + TP_INT_FIFOS);

		if(!Device.mux) {
			schedule_work(&Device.work_adc);
		}
		else
			adc_start_one_conversion();

		return IRQ_HANDLED;
	}
	//if (intstatus & TP_TEMP_IRQ_PENDING) {	}
	writel(intstatus, adc_data->base + TP_INT_FIFOS);
	printk(KERN_WARNING "%s intstatus=0x%x\n", __FUNCTION__, intstatus);
	return IRQ_HANDLED;
}

// period = 30ms
static void adc_proc(struct work_struct *work) {
	struct sky25_adc_device *dev = container_of(work, struct sky25_adc_device, work_adc);
	//struct sky25_adc_data* pdata=(struct sky25_adc_data*)(dev->data);
	int i;
	//u32 step;

	//printk(KERN_ERR "%s\n", __FUNCTION__);

	mutex_lock(&Device.result_mutex);
	getrawmonotonic(&Device.ts);  // timestamp
	for (i=0; i<ADC_VT100V1_MUX_NUM/2; i++) {
		/* board life channels */
		sky25_filter_put(&(dev->brdlife[0]), dev->measure_all[0][i]);
		sky25_filter_put(&(dev->brdlife[1]), dev->measure_all[2][i]);

		/* measure channels */
		sky25_filter_put(&(dev->analog[i*2]), dev->measure_all[1][i]);
		sky25_filter_put(&(dev->analog[i*2+1]), dev->measure_all[3][i]);

		/*
		step= 5000000ul/sky25_filter_get(&dev->brdlife[0]);
		step= (step & 1)? (step>>1)+1 : step>>1;

		printk(KERN_ERR "[%02d]: %d:%d  %04x:%04x  %d:%d  %d:%d\n", i,
				//dev->measure_all[0][i], sky25_filter_get(&(dev->brdlife[0])),
				sky25_filter_get(&dev->brdlife[0]), step,
				dev->measure_all[1][i], sky25_filter_get(&(dev->analog[i*2])),
				//dev->measure_all[2][i], sky25_filter_get(&(dev->brdlife[1])),
				sky25_filter_get(&(dev->brdlife[1])), sky25_filter_get(&(dev->brdlife[1]))* step,
				//dev->measure_all[3][i], sky25_filter_get(&(dev->analog[i*2+1])) );
				sky25_filter_get(&(dev->analog[i*2+1])), sky25_filter_get(&(dev->analog[i*2+1])) * step );
		 */
	}
	mutex_unlock(&Device.result_mutex);
	adc_start_one_conversion();
}



static int __devinit vt900_gpadc_probe(struct platform_device *pdev)
{
	int err = 0;
	int irq = platform_get_irq(pdev, 0);

	dev_info(&pdev->dev, "adc_mux_pin0=%d\tadc_offset=%d\n", adc_mux_pin0, adc_offset);
	if ((adc_mux_pin0 <0) || (abs(adc_offset)>256)) {
		dev_err(&pdev->dev, " invalid parameter\n");
		return -ENODEV;
	}


	mutex_init(&Device.result_mutex);

	Device.res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!Device.res) {
		err = -ENOMEM;
		dev_err(&pdev->dev, "Cannot get the MEMORY\n");
		goto label_err1;
	}
#if 0
	if (!devm_request_mem_region(&pdev->dev, Device.res->start,
			resource_size(Device.res), Device.res->name)) {
		err = -ENOMEM;
		dev_err(&pdev->dev, " can't allocate %d bytes at %d address\n",
				resource_size(Device.res), Device.res->start);
		goto label_err1;
	}

	Device.base = devm_ioremap_nocache(&pdev->dev, Device.res->start, resource_size(Device.res));
	if (!Device.base) {
		err = -ENOMEM;
		dev_err(&pdev->dev, " ioremap failed\n");
		goto label_err1;
	}
#else
	Device.base = (void __iomem *)TP_BASSADDRESS;
#endif

	if (sky25_mux_init() < 0) {
		err = -ENODEV;
		goto label_err1;
	}


	Device.irq = irq;
	err = request_irq(irq, vt900_gpadc_handle_irq,
			IRQF_DISABLED, pdev->name, pdev);
	if (err) {
		dev_err(&pdev->dev, "Cannot request keypad IRQ\n");
		goto label_err1;
	}

	platform_set_drvdata(pdev, (void*)&Device);

	// sysfs interface
	err = device_create_file(&pdev->dev, &dev_attr_analog);
	if (err) {
		dev_err(&pdev->dev, "failed to create analog file in " ADC_VT900_DEV "\n");
		goto label_err2;
	}
	/*
	err = device_create_file(&pdev->dev, &dev_attr_brdlife);
	if (err) {
		device_remove_file(&pdev->dev, &dev_attr_analog);
		dev_err(&pdev->dev, "failed to create brdlife file in " ADC_VT900_DEV "\n");
		goto label_err2;
	}
	*/

	adc_init();
	INIT_WORK(&(Device.work_adc), adc_proc );
	adc_start_one_conversion();

	dev_info(&pdev->dev, "sun7i GPADC initialization success\n");
	return 0;


	label_err2:
	free_irq(Device.irq, pdev);

	label_err1:
	return err;
}

static int __devexit vt900_gpadc_remove(struct platform_device *pdev)
{
	adc_stop();

	free_irq(Device.irq, pdev);

	cancel_work_sync(&Device.work_adc);

	sky25_mux_deinit();

	device_remove_file(&pdev->dev, &dev_attr_analog);
	//device_remove_file(&pdev->dev, &dev_attr_brdlife);
	mutex_destroy(&Device.result_mutex);

	//printk(KERN_ERR "%s Exit\n", __FUNCTION__);
	dev_info(&pdev->dev, "remove\n");

	platform_set_drvdata(pdev, NULL);
	return 0;
}

static struct platform_driver vt900_gpadc_driver = {
		.probe		= vt900_gpadc_probe,
		.remove		= __devexit_p(vt900_gpadc_remove),
		.driver		= {
				.name	= "vt900_gpadc",
		},
};


static void vt900_gpadc_release(struct device *dev)
{
	/* Nothing */
}

static struct resource vt900_gpadc_resource[] = {
		{
				.flags  = IORESOURCE_IRQ,
				.start  = SW_INT_IRQNO_TOUCH_PANEL ,
				.end    = SW_INT_IRQNO_TOUCH_PANEL ,
		},
		{
				.flags	= IORESOURCE_MEM,
				.start	= TP_BASSADDRESS,
				.end	= TP_BASSADDRESS + 0x100-1,
		},
};



struct platform_device vt900_gpadc_device = {
		.name		= "vt900_gpadc",
		.id		    = -1,
		.dev = {
				.release = vt900_gpadc_release,
		},
		.resource	= vt900_gpadc_resource,
		.num_resources	= ARRAY_SIZE(vt900_gpadc_resource),
};

static int __init vt900_gpadc_init(void)
{
	platform_device_register(&vt900_gpadc_device);
	return platform_driver_register(&vt900_gpadc_driver);
}

static void __exit vt900_gpadc_exit(void)
{
	platform_driver_unregister(&vt900_gpadc_driver);
	platform_device_unregister(&vt900_gpadc_device);
}

module_init(vt900_gpadc_init);
module_exit(vt900_gpadc_exit);

MODULE_AUTHOR("Vutlan s.r.o.");
MODULE_DESCRIPTION("VT900 ADC driver");
MODULE_LICENSE("GPL");



