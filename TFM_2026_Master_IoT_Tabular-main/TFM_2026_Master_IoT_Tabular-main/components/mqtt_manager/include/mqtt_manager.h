//      DECLARACIÓN DE LA INTERFAZ Y FUNCIONES PÚBLICAS DEL MÓDULO MQTT
#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

void mqtt_app_start(void);

void mqtt_send_data(float lat, float lon, float temp, float hum,
                    int air, const char *aforo, int redes, int rssi);

#endif
