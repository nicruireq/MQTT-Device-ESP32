/*
 * adc_reader.c
 *
 *  Created on: 12 ago. 2023
 *      Author: NRR
 *
 *  This module gets an adc calibrated mesaure each one second
 *  by default using a timer.
 *  To get started use the function adc_reader_Start()
 *  when mqtt is started and connected and after subscribe
 *  the topic used for ADC info publication.
 *  The sender task of mqtt can use adc_reader_lastReading()
 *  to get the last measure
 */

#include "adc_reader.h"
#include "mqttActionsSignaler.h"


static TaskHandle_t samplingTask; //Identificador de la tarea de muestreo
static SemaphoreHandle_t xBinarySemaphore ; // Semaforo para avisar a la tarea del instante de muestreo

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_6;     //GPIO34 si se usa el ADC1
static const adc_atten_t atten = ADC_ATTEN_DB_11;       //  Rango de entrada 0-3.1V
static const adc_unit_t unit = ADC_UNIT_1; 			// Descomentar para usar calibraci�n previa

static const char *TAG = "ADC_READER";

// holds last measurement
static uint32_t adc_reading;


/*
 * Timer callback
 *
 */
static bool IRAM_ATTR timer_group_isr_callback(void *args)
{

   // gpio_set_level(GPIO_NUM_14, 1); // Se usar�a para medir en un analizador l�gico el tiempo
                                    // en realizar las operaciones

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR( xBinarySemaphore, &xHigherPriorityTaskWoken );

    return xHigherPriorityTaskWoken == pdTRUE; // return whether we need to yield at the end of ISR
}

/*
 * Initialize selected timer of the timer group 0, with autoreload
 *
 * timer_idx - the timer number to initialize
 * timer_interval_sec - the interval of alarm to set
 */
static void tg0_timer_init(int timer_idx, double timer_interval_sec)
{
    /* Select and initialize basic parameters of the timer */
    timer_config_t config;
    config.divider = TIMER_DIVIDER;
    config.counter_dir = TIMER_COUNT_UP;
    config.counter_en = TIMER_PAUSE;
    config.alarm_en = TIMER_ALARM_EN;
    config.intr_type = TIMER_INTR_LEVEL;
    config.auto_reload = TIMER_AUTORELOAD_EN;
#ifdef TIMER_GROUP_SUPPORTS_XTAL_CLOCK
    config.clk_src = TIMER_SRC_CLK_APB;
#endif
    timer_init(TIMER_GROUP_0, timer_idx, &config);

    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0x00000000ULL);

    /* Configure the alarm value and the interrupt on alarm. */
    timer_set_alarm_value(TIMER_GROUP_0, timer_idx, timer_interval_sec * TIMER_SCALE);

    timer_enable_intr(TIMER_GROUP_0, timer_idx);
    example_timer_info_t *timer_info = calloc(1, sizeof(example_timer_info_t));
    timer_info->timer_group = TIMER_GROUP_0;
    timer_info->timer_idx = timer_idx;
    timer_info->auto_reload = TIMER_AUTORELOAD_EN;
    timer_info->alarm_interval = timer_interval_sec;
    timer_isr_callback_add(TIMER_GROUP_0, timer_idx, timer_group_isr_callback, timer_info, 0);
    timer_start(TIMER_GROUP_0, timer_idx);
}


uint32_t adc_reader_lastReading()
{
	return adc_reading;
}


static void timer_example_evt_task(void *arg)
{
    while (1) {

        // Sleep until the ISR gives us something to do
        xSemaphoreTake( xBinarySemaphore, portMAX_DELAY );

        adc_reading = adc1_get_raw((adc1_channel_t)channel);
        // get the measure in volts with calibration data applied
        adc_reading = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        // signal to sender
        signalEvent(EVENT_ADC_SEND_MEASURE);
    }
}


esp_err_t adc_reader_Start()
{
	BaseType_t xReturned;
	esp_err_t err;

	xBinarySemaphore = xSemaphoreCreateBinary();
	if(NULL==xBinarySemaphore)
	{
		LOGRET("The semaphore could not be created\n");
	}

	xReturned = xTaskCreate(timer_example_evt_task, "timer_evt_task", 2048, NULL, 5, &samplingTask);
	if (xReturned != pdPASS)
	{
		LOGRET("The task for adc could not be allocated\n");
	}

	tg0_timer_init(TIMER_1, TIMER_INTERVAL1_SEC);

	err = adc1_config_width(ADC_WIDTH_BIT_12);
	if (err != ESP_OK )
	{
		LOGRET("ADC Attenuation could not be set\n");
	}

	err = adc1_config_channel_atten(channel, atten);

	adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
	if (!adc_chars)
	{
		LOGRET("Memory could not be allocated for adc_chars\n");
	}
	// get data of the curve for calibration in adc_chars structure
	esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);

	return ESP_OK;
}


