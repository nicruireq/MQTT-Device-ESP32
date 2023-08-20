/*
 * mqttActionsSignaler.c
 *
 *  Created on: 12 ago. 2023
 *      Author: NRR
 */


#include "mqttActionsSignaler.h"

#include "esp_err.h"
#include "esp_log.h"

// To handle events that raise activation of sender task
static EventGroupHandle_t senderTriggerEvents;
//Declare a variable to hold the data associated with the created event group
static StaticEventGroup_t senderTriggerEventsStatic;

static const char *TAG = "ACTIONS_SIGNALER";

esp_err_t initActionsSignaler()
{
	senderTriggerEvents = xEventGroupCreateStatic(&senderTriggerEventsStatic);
	if (!senderTriggerEvents)
	{
		ESP_LOGE(TAG, "Event group for sender task could not be allocated");
		return ESP_FAIL;
	}
	return ESP_OK;
}


void signalEvent(Event_t event)
{
	// notify task blocked in the event group
	xEventGroupSetBits(senderTriggerEvents, event);
}

BaseType_t signalEventFromISR(Event_t event, BaseType_t* xHigherPriorityTaskWoken)
{
	*xHigherPriorityTaskWoken = pdFALSE;
	return xEventGroupSetBitsFromISR(
			senderTriggerEvents,
            event,
            xHigherPriorityTaskWoken );
}

static Event_t getGlobalFlagForRegisteredEvents()
{
	Event_t gflag = EVENT_PING_REQ |
					EVENT_POLL_BUTTONS |
					EVENT_ACK_MODE_LEDS_GPIO |
					EVENT_ACK_MODE_LEDS_PWM |
					EVENT_ADC_SEND_MEASURE |
					EVENT_ACK_ASYNC_BUTTONS |
					EVENT_PUSHBUTTONS_EDGE |
					EVENT_ACK_MODE_PUSH_BUTTONS_POLL;

	return gflag;
}

EventBits_t waitForRegisteredEvents()
{
	return xEventGroupWaitBits(senderTriggerEvents,
				getGlobalFlagForRegisteredEvents(),
				pdTRUE,pdFALSE, configTICK_RATE_HZ);
}
