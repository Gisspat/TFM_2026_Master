#pragma once
#include "who_frame_cap.hpp"


//      DECLARACIÓN DE DECLARACIONES Y PROTOTIPOS DE PIPELINES DE CAPTURA DE VÍDEO
#if CONFIG_IDF_TARGET_ESP32S3
// INTERFACES DE COLA DE IMÁGENES POR BUS DVP (PARALELO) PARA HARDWARE ESP32-S3
who::frame_cap::WhoFrameCap *get_lcd_dvp_frame_cap_pipeline();
who::frame_cap::WhoFrameCap *get_term_dvp_frame_cap_pipeline();

#elif CONFIG_IDF_TARGET_ESP32P4
// INTERFACES DE COLA DE IMÁGENES AVANZADAS (MIPI-CSI / UVC) PARA HARDWARE ESP32-P4
who::frame_cap::WhoFrameCap *get_lcd_mipi_csi_frame_cap_pipeline();
who::frame_cap::WhoFrameCap *get_lcd_mipi_csi_ppa_frame_cap_pipeline(
    who::frame_cap::WhoFrameCapNode **lcd_disp_frame_cap_node);
who::frame_cap::WhoFrameCap *get_lcd_uvc_frame_cap_pipeline();
who::frame_cap::WhoFrameCap *get_term_mipi_csi_frame_cap_pipeline();
who::frame_cap::WhoFrameCap *get_term_mipi_csi_ppa_frame_cap_pipeline();
who::frame_cap::WhoFrameCap *get_term_uvc_frame_cap_pipeline();
#endif