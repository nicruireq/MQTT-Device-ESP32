/*
 * gpio_leds.h
 *
 *  Created on: 9 dic. 2020
 *      Author: jcgar
 */

#ifndef GPIO_LEDS_H_
#define GPIO_LEDS_H_

#include<stdint.h>

#define BLINK_GPIO_1 0
#define BLINK_GPIO_2 2
#define BLINK_GPIO_3 4

#define NUM_LEDS	3

extern void GL_initGPIO(void);
extern void GL_initLEDC();
extern void GL_setRGB(uint8_t *RGBLevels);
extern void GL_stopLEDC();
// Check if mode PWM is enabled
extern bool GL_isPWMEnabled();

#endif /* GPIO_LEDS_H_ */
