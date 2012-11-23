#ifndef _CTP_H
#define _CTP_H

#define CTP_IRQ_NUMBER  GPIOA(3)

#define PIO_BASE_ADDRESS	(0x01c20800)
#define PIO_RANGE_SIZE		(0x400)
#define GPIO_ENABLE
#define SYSCONFIG_GPIO_ENABLE

#define PIO_INT_STAT_OFFSET          (0x214)
#define PIO_INT_CTRL_OFFSET          (0x210)

#define PIO_PN_DAT_OFFSET(n)         ((n)*0x24 + 0x10) 
//#define PIOI_DATA                    (0x130)
#define PIOH_DATA                    (0x10c)
#define PIOI_CFG3_OFFSET             (0x12c)

#define PRESS_DOWN	(1)
#define FREE_UP		(0)

#define IRQ_EINT0	(0)
#define IRQ_EINT1	(1)
#define IRQ_EINT2	(2)
#define IRQ_EINT3	(3)
#define IRQ_EINT4	(4)
#define IRQ_EINT5	(5)
#define IRQ_EINT6	(6)
#define IRQ_EINT7	(7)
#define IRQ_EINT8	(8)
#define IRQ_EINT9	(9)
#define IRQ_EINT10	(10)
#define IRQ_EINT11	(11)
#define IRQ_EINT12	(12)
#define IRQ_EINT13	(13)
#define IRQ_EINT14	(14)
#define IRQ_EINT15	(15)
#define IRQ_EINT16	(16)
#define IRQ_EINT17	(17)
#define IRQ_EINT18	(18)
#define IRQ_EINT19	(19)
#define IRQ_EINT20	(20)
#define IRQ_EINT21	(21)
#define IRQ_EINT22	(22)
#define IRQ_EINT23	(23)
#define IRQ_EINT24	(24)
#define IRQ_EINT25	(25)
#define IRQ_EINT26	(26)
#define IRQ_EINT27	(27)
#define IRQ_EINT28	(28)
#define IRQ_EINT29	(29)
#define IRQ_EINT30	(30)
#define IRQ_EINT31	(31)

typedef enum{
     PIO_INT_CFG0_OFFSET = 0x200,
     PIO_INT_CFG1_OFFSET = 0x204,
     PIO_INT_CFG2_OFFSET = 0x208,
     PIO_INT_CFG3_OFFSET = 0x20c
} int_cfg_offset;

typedef enum{
	POSITIVE_EDGE = 0x0,
	NEGATIVE_EDGE = 0x1,
	HIGH_LEVEL = 0x2,
	LOW_LEVEL = 0x3,
	DOUBLE_EDGE = 0x4
} ext_int_mode;

struct ctp_config_info{
        int ctp_used;
        int irq_number;
        __u32 twi_id;
        int screen_max_x;
        int screen_max_y;
        int revert_x_flag;
        int revert_y_flag;
        int exchange_x_y_flag; 
        int gpio_wakeup_hdle;
        int gpio_wakeup_enable;
        int gpio_int_hdle;               
};

enum{
        DEBUG_INIT = 1U << 0,
        DEBUG_SUSPEND = 1U << 1,
        DEBUG_INT_INFO = 1U << 2,
        DEBUG_X_Y_INFO = 1U << 3,
        DEBUG_KEY_INFO = 1U << 4,
        DEBUG_WAKEUP_INFO = 1U << 5,
        DEBUG_OTHERS_INFO = 1U << 6, 
                     
}; 
extern bool ctp_get_int_enable(u32 *enable);
extern bool ctp_set_int_enable(u32 enable);
extern bool ctp_get_int_port_rate(u32 *clk);
extern bool ctp_set_int_port_rate(u32 clk);
extern bool ctp_get_int_port_deb(u32 *clk_pre_scl);
extern bool ctp_set_int_port_deb(u32 clk_pre_scl);
extern int ctp_judge_int_occur(int number);
extern void ctp_clear_penirq(int number);
extern int ctp_set_irq_mode(char *major_key , char *subkey, ext_int_mode int_mode);
extern void ctp_free_platform_resource(void);
extern int  ctp_init_platform_resource(void);
extern void ctp_print_info(struct ctp_config_info info);
extern void ctp_wakeup(int status,int ms);

#endif