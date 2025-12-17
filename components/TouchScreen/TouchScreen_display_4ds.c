#include <sdkconfig.h>

#include "TouchScreen_display.h"

#include "esp_err.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_types.h"

#include "lvgl_port.h"

static const char *TAG = "TouchScreen4DS";

// GEN4-ESP32-43CT-CLB (4DSystems) pinout & timings (from GFX4dESP32 library)
#define GEN4_RGB_BK_LIGHT_ON_LEVEL 1
#define GEN4_RGB_PIN_NUM_BK_LIGHT  2

#define GEN4_RGB_PIN_NUM_HSYNC     39
#define GEN4_RGB_PIN_NUM_VSYNC     41
#define GEN4_RGB_PIN_NUM_DE        40
#define GEN4_RGB_PIN_NUM_PCLK      42
#define GEN4_RGB_PIN_NUM_DISP_EN   (-1)

#define GEN4_RGB_PIN_NUM_DATA0     8
#define GEN4_RGB_PIN_NUM_DATA1     3
#define GEN4_RGB_PIN_NUM_DATA2     46
#define GEN4_RGB_PIN_NUM_DATA3     9
#define GEN4_RGB_PIN_NUM_DATA4     1
#define GEN4_RGB_PIN_NUM_DATA5     5
#define GEN4_RGB_PIN_NUM_DATA6     6
#define GEN4_RGB_PIN_NUM_DATA7     7
#define GEN4_RGB_PIN_NUM_DATA8     15
#define GEN4_RGB_PIN_NUM_DATA9     16
#define GEN4_RGB_PIN_NUM_DATA10    4
#define GEN4_RGB_PIN_NUM_DATA11    45
#define GEN4_RGB_PIN_NUM_DATA12    48
#define GEN4_RGB_PIN_NUM_DATA13    47
#define GEN4_RGB_PIN_NUM_DATA14    21
#define GEN4_RGB_PIN_NUM_DATA15    14

#define GEN4_RGB_H_RES             800
#define GEN4_RGB_V_RES             480
#define GEN4_43CT_PIXEL_CLOCK_HZ   (16 * 1000 * 1000)

IRAM_ATTR static bool rgb_lcd_on_vsync_event(esp_lcd_panel_handle_t panel,
                                            const esp_lcd_rgb_panel_event_data_t *edata,
                                            void *user_ctx)
{
    (void)panel;
    (void)edata;
    (void)user_ctx;
    return lvgl_port_notify_rgb_vsync();
}

static void backlight_on(void)
{
    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << GEN4_RGB_PIN_NUM_BK_LIGHT,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&cfg));
    gpio_set_level(GEN4_RGB_PIN_NUM_BK_LIGHT, GEN4_RGB_BK_LIGHT_ON_LEVEL);
}

esp_err_t TouchScreen_display_init(void)
{
    ESP_LOGI(TAG, "Install RGB LCD panel driver (4DSystems GEN4-ESP32-43CT-CLB)");
    esp_lcd_panel_handle_t panel_handle = NULL;

    esp_lcd_rgb_panel_config_t panel_config = {
        .clk_src = LCD_CLK_SRC_PLL160M,
        .timings = {
            .pclk_hz = GEN4_43CT_PIXEL_CLOCK_HZ,
            .h_res = GEN4_RGB_H_RES,
            .v_res = GEN4_RGB_V_RES,
            .hsync_pulse_width = 4,
            .hsync_back_porch = 8,
            .hsync_front_porch = 8,
            .vsync_pulse_width = 4,
            .vsync_back_porch = 8,
            .vsync_front_porch = 8,
            .flags = {
                .vsync_idle_low = 1,
                .pclk_active_neg = 1,
            },
        },
        .data_width = 16,
        .bits_per_pixel = 16,
        .num_fbs = LVGL_PORT_LCD_RGB_BUFFER_NUMS,
        .bounce_buffer_size_px = GEN4_RGB_H_RES * CONFIG_LCD_RGB_BOUNCE_BUFFER_HEIGHT,
        .sram_trans_align = 4,
        .psram_trans_align = 64,
        .hsync_gpio_num = GEN4_RGB_PIN_NUM_HSYNC,
        .vsync_gpio_num = GEN4_RGB_PIN_NUM_VSYNC,
        .de_gpio_num = GEN4_RGB_PIN_NUM_DE,
        .pclk_gpio_num = GEN4_RGB_PIN_NUM_PCLK,
        .disp_gpio_num = GEN4_RGB_PIN_NUM_DISP_EN,
        .data_gpio_nums = {
            GEN4_RGB_PIN_NUM_DATA0,
            GEN4_RGB_PIN_NUM_DATA1,
            GEN4_RGB_PIN_NUM_DATA2,
            GEN4_RGB_PIN_NUM_DATA3,
            GEN4_RGB_PIN_NUM_DATA4,
            GEN4_RGB_PIN_NUM_DATA5,
            GEN4_RGB_PIN_NUM_DATA6,
            GEN4_RGB_PIN_NUM_DATA7,
            GEN4_RGB_PIN_NUM_DATA8,
            GEN4_RGB_PIN_NUM_DATA9,
            GEN4_RGB_PIN_NUM_DATA10,
            GEN4_RGB_PIN_NUM_DATA11,
            GEN4_RGB_PIN_NUM_DATA12,
            GEN4_RGB_PIN_NUM_DATA13,
            GEN4_RGB_PIN_NUM_DATA14,
            GEN4_RGB_PIN_NUM_DATA15,
        },
        .flags = {
            .refresh_on_demand = 0,
            .fb_in_psram = 1,
            .double_fb = 0,
        },
    };

    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    backlight_on();

    // No touch controller integration yet (unknown controller / pins)
    ESP_ERROR_CHECK(lvgl_port_init(panel_handle, NULL));

    esp_lcd_rgb_panel_event_callbacks_t cbs = {
        .on_vsync = rgb_lcd_on_vsync_event,
    };
    ESP_ERROR_CHECK(esp_lcd_rgb_panel_register_event_callbacks(panel_handle, &cbs, NULL));

    return ESP_OK;
}
