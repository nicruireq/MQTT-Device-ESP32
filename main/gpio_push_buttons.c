/*
 * gpio_push_buttons.c
 *
 *  Created on: 8 ago. 2023
 *      Author: NRR
 */

#include "gpio_push_buttons.h"

void configurePushButtons(void)
{
	// config pins as inputs with actives pull-ups
	gpio_pad_select_gpio(PUSH_BUTTON1);
	gpio_set_direction(PUSH_BUTTON1, GPIO_MODE_INPUT);
	gpio_set_pull_mode(PUSH_BUTTON1, GPIO_PULLUP_ONLY);

	gpio_pad_select_gpio(PUSH_BUTTON2);
	gpio_set_direction(PUSH_BUTTON2, GPIO_MODE_INPUT);
	gpio_set_pull_mode(PUSH_BUTTON2, GPIO_PULLUP_ONLY);
}
