#pragma once
#include "sdkconfig.h"
//      INCLUSIÓN CONDICIONAL DE DRIVERS GRÁFICOS SEGÚN ARQUITECTURA DE TARGET DE SOC
#if CONFIG_IDF_TARGET_ESP32S3
#include "who_s3_cam.hpp"
#elif CONFIG_IDF_TARGET_ESP32P4
#include "who_p4_cam.hpp"
#endif
#include "who_uvc_cam.hpp"
