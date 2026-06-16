#pragma once
#include "esp_lcd_types.h"
#include "bsp/esp-bsp.h"
#if !BSP_CONFIG_NO_GRAPHIC_LIB
#include "lvgl.h"

namespace who {
namespace lcd {
//      CLASE CONTROLADORA PARA LA INICIALIZACIÓN DE LA INTERFAZ GRÁFICA LVGL
class WhoLCD {
public:
// 1. CONSTRUCTOR DE LA CLASE CON PARÁMETROS PRE CONFIGURADOS DE ASIGNACIÓN DE MEMORIA RAM
    WhoLCD(const lvgl_port_cfg_t &lvgl_port_cfg = {4, 6144, 0, 500, MALLOC_CAP_INTERNAL, 5}) { init(lvgl_port_cfg); }
// 2. DESTRUCTOR DE LA CLASE ENCARGADO DE LA LIBERACIÓN SEGURA DEL CONTROLADOR
    ~WhoLCD() { deinit(); }
// 3. PROTOTIPOS DE LAS RUTINAS FÍSICAS DE ARRANQUE Y PARADA DEL PERIFÉRICO
    void init(const lvgl_port_cfg_t &lvgl_port_cfg);
    void deinit();

private:
// 4. PUNTERO INTERNO AL MANIPULADOR DE ESTRUCTURA DE LA PANTALLA EN LVGL
    lv_display_t *m_disp;
};
} // namespace lcd
} // namespace who
#endif
