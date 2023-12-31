/*
 * mqttActionsSignaler.h
 *
 *  Created on: 12 ago. 2023
 *      Author: NRR
 */

#ifndef MAIN_MQTTACTIONSSIGNALER_H_
#define MAIN_MQTTACTIONSSIGNALER_H_

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define MAX_EVENTS	4

typedef EventBits_t Event_t;

// Eventos para publicacion de mensajes
#define EVENT_PING_REQ				0x000001
#define EVENT_POLL_BUTTONS			0x000002
#define EVENT_ACK_MODE_LEDS_GPIO	0x000004
#define EVENT_ACK_MODE_LEDS_PWM		0X000008
#define EVENT_ADC_SEND_MEASURE		0x000010


esp_err_t initActionsSignaler();
void signalEvent(Event_t event);
EventBits_t waitForRegisteredEvents();


#endif /* MAIN_MQTTACTIONSSIGNALER_H_ */
