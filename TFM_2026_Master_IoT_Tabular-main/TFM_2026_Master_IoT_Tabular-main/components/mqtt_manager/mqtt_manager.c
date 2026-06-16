//      DEPENDENCIAS Y CONFIGURACIÓN LOCAL DEL MÓDULO MQTT
#include "mqtt_manager.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include "location_parser.h"

static const char *TAG = "MQTT";

// Cliente global 
static esp_mqtt_client_handle_t client = NULL;

// Declaración del certificado CA embebido
extern const uint8_t hivemq_ca_pem_start[] asm("_binary_hivemq_ca_pem_start");
extern const uint8_t hivemq_ca_pem_end[]   asm("_binary_hivemq_ca_pem_end");


// FUNCIÓN PARA ENVIAR DATOS
void mqtt_send_data(float lat, float lon, float temp, float hum,
                    int air, const char *aforo, int redes, int rssi)
{
    if (client == NULL) {
        ESP_LOGW(TAG, "Cliente MQTT no inicializado, no se envían datos");
        return;
    }

    char data[256];

    sprintf(data,
        "{\"lat\":%.6f,\"lon\":%.6f,\"temp\":%.2f,\"hum\":%.2f,"
        "\"air\":%d,\"aforo\":\"%s\",\"redes\":%d,\"rssi\":%d}",
        lat, lon, temp, hum, air, aforo, redes, rssi);

    esp_mqtt_client_publish(client, "bus/data", data, 0, 1, 0);

    ESP_LOGI(TAG, "Datos enviados: %s", data);
}


// MANEJADOR DE EVENTOS MQTT
static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch (event->event_id)
    {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT conectado");

            esp_mqtt_client_subscribe(
                event->client,
                "owntracks/esp32_user/telefono",
                0);
            break;

        case MQTT_EVENT_DATA:
            parse_location(event->data, event->data_len);
            break;

        default:
            break;
    }
}

// INICIO DEL CLIENTE MQTT
void mqtt_app_start(void)
{
    ESP_LOGI(TAG, "Iniciando cliente MQTT...");

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtts://c02751a84b9f4525b8321654975122c8.s1.eu.hivemq.cloud:8883",

        .broker.verification.certificate = (const char *)hivemq_ca_pem_start,

        .credentials.username = "esp32_user",
        .credentials.authentication.password = "Esp32_pass",
    };

    client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(
        client,
        ESP_EVENT_ANY_ID,
        mqtt_event_handler,
        NULL);

    esp_mqtt_client_start(client);
}
