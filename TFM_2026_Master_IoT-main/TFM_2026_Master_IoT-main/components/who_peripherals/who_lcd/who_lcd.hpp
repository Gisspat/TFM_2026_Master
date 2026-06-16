#pragma once
#include "esp_lcd_types.h"
#include "bsp/esp-bsp.h"
#if !BSP_CONFIG_NO_GRAPHIC_LIB
#include "who_lvgl_lcd.hpp"
#else

namespace who {
namespace lcd {
//      CLASE CONTROLADORA PARA VOLCADO DIRECTO A BAJO NIVEL (SIN PILA LVGL)
class WhoLCD {
public:
//1. CONSTRUCTOR Y DESTRUCTOR AUTOMÁTICO DE INICIALIZACIÓN FÍSICA DEL PANEL
    WhoLCD() { init(); }
    ~WhoLCD() { deinit(); }
// 2. INTERFACES DE CONTROL DE ESTADO DEL PERIFÉRICO DE PANTALLA
    void init();
    void deinit();
// 3. RUTINAS DE ACCESO AL MANIPULADOR Y VOLCADO CRUDO DE MATRICES DE COLOR (BITMAPS)
    esp_lcd_panel_handle_t get_lcd_panel_handle();
    void draw_bitmap(const void *data, int width, int height, int x_start, int y_start);

private:
// 4. DESACOPLAMIENTO DE PUNTEROS GRÁFICOS INTERNOS SEGÚN LA ARQUITECTURA DEL SOC
#if CONFIG_IDF_TARGET_ESP32S3
// ESTRUCTURAS ESPECÍFICAS PARA BUS PARALELO E INYECCIÓN POR ACCESO DIRECTO EN S3
    esp_lcd_panel_handle_t m_panel_handle;
    esp_lcd_panel_io_handle_t m_io_handle;
    void *m_lcd_buffer;
#elif CONFIG_IDF_TARGET_ESP32P4
// CONTENEDOR INTEGRADO DE MANIPULADORES PARA LA ARQUITECTURA P4 (MIPI-DSI)
    bsp_lcd_handles_t m_lcd_handles;
#endif
};
} // namespace lcd
} // namespace who
#endif
