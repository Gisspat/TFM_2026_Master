//      DEPENDENCIAS, CONFIGURACIONES Y CONSTANTES DEL SISTEMA
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"

//  LIBRERÍAS DE LOS COMPONENTES DE LOS SENSORES 
#include "si7021.h"
#include "sgp30.h"

//  LIBRERÍA  NECESARIA PARA ACTIVAR EL WIFI 
#include "wifi_manager.h"

// MQTT
#include "mqtt_manager.h"

//  GPS desde OwnTracks 
extern float g_lat;
extern float g_lon;
//  DEFINICIÓN DE PINES Y PARÁMETROS DE FILTRADO 
#define SDA_PIN 21
#define SCL_PIN 22
#define UMBRAL_RSSI_DISTANTE -75
//      TAREA PRINCIPAL: MONITOREO Y TELEMETRÍA DEL AUTOBÚS
void tarea_monitoreo_bus(void *pvParameters) {
//  1. CONFIGURACIÓN E INICIALIZACIÓN DEL BUS 
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000
    };
    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
//  2. CONFIGURACIÓN INICIAL DE SENSORES AMBIENTALES
    si7021_init();
    vTaskDelay(pdMS_TO_TICKS(500)); 
    sgp30_init();
//  3. TIEMPO DE ESTABILIZACIÓN (PRECALENTAMIENTO DEL SGP30)
    printf(">>> SENSORES INICIADOS. Calentando SGP30 (20 seg)...\n");
    vTaskDelay(pdMS_TO_TICKS(20000)); 
//      BUCLE INFINITO: ADQUISICIÓN, PROCESAMIENTO Y ENVÍO
    while (1) {
//  A. LECTURA DE SENSORES HARDWARE
        float temp = si7021_read_temperature();
        float hum  = si7021_read_humidity();
        uint16_t eco2, tvoc;
        sgp30_read(&eco2, &tvoc);
//  B. ESCANEO ACTIVO DE REDES WI-FI EN EL ENTORNO
        wifi_scan_config_t scan_config = {
            .ssid = NULL,
            .bssid = NULL,
            .channel = 0,
            .show_hidden = false
        };
        esp_wifi_scan_start(&scan_config, true);
//  C. CONTEO Y RESERVA DE MEMORIA PARA PUNTOS DE ACCESO        
        uint16_t ap_count = 0;
        esp_wifi_scan_get_ap_num(&ap_count);

        wifi_ap_record_t *ap_info = NULL;
        if (ap_count > 0) {
            ap_info = malloc(sizeof(wifi_ap_record_t) * ap_count);
            if (ap_info != NULL) {
                esp_wifi_scan_get_ap_records(&ap_count, ap_info);
            }
        }
//  D. FILTRADO POR RSSI
        int redes_en_rango = 0;
        int mejor_ap_index = -1;

        if (ap_info != NULL) {
            for (int i = 0; i < ap_count; i++) {
                if (ap_info[i].rssi >= UMBRAL_RSSI_DISTANTE) {
                    redes_en_rango++;
                    if (mejor_ap_index == -1 || ap_info[i].rssi > ap_info[mejor_ap_index].rssi) {
                        mejor_ap_index = i;
                    }
                }
            }
        }
//  E. PROCESAMIENTO DE UMBRALES Y CÁLCULO DE AFORO ESTIMADO
        char *nivel_temp = (temp < 18) ? "BAJA" : (temp > 27) ? "ALTA" : "NORMAL";
        char *nivel_hum  = (hum < 30) ? "BAJA" : (hum > 70) ? "ALTA" : "NORMAL";
        char *nivel_co2;
        char *estado_aforo;

        if (eco2 <= 700) { 
            nivel_co2 = "BAJO";
            estado_aforo = "AFORO BAJO";
        } else if (eco2 <= 1000) {
            nivel_co2 = "MEDIO";
            estado_aforo = "AFORO MEDIO";
        } else {
            nivel_co2 = "ALTO";
            estado_aforo = "AFORO ALTO";
        }
//  F. IMPRESIÓN DEL REPORTE DE MONITOREO LOCAL
        printf("\n==============================================\n");
        printf("   REPORTE DE MONITOREO - AUTOBÚS SMART\n");
        printf("==============================================\n");
        printf(" ESTADO AFORO : %s\n", estado_aforo);
        
        if (ap_info != NULL && mejor_ap_index != -1) {
            printf(" UBICACIÓN    : %d redes en rango de 20m\n", redes_en_rango);
            printf(" ID PARADA    : %02x:%02x:%02x:%02x:%02x:%02x (%d dBm)\n", 
                   ap_info[mejor_ap_index].bssid[0], ap_info[mejor_ap_index].bssid[1], 
                   ap_info[mejor_ap_index].bssid[2], ap_info[mejor_ap_index].bssid[3], 
                   ap_info[mejor_ap_index].bssid[4], ap_info[mejor_ap_index].bssid[5],
                   ap_info[mejor_ap_index].rssi);
        } else {
            printf(" UBICACIÓN    : EN TRÁNSITO\n");
        }

        printf("----------------------------------------------\n");
        printf(" TEMPERATURA  : %.1f C  -> [%s]\n", temp, nivel_temp);
        printf(" HUMEDAD      : %.1f %%  -> [%s]\n", hum, nivel_hum);
        printf(" CALIDAD AIRE : %d ppm -> [%s]\n", eco2, nivel_co2);
        printf("==============================================\n");
//  G. PREPARACIÓN Y TELEMETRÍA DE DATOS HACIA EL BROKER MQTT
        float lat = g_lat;
        float lon = g_lon;
        int rssi_final = (mejor_ap_index != -1) ? ap_info[mejor_ap_index].rssi : -100;

        mqtt_send_data(
            lat,
            lon,
            temp,
            hum,
            eco2,
            estado_aforo,
            redes_en_rango,
            rssi_final
        );
//  H. LIBERACIÓN DE MEMORIA DINÁMICA Y RETARDO DE CICLO
        if (ap_info != NULL) {
            free(ap_info);
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
//      PUNTO DE ENTRADA PRINCIPAL DEL SISTEMA 
void app_main(void)
{
//  1. INICIALIZACIÓN DEL ALMACENAMIENTO NO VOLÁTIL (NVS)    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
//  2. CREACIÓN DE LA TAREA EN EL PLANIFICADOR FREERTOS
    xTaskCreate(tarea_monitoreo_bus, "tarea_monitoreo_bus", 8192, NULL, 5, NULL);
//  3. INICIALIZACIÓN DE LA PILA DE CONEXIÓN WI-FI
    wifi_init_sta();   
}
