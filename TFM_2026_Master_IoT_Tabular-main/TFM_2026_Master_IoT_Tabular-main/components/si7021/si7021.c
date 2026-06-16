//      DEPENDENCIAS, DIRECCIONAMIENTO I2C Y VARIABLES LOCALES
#include "si7021.h"
#include "driver/i2c.h"
#include "esp_log.h"
//  PARÁMETROS DE COMUNICACIÓN HARDWARE DEL SENSOR
#define SI7021_ADDR 0x40
static const char *TAG = "SI7021";
//      MÉTODOS DE CONFIGURACIÓN Y ADQUISICIÓN DE BAJO NIVEL
void si7021_init() {
    
}

static float si7021_read(uint8_t cmd, float scale, float offset) {
    uint8_t data[2];
//  1. ENVÍO DEL COMANDO DE MEDICIÓN HACIA EL PERIFÉRICO
    i2c_master_write_to_device(I2C_NUM_0, SI7021_ADDR, &cmd, 1, 1000 / portTICK_PERIOD_MS);
//  2. TIEMPO DE ESPERA ASÍNCRONO PARA LA CONVERSIÓN INTERNA DEL SENSOR    
    vTaskDelay(20 / portTICK_PERIOD_MS);
//  3. LECTURA DE LOS BYTES DE TELEMETRÍA DESDE EL PERIFÉRICO
    if (i2c_master_read_from_device(I2C_NUM_0, SI7021_ADDR, data, 2, 1000 / portTICK_PERIOD_MS) != ESP_OK) {
        ESP_LOGE(TAG, "Error leyendo");
        return -999;
    }
//  4. RECONSTRUCCIÓN BINARIA DE LOS DATOS (COMBINACIÓN DE BYTES MSB Y LSB)
    uint16_t raw = (data[0] << 8) | data[1];
//  5. APLICACIÓN DE LA FUNCIÓN DE TRANSFERENCIA DEL FABRICANTE   
    return (scale * raw) / 65536.0 - offset;
}
//      INTERFAZ API PÚBLICA DEL DRIVER
float si7021_read_temperature() {
    return si7021_read(0xF3, 175.72, 46.85);
}

float si7021_read_humidity() {
    return si7021_read(0xF5, 125.0, 6.0);
}
