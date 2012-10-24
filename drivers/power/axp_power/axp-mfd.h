#ifndef __LINUX_AXP_MFD_H_
#define __LINUX_AXP_MFD_H_

/* Unified sub device IDs for AXP */
/* LDO0 For RTCLDO ,LDO1-3 for ALDO,LDO*/
enum {		
	AW1636_ID_LDO1,   //RTCLDO
	AW1636_ID_LDO2,   //ALDO1
	AW1636_ID_LDO3,   //ALDO2
	AW1636_ID_LDO4,   //ALDO3
	AW1636_ID_LDO5,   //DLDO1
	AW1636_ID_LDO6,   //DLDO2
	AW1636_ID_LDO7,   //DLDO3
	AW1636_ID_LDO8,   //DLDO4
	AW1636_ID_LDO9,   //ELDO1
	AW1636_ID_LDO10,  //ELDO2
	AW1636_ID_LDO11,  //ELDO3
	AW1636_ID_LDO12,  //DC5LDO
	AW1636_ID_DCDC1,
	AW1636_ID_DCDC2,
	AW1636_ID_DCDC3,
	AW1636_ID_DCDC4,
	AW1636_ID_DCDC5,
	AW1636_ID_LDOIO0,
	AW1636_ID_LDOIO1.
};

#define AXP_MFD_ATTR(_name)					\
{									\
	.attr = { .name = #_name,.mode = 0644 },					\
	.show =  _name##_show,				\
	.store = _name##_store, \
}

/* AXP battery charger data */
struct power_supply_info;

struct axp_supply_init_data {
	/* battery parameters */
	struct power_supply_info *battery_info;

	/* current and voltage to use for battery charging */
	unsigned int chgcur;
	unsigned int chgvol;
	unsigned int chgend;
	/*charger control*/
	bool chgen;
	bool limit_on;
	/*charger time */
	int chgpretime;
	int chgcsttime;

	/*adc sample time */
	unsigned int sample_time;

	/* platform callbacks for battery low and critical IRQs */
	void (*battery_low)(void);
	void (*battery_critical)(void);
};

struct axp_funcdev_info {
	int		id;
	const char	*name;
	void	*platform_data;
};

struct axp_platform_data {
	int num_regl_devs;
	int num_sply_devs;
	int num_gpio_devs;
	int gpio_base;
	struct axp_funcdev_info *regl_devs;
	struct axp_funcdev_info *sply_devs;
	struct axp_funcdev_info *gpio_devs;

};

struct axp_mfd_chip {
	struct i2c_client	*client;
	struct device		*dev;
	struct axp_mfd_chip_ops	*ops;

	int			type;
	uint64_t		irqs_enabled;

	struct mutex		lock;
	struct work_struct	irq_work;

	struct blocking_notifier_head notifier_list;
};

struct axp_mfd_chip_ops {
	int	(*init_chip)(struct axp_mfd_chip *);
	int	(*enable_irqs)(struct axp_mfd_chip *, uint64_t irqs);
	int	(*disable_irqs)(struct axp_mfd_chip *, uint64_t irqs);
	int	(*read_irqs)(struct axp_mfd_chip *, uint64_t *irqs);
};


/*For AW1636*/ 
#define AW1636                       36
#define AW1636_STATUS              (0x00)
#define AW1636_MODE_CHGSTATUS      (0x01)
#define AW1636_PMU_TYPE		       (0x03)
#define AW1636_BUFFER1   	       (0x04)
#define AW1636_BUFFER2             (0x05)
#define AW1636_BUFFER3             (0x06)
#define AW1636_BUFFER4             (0x07)
#define AW1636_BUFFER5             (0x08)
#define AW1636_BUFFER6             (0x09)
#define AW1636_BUFFER7             (0x0A)
#define AW1636_BUFFER8             (0x0B)
#define AW1636_BUFFER9             (0x0C)
#define AW1636_BUFFERA             (0x0D)
#define AW1636_BUFFERB             (0x0E)
#define AW1636_BUFFERC             (0x0F)
#define AW1636_IPS_SET             (0x30)
#define AW1636_VOFF_SET            (0x31)
#define AW1636_OFF_CTL             (0x32)
#define AW1636_CHARGE1             (0x33)
#define AW1636_CHARGE2             (0x34)
#define AW1636_POK_SET             (0x36)
#define AW1636_INTEN1              (0x40)
#define AW1636_INTEN2              (0x41)
#define AW1636_INTEN3              (0x42)
#define AW1636_INTEN4              (0x43)
#define AW1636_INTEN5              (0x44)
#define AW1636_INTSTS1             (0x48)
#define AW1636_INTSTS2             (0x49)
#define AW1636_INTSTS3             (0x4A)
#define AW1636_INTSTS4             (0x4B)
#define AW1636_INTSTS5             (0x4C)

#define AW1636_LDO_DC_EN1          (0X10)
#define AW1636_LDO_DC_EN2          (0X12)
#define AW1636_LDO_DC_EN3          (0X13)
#define AW1636_DLDO1OUT_VOL        (0x15)
#define AW1636_DLDO2OUT_VOL        (0x16)
#define AW1636_DLDO3OUT_VOL        (0x17)
#define AW1636_DLDO4OUT_VOL        (0x18)
#define AW1636_ELDO1OUT_VOL        (0x19)
#define AW1636_ELDO2OUT_VOL        (0x1A)
#define AW1636_ELDO3OUT_VOL        (0x1B)
#define AW1636_DC5LDOOUT_VOL       (0x1C)
#define AW1636_DC1OUT_VOL          (0x21)
#define AW1636_DC2OUT_VOL          (0x22)
#define AW1636_DC3OUT_VOL          (0x23)
#define AW1636_DC4OUT_VOL          (0x24)
#define AW1636_DC5OUT_VOL          (0x25)
#define AW1636_GPIO0LDOOUT_VOL     (0x91)
#define AW1636_GPIO1LDOOUT_VOL     (0x93)
#define AW1636_ALDO1OUT_VOL        (0x28)
#define AW1636_ALDO2OUT_VOL        (0x29)
#define AW1636_ALDO3OUT_VOL        (0x2A)

#define AW1636_DCDC_MODESET        (0x80)
#define AW1636_DCDC_FREQSET        (0x37) 
#define AW1636_ADC_EN              (0x82)
#define AW1636_HOTOVER_CTL         (0x8F)

#define AW1636_GPIO0_CTL           (0x90)
#define AW1636_GPIO1_CTL           (0x92)
#define AW1636_GPIO01_SIGNAL       (0x94)
#define AW1636_BAT_CHGCOULOMB3         (0xB0)
#define AW1636_BAT_CHGCOULOMB2         (0xB1)
#define AW1636_BAT_CHGCOULOMB1         (0xB2)
#define AW1636_BAT_CHGCOULOMB0         (0xB3)
#define AW1636_BAT_DISCHGCOULOMB3      (0xB4)
#define AW1636_BAT_DISCHGCOULOMB2      (0xB5)
#define AW1636_BAT_DISCHGCOULOMB1      (0xB6)
#define AW1636_BAT_DISCHGCOULOMB0      (0xB7)
#define AW1636_COULOMB_CTL             (0xB8)



/* bit definitions for AXP events ,irq event */
/*  AW1636  */
#define	AW1636_IRQ_USBLO		( 1 <<  1)
#define	AW1636_IRQ_USBRE		( 1 <<  2)
#define	AW1636_IRQ_USBIN		( 1 <<  3)
#define	AW1636_IRQ_USBOV     ( 1 <<  4)
#define	AW1636_IRQ_ACRE     ( 1 <<  5)
#define	AW1636_IRQ_ACIN     ( 1 <<  6)
#define	AW1636_IRQ_ACOV     ( 1 <<  7)
#define	AW1636_IRQ_TEMLO      ( 1 <<  8)
#define	AW1636_IRQ_TEMOV      ( 1 <<  9)
#define	AW1636_IRQ_CHAOV		( 1 << 10)
#define	AW1636_IRQ_CHAST 	    ( 1 << 11)
#define	AW1636_IRQ_BATATOU    ( 1 << 12)
#define	AW1636_IRQ_BATATIN  	( 1 << 13)
#define AW1636_IRQ_BATRE		( 1 << 14)
#define AW1636_IRQ_BATIN		( 1 << 15)
#define	AW1636_IRQ_POKLO		( 1 << 16)
#define	AW1636_IRQ_POKSH	    ( 1 << 17)
#define AW1636_IRQ_ICTEMOV    ( 1 << 23)
#define AW1636_IRQ_EXTLOWARN1  ( 1 << 24)
#define AW1636_IRQ_EXTLOWARN2  ( 1 << 25)
#define AW1636_IRQ_GPIO0TG     ( 1 << 32)
#define AW1636_IRQ_GPIO1TG     ( 1 << 33)
#define AW1636_IRQ_GPIO2TG     ( 1 << 34)
#define AW1636_IRQ_GPIO3TG     ( 1 << 35)

//#define AW1636_IRQ_PEKFE     ( 1 << 37)
//#define AW1636_IRQ_PEKRE     ( 1 << 38)
#define AW1636_IRQ_TIMER     ( 1 << 39)


/* Status Query Interface */
/*  AW1636  */
#define AW1636_STATUS_SOURCE    ( 1 <<  0)
#define AW1636_STATUS_ACUSBSH ( 1 <<  1)
#define AW1636_STATUS_BATCURDIR ( 1 <<  2)
#define AW1636_STATUS_USBLAVHO ( 1 <<  3)
#define AW1636_STATUS_USBVA    ( 1 <<  4)
#define AW1636_STATUS_USBEN    ( 1 <<  5)
#define AW1636_STATUS_ACVA	    ( 1 <<  6)
#define AW1636_STATUS_ACEN	    ( 1 <<  7)

#define AW1636_STATUS_BATINACT  ( 1 << 11)

#define AW1636_STATUS_BATEN     ( 1 << 13)
#define AW1636_STATUS_INCHAR    ( 1 << 14)
#define AW1636_STATUS_ICTEMOV   ( 1 << 15)


extern struct device *axp_get_dev(void);
extern int axp_register_notifier(struct device *dev,
		struct notifier_block *nb, uint64_t irqs);
extern int axp_unregister_notifier(struct device *dev,
		struct notifier_block *nb, uint64_t irqs);


/* NOTE: the functions below are not intended for use outside
 * of the AXP sub-device drivers
 */
extern int axp_write(int reg, uint8_t val);
extern int axp_writes(int reg, int len, uint8_t *val);
extern int axp_read(int reg, uint8_t *val);
extern int axp_reads(int reg, int len, uint8_t *val);
extern int axp_update(int reg, uint8_t val, uint8_t mask);
extern int axp_set_bits(int reg, uint8_t bit_mask);
extern int axp_clr_bits(int reg, uint8_t bit_mask);
//extern struct i2c_client *axp;
#endif /* __LINUX_PMIC_AXP_H */
