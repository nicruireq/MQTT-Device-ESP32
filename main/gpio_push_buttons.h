/*
 * gpio_push_buttons.h
 *
 *  Created on: 8 ago. 2023
 *      Author: NRR
 */

#ifndef MAIN_GPIO_PUSH_BUTTONS_H_
#define MAIN_GPIO_PUSH_BUTTONS_H_

#include "esp_err.h"
#include "driver/gpio.h"

//Include FreeRTOS headers
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// Pin IO26
#define PUSH_BUTTON1	26
// Pin IO27
#define PUSH_BUTTON2	27
// bit mask for pin
#define GPIO_INPUT_PIN_SEL(IOP)  ((1ULL<<(IOP)))

#define ESP_INTR_FLAG_DEFAULT 0 // Flags for interrupt definition (NONE in that case)

#define DEBOUNCE_TIME_MS	(0.1*configTICK_RATE_HZ)	// milliseconds


typedef enum ButtonState {
	BUTTON_PUSHED,
	BUTTON_RELEASED
} ButtonState;

typedef enum EdgeType
{
	FALLING, RISING
} EdgeType;

/**
 * Type to store the level of a button
 * in an instant of time
 */
typedef struct ButtonSnapshot
{
	TickType_t instant;	// ticks
	EdgeType triggeringEdge;
	int gpio;
} ButtonSnapshot;


void configurePushButtons(void);
/**
 * WARNING:
 *
 * startAsyncMode need to be called
 * in a task that was started in the
 * same core that this function starts
 * the button task.
 *
 * The same stands for stopAsyncMode
 */
esp_err_t startAsyncMode(void);
esp_err_t stopAsyncMode(void);
ButtonState getButton1State(void);
ButtonState getButton2State(void);



#endif /* MAIN_GPIO_PUSH_BUTTONS_H_ */
