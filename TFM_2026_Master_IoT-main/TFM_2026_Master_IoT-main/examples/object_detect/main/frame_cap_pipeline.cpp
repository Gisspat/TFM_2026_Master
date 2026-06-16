#include "frame_cap_pipeline.hpp"
#include "who_cam.hpp"

using namespace who::cam;
using namespace who::frame_cap;

//      CONFIGURACIÓN DINÁMICA DE RESOLUCIONES Y TIEMPOS DE INFERENCIA DE MODELOS CNN
#if defined(CONFIG_HUMAN_FACE_DETECT_MODEL_LOCATION)
// PARAMETRIZACIÓN PARA EL MODELO DE DETECCIÓN DE ROSTROS HUMANOS
#define MODEL_TIME 2
#define MODEL_INPUT_W 160
#define MODEL_INPUT_H 120
#elif defined(CONFIG_PEDESTRIAN_DETECT_MODEL_LOCATION)
// PARAMETRIZACIÓN PARA EL MODELO DE DETECCIÓN DE PEATONES
#define MODEL_TIME 3
#define MODEL_INPUT_W 224
#define MODEL_INPUT_H 224
#elif defined(CONFIG_CAT_DETECT_MODEL_LOCATION)
// PARAMETRIZACIÓN PARA EL MODELO DE DETECCIÓN CLASIFICADOR FELINO
#if defined(CONFIG_ESPDET_PICO_224_224_CAT)
#define MODEL_TIME 3
#define MODEL_INPUT_W 224
#define MODEL_INPUT_H 224
#endif
#if defined(CONFIG_ESPDET_PICO_416_416_CAT)
#define MODEL_TIME 8
#define MODEL_INPUT_W 416
#define MODEL_INPUT_H 416
#endif
#elif defined(CONFIG_DOG_DETECT_MODEL_LOCATION)
// PARAMETRIZACIÓN PARA EL MODELO DE DETECCIÓN CLASIFICADOR CANINO
#if defined(CONFIG_ESPDET_PICO_224_224_DOG)
#define MODEL_TIME 3
#define MODEL_INPUT_W 224
#define MODEL_INPUT_H 224
#endif
#if defined(CONFIG_ESPDET_PICO_416_416_DOG)
#define MODEL_TIME 8
#define MODEL_INPUT_W 416
#define MODEL_INPUT_H 416
#endif
#endif

//      PIPELINES DE CAPTURA PARA ARQUITECTURAS DE HARDWARE ESP32-S3
#if CONFIG_IDF_TARGET_ESP32S3
WhoFrameCap *get_lcd_dvp_frame_cap_pipeline()
{
    // 1. OBTENCIÓN DEL TAMAÑO DE FRAME SEGÚN LA RESOLUCIÓN DE LA PANTALLA LCD
    framesize_t frame_size = get_cam_frame_size_from_lcd_resolution();
    
    // 2. ASIGNACIÓN DEL BUFFER DE CÁMARA CALCULANDO EL TIEMPO DE PROCESAMIENTO SIN RETARDO
#ifdef BSP_BOARD_ESP32_S3_KORVO_2
    auto cam = new WhoS3Cam(PIXFORMAT_RGB565, frame_size, MODEL_TIME + 3, true, true);
#else
    auto cam = new WhoS3Cam(PIXFORMAT_RGB565, frame_size, MODEL_TIME + 3);
#endif

    // 3. ENLAZADO E INYECCIÓN DEL NODO DE CAPTURA AL FLUJO DE TRABAJO PRINCIPAL
    auto frame_cap = new WhoFrameCap();
    frame_cap->add_node<WhoFetchNode>("FrameCapFetch", cam);
    return frame_cap;
}

WhoFrameCap *get_term_dvp_frame_cap_pipeline()
{
    // 1. CONFIGURACIÓN DEL BUFFER REDUCIDO OPTIMIZADO PARA SALIDA UART SIN PANEL LCD
    framesize_t frame_size = get_cam_frame_size_from_lcd_resolution();
#ifdef BSP_BOARD_ESP32_S3_KORVO_2
    auto cam = new WhoS3Cam(PIXFORMAT_RGB565, frame_size, MODEL_TIME + 2, true, true);
#else
    auto cam = new WhoS3Cam(PIXFORMAT_RGB565, frame_size, MODEL_TIME + 2);
#endif

    // 2. REGISTRO DEL NODO DE ADQUISICIÓN DIRECTA EN TERMINAL
    auto frame_cap = new WhoFrameCap();
    frame_cap->add_node<WhoFetchNode>("FrameCapFetch", cam);
    return frame_cap;
}

// ==================================================================================
//      PIPELINES DE CAPTURA PARA ARQUITECTURAS DE HARDWARE ESP32-P4
// ==================================================================================
#elif CONFIG_IDF_TARGET_ESP32P4
WhoFrameCap *get_lcd_mipi_csi_frame_cap_pipeline()
{
    // 1. INICIALIZACIÓN DE ADQUISICIÓN DE VÍDEO POR BUS DE ALTA VELOCIDAD MIPI-CSI
    auto cam = new WhoP4Cam(V4L2_PIX_FMT_RGB565, MODEL_TIME + 3);
    auto frame_cap = new WhoFrameCap();
    frame_cap->add_node<WhoFetchNode>("FrameCapFetch", cam);
    return frame_cap;
}

WhoFrameCap *get_lcd_mipi_csi_ppa_frame_cap_pipeline(WhoFrameCapNode **lcd_disp_frame_cap_node)
{
    // 1. INICIALIZACIÓN DE LA CÁMARA CON BUFFER EXTENDIDO PARA POST-PROCESAMIENTO CONTRASTADO
    auto cam = new WhoP4Cam(V4L2_PIX_FMT_RGB565, MODEL_TIME + 4);
    auto frame_cap = new WhoFrameCap();
    frame_cap->add_node<WhoFetchNode>("FrameCapFetch", cam);
    
    // 2. ACELERACIÓN POR ACCESO DIRECTO DE ACELERADOR GRÁFICO HARDWARE (PPA RESIZE NODE)
    frame_cap->add_node<WhoPPAResizeNode>(
        "FrameCapPPAResize", MODEL_INPUT_W, MODEL_INPUT_H, dl::image::DL_IMAGE_PIX_TYPE_RGB565, MODEL_TIME);
        
    // 3. EXTRACCIÓN DE REFERENCIA DE PUNTERO PARA DESACOPLAR CAPTURA Y VISUALIZACIÓN
    *lcd_disp_frame_cap_node = frame_cap->get_node("FrameCapFetch");
    return frame_cap;
}

WhoFrameCap *get_lcd_uvc_frame_cap_pipeline()
{
    // 1. ENLACE DE CÁMARA EXTERNA MEDIANTE ARQUITECTURA DE PROTOCOLO USB UVC
    auto cam = new WhoUVCCam(UVC_VS_FORMAT_MJPEG, 640, 480, 30, 4);
    auto frame_cap = new WhoFrameCap();
    frame_cap->add_node<WhoFetchNode>("FrameCapFetch", cam, false);
    
    // 2. INSTANCIACIÓN DEL NODO TRADUCTOR DE DESCOMPRESIÓN HARDWARE EN FORMATO RAW
    frame_cap->add_node<WhoDecodeNode>("FrameCapDecode", dl::image::DL_IMAGE_PIX_TYPE_RGB565, 2, false);
    
    // 3. ADAPTACIÓN ASÍNCRONA DE ESCALA A TRAVÉS DE PPA PARA SINCRONISMO DE INFERENCIA
    frame_cap->add_node<WhoPPAResizeNode>(
        "FrameCapPPAResize", 800, 600, dl::image::DL_IMAGE_PIX_TYPE_RGB565, MODEL_TIME + 1);
    return frame_cap;
}

WhoFrameCap *get_term_mipi_csi_frame_cap_pipeline()
{
    // 1. CAPTURA PURA MIPI CON DIMENSIÓN DE MEMORIA ESTÁTICA BAJA PARA ANÁLISIS MATRICIAL
    auto cam = new WhoP4Cam(V4L2_PIX_FMT_RGB565, MODEL_TIME + 2);
    auto frame_cap = new WhoFrameCap();
    frame_cap->add_node<WhoFetchNode>("FrameCapFetch", cam);
    return frame_cap;
}

WhoFrameCap *get_term_mipi_csi_ppa_frame_cap_pipeline()
{
    // 1. INICIALIZACIÓN DE CAPTURA MEDIANTE LA PILA COMPILADA MIPI-CSI CON BUFFER INTERMEDIO
    auto cam = new WhoP4Cam(V4L2_PIX_FMT_RGB565, MODEL_TIME + 3);
    auto frame_cap = new WhoFrameCap();
    frame_cap->add_node<WhoFetchNode>("FrameCapFetch", cam);
    
    // 2. REDIMENSIONAMIENTO AUTOMÁTICO DE ENTRADA PARA EL MODELO MATEMÁTICO DE IA
    frame_cap->add_node<WhoPPAResizeNode>(
        "FrameCapPPAResize", MODEL_INPUT_W, MODEL_INPUT_H, dl::image::DL_IMAGE_PIX_TYPE_RGB565, MODEL_TIME);
    return frame_cap;
}

WhoFrameCap *get_term_uvc_frame_cap_pipeline()
{
    // 1. INTERCONEXIÓN USB DE ALTA VELOCIDAD Y ASIGNACIÓN AUTOMÁTICA DE INTERRUPCIONES
    auto cam = new WhoUVCCam(UVC_VS_FORMAT_MJPEG, 640, 480, 30, 4);
    auto frame_cap = new WhoFrameCap();
    frame_cap->add_node<WhoFetchNode>("FrameCapFetch", cam, false);
    
    // 2. LANZAMIENTO DEL DECODIFICADOR REACTIVO SÍNCRONO PARA MONITOREO DE EVENTOS UART
    frame_cap->add_node<WhoDecodeNode>("FrameCapDecode", dl::image::DL_IMAGE_PIX_TYPE_RGB565, MODEL_TIME);
    return frame_cap;
}
#endif