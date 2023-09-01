#ifndef __MQTT_H__
#define __MQTT_H__

//*****************************************************************************
//      DEFINICIONES
//*****************************************************************************

#define MQTT_BROKER_URL      CONFIG_EXAMPLE_MQTT_BROKER_URI
#define MQTT_TOPIC_SUBSCRIBE_BASE      CONFIG_EXAMPLE_MQTT_TOPIC_SUBSCRIBE_BASE
#define MQTT_TOPIC_PUBLISH_BASE  CONFIG_EXAMPLE_MQTT_TOPIC_PUBLISH_BASE

#define RETAIN_ON	1

// TOPICS
#define TOPIC_COMMAND	MQTT_TOPIC_SUBSCRIBE_BASE "/COMMAND"
#define TOPIC_LED		MQTT_TOPIC_SUBSCRIBE_BASE "/LED"
#define TOPIC_BUTTONS		MQTT_TOPIC_SUBSCRIBE_BASE "/BUTTONS"
#define TOPIC_ADC		MQTT_TOPIC_SUBSCRIBE_BASE "/ADC"
#define TOPIC_LWT		MQTT_TOPIC_SUBSCRIBE_BASE "/BOARD/STATUS"
#define TOPIC_TEMP			MQTT_TOPIC_SUBSCRIBE_BASE "/TEMP"
#define TOPIC_WEATHER	MQTT_TOPIC_SUBSCRIBE_BASE "/WEATHER"
#define TOPIC_BLEINFO	MQTT_TOPIC_SUBSCRIBE_BASE "/BLEINFO"

#define LWT_MESSAGE		"{ \"status\": \"died\" }"
#define MSG_ALIVE		"{ \"status\": \"alive\" }"

#define SIZE_STR_PRINT_BLE	(sizeof("{name: , address: , RSSI: },"))

//*****************************************************************************
//      PROTOTIPOS DE FUNCIONES
//*****************************************************************************

esp_err_t mqtt_app_start(const char* url);


#endif //  __MQTT_H__
