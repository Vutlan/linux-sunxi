 #ifndef	_LINUX_AXP_SPLY_H_
#define	_LINUX_AXP_SPLY_H_
#include <linux/power_supply.h>
#include <linux/mfd/axp-mfd.h>

/*      AW1636      */
static 	struct input_dev * powerkeydev;

#define AW1636_CHARGE_STATUS					AW1636_STATUS
#define AW1636_IN_CHARGE						(1 << 6)
#define AW1636_PDBC                             (0x32)

#define AW1636_CHARGE_CONTROL1				AW1636_CHARGE1
#define AW1636_CHARGER_ENABLE				(1 << 7)
#define AW1636_CHARGE_CONTROL2				AW1636_CHARGE2
#define AW1636_BUCHARGE_CONTROL				AW1636_BACKUP_CHG
#define AW1636_BUCHARGER_ENABLE				(1 << 7)
#define AW1636_CHARGE_VBUS					AW1636_IPS_SET
#define AW1636_CAP                          (0xB9)
#define AW1636_CCHAR3_RES					AW1636_BAT_CHGCOULOMB3
#define AW1636_CCHAR2_RES					AW1636_BAT_CHGCOULOMB2
#define AW1636_CCHAR1_RES					AW1636_BAT_CHGCOULOMB1
#define AW1636_CCHAR0_RES					AW1636_BAT_CHGCOULOMB0
#define AW1636_CDISCHAR3_RES					AW1636_BAT_DISCHGCOULOMB3
#define AW1636_CDISCHAR2_RES					AW1636_BAT_DISCHGCOULOMB2
#define AW1636_CDISCHAR1_RES					AW1636_BAT_DISCHGCOULOMB1
#define AW1636_CDISCHAR0_RES					AW1636_BAT_DISCHGCOULOMB0

#define AW1636_FAULT_LOG1					AW1636_MODE_CHGSTATUS
#define AW1636_FAULT_LOG_CHA_CUR_LOW			(1 << 2)
#define AW1636_FAULT_LOG_BATINACT			(1 << 3)

#define AW1636_FAULT_LOG_OVER_TEMP			(1 << 7)

#define AW1636_FAULT_LOG2					AW1636_INTSTS2
#define AW1636_FAULT_LOG_COLD				(1 << 0)

#define AW1636_FINISH_CHARGE					(1 << 2)

#define AW1636_COULOMB_CONTROL				AW1636_COULOMB_CTL
#define AW1636_COULOMB_ENABLE				(1 << 7)
#define AW1636_COULOMB_SUSPEND				(1 << 6)
#define AW1636_COULOMB_CLEAR					(1 << 5)


#define AW1636_ADC_CONTROL					AW1636_ADC_EN
#define AW1636_ADC_BATVOL_ENABLE				(1 << 7)
#define AW1636_ADC_BATCUR_ENABLE				(1 << 6)
#define AW1636_ADC_DCINVOL_ENABLE			(1 << 5)
#define AW1636_ADC_DCINCUR_ENABLE			(1 << 4)
#define AW1636_ADC_USBVOL_ENABLE				(1 << 3)
#define AW1636_ADC_USBCUR_ENABLE				(1 << 2)
#define AW1636_ADC_APSVOL_ENABLE				(1 << 1)
#define AW1636_ADC_TSVOL_ENABLE				(1 << 0)
#define AW1636_ADC_INTERTEM_ENABLE			(1 << 7)

#define AW1636_ADC_GPIO0_ENABLE				(1 << 3)
#define AW1636_ADC_GPIO1_ENABLE				(1 << 2)
#define AW1636_ADC_GPIO2_ENABLE				(1 << 1)
#define AW1636_ADC_GPIO3_ENABLE				(1 << 0)

#define AW1636_ADC_CONTROL3                 (0x84)

#define AW1636_INTTEMP                      (0x56)
#define AW1636_DATA_BUFFER0					AW1636_BUFFER1
#define AW1636_DATA_BUFFER1					AW1636_BUFFER2
#define AW1636_DATA_BUFFER2					AW1636_BUFFER3
#define AW1636_DATA_BUFFER3					AW1636_BUFFER4
#define AW1636_DATA_BUFFER4					AW1636_BUFFER5
#define AW1636_DATA_BUFFER5					AW1636_BUFFER6
#define AW1636_DATA_BUFFER6					AW1636_BUFFER7
#define AW1636_DATA_BUFFER7					AW1636_BUFFER8
#define AW1636_DATA_BUFFER8					AW1636_BUFFER9
#define AW1636_DATA_BUFFER9					AW1636_BUFFERA
#define AW1636_DATA_BUFFERA					AW1636_BUFFERB
#define AW1636_DATA_BUFFERB					AW1636_BUFFERC
#define AW1636_IC_TYPE						AW1636_PMU_TYPE

// const uint32_t AW1636_NOTIFIER_ON =	(AW1636_IRQ_USBIN |AW1636_IRQ_USBRE |AW1636_IRQ_ACIN |AW1636_IRQ_ACRE |AW1636_IRQ_BATIN |
// 				       				AW1636_IRQ_BATRE |
// 				       				AW1636_IRQ_CHAST |
// 				       				//AW1636_IRQ_PEKFE |
// 				       				//AW1636_IRQ_PEKRE |
// 				       				AW1636_IRQ_CHAOV );

#define AXP_CHG_ATTR(_name)					\
{									\
	.attr = { .name = #_name,.mode = 0644 },					\
	.show =  _name##_show,				\
	.store = _name##_store, \
}

struct axp_adc_res {//struct change
	uint16_t vbat_res;
	uint16_t ibat_res;
	uint16_t ichar_res;
	uint16_t idischar_res;
	uint16_t vac_res;
	uint16_t iac_res;
	uint16_t vusb_res;
	uint16_t iusb_res;
};

struct axp_charger {
	/*power supply sysfs*/
	struct power_supply batt;
	struct power_supply	ac;
	struct power_supply	usb;
	struct power_supply bubatt;

	/*i2c device*/
	struct device *master;

	/* adc */
	struct axp_adc_res *adc;
	unsigned int sample_time;

	/*monitor*/
	struct delayed_work work;
	unsigned int interval;

	/*battery info*/
	struct power_supply_info *battery_info;

	/*charger control*/
	bool chgen;
	bool limit_on;
	unsigned int chgcur;
	unsigned int chgvol;
	unsigned int chgend;

	/*charger time */
	int chgpretime;
	int chgcsttime;

	/*external charger*/
	bool chgexten;
	int chgextcur;

	/* charger status */
	bool bat_det;
	bool is_on;
	bool is_finish;
	bool ac_not_enough;
	bool ac_det;
	bool usb_det;
	bool ac_valid;
	bool usb_valid;
	bool ext_valid;
	bool bat_current_direction;
	bool in_short;
	bool batery_active;
	bool low_charge_current;
	bool int_over_temp;
	uint8_t fault;
	int charge_on;

	int vbat;
	int ibat;
	int pbat;
	int vac;
	int iac;
	int vusb;
	int iusb;
	int ocv;
	
	int disvbat;
	int disibat;

	/*rest time*/
	int rest_vol;
	int ocv_rest_vol;
	int base_restvol;
	int rest_time;

	/*ic temperature*/
	int ic_temp;

	/*irq*/
	struct notifier_block nb;

	/* platform callbacks for battery low and critical events */
	void (*battery_low)(void);
	void (*battery_critical)(void);

	struct dentry *debug_file;
};

static struct task_struct *main_task;
//static uint8_t coulomb_flag;
static struct axp_charger *axp_charger;
//static int Total_Cap = 0;
//static int Cap_Index = 0;
//static int flag_state_change = 0;
//static int Bat_Cap_Buffer[AW1636_VOL_MAX];
//static int counter = 0;
static int bat_cap = 0;

#endif
