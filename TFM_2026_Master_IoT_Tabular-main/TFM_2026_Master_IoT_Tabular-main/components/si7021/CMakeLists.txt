#      REGISTRO DE COMPONENTE Y CONTROL DE ENLACES (DRIVER SI7021)
idf_component_register(
    SRCS "si7021.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES driver
)

