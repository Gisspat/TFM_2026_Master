//      DEPENDENCIAS Y PARÁMETROS DE CONFIGURACIÓN DE CONEXIÓN
#include "wifi_manager.h"
#include "mqtt_manager.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
//  CREDENCIALES DE LA RED DE COMUNICACIÓN (PUNTO DE ACCESO)
#define WIFI_SSID "Livebox6-7ED0-24G"
#define WIFI_PASS "iA4HNgbRj7Zb"

static const char *TAG = "WIFI";
//      MANEJADOR DE EVENTOS (MAQUINA DE ESTADOS WIFI E IP)
static void wifi_event_handler(void* arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void* event_data)
{
//  A. INICIO DEL MÓDULO WI-FI EN MODO ESTACIÓN (STA)    
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(TAG, "Conectando al WiFi...");
        esp_wifi_connect();
    }
//  B. CONTROL DE RECONEXIÓN AUTOMÁTICA EN CASO DE PÉRDIDA    
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGW(TAG, "WiFi desconectado, reintentando...");
        esp_wifi_connect();
    }
//  C. ASIGNACIÓN EXITOSA DE DIRECCIÓN IP (ACTIVACIÓN GATEWAY MQTT)    
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;

        ESP_LOGI(TAG, "IP obtenida: " IPSTR, IP2STR(&event->ip_info.ip));

//  Estructura de  secuencia: Inicio seguro del cliente de telemetría
        mqtt_app_start();
    }
}
//      INICIALIZACIÓN DE COMPONENTES DE RED Y ARRANQUE DEL HARDWARE
void wifi_init_sta(void)
{
//  1. PREPARACIÓN DEL ALMACENAMIENTO E INTERFACES DE RED    
    nvs_flash_init();

    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
//  2. ASIGNACIÓN DE PARÁMETROS POR DEFECTO DEL DRIVER WI-FI
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

//  3. REGISTRO DE FILTROS EN EL LOOP DE EVENTOS DEL SISTEMA
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
//  4. CONFIGURACIÓN E INYECCIÓN DE CREDENCIALES DE SEGURIDAD
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    
//  5. DISPARO FÍSICO DEL CONTROLADOR INALÁMBRICO
    esp_wifi_start();
}
