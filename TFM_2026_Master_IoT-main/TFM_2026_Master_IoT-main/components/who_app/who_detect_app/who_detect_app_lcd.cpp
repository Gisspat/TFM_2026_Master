// LIBRERÍAS NECESARIAS
#include "who_detect_app_lcd.hpp"
#include "who_yield2idle.hpp"
#include "esp_http_client.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "img_converters.h"


namespace who {
namespace app {
//      MÉTODO CONSTRUCTOR: CONFIGURACIÓN DE TAREAS, COLA DE VIDEO Y HILO HTTP

WhoDetectAppLCD::WhoDetectAppLCD(const std::vector<std::vector<uint8_t>> &palette,
                                 frame_cap::WhoFrameCap *frame_cap,
                                 frame_cap::WhoFrameCapNode *lcd_disp_frame_cap_node) :
    WhoDetectAppBase(frame_cap)
{
// 1. CONTROL DE ASIGNACIÓN DE NODOS DE CAPTURA DE VIDEO
    if (!lcd_disp_frame_cap_node) {
        lcd_disp_frame_cap_node = frame_cap->get_last_node();
    }
// 2. INICIALIZACIÓN DEL DISPOSITIVO LCD Y ASIGNACIÓN DEL CALLBACK DE REFRESCO
    m_lcd_disp = new lcd_disp::WhoFrameLCDDisp("LCDDisp", lcd_disp_frame_cap_node);
    WhoApp::add_task(m_lcd_disp);
    m_lcd_disp->set_lcd_disp_cb(std::bind(&WhoDetectAppLCD::lcd_disp_cb, this, std::placeholders::_1));
// 3. CONFIGURACIÓN DEL CANVAS GRÁFICO PARA DELINEADO DE CAJAS EN PANTALLA (BOUNDING BOXES)
#if !BSP_CONFIG_NO_GRAPHIC_LIB
    m_result_lcd_disp = new lcd_disp::WhoDetectResultLCDDisp(m_detect, palette, m_lcd_disp->get_canvas());
#else
    m_result_lcd_disp = new lcd_disp::WhoDetectResultLCDDisp(m_detect, palette);
#endif
// 4. ASIGNACIÓN DE CALLBACKS PARA EL MOTOR DE INFERENCIA LOCAL Y LIMPIEZA DE EVENTOS
    m_detect->set_detect_result_cb(std::bind(&WhoDetectAppLCD::detect_result_cb, this, std::placeholders::_1));
    m_detect->set_cleanup_func(std::bind(&WhoDetectAppLCD::cleanup, this));

    auto detect_frame_cap_node = frame_cap->get_last_node();
// 5. ESCALADO MATRICIAL ASISTIDO POR ACELERACIÓN POR HARDWARE (SI EL SOC SOPORTA PPA)
#if CONFIG_SOC_PPA_SUPPORTED
    if (lcd_disp_frame_cap_node != detect_frame_cap_node) {
        if (detect_frame_cap_node->get_type() == "PPAResizeNode" &&
            detect_frame_cap_node->get_prev_node() == lcd_disp_frame_cap_node) {
            float rescale_x = dl::image::get_ppa_scale(lcd_disp_frame_cap_node->get_fb_width(),
                                                       detect_frame_cap_node->get_fb_width());
            float rescale_y = dl::image::get_ppa_scale(lcd_disp_frame_cap_node->get_fb_height(),
                                                       detect_frame_cap_node->get_fb_height());
            m_detect->set_rescale_params(rescale_x,
                                         rescale_y,
                                         lcd_disp_frame_cap_node->get_fb_width(),
                                         lcd_disp_frame_cap_node->get_fb_height());
        } else {
            ESP_LOGE("WhoDetectAppLCD", "Wrong frame cap node.");
        }
    }
#else
    assert(lcd_disp_frame_cap_node == detect_frame_cap_node);
#endif
// 6. CREACIÓN DE LA COLA DE INTERCAMBIO DE TRÁFICO ENTRE HILOS (MÁXIMO 2 IMÁGENES EN BUFFER)
        // Usamos la variable que definimos en el .hpp
        this->frame_queue = xQueueCreate(2, sizeof(http_frame_t));
// 7. LANZAMIENTO DEL HILO INDEPENDIENTE EXCLUSIVO PARA ENVÍOS HTTP (ANCLADO AL NÚCLEO CPU1)
        // Crear task dedicado para enviar imágenes al servidor
        // Crear task dedicado para enviar imágenes al servidor (fijado en CPU1)
        xTaskCreatePinnedToCore(
            &WhoDetectAppLCD::http_sender_task,
            "http_sender",
            8192,
            this,
            5,
            NULL,
            1   // CPU1
        );


}
//      MÉTODO DESTRUCTOR: LIBERACIÓN DE MEMORIA DEL MÓDULO DE RESULTADOS
WhoDetectAppLCD::~WhoDetectAppLCD()
{
    delete m_result_lcd_disp;
}
//      RUTINA DE INICIO Y EJECUCIÓN ASÍNCRONA DE TODOS LOS NODOS DE PROCESAMIENTO
bool WhoDetectAppLCD::run()
{
    bool ret = WhoYield2Idle::get_instance()->run();
    for (const auto &frame_cap_node : m_frame_cap->get_all_nodes()) {
        ret &= frame_cap_node->run(4096, 2, 0);
    }
    ret &= m_lcd_disp->run(2560, 2, 0);
    ret &= m_detect->run(4096, 2, 1);
    return ret;
}
//      CALLBACK: GUARDADO Y VINCULACIÓN DE LAS CAJAS DE DETECCIÓN CON EL PANEL LCD
void WhoDetectAppLCD::detect_result_cb(const detect::WhoDetect::result_t &result)
{
    // Esta función conecta el resultado del modelo YOLO con la pantalla LCD
    if (m_result_lcd_disp) {
        m_result_lcd_disp->save_detect_result(result);
    }
}
//      CALLBACK DE REFRESCO DE EVENTO LCD: FILTRADO Y CONVERSIÓN DE FORMATO (JPEG)
void WhoDetectAppLCD::lcd_disp_cb(who::cam::cam_fb_t *fb)
{
    m_result_lcd_disp->lcd_disp_cb(fb);

    static int frame_count = 0;
    frame_count++;

    // Enviamos cada 50 frames
    if (frame_count % 50 == 0) {
        uint8_t *jpg_buf = NULL;
        size_t jpg_len = 0;
//  COMPROBACIÓN Y CONVERSIÓN CRUCIAL DE MATRIZ CRUDA RGB565 A COMPRESIÓN JPEG
        // Si la cámara está en RGB565 (formato 0), la convertimos a JPEG
        if ((pixformat_t)fb->format == PIXFORMAT_RGB565) {
    ESP_LOGI("YOLO", "Convirtiendo RGB a JPEG (Calidad 15)...");
    
    // Cambiamos calidad a 15 y nos aseguramos de que jpg_buf empiece en NULL
    jpg_buf = NULL; 
    jpg_len = 0;
// COMPRESIÓN DIRECTA MEDIANTE EL COMPONENTE DE IMÁGENES DE ESP-WHO
    // IMPORTANTE: usamos fmt2jpg que es más directo para RGB565
    // Añadimos (uint8_t *) antes de fb->buf
    bool converted = fmt2jpg((uint8_t *)fb->buf, fb->width * fb->height * 2, fb->width, fb->height, PIXFORMAT_RGB565, 15, &jpg_buf, &jpg_len);
    
    if (converted && jpg_buf && jpg_len > 0) {
// ALTERNATIVA: CLONACIÓN DEL BUFFER COMPRIMIDO HACIA LA COLA PARA CONTROL SEGURO DE MEMORIA
        uint8_t *copy_buf = (uint8_t *)malloc(jpg_len);
        if (copy_buf) {
            memcpy(copy_buf, jpg_buf, jpg_len);
            http_frame_t item = {.buf = copy_buf, .len = jpg_len};
// ENVÍO DE DATOS NO BLOQUEANTE HACIA EL HILO DE RED (TIEMPO DE ESPERA = 0)
            if (xQueueSend(this->frame_queue, &item, 0) != pdPASS) {
                free(copy_buf);
                ESP_LOGW("YOLO", "Cola llena, frame descartado");
            }
        }
        // Liberar el buffer que creó fmt2jpg
        free(jpg_buf); 
    } else {
        ESP_LOGE("YOLO", "Fallo crítico en conversión. ¿Falta RAM?");
    }
}
    }
}
//      RUTINA DE LIMPIEZA DE RECURSOS DE CONTEXTO GRÁFICO
void WhoDetectAppLCD::cleanup()
{
    m_result_lcd_disp->cleanup();
}
//      CLIENTE HTTP: CONSTRUCCIÓN DEL PAQUETE POST Y ENVÍO DE LA IMAGEN AL BACKEND
void WhoDetectAppLCD::send_frame_to_server(uint8_t *jpg_buf, size_t jpg_len)
{
    if (jpg_buf == NULL || jpg_len == 0) return;
// 1. DECLARACIÓN DEL CONFIGURADOR DEL CLIENTE DE RED
    esp_http_client_config_t config = {};
    // Cambia esto en send_frame_to_server
    config.url = "http://192.168.1.48:8000/detect"; 
    config.method = HTTP_METHOD_POST;
    config.timeout_ms = 5000; // Aumentamos a 5s para imágenes grandes
    config.buffer_size = 1024 * 4; // Buffer interno del cliente

    esp_http_client_handle_t client = esp_http_client_init(&config);

    // 1. Establecer el encabezado de tamaño (CRUCIAL para FastAPI)
    char len_str[10];
    sprintf(len_str, "%d", jpg_len);
    esp_http_client_set_header(client, "Content-Type", "image/jpeg");
    esp_http_client_set_header(client, "Content-Length", len_str);

    // 2. Pasar los datos
    esp_http_client_set_post_field(client, (const char *)jpg_buf, jpg_len);

    // 3. Ejecutar
    esp_err_t err = esp_http_client_perform(client);
//  EVALUACIÓN DE RESPUESTA DE RETORNO DEL SERVIDOR
    if (err == ESP_OK) {
        int status = esp_http_client_get_status_code(client);
        char response[64];
        int len = esp_http_client_read(client, response, sizeof(response)-1);
        if (len > 0) {
            response[len] = 0;
            ESP_LOGI("YOLO", "Status: %d, Msg: %s", status, response);
        }
    } else {
        ESP_LOGE("YOLO", "Error en POST: %s", esp_err_to_name(err));
    }
//  DESTRUCCIÓN DEL CLIENTE Y LIBERACIÓN DE SOCKETS TCP
    esp_http_client_cleanup(client);
}
//      TAREA INDEPENDIENTE FREERTOS: MANEJO ASÍNCRONO Y MONITOREO DE CONEXIÓN IP
void WhoDetectAppLCD::http_sender_task(void *arg)
{
    WhoDetectAppLCD *self = (WhoDetectAppLCD*)arg;
    http_frame_t item;

    ESP_LOGI("YOLO", "Tarea HTTP: Esperando IP válida...");
// 1. BLOQUEO PREVIO: EL HILO NO SIFONA LA COLA HASTA QUE EL ESP32 OBTENGA CONEXIÓN WI-FI ESTABLE    
    // Esperar hasta que el sistema tenga una IP asignada (más fiable)
    esp_netif_ip_info_t ip_info;
    esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    
    while (true) {
        if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK && ip_info.ip.addr != 0) {
            ESP_LOGI("YOLO", "¡IP Detectada! Iniciando envíos al servidor.");
            break; 
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
// 2. BUCLE INFINITO DE ESCUCHA DE LA COLA DE TRANSMISIÓN DE VIDEO
    while (true) {
// EL HILO SE QUEDA COMPLETAMENTE DORMIDO HASTA QUE LLEGUE UN ELEMENTO (AHORRO DE RECURSOS)
        // Esperamos el frame de la cola
        if (xQueueReceive(self->frame_queue, &item, portMAX_DELAY)) {
            // Log informativo para el monitor
            ESP_LOGI("YOLO", "Procesando frame de la cola (%d bytes)...", item.len);
// DISPARO DEL ENVÍO AL ENDPOINT DE PYTHON

            self->send_frame_to_server(item.buf, item.len);
            
            // Liberamos la memoria de la copia
            free(item.buf); 
            ESP_LOGI("YOLO", "Frame procesado y memoria liberada.");
        }
    }
}


} // namespace app
} // namespace who
