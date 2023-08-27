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

// to hold handle to the created timer
TimerHandle_t xTimer;

// static space to store state for soft timer
StaticTimer_t xTimerBuffer;

// holds the last temperature reading
static float lastTemp = 0.0;


/**
 * This callback is called everytime the timer expires
 */
static void vTimerCallback( TimerHandle_t xTimer )
{
	esp_err_t ret = ds1621_read_temperature_high_resolution(&lastTemp);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "Temperature reading error: %s", esp_err_to_name(ret));
	}
	else
	{
		ESP_LOGI(TAG, "Temp CALLBACK - lastTemp = %f", lastTemp);
		// notify mqtt sender and return
		signalEvent(EVENT_TEMPERATURE);
	}
}


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

	xTimer = xTimerCreateStatic("TemperatureServiceTimer",
					pdMS_TO_TICKS(MIN_INTERVAL_MS), pdTRUE, (void *) 0,
					vTimerCallback, &xTimerBuffer
				);
	if (xTimer == NULL)
	{
		ESP_LOGE(TAG, "The timer was not created");
		ret = ESP_FAIL;
	}

	return ret;
}

/**
 * Returns last temperature read
 */
float temperatureServiceGetLastReading()
{
	return lastTemp;
}


esp_err_t temperatureServiceStartTimer(int milliseconds)
{
	if (milliseconds < MIN_INTERVAL_MS)
	{
		ESP_LOGE(TAG, "milliseconds parameter is too low");
		return ESP_FAIL;
	}

	// Start the timer.  No block time is specified
	if (xTimerChangePeriod(xTimer, pdMS_TO_TICKS(milliseconds), 0) != pdPASS)
	{
		// The timer could not be set into the Active state
		return ESP_FAIL;
	}

	return ESP_OK;
}


esp_err_t temperatureServiceStopTimer()
{
	return xTimerStop(xTimer, 0);
}
