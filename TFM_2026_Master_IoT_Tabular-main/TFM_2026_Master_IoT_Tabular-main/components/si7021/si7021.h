//      INTERFAZ PÚBLICA Y API DEL DRIVER DEL SENSOR SI7021
#pragma once

void si7021_init();
float si7021_read_temperature();
float si7021_read_humidity();
