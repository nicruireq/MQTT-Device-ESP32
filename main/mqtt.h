#ifndef __MQTT_H__
#define __MQTT_H__

//*****************************************************************************
//      DEFINICIONES
//*****************************************************************************

#define MQTT_BROKER_URL      CONFIG_EXAMPLE_MQTT_BROKER_URI
#define MQTT_TOPIC_SUBSCRIBE_BASE      CONFIG_EXAMPLE_MQTT_TOPIC_SUBSCRIBE_BASE
#define MQTT_TOPIC_PUBLISH_BASE  CONFIG_EXAMPLE_MQTT_TOPIC_PUBLISH_BASE


// TOPICS
#define TOPIC_COMMAND	MQTT_TOPIC_SUBSCRIBE_BASE "/COMMAND"
#define TOPIC_LED		MQTT_TOPIC_SUBSCRIBE_BASE "/LED"
#define TOPIC_BUTTONS		MQTT_TOPIC_SUBSCRIBE_BASE "/BUTTONS"
#define TOPIC_ADC		MQTT_TOPIC_SUBSCRIBE_BASE "/ADC"


//*****************************************************************************
//      PROTOTIPOS DE FUNCIONES
//*****************************************************************************

esp_err_t mqtt_app_start(const char* url);


#endif //  __MQTT_H__
