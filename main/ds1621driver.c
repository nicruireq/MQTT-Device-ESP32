// EJERCICIO EF11
// MSEEI-UMA

#include "ds1621driver.h"

//****************************************************************************
//      VARIABLES
//****************************************************************************

static unsigned char ds1621_slave_address = 0;
static i2c_port_t ds1621_i2c_port;

//****************************************************************************
//     FUNCIONES
//****************************************************************************

esp_err_t ds1621_i2c_master_init(uint8_t address, i2c_port_t i2c_master_port)
{


    ds1621_slave_address = DS1621_BASE_ADDRESS | (address & 0x7);
    ds1621_i2c_port = i2c_master_port;

    return 0;

}

esp_err_t ds1621_config(uint8_t config)
{
    unsigned char data_buffer[2];
    if(ds1621_slave_address == 0)
        return ESP_FAIL;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ds1621_slave_address << 1) | I2C_MASTER_WRITE, true);
    data_buffer[0] = DS1621_CMD_ACCESS_CONFIG;
    data_buffer[1] = config;
    i2c_master_write(cmd, data_buffer, 2, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(ds1621_i2c_port, cmd, DS1621_WAIT_TIME_MS / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    vTaskDelay(DS1621_BUS_FREE_TIME_MS / portTICK_RATE_MS);
    return ret;
};

esp_err_t ds1621_read_temperature_low_resolution(float* temperature)
{
	uint8_t TH_reg = 0;
    if(ds1621_slave_address == 0)
        return ESP_FAIL;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ds1621_slave_address << 1) | I2C_MASTER_WRITE, true);
    //i2c_master_write_byte(cmd, DS1621_CMD_ACCESS_TH, true);	// wrong register
    i2c_master_write_byte(cmd, DS1621_CMD_READ_TEMP, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ds1621_slave_address << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, &TH_reg, 1, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(ds1621_i2c_port, cmd, DS1621_WAIT_TIME_MS / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    vTaskDelay(DS1621_BUS_FREE_TIME_MS / portTICK_RATE_MS);
    *temperature = (float)TH_reg;
    return ret;
};

esp_err_t ds1621_read_counter(uint8_t* counter)
{
    if(ds1621_slave_address == 0)
        return ESP_FAIL;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ds1621_slave_address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, DS1621_CMD_READ_COUNTER, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ds1621_slave_address << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, counter, 1, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(ds1621_i2c_port, cmd, DS1621_WAIT_TIME_MS / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    vTaskDelay(DS1621_BUS_FREE_TIME_MS / portTICK_RATE_MS);
    return ret;
};

esp_err_t ds1621_read_slope(uint8_t* slope)
{
    if(ds1621_slave_address == 0)
        return ESP_FAIL;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ds1621_slave_address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, DS1621_CMD_READ_SLOPE, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ds1621_slave_address << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, slope, 1, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(ds1621_i2c_port, cmd, DS1621_WAIT_TIME_MS / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    vTaskDelay(DS1621_BUS_FREE_TIME_MS / portTICK_RATE_MS);
    return ret;
};

esp_err_t ds1621_start_conversion(void)
{
    if(ds1621_slave_address == 0)
        return ESP_FAIL;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ds1621_slave_address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, DS1621_CMD_START_CONVERT, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(ds1621_i2c_port, cmd, DS1621_WAIT_TIME_MS / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    vTaskDelay(DS1621_BUS_FREE_TIME_MS / portTICK_RATE_MS);
    return ret;
};

esp_err_t ds1621_stop_conversion(void)
{
    if(ds1621_slave_address == 0)
        return ESP_FAIL;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ds1621_slave_address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, DS1621_CMD_STOP_CONVERT, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(ds1621_i2c_port, cmd, DS1621_WAIT_TIME_MS / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    vTaskDelay(DS1621_BUS_FREE_TIME_MS / portTICK_RATE_MS);
    return ret;
};

esp_err_t ds1621_read_temperature_high_resolution(float* temperature)
{
	uint8_t slope, counter;
	float low_resolution_temp;
	esp_err_t ret;
	// wait for start conversion time
	vTaskDelay(DS1621_CONVERSION_TIME_MS / portTICK_RATE_MS);

	while ((ret = ds1621_read_slope(&slope)) != ESP_OK) {
	    vTaskDelay(500 / portTICK_RATE_MS);
	}

	while ((ret = ds1621_read_counter(&counter)) != ESP_OK) {
		vTaskDelay(500 / portTICK_RATE_MS);
	}

	while ((ret = ds1621_read_temperature_low_resolution(&low_resolution_temp)) != ESP_OK) {
		vTaskDelay(500 / portTICK_RATE_MS);
	}

	*temperature = low_resolution_temp-0.25+((float)(slope-counter)/(float)slope);

	return ret;
};

esp_err_t ds1621_write_TH(float temperature)
{
       uint8_t data_buffer[2];
       int16_t value;
// Conversión de la temperatura a valores a escribir con el comando ACCESS_TH
       value= (int16_t)(temperature*2.0);
       data_buffer[0]=(value>>1)&0xFF;  // 8 bits más significativos
       data_buffer[1]= (value & 0x01)<<7;  // noveno bit

// AQUÍ  COMPLETAR EL CÓDIGO DE LA FUNCIÓN
       if(ds1621_slave_address == 0)
           return ESP_FAIL;
       i2c_cmd_handle_t cmd = i2c_cmd_link_create();
       i2c_master_start(cmd);
       i2c_master_write_byte(cmd, (ds1621_slave_address << 1) | I2C_MASTER_WRITE, true);
       i2c_master_write_byte(cmd, DS1621_CMD_ACCESS_TH, true);
       i2c_master_write(cmd, data_buffer, 2, true);
       i2c_master_stop(cmd);

      esp_err_t ret = i2c_master_cmd_begin(ds1621_i2c_port, cmd, DS1621_WAIT_TIME_MS / portTICK_RATE_MS);
      i2c_cmd_link_delete(cmd);
      vTaskDelay(DS1621_BUS_FREE_TIME_MS / portTICK_RATE_MS);
      return ret;
};
