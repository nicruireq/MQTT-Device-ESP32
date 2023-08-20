/*
 * gpio_push_buttons.c
 *
 *  Created on: 8 ago. 2023
 *      Author: NRR
 */

#include "esp_log.h"

#include "gpio_push_buttons.h"
#include "mqttActionsSignaler.h"

static const char * TAG = "GPIO_PUSH_BUTTONS";

static QueueHandle_t queueButtons = NULL;
static TaskHandle_t buttonsTaskHandler = NULL;

//	Variables for the updated filtered pushes and releases of the buttons
static ButtonState btn1DebouncedState = BUTTON_RELEASED,
				   btn2DebouncedState = BUTTON_RELEASED;
// Variable to use in ISR handler for buttons to pass into the queue
static ButtonSnapshot currentBtnsState;


ButtonState getButton1State(void)
{
	return btn1DebouncedState;
}


ButtonState getButton2State(void)
{
	return btn2DebouncedState;
}


void configurePushButtons(void)
{
	// config pins as inputs with actives pull-ups
	gpio_pad_select_gpio(PUSH_BUTTON1);
	gpio_set_direction(PUSH_BUTTON1, GPIO_MODE_INPUT);
	gpio_set_pull_mode(PUSH_BUTTON1, GPIO_PULLUP_ONLY);

	gpio_pad_select_gpio(PUSH_BUTTON2);
	gpio_set_direction(PUSH_BUTTON2, GPIO_MODE_INPUT);
	gpio_set_pull_mode(PUSH_BUTTON2, GPIO_PULLUP_ONLY);
}


static inline EdgeType detectEdgeInISR(int level)
{
	return (level==0)?FALLING:RISING;
}


static void IRAM_ATTR pushButton_isr_handler(void* arg)
{
	uint32_t gpio_num = (uint32_t) arg;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	ButtonSnapshot* btnState = &currentBtnsState;

	//ets_delay_us(5000); //Antirrebote SW "cutre". Por simplicidad lo dejamos as�

	btnState->triggeringEdge = detectEdgeInISR(gpio_get_level(gpio_num));
	btnState->instant = xTaskGetTickCountFromISR();
	btnState->gpio = gpio_num;

	xQueueSendFromISR(queueButtons, (void*) btnState, &xHigherPriorityTaskWoken);

	//gpio_intr_disable(gpio_num);

	if( xHigherPriorityTaskWoken )
	{
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}


static void buttonsDebouncedTask(void * parameters)
{
	TickType_t xLastWakeTime;
	ButtonSnapshot recvBtnState;
	int currentLevel;
	// not pushed by default
	ButtonState buttonStateNow = BUTTON_RELEASED;

	while (1)
	{
		// receive from queue
		if (xQueueReceive(queueButtons, &recvBtnState, portMAX_DELAY) == pdPASS)
		{
			// disabled interrupt
			if (recvBtnState.gpio == PUSH_BUTTON1)
				gpio_intr_disable(PUSH_BUTTON1);
			if (recvBtnState.gpio == PUSH_BUTTON2)
				gpio_intr_disable(PUSH_BUTTON2);
			// sleep during debounce time and read new values
			xLastWakeTime = recvBtnState.instant;
			vTaskDelayUntil(&xLastWakeTime, DEBOUNCE_TIME_MS);
			currentLevel = gpio_get_level(recvBtnState.gpio);

			/**
			 * Make a choice under next conditions:
			 *
			 * 	Before | Level pin now | State
			 * 	-------+---------------+------
			 * 		F			0		  Push
			 * 		F			1		  Release
			 * 		R			0	   	  Push
			 * 		R			1		  Releaase
			 */

			if ( (recvBtnState.triggeringEdge == FALLING)
					&& (currentLevel == 0) )
			{
				buttonStateNow = BUTTON_PUSHED;
			}
			else if ( (recvBtnState.triggeringEdge == FALLING)
					&& (currentLevel == 1) )
			{
				buttonStateNow = BUTTON_RELEASED;
			}
			else if ( (recvBtnState.triggeringEdge == RISING)
					&& (currentLevel == 0) )
			{
				buttonStateNow = BUTTON_PUSHED;
			}
			else if ( (recvBtnState.triggeringEdge == RISING)
					&& (currentLevel == 1) )
			{
				buttonStateNow = BUTTON_RELEASED;
			}

			// record updated state for the button that has awoken the task
			switch (recvBtnState.gpio)
			{
			case PUSH_BUTTON1:
				btn1DebouncedState = buttonStateNow;
				break;
			case PUSH_BUTTON2:
				btn2DebouncedState = buttonStateNow;
				break;
			}
			// signal mqtt sender
			signalEvent(EVENT_PUSHBUTTONS_EDGE);

			if (recvBtnState.gpio == PUSH_BUTTON1)
				gpio_intr_enable(PUSH_BUTTON1);
			if (recvBtnState.gpio == PUSH_BUTTON2)
				gpio_intr_enable(PUSH_BUTTON2);
		}

	}
}


esp_err_t startAsyncMode(void)
{
	gpio_config_t io_conf;
	esp_err_t ret;

	/**
	 * Create IPC to manage task for buttons debounce
	 */
	if (queueButtons == NULL)
	{
		// create only one time
		queueButtons = xQueueCreate(1, sizeof(ButtonSnapshot));
		if (queueButtons == NULL)
		{
			ESP_LOGE(TAG, "Not enough heap available for queue creation");
			return ESP_FAIL;
		}
	}

	// Create task on APP CORE
	if (xTaskCreatePinnedToCore(buttonsDebouncedTask, "debounce_buttons_task",
			configMINIMAL_STACK_SIZE, NULL,
			4, &buttonsTaskHandler, 1 ) != pdPASS)
	{
		ESP_LOGE(TAG, "Buttons task creation fail");
		return ESP_FAIL;
	}

	/**
	 * Configure pin of button 1 and 2 as input with pull-up
	 * and falling edge interrupt
	 */
	io_conf.intr_type = GPIO_INTR_ANYEDGE;
	io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL(PUSH_BUTTON1)
							| GPIO_INPUT_PIN_SEL(PUSH_BUTTON2);
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pull_up_en = 1;
	io_conf.pull_down_en = 0;
	gpio_config(&io_conf);

	// Equal for button 2
	//io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL(PUSH_BUTTON2);
	//gpio_config(&io_conf);
	// Install ISR driver that allows per-pin GPIO interrupt handlers
	if ( (ret = gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT)) != ESP_OK)
	{
		ESP_LOGE(TAG, "gpio_install_isr_service returns: %s", esp_err_to_name(ret));
		return ESP_FAIL;
	}
	// install handlers for each push button interruption
	if ( (ret = gpio_isr_handler_add(PUSH_BUTTON1, pushButton_isr_handler, (void*) PUSH_BUTTON1)) != ESP_OK)
	{
		ESP_LOGE(TAG, "gpio_isr_handler_add for button 1 returns: %s", esp_err_to_name(ret));
		return ESP_FAIL;
	}

	if ( (ret = gpio_isr_handler_add(PUSH_BUTTON2, pushButton_isr_handler, (void*) PUSH_BUTTON2)) != ESP_OK)
	{
		ESP_LOGE(TAG, "gpio_isr_handler_add for button 2 returns: %s", esp_err_to_name(ret));
		return ESP_FAIL;
	}

	return ESP_OK;
}


esp_err_t stopAsyncMode(void)
{
	esp_err_t ret;
	// STEPS:
	// remove isr handlers for each pin
	if ((ret = gpio_isr_handler_remove(PUSH_BUTTON1)) != ESP_OK)
	{
		ESP_LOGE(TAG, "handler remove for button 1 returns: %s", esp_err_to_name(ret));
		return ESP_FAIL;
	}
	if ((ret = gpio_isr_handler_remove(PUSH_BUTTON2)) != ESP_OK)
	{
		ESP_LOGE(TAG, "handler remove for button 2 returns: %s", esp_err_to_name(ret));
		return ESP_FAIL;
	}

	// uninstall isr service (isr per pin)
	gpio_uninstall_isr_service();

	// disable gpio interrupts on pins
	if ((ret = gpio_intr_disable(PUSH_BUTTON1)) != ESP_OK)
	{
		ESP_LOGE(TAG, "trying to disable interrupts on pin %d returns: %s", PUSH_BUTTON1, esp_err_to_name(ret));
		return ESP_FAIL;
	}

	if ((ret = gpio_intr_disable(PUSH_BUTTON2)) != ESP_OK)
	{
		ESP_LOGE(TAG, "trying to disable interrupts on pin %d returns: %s", PUSH_BUTTON1, esp_err_to_name(ret));
		return ESP_FAIL;
	}

	// delete task
	if (buttonsTaskHandler != NULL)
	{
		vTaskDelete(buttonsTaskHandler);
	}

	// change config to default for pins
	// in this module
	configurePushButtons();

	return ESP_OK;
}
