// LIBRERÍAS NECESARIAS
#pragma once
#include "who_detect_app_base.hpp"
#include "who_detect_result_handle.hpp"
#include "who_frame_lcd_disp.hpp"
#include "freertos/FreeRTOS.h" 
#include "freertos/queue.h"
#include "freertos/task.h"

#include "esp_http_client.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "esp_camera.h"
//      ESTRUCTURA DE BUFFER INTERMEDIO PARA TRÁFICO COMPRIMIDO (JPEG)

typedef struct {
    uint8_t *buf;  // PUNTERO AL MAPA DE MEMORIA HEAP DONDE SE GUARDA LA IMAGEN
    size_t len;    // LONGITUD EXACTA EN BYTES DEL ARCHIVO COMPRIMIDO
} http_frame_t;

namespace who {
namespace app {
//      CLASE APLICACIÓN: INTEGRACIÓN DE IA LOCAL, PANTALLA LCD Y STREAMING HTTP
class WhoDetectAppLCD : public WhoDetectAppBase {
public:
// 1. MÉTODOS CONSTRUCTOR Y DESTRUCTOR DE LA APLICACIÓN GRÁFICA
    WhoDetectAppLCD(const std::vector<std::vector<uint8_t>> &palette,
                    frame_cap::WhoFrameCap *frame_cap,
                    frame_cap::WhoFrameCapNode *lcd_disp_frame_cap_node = nullptr);
    ~WhoDetectAppLCD();
// 2. FUNCIÓN DE ARRANQUE DE HILOS Y PLANIFICACIÓN EN EL KERNEL
    bool run() override;
protected:
// 3. MÉTODOS CALLBACK DE RESPUESTA ASÍNCRONA A EVENTOS DE CÁMARA E INFERENCIA
    virtual void detect_result_cb(const detect::WhoDetect::result_t &result);
    virtual void lcd_disp_cb(who::cam::cam_fb_t *fb);
    virtual void cleanup();

private:
// 4. CONTROLADORES INTERNOS DE VISUALIZACIÓN Y RENDERIZADO DE CAJAS
    lcd_disp::WhoFrameLCDDisp *m_lcd_disp;
    lcd_disp::WhoDetectResultLCDDisp *m_result_lcd_disp;
// 5. MANIPULADORES DE CONTEXTO DE FREERTOS PARA COLA Y TAREA EN CPU1
    QueueHandle_t frame_queue;       // La cola para las fotos
    TaskHandle_t http_task_handle;   // El control de la tarea HTTP

private:
// 6. FUNCIONES PRIVADAS COMPLEMENTARIAS PARA EL PROCESAMIENTO Y ENVÍO HTTP
    void send_frame_to_server(uint8_t *jpg_buf, size_t jpg_len);
    static void http_sender_task(void *arg);


};
} // namespace app
} // namespace who
