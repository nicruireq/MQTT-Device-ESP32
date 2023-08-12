/*
 * gpio_push_buttons.h
 *
 *  Created on: 8 ago. 2023
 *      Author: NRR
 */

#ifndef MAIN_GPIO_PUSH_BUTTONS_H_
#define MAIN_GPIO_PUSH_BUTTONS_H_

#include "driver/gpio.h"

// Pin IO26
#define PUSH_BUTTON1	26
// Pin IO27
#define PUSH_BUTTON2	27


void configurePushButtons(void);

#endif /* MAIN_GPIO_PUSH_BUTTONS_H_ */
