/*
 * mqttTFTChannel.c
 *
 *  Created on: 30 ago. 2023
 *      Author: NRR
 */

#include "weatherDataChannel.h"

#include "esp_log.h"
#include "esp_err.h"

#include "frozen.h"

static const char * TAG = "MQTT_TFT_CHANNEL";

static QueueHandle_t queue = NULL;


/**
 * Call it from app_main to configure before
 * tft and mqtt start
 */
esp_err_t weatherDataChannel_initialize()
{
	if (queue == NULL)
	{
		// create only one time
		queue = xQueueCreate(WEATHERDATA_QUEUE_LENGTH, sizeof(WeatherData));
		if (queue == NULL)
		{
			ESP_LOGE(TAG, "Not enough heap available for queue creation");
			return ESP_FAIL;
		}
	}

	return ESP_OK;
}

/**
 * Returns ESP_OK if data could be allocated.
 * If could not, returns ESP_FAIL indicating
 * it is not enough space in the queue
 *
 * It's non-blocking
 */
esp_err_t weatherDataChannel_send(WeatherData* data)
{
	if ( xQueueSend( queue, (void *) data, 0) != pdTRUE )
	{
		return ESP_FAIL;
	}
	return ESP_OK;
}


/**
 * Returns ESP_OK if data could be received.
 * If could not, returns ESP_FAIL
 *
 * It's non-blocking
 */
esp_err_t weatherDataChannel_receive(WeatherData* data)
{
	if( xQueueReceive( queue, (void *) data, 0) != pdPASS )
	{
		return ESP_FAIL;
	}
	return ESP_OK;
}


/**
 * Function to parse incoming data inside a C string
 * in json format and returns the parsed data in weatherData
 * argument by reference.
 * If data is not well formatted the function returns ESP_FAIL
 * else return ESP_OK
 */
esp_err_t weatherDataChannel_parse(char *data, int data_len, WeatherData* weatherData)
{
	int ret;

	ret = json_scanf(data, data_len,
			"{ city: %Q,"
				"description: %Q,"
				"feels_like: %f,"
				"humidity: %f,"
				"temp: %f,"
				"temp_max: %f,"
				"temp_min: %f,"
				"wind_speed: %f,",
			&(weatherData->city), &(weatherData->description),
			&(weatherData->feels_like), &(weatherData->humidity),
			&(weatherData->temp), &(weatherData->temp_max),
			&(weatherData->temp_min), &(weatherData->wind_speed)
		);

	if (!ret)
	{
		ret = ESP_FAIL;
	}
	else
	{
		ret = ESP_OK;
	}

	return ret;
}
