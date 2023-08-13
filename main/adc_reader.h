/*
 * adc_reader.h
 *
 *  Created on: 12 ago. 2023
 *      Author: NRR
 */

#ifndef MAIN_ADC_READER_H_
#define MAIN_ADC_READER_H_

#include "esp_types.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"


#define TIMER_DIVIDER         16  //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds
#define TIMER_INTERVAL1_SEC   (1)   // sample test interval for the timer
#define DEFAULT_VREF 		  1093

// Macro for log error and return
#define LOGRET(msg)	\
		ESP_LOGE(TAG, msg); \
		return ESP_FAIL

typedef struct {
    int timer_group;
    int timer_idx;
    int alarm_interval;
    bool auto_reload;
} example_timer_info_t;

/**
 * @brief A sample structure to pass events from the timer ISR to task
 *
 */
typedef struct {
    example_timer_info_t info;
    uint64_t timer_counter_value;
} example_timer_event_t;


esp_err_t adc_reader_Start();
uint32_t adc_reader_lastReading();

#endif /* MAIN_ADC_READER_H_ */
