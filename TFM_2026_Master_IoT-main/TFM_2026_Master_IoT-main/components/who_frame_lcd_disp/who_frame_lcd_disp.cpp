#include "who_frame_lcd_disp.hpp"

using namespace who::lcd;

namespace who {
namespace lcd_disp {
//      MÉTODO CONSTRUCTOR: INICIALIZACIÓN DE LA TAREA DE PANTALLA Y CANVAS LVGL

WhoFrameLCDDisp::WhoFrameLCDDisp(const std::string &name, frame_cap::WhoFrameCapNode *frame_cap_node, int peek_index) :
    task::WhoTask(name), m_lcd(new lcd::WhoLCD()), m_frame_cap_node(frame_cap_node), m_peek_index(peek_index)
{
// 1. REGISTRO DEL DISPOSITIVO COMO SUSCRIPTOR AL EVENTO DE NUEVO FRAME DE VÍDEO
    frame_cap_node->add_new_frame_signal_subscriber(this);
#if !BSP_CONFIG_NO_GRAPHIC_LIB
// 2. CONFIGURACIÓN EXCLUSIVA DEL CANVAS GRÁFICO (MUTEX DE BLOQUEO DE PANTALLA)
    bsp_display_lock(0);
    m_canvas = lv_canvas_create(lv_scr_act());
    lv_obj_set_size(m_canvas, frame_cap_node->get_fb_width(), frame_cap_node->get_fb_height());
    bsp_display_unlock();
#endif
}
//      MÉTODO DESTRUCTOR: LIBERACIÓN DE MEMORIA DINÁMICA Y RECURSOS GRÁFICOS
WhoFrameLCDDisp::~WhoFrameLCDDisp()
{
#if !BSP_CONFIG_NO_GRAPHIC_LIB
// 1. DESTRUCCIÓN SEGURA DE OBJETOS LVGL DE LA MEMORIA DE RENDERIZADO
    bsp_display_lock(0);
    lv_obj_del(m_canvas);
    bsp_display_unlock();
#endif
// 2. LIBERACIÓN DEL PUNTERO DEL CONTROLADOR FISICO DE LA PANTALLA
    delete m_lcd;
}
//      CONFIGURACIÓN DEL CALLBACK DE RENDERIZADO PERSONALIZADO

void WhoFrameLCDDisp::set_lcd_disp_cb(const std::function<void(who::cam::cam_fb_t *)> &lcd_disp_cb)
{
    m_lcd_disp_cb = lcd_disp_cb;
}

#if !BSP_CONFIG_NO_GRAPHIC_LIB
lv_obj_t *WhoFrameLCDDisp::get_canvas()
{
    return m_canvas;
}
#endif
//      BUCLE PRINCIPAL DE LA TAREA FREERTOS: CONTROLADOR ASÍNCRONO DE VISUALIZACIÓN

void WhoFrameLCDDisp::task()
{
    while (true) {
// 1. ESPERA BLOQUEANTE DE SEÑALES DE CONTROL MEDIANTE GRUPO DE EVENTOS FREERTOS
        EventBits_t event_bits =
            xEventGroupWaitBits(m_event_group, NEW_FRAME | TASK_PAUSE | TASK_STOP, pdTRUE, pdFALSE, portMAX_DELAY);
// 2. CONTROL DE MÁQUINA DE ESTADOS DE LA TAREA (ARRANQUE / PAUSA / PARADA)
        if (event_bits & TASK_STOP) {
            break;
        } else if (event_bits & TASK_PAUSE) {
            xEventGroupSetBits(m_event_group, TASK_PAUSED);
            EventBits_t pause_event_bits =
                xEventGroupWaitBits(m_event_group, TASK_RESUME | TASK_STOP, pdTRUE, pdFALSE, portMAX_DELAY);
            if (pause_event_bits & TASK_STOP) {
                break;
            } else {
                continue;
            }
        }
// 3. EXTRACCIÓN DEL FRAME BUFFER DESDE LA COLA SIN ELIMINAR EL NODO (PEEK)
        auto fb = m_frame_cap_node->cam_fb_peek(m_peek_index);
// 4. ENVÍO DE DATOS A LA PANTALLA SEGÚN LA DISPONIBILIDAD DE LIBRERÍAS GRÁFICAS
#if BSP_CONFIG_NO_GRAPHIC_LIB
// MODO SIN LIBRERÍA GRÁFICA: VOLCADO DIRECTO DE MAPA DE BITS A HARDWARE
        if (m_lcd_disp_cb) {
            m_lcd_disp_cb(fb);
        }
        m_lcd->draw_bitmap(fb->buf, (int)fb->width, (int)fb->height, 0, 0);
#else
// MODO CON LIBRERÍA GRÁFICA: CONFIGURACIÓN DE BUFFER DE COLOR NATIVO EN CANVAS LVGL
        bsp_display_lock(0);
        lv_canvas_set_buffer(m_canvas, fb->buf, fb->width, fb->height, LV_COLOR_FORMAT_NATIVE);
        if (m_lcd_disp_cb) {
            m_lcd_disp_cb(fb);
        }
        bsp_display_unlock();
#endif
    }
// 5. RUTINA DE CIERRE SEGURO Y AUTODESTRUCCIÓN DE LA TAREA EN EL KERNEL
    xEventGroupSetBits(m_event_group, TASK_STOPPED);
    vTaskDelete(NULL);
}
} // namespace lcd_disp
} // namespace who
