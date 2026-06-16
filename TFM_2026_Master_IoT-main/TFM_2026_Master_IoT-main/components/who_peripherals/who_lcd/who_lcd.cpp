#include "who_lcd.hpp"
#include "esp_lcd_panel_ops.h"
#include <string.h>
#if BSP_CONFIG_NO_GRAPHIC_LIB
namespace who {
namespace lcd {
//      MÉTODO GETTER: EXTRACCIÓN DEL MANIPULADOR DE PANEL SEGÚN LA ARQUITECTURA
esp_lcd_panel_handle_t WhoLCD::get_lcd_panel_handle()
{
#if CONFIG_IDF_TARGET_ESP32S3
    return m_panel_handle;
#elif CONFIG_IDF_TARGET_ESP32P4
    return m_lcd_handles.panel;
#endif
}
//      IMPLEMENTACIONES FÍSICAS DE CONTROLADORES PARA HARDWARE ESP32-S3 (DVP)
#if CONFIG_IDF_TARGET_ESP32S3
void WhoLCD::init()
{
// 1. CONFIGURACIÓN DEL ANCHO DE BANDA MÁXIMO DE TRANSFERENCIA POR PÍXEL
    const bsp_display_config_t bsp_disp_cfg = {
        .max_transfer_sz = BSP_LCD_H_RES * BSP_LCD_V_RES * (BSP_LCD_BITS_PER_PIXEL / 8),
    };
// 2. ASIGNACIÓN ASÍNCRONA DE MANIPULADORES E INICIALIZACIÓN DEL BACKLIGHT
    ESP_ERROR_CHECK(bsp_display_new(&bsp_disp_cfg, &m_panel_handle, &m_io_handle));
    esp_lcd_panel_disp_on_off(m_panel_handle, true);
    ESP_ERROR_CHECK(bsp_display_backlight_on());
// 3. RESERVA ESTÁTICA DE UN BUFFER EN MEMORIA INTERNA RAM CON CAPACIDAD DMA
    m_lcd_buffer = heap_caps_malloc(BSP_LCD_H_RES * BSP_LCD_V_RES * (BSP_LCD_BITS_PER_PIXEL / 8),
                                    MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
}

void WhoLCD::deinit()
{
// 1. LIBERACIÓN ASIGNADA DEL ESPACIO DE MEMORIA RESERVADO PARA TRANSFERENCIAS DMA
    // TODO release lcd resources.
    heap_caps_free(m_lcd_buffer);
}

void WhoLCD::draw_bitmap(const void *data, int width, int height, int x_start, int y_start)
{
// 1. COPIA DEL FRAME BUFFER DESDE LA PSRAM HACIA LA MEMORIA INTERNA RAM (REQUISITO DMA)
    // Currently esp-idf doesn't support DMA for PSRAM in esp32s3.
    // Display the fb must copy it from PSRAM to internal RAM.
    // Use a static internal memory chunk to avoid alloc internal memory each time dynamically.
    memcpy(m_lcd_buffer, data, width * height * (BSP_LCD_BITS_PER_PIXEL / 8));
// 2. VOLCADO DIRECTO POR HARDWARE DEL MAPA DE BITS INTERMEDIO HACIA EL PANEL LCD
    esp_lcd_panel_draw_bitmap(m_panel_handle, x_start, y_start, x_start + width, y_start + height, m_lcd_buffer);
}
//      IMPLEMENTACIONES FÍSICAS DE CONTROLADORES PARA HARDWARE ESP32-P4 (MIPI-DSI)
#elif CONFIG_IDF_TARGET_ESP32P4
void WhoLCD::init()
{
// 1. EVALUACIÓN Y MAPEO DINÁMICO DE CONFIGURACIÓN DE RESOLUCIONES HDMI DE LA PLACA
    bsp_display_config_t bsp_disp_cfg = {
#if CONFIG_BSP_LCD_TYPE_HDMI
#if CONFIG_BSP_LCD_HDMI_800x600_60HZ
        .hdmi_resolution = BSP_HDMI_RES_800x600,
#elif CONFIG_BSP_LCD_HDMI_1280x720_60HZ
        .hdmi_resolution = BSP_HDMI_RES_1280x720,
#elif CONFIG_BSP_LCD_HDMI_1280x800_60HZ
        .hdmi_resolution = BSP_HDMI_RES_1280x800,
#elif CONFIG_BSP_LCD_HDMI_1920x1080_30HZ
        .hdmi_resolution = BSP_HDMI_RES_1920x1080,
#endif
#else
        .hdmi_resolution = BSP_HDMI_RES_NONE,
#endif
// 2. ESTRUCTURACIÓN DE RELOJES DE CAPA FÍSICA Y TASA DE BITS PARA EL BUS MIPI-DSI
        .dsi_bus = {
            .phy_clk_src = MIPI_DSI_PHY_CLK_SRC_DEFAULT,
            .lane_bit_rate_mbps = BSP_LCD_MIPI_DSI_LANE_BITRATE_MBPS,
        }};
// 3. ARRANQUE DEL SUBSISTEMA GRÁFICO AVANZADO DEL SOC P4 E ILUMINACIÓN
    bsp_display_new_with_handles(&bsp_disp_cfg, &m_lcd_handles);
    ESP_ERROR_CHECK(bsp_display_brightness_init());
    ESP_ERROR_CHECK(bsp_display_backlight_on());
}

void WhoLCD::deinit()
{
// 1. DESTRUCCIÓN COMPLETA DE ESTRUCTURAS Y CONEXIONES DE VIDEO DE BAJO NIVEL
    bsp_display_delete();
}

void WhoLCD::draw_bitmap(const void *data, int width, int height, int x_start, int y_start)
{
// 1. VOLCADO DIRECTO SIN BUFFER INTERMEDIO GRACIAS A LA ARQUITECTURA DE ACCESO P4
    esp_lcd_panel_draw_bitmap(m_lcd_handles.panel, x_start, y_start, x_start + width, y_start + height, data);
}
#endif
} // namespace lcd
} // namespace who
#endif
