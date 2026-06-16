#      REGISTRO DE COMPONENTE Y CONTROL DE ENLACES (DRIVER SGP30)
idf_component_register(
    SRCS "sgp30.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES driver
)