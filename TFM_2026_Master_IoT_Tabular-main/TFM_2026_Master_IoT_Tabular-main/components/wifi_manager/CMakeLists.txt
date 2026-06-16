#      REGISTRO DE COMPONENTE Y CONTROL DE DEPENDENCIAS (MÓDULO WI-FI)
idf_component_register(
    SRCS "wifi_manager.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES mqtt_manager
    REQUIRES esp_wifi nvs_flash
)
