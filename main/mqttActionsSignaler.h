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
#define EVENT_PING_REQ						0x000001
#define EVENT_POLL_BUTTONS					0x000002
#define EVENT_ACK_MODE_LEDS_GPIO			0x000004
#define EVENT_ACK_MODE_LEDS_PWM				0X000008
#define EVENT_ADC_SEND_MEASURE				0x000010
#define EVENT_ACK_ASYNC_BUTTONS				0x000020
#define EVENT_PUSHBUTTONS_EDGE				0x000040
#define EVENT_ACK_MODE_PUSH_BUTTONS_POLL	0X000080
#define EVENT_ACK_START_TEMP				0x000100
#define EVENT_ACK_STOP_TEMP					0x000200
#define EVENT_TEMPERATURE					0x000400
#define EVENT_BLESCAN_FINISHED				0X000800


esp_err_t initActionsSignaler();
void signalEvent(Event_t event);
BaseType_t signalEventFromISR(Event_t event, BaseType_t* xHigherPriorityTaskWoken);
EventBits_t waitForRegisteredEvents();


#endif /* MAIN_MQTTACTIONSSIGNALER_H_ */
