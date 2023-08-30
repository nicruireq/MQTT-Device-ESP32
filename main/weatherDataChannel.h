/*
 * mqttTFTChannel.h
 *
 *  Created on: 30 ago. 2023
 *      Author: NRR
 */

/**
 * Encapsulates a freertos queue
 * to communicate mqtt task with
 * TFT task, passing data between them
 */

#ifndef MAIN_WEATHERDATACHANNEL_H_
#define MAIN_WEATHERDATACHANNEL_H_

//Include FreeRTOS headers
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

//#define WEATHERDATA_MAX_DESCRIPTION_LENGTH		20
//#define WEATHERDATA_MAX_DESCRIPTION_LENGTH_STR	"20"
//#define WEATHERDATA_MAX_CITY_LENGTH				20
//#define WEATHERDATA_MAX_CITY_LENGTH_STR			"20"
//#define WEATHERDATA_MAX_FLOAT_LENGTH_STR		"5"

#define WEATHERDATA_QUEUE_LENGTH				10

typedef struct WeatherData
{
	char* description;
	char* city;
	float temp, feels_like, temp_min,
		  temp_max, wind_speed, humidity;
} WeatherData;

esp_err_t weatherDataChannel_initialize();
esp_err_t weatherDataChannel_send(WeatherData* data);
esp_err_t weatherDataChannel_receive(WeatherData* data);
esp_err_t weatherDataChannel_parse(char *data, int data_len, WeatherData* weatherData);

#endif /* MAIN_WEATHERDATACHANNEL_H_ */
