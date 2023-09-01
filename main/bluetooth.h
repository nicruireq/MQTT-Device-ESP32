//*****************************************************************************

#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__

//Include FreeRTOS headers
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

//*****************************************************************************
//      DEFINICIONES
//*****************************************************************************

//#define WIFI_MODO_STATION           0
//#define WIFI_MODO_SOFTAP            1

#define BLE_MAX_DISCOVERED_DEVS		  10
#define BLE_DEVICE_ADDR_MAX_SIZE		 30
#define BLE_DEVICE_NAME_MAX_SIZE		 30
#define BLE_RSSI_STR_MAX_SIZE				 5

typedef enum BleDevReq
{
	BLE_SCAN_FROM_TERMINAL,
	BLE_SCAN_FROM_MQTT
} BleDevReq;

typedef struct BleScanResult
{
	int rssi;
	char address[BLE_DEVICE_ADDR_MAX_SIZE];
	char deviceName[BLE_DEVICE_NAME_MAX_SIZE];

} BleScanResult;


//*****************************************************************************
//      PROTOTIPOS DE FUNCIONES
//*****************************************************************************

esp_err_t bluetooth_start_scan(BleDevReq reqType);

esp_err_t bluetooth_stop_scan(void);

//esp_err_t bluetooth_start_adv(const char* nombre);
//esp_err_t bluetooth_stop_adv(void);

esp_err_t bluetooth_init(void);

QueueHandle_t bluetooth_getScannedDevicesQueue();

#endif //  __BLUETOOTH_H__
