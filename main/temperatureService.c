/*
 * temperatureService.c
 *
 *  Created on: 26 ago. 2023
 *      Author: NRR
 */

#include "temperatureService.h"
#include "ds1621driver.h"
#include "mqttActionsSignaler.h"
#include "i2c_master.h"

#include "esp_log.h"

static const char * TAG = "TEMPERATURE_SERVICE";
//
//// to hold handle to the created timer
//TimerHandle_t xTimer;
//
//// static space to store state for soft timer
//StaticTimer_t xTimerBuffer;

static TaskHandle_t tempTaskHandler = NULL;

// holds the last temperature reading
static float lastTemp = 0.0;


/**
 * Configure DS1621 sensor to start conversions
 *
 * NOTE: Blocking function, don't use from
 * 		 mqtt event handler, instead call it
 * 		 at main app
 */
esp_err_t temperatureServiceConfig()
{
	esp_err_t ret = ESP_OK;

	// i2c controller init
	ret = i2c_master_init(I2C_NUM_0, SDA_PIN, SCL_PIN, GPIO_PULLUP_ENABLE, false);
	ret = ds1621_i2c_master_init(0, I2C_NUM_0);
	ESP_ERROR_CHECK(ret);
	// DS1621 sensor configuration command
	// with continous conversion mode
	while (ds1621_config(DS1621_CTRL_CONTINUOUS) != ESP_OK)
	{
		ESP_LOGE(TAG, "ERROR ds1621Config");
		vTaskDelay(500 / portTICK_RATE_MS);
	}
	// sends command to begin temperature conversion
	while (ds1621_start_conversion())
	{
		ESP_LOGE(TAG, "ERROR ds1621Config");
		vTaskDelay(500 / portTICK_RATE_MS);
	}
	// don't continue until command sequence is completed

	return ret;
}

/**
 * Returns last temperature read
 */
float temperatureServiceGetLastReading()
{
	return lastTemp;
}


/**
 * Task for temperature reading each interval
 */
static void vTaskTemp( void * pvParameters )
{
	int milliseconds;
	TickType_t xLastWakeTime;

	while (1)
	{
		milliseconds = (int) pvParameters;
		// simulate timer functionality
		xLastWakeTime=xTaskGetTickCount();
		vTaskDelayUntil(&xLastWakeTime,  pdMS_TO_TICKS(milliseconds));

		esp_err_t ret = ds1621_read_temperature_high_resolution(&lastTemp);
		if (ret != ESP_OK)
		{
			ESP_LOGE(TAG, "Temperature reading error: %s", esp_err_to_name(ret));
		}
		else
		{
			// notify mqtt sender and return
			signalEvent(EVENT_TEMPERATURE);
		}
	}
}


esp_err_t temperatureServiceStart(int milliseconds)
{
	esp_err_t ret = ESP_OK;

	if (milliseconds < MIN_INTERVAL_MS)
	{
		ESP_LOGE(TAG, "milliseconds parameter is too low");
		return ESP_FAIL;
	}

	if (tempTaskHandler == NULL)
	{
		// create task and set interval
		if (xTaskCreate(vTaskTemp, "TempSrvTask", 2048, (void*)milliseconds, 4, &tempTaskHandler) != pdPASS)
		{
			ESP_LOGE(TAG, "Task for temperature service could not be allocated");
			ret = ESP_FAIL;
		}
		// Is not thread-safe
		lastTemp = milliseconds;
	}
	else
	{
		// if task is already created, change interval
		vTaskSuspend(tempTaskHandler);
		lastTemp = milliseconds;
		vTaskResume(tempTaskHandler);
	}

	return ret;
}


void temperatureServiceStop()
{
	if (tempTaskHandler != NULL)
	{
		vTaskDelete(tempTaskHandler);
		tempTaskHandler = NULL;
	}
}
