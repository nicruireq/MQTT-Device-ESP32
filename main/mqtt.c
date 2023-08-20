//Include standard lib headers
#include <stdbool.h>
#include <stdint.h>
#include <string.h>


//Include FreeRTOS headers
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

//Include ESP submodules headers.
#include "esp_event.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "driver/gpio.h"

//Include own project  headers
#include "gpio_leds.h"
#include "mqtt.h"
#include "gpio_push_buttons.h"
#include "mqttActionsSignaler.h"
#include "adc_reader.h"

//FROZEN JSON parsing/fotmatting library header
#include "frozen.h"

//****************************************************************************
//      VARIABLES GLOBALES STATIC
//****************************************************************************

static const char *TAG = "MQTT_CLIENT";
static esp_mqtt_client_handle_t client=NULL;
static TaskHandle_t senderTaskHandler=NULL;


//****************************************************************************
// Funciones.
//****************************************************************************

static void mqtt_sender_task(void *pvParameters);


// callback that will handle MQTT events. Will be called by  the MQTT internal task.
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
        {
        	// subscribe to topics
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, MQTT_TOPIC_SUBSCRIBE_BASE, 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
            msg_id = esp_mqtt_client_subscribe(client, TOPIC_COMMAND, 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
			msg_id = esp_mqtt_client_subscribe(client, TOPIC_LED, 0);
			ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
			msg_id = esp_mqtt_client_subscribe(client, TOPIC_BUTTONS, 0);
			ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
			msg_id = esp_mqtt_client_subscribe(client, TOPIC_ADC, 0);
			ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

			// publish message with retain in LWT topic (board is alive)
			msg_id = esp_mqtt_client_publish(client, TOPIC_LWT, MSG_ALIVE, 0, 0, RETAIN_ON);
			ESP_LOGE(TAG, "Alive message not published on LWT topic, msg_id=%d", msg_id);

            //xTaskCreate(mqtt_sender_task, "mqtt_sender", 4096, NULL, 5, &senderTaskHandler); //Crea la tarea MQTT sender
			xTaskCreatePinnedToCore(mqtt_sender_task, "mqtt_sender", 4096, NULL, 5, &senderTaskHandler, 1);

            // Start ADC reader task and all its dependencies
            if (adc_reader_Start() != ESP_OK)
            {
            	ESP_LOGE(TAG, "ADC reader task not working");
            }
        }
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            //Deber�amos destruir la tarea que env�a....
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            //ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
        {
        	//Para poder imprimir el nombre del topic lo tengo que copiar en una cadena correctamente terminada....
        	char topic_name[event->topic_len+1]; //Esto va a la pila y es potencialmente peligroso si el nombre del topic es grande....
        	strncpy(topic_name,event->topic,event->topic_len);
        	topic_name[event->topic_len]=0; //a�ade caracter de terminacion al final.

        	ESP_LOGI(TAG, "MQTT_EVENT_DATA: Topic %s",topic_name);

        	// Handle data subscribed on topic COMMAND
        	if (strncmp(TOPIC_COMMAND, topic_name, strlen(TOPIC_COMMAND)) == 0)
        	{
				// Read incoming json data
				char *strCmd = NULL;
				if (json_scanf(event->data, event->data_len, "{ cmd: %Q}",
						&strCmd))
				{
					// manage received ping command
					if (strncmp(strCmd, "ping", strlen(strCmd)) == 0)
					{
						ESP_LOGI(TAG, "cmd: %s", strCmd);
						// notify sender task to send ping response
						signalEvent(EVENT_PING_REQ);
					}
					else if (strncmp(strCmd, "poll_buttons", strlen(strCmd)) == 0)
					{
						// manage received poll buttons command
						ESP_LOGI(TAG, "cmd: %s", strCmd);
						// notify sender task to send ping response
						signalEvent(EVENT_POLL_BUTTONS);
					}
					else if (strncmp(strCmd, "mode_leds_gpio", strlen(strCmd)) == 0)
					{
						// manage received change led mode to gpio command
						ESP_LOGI(TAG, "cmd: %s", strCmd);
						if (GL_isPWMEnabled())
						{
							// notify sender task to send ack response
							signalEvent(EVENT_ACK_MODE_LEDS_GPIO);
						}
					}
					else if (strncmp(strCmd, "mode_leds_pwm", strlen(strCmd)) == 0)
					{
						// manage received change led mode to pwm command
						ESP_LOGI(TAG, "cmd: %s", strCmd);
						if (!GL_isPWMEnabled())
						{
							// notify sender task to send ack response
							signalEvent(EVENT_ACK_MODE_LEDS_PWM);
						}
					}
					else if (strncmp(strCmd, "async_buttons", strlen(strCmd)) == 0)
					{
						// manage received change push buttons read mode to async
						ESP_LOGI(TAG, "cmd: %s", strCmd);
						signalEvent(EVENT_ACK_ASYNC_BUTTONS);

					}
					else if (strncmp(strCmd, "no_async_buttons", strlen(strCmd)) == 0)
					{
						// manage received change push buttons read mode to poll
						ESP_LOGI(TAG, "cmd: %s", strCmd);
						signalEvent(EVENT_ACK_MODE_PUSH_BUTTONS_POLL);
					}
					free(strCmd);	// fragmentacion?
				}
        	}

        	// Handle data subscribed on topic LED
        	if (strncmp(TOPIC_LED, topic_name, strlen(TOPIC_LED)) == 0)
        	{
        		if (GL_isPWMEnabled())
        		{
        			// process like pwm mode
        			uint8_t ledsIncomingValue[NUM_LEDS] = {0 , 0, 0};
        			if(json_scanf(event->data, event->data_len, "{ redLed: %hhu }",
        					&(ledsIncomingValue[0]) )==1)
					{
						ESP_LOGI(TAG, "redLed: %i", (int)ledsIncomingValue );
					}

					if (json_scanf(event->data, event->data_len, "{ greenLed: %hhu }",
							&(ledsIncomingValue[1]) ) == 1)
					{
						ESP_LOGI(TAG, "greenLed: %i", (int)ledsIncomingValue );
					}

					if (json_scanf(event->data, event->data_len, "{ blueLed: %hhu }",
							&(ledsIncomingValue[2]) ) == 1)
					{
						ESP_LOGI(TAG, "blueLed: %i", (int)ledsIncomingValue );
					}
					GL_setRGB(ledsIncomingValue);
        		}
        		else
        		{
					// process like gpio mode
					bool ledIncomingState;
					if(json_scanf(event->data, event->data_len, "{ redLed: %B }", &ledIncomingState)==1)
					{
						ESP_LOGI(TAG, "redLed: %s", ledIncomingState ? "true":"false");

						gpio_set_level(BLINK_GPIO_1, ledIncomingState);
					}

					if(json_scanf(event->data, event->data_len, "{ greenLed: %B }", &ledIncomingState)==1)
					{
						ESP_LOGI(TAG, "greenLed: %s", ledIncomingState ? "true":"false");

						gpio_set_level(BLINK_GPIO_2, ledIncomingState);
					}

					if(json_scanf(event->data, event->data_len, "{ blueLed: %B }", &ledIncomingState)==1)
					{
						ESP_LOGI(TAG, "blueLed: %s", ledIncomingState ? "true":"false");

						gpio_set_level(BLINK_GPIO_3, ledIncomingState);
					}
        		}
        	}

		}
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        case MQTT_EVENT_BEFORE_CONNECT:
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
}


static void mqtt_sender_task(void *pvParameters)
{
	char buffer[100]; //"buffer" para guardar el mensaje. Me debo asegurar que quepa...
	bool booleano=0;
	ButtonState button1State, button2State;
	int button1PollState, button2PollState;
	Event_t activationEvents = 0;

	while (1)
	{
		// wait for the activation of an event that requires to publish a message
		activationEvents = waitForRegisteredEvents();

		struct json_out out1 = JSON_OUT_BUF(buffer, sizeof(buffer)); // Inicializa la estructura que gestiona el buffer.

		switch (activationEvents)
		{
			case EVENT_PING_REQ:
			{
				json_printf(&out1," { cmd: ping_response }");
				int msg_id = esp_mqtt_client_publish(client, TOPIC_COMMAND, buffer, 0, 0, 0);
				ESP_LOGI(TAG, "sent successful on TOPIC_COMMAND, msg_id=%d: %s", msg_id, buffer);
			}
				break;
			case EVENT_POLL_BUTTONS:
			{
				button1PollState = gpio_get_level(PUSH_BUTTON1);
				button2PollState = gpio_get_level(PUSH_BUTTON2);

				json_printf(&out1, " { button1: %Q, button2: %Q }",
						(button1PollState==0?"on":"off"),
						(button2PollState==0?"on":"off")
				);

				int msg_id = esp_mqtt_client_publish(client, TOPIC_BUTTONS, buffer, 0, 0, 0);
				ESP_LOGI(TAG, "sent successful on TOPIC_BUTTONS, msg_id=%d: %s", msg_id, buffer);
			}
				break;
			case EVENT_ACK_MODE_LEDS_GPIO:
			{
				// change mode to gpio with gpio leds hal
				GL_stopLEDC();
				GL_initGPIO();
				// send ack
				json_printf(&out1," { cmd: ack_mode_leds_gpio }");
				int msg_id = esp_mqtt_client_publish(client, TOPIC_COMMAND, buffer, 0, 0, 0);
				ESP_LOGI(TAG, "sent successful on TOPIC_COMMAND, msg_id=%d: %s", msg_id, buffer);
			}
				break;
			case EVENT_ACK_MODE_LEDS_PWM:
			{
				// change mode to pwm with gpio leds hal
				GL_initLEDC();
				// send ack
				json_printf(&out1," { cmd: ack_mode_leds_pwm }");
				int msg_id = esp_mqtt_client_publish(client, TOPIC_COMMAND, buffer, 0, 0, 0);
				ESP_LOGI(TAG, "sent successful on TOPIC_COMMAND, msg_id=%d: %s", msg_id, buffer);
			}
				break;
			case EVENT_ADC_SEND_MEASURE:
			{
				uint32_t last = adc_reader_lastReading();
				json_printf(&out1," { last_reading: %d }", last);
				int msg_id = esp_mqtt_client_publish(client, TOPIC_ADC, buffer, 0, 0, 0);
				//ESP_LOGI(TAG, "sent successful on TOPIC_ADC, msg_id=%d: %s", msg_id, buffer);
			}
				break;
			case EVENT_ACK_ASYNC_BUTTONS:
			{
				// Start async mode
				startAsyncMode();

				json_printf(&out1," { cmd: ack_async_buttons }");
				int msg_id = esp_mqtt_client_publish(client, TOPIC_COMMAND, buffer, 0, 0, 0);
				ESP_LOGI(TAG, "sent successful on TOPIC_COMMAND, msg_id=%d: %s", msg_id, buffer);
			}
				break;
			case EVENT_PUSHBUTTONS_EDGE:
			{
				button1State = getButton1State();
				button2State = getButton2State();

				json_printf(&out1, " { button1: %Q, button2: %Q }",
						((button1State == BUTTON_PUSHED)?"on":"off"),
						((button2State == BUTTON_PUSHED)?"on":"off")
				);

				int msg_id = esp_mqtt_client_publish(client, TOPIC_BUTTONS, buffer, 0, 0, 0);
				ESP_LOGI(TAG, "sent successful on TOPIC_BUTTONS, msg_id=%d: %s", msg_id, buffer);
			}
				break;
			case EVENT_ACK_MODE_PUSH_BUTTONS_POLL:
			{
				// change to poll mode for buttons
				stopAsyncMode();

				json_printf(&out1," { cmd: ack_no_async_buttons }");
				int msg_id = esp_mqtt_client_publish(client, TOPIC_COMMAND, buffer, 0, 0, 0);
				ESP_LOGI(TAG, "sent successful on TOPIC_COMMAND, msg_id=%d: %s", msg_id, buffer);
			}
				break;
			default:
				break;
		}

	}

//	while (1)
//	{
//		vTaskDelay(configTICK_RATE_HZ);
//		struct json_out out1 = JSON_OUT_BUF(buffer, sizeof(buffer)); // Inicializa la estructura que gestiona el buffer.
//																	// Hay que hacerlo cada vez para empezar a rellenar desde el principio
//																	// si quiero acumular varios "printf" en la misma cadena, no reinicio out1....
//		json_printf(&out1," { button: %B }",booleano);
//		booleano=!booleano;
//
//		int msg_id = esp_mqtt_client_publish(client, MQTT_TOPIC_PUBLISH_BASE, buffer, 0, 0, 0); //al utilizar la biblioteca Frozen, buffer es una cadena correctamente terminada con el caracter 0.
//																								//as� que puedo no indicar la longitud (cuarto par�metro vale 0).
//																								// Si fuese un puntero a datos binarios arbitr�rios, tendr�a que indicar la longitud de los datos en el cuarto par�metro de la funci�n.
//		ESP_LOGI(TAG, "sent successful, msg_id=%d: %s", msg_id, buffer);
//	}
}

esp_err_t mqtt_app_start(const char* url)
{
	esp_err_t error;

	// initialize group of events
	initActionsSignaler();

	if (client==NULL){

		esp_mqtt_client_config_t mqtt_cfg = {
				.uri = MQTT_BROKER_URL,
				.lwt_topic = TOPIC_LWT,
				.lwt_msg = LWT_MESSAGE,
				.lwt_qos = 1,
				.lwt_retain = RETAIN_ON
		};
		if(url[0] != '\0'){
			mqtt_cfg.uri= url;
		}

		client = esp_mqtt_client_init(&mqtt_cfg);
		esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
		error=esp_mqtt_client_start(client);
		return error;
	}
	else {

		ESP_LOGE(TAG,"MQTT client already running");
		return ESP_FAIL;
	}
}


