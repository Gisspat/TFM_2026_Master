#      REGISTRO DE COMPONENTE Y CONFIGURACIÓN DE COMPILACIÓN Cmake
idf_component_register(
    SRCS "mqtt_manager.c"
    INCLUDE_DIRS "include"
    REQUIRES mqtt location_parser
    EMBED_TXTFILES "certs/hivemq_ca.pem"
)
