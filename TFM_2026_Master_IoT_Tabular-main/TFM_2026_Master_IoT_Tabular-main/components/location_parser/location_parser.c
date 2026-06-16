//      DEPENDENCIAS Y DECLARACIÓN DE VARIABLES GLOBALES DE UBICACIÓN
#include "location_parser.h"
#include "cJSON.h"
#include <stdio.h>

//  VARIABLES GLOBALES DINÁMICAS (MEMORIA DE POSICIONAMIENTO)
float g_lat = 0.0;
float g_lon = 0.0;
//      MÓDULO DE DECODIFICACIÓN Y PROCESAMIENTO (JSON PARSER)

void parse_location(char *data, int len)
{
//  1. CONSTRUCCIÓN DE LA ESTRUCTURA ÁRBOL DESDE EL TEXTO CRUDO    
    cJSON *json = cJSON_ParseWithLength(data, len);

    if(!json)
        return;
//  2. EXTRACCIÓN DE LOS PARÁMETROS ESPECÍFICOS DEL PAYLOAD
    cJSON *lat = cJSON_GetObjectItem(json, "lat");
    cJSON *lon = cJSON_GetObjectItem(json, "lon");
//  3. VALIDACIÓN DE CAMPOS Y ASIGNACIÓN A MEMORIA GLOBAL
    if(lat && lon)
    {
        g_lat = lat->valuedouble;
        g_lon = lon->valuedouble;

        printf("Latitud: %f\n", g_lat);
        printf("Longitud: %f\n", g_lon);
    }
// --- 4. LIBERACIÓN ESTRUCTURADA DE LA MEMORIA DINÁMICA 
    cJSON_Delete(json);
}