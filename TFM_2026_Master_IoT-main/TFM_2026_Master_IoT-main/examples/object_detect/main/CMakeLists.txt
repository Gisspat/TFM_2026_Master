#      REGISTRO DE FUENTES, DEPENDENCIAS Y MODELOS DE INFERENCIA PARA EL COMPONENTE
set(src_dirs        ./)

set(include_dirs    ./)

set(requires 
    who_detect_app 
    esp_wifi 
    nvs_flash 
    esp_event 
    esp_netif 
    esp_http_client
)

set(detect_component ${DETECT_MODEL} espressif__${DETECT_MODEL})

idf_component_register(SRC_DIRS ${src_dirs} INCLUDE_DIRS ${include_dirs} REQUIRES ${requires})
idf_component_optional_requires(PUBLIC ${detect_component})