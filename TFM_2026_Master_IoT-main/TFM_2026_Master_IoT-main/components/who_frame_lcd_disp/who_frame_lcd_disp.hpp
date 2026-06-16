#pragma once
#include "who_frame_cap.hpp"
#include "who_lcd.hpp"
#include <vector>

namespace who {
namespace lcd_disp {
//      DEFINICIÓN DE LA CLASE CONTROLADORA DE VISUALIZACIÓN EN PANTALLA LCD
class WhoFrameLCDDisp : public task::WhoTask {
public:
// 1. MÁSCARA DE BITS PARA REACCIÓN ASÍNCRONA DE EVENTOS DE VÍDEO
    static inline constexpr EventBits_t NEW_FRAME = frame_cap::WhoFrameCapNode::NEW_FRAME;
// 2. CONSTRUCTOR, DESTRUCTOR Y MÉTODOS DE CONFIGURACIÓN PÚBLICOS
    WhoFrameLCDDisp(const std::string &name, frame_cap::WhoFrameCapNode *frame_cap_node, int peek_index = 0);
    ~WhoFrameLCDDisp();
    void set_lcd_disp_cb(const std::function<void(who::cam::cam_fb_t *)> &lcd_disp_cb);
#if !BSP_CONFIG_NO_GRAPHIC_LIB
// METODO GETTER DE ACCESO AL CANVAS DE RENDERIZADO LVGL
    lv_obj_t *get_canvas();
#endif

private:
// 3. RUTINA ESTRUCTURADA DE HILO DE EJECUCIÓN (CORE DE FREERTOS)
    void task() override;
// 4. ATRIBUTOS Y PUNTEROS DE GESTIÓN DE HARDWARE Y MEMORIA GRÁFICA
    lcd::WhoLCD *m_lcd;
#if !BSP_CONFIG_NO_GRAPHIC_LIB
    lv_obj_t *m_canvas;
#endif
    frame_cap::WhoFrameCapNode *m_frame_cap_node;
    bool m_peek_index;
    std::function<void(who::cam::cam_fb_t *)> m_lcd_disp_cb;
};
} // namespace lcd_disp
} // namespace who
