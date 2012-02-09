#ifndef GPIO_AW_H_INCLUDED
#define GPIO_AW_H_INCLUDED
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/rwsem.h>
#include <linux/timer.h>
struct gpio_aw_platdata {
	u32                 pio_hdle;
	unsigned int		 flags;
	char				*name;
	
};
struct gpio_aw_classdev{
	const char	*name;
	int port;					/*GPIO使用的端口号 1:PA 2:PB .... */
	int port_num;				/*GPIO在当前端口的序号(第几个引脚)  */
	int mul_sel;				/*GPIO的功能选择 0：输入  1：输出 */
	int pull;					/*GPIO的内置电阻状态 0代表高阻，1代表上拉，2代表下拉 */
	int drv_level;				/*GPIO的驱动能力 有0到3四个等级 */
	int data;					/*GPIO的电平 */
	int flags;
	#define AW_GPIO_SUSPENDED		(1 << 0)
	#define AW_GPIO_CORE_SUSPENDED		(1 << 16)
	int		(*gpio_aw_cfg_set)(struct gpio_aw_classdev *gpio_aw_cdev,
					int  mul_sel);		/*设置gpio的输入输出状态 */
	int		(*gpio_aw_pull_set)(struct gpio_aw_classdev *gpio_aw_cdev,
					int  pull);			/*设置gpio的电阻是上拉还是高阻或者下拉 */
	int		(*gpio_aw_data_set)(struct gpio_aw_classdev *gpio_aw_cdev,
					int  data);			/*设置gpio的输出电平*/ 				  
	int		(*gpio_aw_drv_level_set)(struct gpio_aw_classdev *gpio_aw_cdev,
					int  drv_level);	/*设置gpio的驱动等级 */
	int		(*gpio_aw_cfg_get)(struct gpio_aw_classdev *gpio_aw_cdev);	
										/*获取gpio的输入输出 */
	int		(*gpio_aw_pull_get)(struct gpio_aw_classdev *gpio_aw_cdev);	
										/*获取gpio的电阻是上拉还是高阻或者下拉 */
	int		(*gpio_aw_data_get)(struct gpio_aw_classdev *gpio_aw_cdev);			
										/*获取pio的输出电平*/
	int		(*gpio_aw_drv_level_get)(struct gpio_aw_classdev *gpio_aw_cdev);	
										/*获取gpio的驱动等级 */
	struct device		*dev;
	struct list_head	 node;
					
};


extern void gpio_aw_classdev_suspend(struct gpio_aw_classdev *gpio_aw_cdev);
extern void gpio_aw_classdev_resume(struct gpio_aw_classdev *gpio_aw_cdev);

extern int gpio_aw_classdev_register(struct device *parent, 
				struct gpio_aw_classdev *gpio_aw_cdev);
extern void gpio_aw_classdev_unregister(struct gpio_aw_classdev *gpio_aw_cdev);


#endif
