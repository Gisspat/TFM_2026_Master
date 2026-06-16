#      ESTRUCTURA DE COMPILACIÓN PRINCIPAL Y ENLACE DE MÓDULOS
idf_component_register(SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES nvs_flash esp_wifi wifi_manager mqtt_manager si7021 sgp30 driver
)