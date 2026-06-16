#      REGISTRO DE COMPONENTE Y VINCULACIÓN DEL PARSER JSON
idf_component_register(
    SRCS "location_parser.c"
    INCLUDE_DIRS "include"
    REQUIRES cjson
)
