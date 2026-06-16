#pragma once
#include "who_cam_define.hpp"

namespace who {
namespace cam {
//      CLASE BASE ABSTRACTA (INTERFAZ) PARA EL CONTROL DE DRIVERS DE CÁMARA
class WhoCam {
public:
// 1. MÉTODOS CONSTRUCTORES (SOBRECARGADOS PARA ASIGNACIÓN DE FRAME BUFFER)
    WhoCam(uint8_t fb_count) : WhoCam(fb_count, 0, 0) {}
    WhoCam(uint8_t fb_count, uint16_t fb_width, uint16_t fb_height) :
        m_fb_count(fb_count), m_cam_fbs(new cam_fb_t[fb_count]), m_fb_width(fb_width), m_fb_height(fb_height)
    {
    }
// 2. DESTRUCTOR VIRTUAL: LIBERACIÓN DEL ARREGO DINÁMICO DE FRAME BUFFERS
    virtual ~WhoCam() { delete[] m_cam_fbs; }
// 3. MÉTODOS VIRTUALES PUROS: INTERFAZ OBLIGATORIA PARA DRIVERS DERIVADOS
    virtual cam_fb_t *cam_fb_get() = 0;
    virtual void cam_fb_return(cam_fb_t *fb) = 0;
// 4. MÉTODOS GETTERS: CONSULTA DE PARÁMETROS GEOMÉTRICOS Y DE MEMORIA
    uint16_t get_fb_width() { return m_fb_width; }
    uint16_t get_fb_height() { return m_fb_height; }
    uint8_t get_fb_count() { return m_fb_count; }
    virtual cam_fb_fmt_t get_fb_format() = 0;
// 5. ATRIBUTOS PROTEGIDOS ACCESIBLES POR LAS IMPLEMENTACIONES DE HARDWARE

protected:
    uint8_t m_fb_count;
    cam_fb_t *m_cam_fbs;
    uint16_t m_fb_width;
    uint16_t m_fb_height;
};

} // namespace cam
} // namespace who
