/*
 * gpio.h  --  GPIO Driver for X-powers axp199 PMIC
 *
 * Copyright 2011 X-powers Microelectronics PLC
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#ifndef _LINUX_AXP_GPIO_H_
#define _LINUX_AXP_GPIO_H_

/*    GPIO Registers    */
/*    AW1636   */
#define AW1636_GPIO0_CFG                   (AW1636_GPIO0_CTL)
#define AW1636_GPIO1_CFG                   (AW1636_GPIO1_CTL)

#define AW1636_GPIO01_STATE               (AW1636_GPIO01_SIGNAL)


extern int axp_gpio_set_io(int gpio, int io_state);
extern int axp_gpio_get_io(int gpio, int *io_state);
extern int axp_gpio_set_value(int gpio, int value);
extern int axp_gpio_get_value(int gpio, int *value);
#endif
