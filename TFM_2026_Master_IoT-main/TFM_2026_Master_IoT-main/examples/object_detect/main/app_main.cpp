//      DEPENDENCIAS Y DECLARACIÓN DE LIBRERÍAS DEL SISTEMA Y CÁMARA

#include "frame_cap_pipeline.hpp"
#include "who_detect_app_lcd.hpp"
#include "who_detect_app_term.hpp"
#include "bsp/esp-bsp.h"
#include "esp_camera.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_log.h"
#include <string.h>
//      CONMUTACIÓN ESTÁTICA Y SELECCIÓN DEL MODELO DE DEEP LEARNING (CNN)

#if defined(CONFIG_HUMAN_FACE_DETECT_MODEL_LOCATION)
#include "human_face_detect.hpp"
#elif defined(CONFIG_PEDESTRIAN_DETECT_MODEL_LOCATION)
#include "pedestrian_detect.hpp"
#elif defined(CONFIG_CAT_DETECT_MODEL_LOCATION)
#include "cat_detect.hpp"
#elif defined(CONFIG_DOG_DETECT_MODEL_LOCATION)
#include "dog_detect.hpp"
#endif

using namespace who::frame_cap;
using namespace who::app;

//  FUNCIÓN DE CONEXIÓN MANUAL 
void wifi_init_manual(void) {
// 1. INICIALIZACIÓN DE LA CAPA DE ABSTRACCIÓN DE RED Y BUCLE DE EVENTOS
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
// 2. CONFIGURACIÓN DEL DRIVER DE RADIO CON PARÁMETROS POR DEFECTO
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
// 3. INYECCIÓN ESTÁTICA DE CREDENCIALES DE SEGURIDAD (SSID Y PASSWORD)
    wifi_config_t wifi_config = {};

    strcpy((char*)wifi_config.sta.ssid, "Livebox6-7ED0-24G"); 
    strcpy((char*)wifi_config.sta.password, "iA4HNgbRj7Zb");
// 4. ESTABLECIMIENTO DEL MODO ESTACIÓN Y ARRANQUE DE LA CONEXIÓN FISICA
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());

    ESP_LOGI("WIFI", "Conectando a %s...", wifi_config.sta.ssid);
}
//      INSTANCIACIÓN POLIMÓRFICA DEL MODELO DE INFERENCIA DE RED NEURONAL (CNN)
dl::detect::Detect *get_detect_model()
{
#if defined(CONFIG_HUMAN_FACE_DETECT_MODEL_LOCATION)
// RETORNA CONSTRUCTOR ACTIVO PARA RED CONVOLUCIONAL DE ROSTROS
    return new HumanFaceDetect(static_cast<HumanFaceDetect::model_type_t>(CONFIG_DEFAULT_HUMAN_FACE_DETECT_MODEL),
                               false);
#elif defined(CONFIG_PEDESTRIAN_DETECT_MODEL_LOCATION)
// RETORNA CONSTRUCTOR ACTIVO PARA RED CONVOLUCIONAL DE PEATONES
    return new PedestrianDetect(static_cast<PedestrianDetect::model_type_t>(CONFIG_DEFAULT_PEDESTRIAN_DETECT_MODEL),
                                false);
#elif defined(CONFIG_CAT_DETECT_MODEL_LOCATION)
// RETORNA CONSTRUCTOR ACTIVO PARA RED CONVOLUCIONAL FELINA
    return new CatDetect(static_cast<CatDetect::model_type_t>(CONFIG_DEFAULT_CAT_DETECT_MODEL), false);
#elif defined(CONFIG_DOG_DETECT_MODEL_LOCATION)
// RETORNA CONSTRUCTOR ACTIVO PARA RED CONVOLUCIONAL CANINA
    return new DogDetect(static_cast<DogDetect::model_type_t>(CONFIG_DEFAULT_DOG_DETECT_MODEL), false);
#else
// MANEJO DE EXCEPCIÓN EN CASO DE AUSENCIA DE COMPONENTES DE INTELIGENCIA ARTIFICIAL
    ESP_LOGE("MAIN", "No detect model component in idf_component.yml");
    return nullptr;
#endif
}
//      PIPELINE DE VISIÓN ARTIFICIAL LOCAL: DETECCIÓN Y RENDERIZADO EN PANEL LCD
void run_detect_lcd()
{
    WhoFrameCapNode *lcd_disp_frame_cap_node = nullptr;
// 1. SELECCIÓN DINÁMICA DE LA CANALIZACIÓN HARDWARE SEGÚN LA ARQUITECTURA DEL SOC    
#if CONFIG_IDF_TARGET_ESP32S3
    auto frame_cap = get_lcd_dvp_frame_cap_pipeline();
#elif CONFIG_IDF_TARGET_ESP32P4
    auto frame_cap = get_lcd_mipi_csi_frame_cap_pipeline();
    // auto frame_cap = get_lcd_mipi_csi_ppa_frame_cap_pipeline(&lcd_disp_frame_cap_node);
    // auto frame_cap = get_lcd_uvc_frame_cap_pipeline();
#endif
// 2. INSTANCIACIÓN DE LA APLICACIÓN CON MÁSCARA DE COLOR BGR PARA CAJAS DELIMITADORAS
    auto detect_app = new WhoDetectAppLCD({{255, 0, 0}}, frame_cap, lcd_disp_frame_cap_node);
    // create model later to avoid memory fragmentation.
    detect_app->set_model(get_detect_model());
    detect_app->run();
}
//      PIPELINE DE VISIÓN ARTIFICIAL REMOTA: TELEMETRÍA Y DIAGNÓSTICO POR TERMINAL UART
void run_detect_term()
{
// 1. SELECCIÓN DE CANALIZACIÓN DE VÍDEO COMPILADA POR HARDWARE PARA LA TERMINAL    
#if CONFIG_IDF_TARGET_ESP32S3
    auto frame_cap = get_term_dvp_frame_cap_pipeline();
#elif CONFIG_IDF_TARGET_ESP32P4
    auto frame_cap = get_term_mipi_csi_frame_cap_pipeline();
    // auto frame_cap = get_term_mipi_csi_ppa_frame_cap_pipeline();
    // auto frame_cap = get_term_uvc_frame_cap_pipeline();
#endif
// 2. LANZAMIENTO DEL HILO DE PROCESAMIENTO MATRICIAL EN MODO CONSOLA SERIE
    auto detect_app = new WhoDetectAppTerm(frame_cap);
    // create model later to avoid memory fragmentation.
// 3. ENLACE CON EL MOTOR DE IA Y EJECUCIÓN CÍCLICA DEL PROCESO DE INFERENCIA    
    detect_app->set_model(get_detect_model());
    detect_app->run();
}
//      PUNTO DE ENTRADA PRINCIPAL DEL FIRMWARE DEL SISTEMA EMBEBIDO

extern "C" void app_main(void)
{
    // 1. INICIALIZAR NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. INICIALIZAR WI-FI
    wifi_init_manual();

    // 3. PRIORIDAD Y CONFIGURACIÓN ORIGINAL
    vTaskPrioritySet(xTaskGetCurrentTaskHandle(), 5);

#if CONFIG_HUMAN_FACE_DETECT_MODEL_IN_SDCARD || CONFIG_PEDESTRIAN_DETECT_MODEL_IN_SDCARD || \
    CONFIG_CAT_DETECT_MODEL_IN_SDCARD || CONFIG_DOG_DETECT_MODEL_IN_SDCARD
    ESP_ERROR_CHECK(bsp_sdcard_mount());
#endif

    // Configuración de LEDs
#ifdef BSP_BOARD_ESP32_S3_EYE
    ESP_ERROR_CHECK(bsp_leds_init());
    ESP_ERROR_CHECK(bsp_led_set(BSP_LED_GREEN, false));
#endif

    // 4. ESPERAR 2 SEGUNDOS
    vTaskDelay(pdMS_TO_TICKS(2000));

    // 5. ARRANCAR LA APLICACIÓN
    // La conversión se hará internamente en who_detect_app_lcd.cpp
    run_detect_lcd();
}