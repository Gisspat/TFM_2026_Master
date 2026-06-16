//      INTERFAZ PÚBLICA Y EXPORTACIÓN DE VARIABLES GLOBALES GPS
#ifndef LOCATION_PARSER_H
#define LOCATION_PARSER_H

void parse_location(char *data, int len);

//  EXPORTAR VARIABLES
extern float g_lat;
extern float g_lon;

#endif