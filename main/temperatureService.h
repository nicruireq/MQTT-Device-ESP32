/*
 * temperatureService.h
 *
 *  Created on: 26 ago. 2023
 *      Author: Sigma
 */

#ifndef MAIN_TEMPERATURESERVICE_H_
#define MAIN_TEMPERATURESERVICE_H_

#include "esp_err.h"

//Include FreeRTOS headers
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#define SDA_PIN		GPIO_NUM_26
#define SCL_PIN		GPIO_NUM_27

// Minimum interval in milliseconds
#define MIN_INTERVAL_MS		1000

esp_err_t temperatureServiceConfig();
esp_err_t temperatureServiceStartTimer(int milliseconds);
esp_err_t temperatureServiceStopTimer();
float temperatureServiceGetLastReading();

#endif /* MAIN_TEMPERATURESERVICE_H_ */
