//      DEPENDENCIAS, DIRECCIONAMIENTO I2C Y VARIABLES LOCALES
#include "sgp30.h"
#include "driver/i2c.h"
#include "esp_log.h"
//  PARÁMETROS DE COMUNICACIÓN HARDWARE DEL SENSOR
#define SGP30_ADDR 0x58
static const char *TAG = "SGP30";
//      MÉTODOS DE CONFIGURACIÓN Y ADQUISICIÓN DE DATOS (I2C)
void sgp30_init() {
    uint8_t cmd[2] = {0x20, 0x03};
//  1. ENVÍO DEL COMANDO DE INICIALIZACIÓN AL DISPOSITIVO    
    i2c_master_write_to_device(I2C_NUM_0, SGP30_ADDR, cmd, 2, 1000 / portTICK_PERIOD_MS);
//  2. TIEMPO DE ASENTAMIENTO DEL SENSOR    
    vTaskDelay(10 / portTICK_PERIOD_MS);
}

void sgp30_read(uint16_t *eco2, uint16_t *tvoc) {
    uint8_t cmd[2] = {0x20, 0x08};
    uint8_t data[6];
//  1. DISPARO DE LA SOLICITUD DE MEDICIÓN HÁPTICA
    i2c_master_write_to_device(I2C_NUM_0, SGP30_ADDR, cmd, 2, 1000 / portTICK_PERIOD_MS);
//  2. ESPERA ESTABLECIDA POR DATASHEET PARA EL PROCESAMIENTO QUÍMICO    
    vTaskDelay(20 / portTICK_PERIOD_MS);
//  3. EXTRACCIÓN Y TRATAMIENTO DE LA TRAMA DE DATOS
    if (i2c_master_read_from_device(I2C_NUM_0, SGP30_ADDR, data, 6, 1000 / portTICK_PERIOD_MS) == ESP_OK) {
        *eco2 = (data[0] << 8) | data[1];
        *tvoc = (data[3] << 8) | data[4];
    } else {
// 4. CONTROL DE EXCEPCIONES EN CASO DE ERROR         
        ESP_LOGE(TAG, "Error leyendo SGP30");
        *tvoc = 0;
        *eco2 = 0;
    }
}
