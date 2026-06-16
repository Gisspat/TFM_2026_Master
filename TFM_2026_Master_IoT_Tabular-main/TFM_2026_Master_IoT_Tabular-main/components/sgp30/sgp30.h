//      INTERFAZ PÚBLICA Y API DEL DRIVER DEL SENSOR SGP30
#pragma once
#include <stdint.h>

void sgp30_init();
void sgp30_read(uint16_t *eco2, uint16_t *tvoc);
