// BEGIN --- Standard C headers section ---
#include <stdio.h>
#include <string.h>

// END   --- Standard C headers section ---

// BEGIN --- SDK config section---
#include <sdkconfig.h>
// END   --- SDK config section---

// BEGIN --- FreeRTOS headers section ---
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#if CONFIG_TOUCHSCREEN_USE_THREAD
  #include <freertos/semphr.h>
#endif

// END   --- FreeRTOS headers section ---


// BEGIN --- ESP-IDF headers section ---
#include <esp_log.h>

// END   --- ESP-IDF headers section ---

// BEGIN --- LVGL headers section ---
#include <lvgl.h>
// END   --- LVGL headers section ---
// BEGIN --- Project configuration section ---
#include <PrjCfg.h> // Including project configuration module 
// END   --- Project configuration section ---

// BEGIN --- Project configuration section ---

// end   --- Project configuration section ---

// BEGIN --- Self-includes section ---
#include "TouchScreen.h"
#include "TouchScreen_netvars.h"
#include "lvgl_port.h"
// END --- Self-includes section ---

// BEGIN --- Logging related variables
static const char *TAG = "TouchScreen";
// END --- Logging related variables

// BEGIN --- Internal variables (DRE)
TouchScreen_dre_t TouchScreen_dre = {
    .enabled = true,
    .last_return_code = TouchScreen_ret_ok,
    .line1 = "linea1",
    .line2 = "linea2",
    .line3 = "linea3",
    .line4 = "linea4",
    .line5 = "linea5",
    .line6 = "linea6",
    .line7 = "linea7",
    .line8 = "linea8",
    .line9 = "linea9",
    .line10 = "linea10"
};

// END   --- Internal variables (DRE)

// BEGIN --- Multitasking variables and handlers

#if CONFIG_TOUCHSCREEN_USE_THREAD
static TaskHandle_t s_task = NULL;
static volatile bool s_run = false;
static uint32_t s_period_ms =
    #ifdef CONFIG_TOUCHSCREEN_PERIOD_MS
      CONFIG_TOUCHSCREEN_PERIOD_MS
    #else
      1000
    #endif
;
static SemaphoreHandle_t s_mutex = NULL;

static inline void _lock(void)   { if (s_mutex) xSemaphoreTake(s_mutex, portMAX_DELAY); }
static inline void _unlock(void) { if (s_mutex) xSemaphoreGive(s_mutex); }

#ifdef CONFIG_TOUCHSCREEN_MINIMIZE_JITTER
    static TickType_t xLastWakeTime;
    static TickType_t xFrequency;
#endif

static TouchScreen_return_code_t TouchScreen_spin(void);  // In case we are using a thread, this function should not be part of the public API

static inline BaseType_t _create_mutex_once(void)
{
    if (!s_mutex) {
        s_mutex = xSemaphoreCreateMutex();
        if (!s_mutex) return pdFAIL;
    }
    return pdPASS;
}

static inline BaseType_t _get_core_affinity(void)
{
    #if CONFIG_TOUCHSCREEN_PIN_CORE_ANY
        return tskNO_AFFINITY;
    #elif CONFIG_TOUCHSCREEN_PIN_CORE_0
        return 0;
    #elif CONFIG_TOUCHSCREEN_PIN_CORE_1
        return 1;
    #else
        return tskNO_AFFINITY;
    #endif
}

static void TouchScreen_task(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "task started (period=%u ms)", (unsigned)s_period_ms);
#ifdef CONFIG_TOUCHSCREEN_MINIMIZE_JITTER
    xLastWakeTime = xTaskGetTickCount();
    xFrequency = (s_period_ms / portTICK_PERIOD_MS);
#endif
    while (s_run) {
        TouchScreen_return_code_t ret = TouchScreen_spin();
        if (ret != TouchScreen_ret_ok)
        {
            ESP_LOGW(TAG, "Error in spin");
        }
#ifdef CONFIG_TOUCHSCREEN_MINIMIZE_JITTER
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
#else
        vTaskDelay(pdMS_TO_TICKS(s_period_ms));
#endif
    }
    ESP_LOGI(TAG, "task exit");
    vTaskDelete(NULL);
}

#endif // CONFIG_TOUCHSCREEN_USE_THREAD

// END   --- Multitasking variables and handlers

// BEGIN --- UI helpers
#define TOUCHSCREEN_MAX_LINES 10
#define TOUCHSCREEN_LINE_MAX_CHARS 128

static lv_obj_t *s_labels[TOUCHSCREEN_MAX_LINES] = {0};
static char s_prev_lines[TOUCHSCREEN_MAX_LINES][TOUCHSCREEN_LINE_MAX_CHARS + 1] = {{0}};
static bool s_ui_ready = false;

static lv_obj_t *s_screen = NULL;

static void touchscreen_update(const TouchScreen_dre_t *snapshot)
{
    if (!s_ui_ready) return;
    if (!lvgl_port_lock(-1)) return;
    for (size_t i = 0; i < TOUCHSCREEN_MAX_LINES; ++i) {
        if (!s_labels[i]) continue;
        const char *src = NULL;
        switch (i) {
        case 0: src = snapshot->line1; break;
        case 1: src = snapshot->line2; break;
        case 2: src = snapshot->line3; break;
        case 3: src = snapshot->line4; break;
        case 4: src = snapshot->line5; break;
        case 5: src = snapshot->line6; break;
        case 6: src = snapshot->line7; break;
        case 7: src = snapshot->line8; break;
        case 8: src = snapshot->line9; break;
        case 9: src = snapshot->line10; break;
        default: break;
        }
        if (!src) continue;
        if (strncmp(src, s_prev_lines[i], TOUCHSCREEN_LINE_MAX_CHARS) != 0) {
            lv_label_set_text(s_labels[i], src);
            strlcpy(s_prev_lines[i], src, sizeof(s_prev_lines[i]));
        }
    }
    lvgl_port_unlock();
}

// BEGIN ------------------ Public API (MULTITASKING)------------------


#if CONFIG_TOUCHSCREEN_USE_THREAD

TouchScreen_return_code_t TouchScreen_start(void)
{
    if (_create_mutex_once() != pdPASS) {
        ESP_LOGE(TAG, "mutex creation failed");
        return TouchScreen_ret_error;
    }
    if (s_task) {
        // idempotente
        return TouchScreen_ret_ok;
    }
    s_run = true;

    BaseType_t core = _get_core_affinity();
    BaseType_t ok = xTaskCreatePinnedToCore(
        TouchScreen_task,
        "TouchScreen",
        CONFIG_TOUCHSCREEN_TASK_STACK,
        NULL,
        CONFIG_TOUCHSCREEN_TASK_PRIO,
        &s_task,
        core
    );
    if (ok != pdPASS) {
        s_task = NULL;
        s_run = false;
        ESP_LOGE(TAG, "xTaskCreatePinnedToCore failed");
        return TouchScreen_ret_error;
    }
    return TouchScreen_ret_ok;
}

TouchScreen_return_code_t TouchScreen_stop(void)
{
    if (!s_task) return TouchScreen_ret_ok; // idempotente
    s_run = false;
    // Espera una vuelta de scheduler para que el loop salga y se autodelete
    vTaskDelay(pdMS_TO_TICKS(1));
    // Si aún vive por cualquier motivo, fuerza delete
    if (s_task) {
        TaskHandle_t t = s_task;
        s_task = NULL;
        vTaskDelete(t);
    }
    ESP_LOGI(TAG, "stopped");
    return TouchScreen_ret_ok;
}

TouchScreen_return_code_t TouchScreen_get_dre_clone(TouchScreen_dre_t *dst)
{
    if (!dst) return TouchScreen_ret_error;
    _lock();
    memcpy(dst, &TouchScreen_dre, sizeof(TouchScreen_dre));
    _unlock();
    return TouchScreen_ret_ok;
}

TouchScreen_return_code_t TouchScreen_set_period_ms(uint32_t period_ms)
{
    if (period_ms < 10) period_ms = 10;
    _lock();
    s_period_ms = period_ms;
#ifdef CONFIG_TOUCHSCREEN_MINIMIZE_JITTER    
    xFrequency = (s_period_ms / portTICK_PERIOD_MS);
#endif
    _unlock();
    ESP_LOGI(TAG, "period set to %u ms", (unsigned)period_ms);
    return TouchScreen_ret_ok;
}

uint32_t TouchScreen_get_period_ms(void)
{
    _lock();
    uint32_t v = s_period_ms;
    _unlock();
    return v;
}

/**
 *  Execute a function wrapped with locks so you can access the DRE variables in thread-safe mode
*/
void TouchScreen_execute_function_safemode(void (*callback)())
{
    _lock();
    callback();
    _unlock();
}

#endif // CONFIG_TOUCHSCREEN_USE_THREAD

// END   ------------------ Public API (MULTITASKING)------------------


#include "waveshare_rgb_lcd_port.h"

void touchscreen_main(void)
{
    if (s_ui_ready) return;

    waveshare_esp32_s3_rgb_lcd_init(); // Initialize the Waveshare ESP32-S3 RGB LCD 
    // wavesahre_rgb_lcd_bl_on();  //Turn on the screen backlight 
    // wavesahre_rgb_lcd_bl_off(); //Turn off the screen backlight 

    if (!lvgl_port_lock(-1)) {
        ESP_LOGW(TAG, "LVGL not ready (lvgl_port_lock failed)");
        return;
    }

    s_screen = lv_obj_create(NULL);
    lv_obj_set_size(s_screen, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_pad_all(s_screen, 8, 0);
    lv_obj_set_flex_flow(s_screen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(s_screen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    for (size_t i = 0; i < TOUCHSCREEN_MAX_LINES; ++i) {
        s_labels[i] = lv_label_create(s_screen);
        lv_obj_set_width(s_labels[i], LV_PCT(100));
        lv_label_set_long_mode(s_labels[i], LV_LABEL_LONG_WRAP);
        lv_label_set_text(s_labels[i], s_prev_lines[i]);
    }

    lv_scr_load(s_screen);
    s_ui_ready = true;
    lvgl_port_unlock();

    // Prime the UI with current values
    touchscreen_update(&TouchScreen_dre);
}


// BEGIN ------------------ Public API (COMMON + SPIN)------------------

TouchScreen_return_code_t TouchScreen_setup(void)
{
    // Init liviano; no arranca tarea.
    ESP_LOGD(TAG, "setup()");
    // Loading values from NVS
    TouchScreen_netvars_nvs_load();    
#if CONFIG_TOUCHSCREEN_USE_THREAD
    if (_create_mutex_once() != pdPASS) {
        ESP_LOGE(TAG, "mutex creation failed");
        return TouchScreen_ret_error;
    }
#endif
    TouchScreen_dre.last_return_code = TouchScreen_ret_ok;

        ESP_LOGI(TAG, "Launching touchscreen_main()");
        touchscreen_main();

    return TouchScreen_ret_ok;
}

#if CONFIG_TOUCHSCREEN_USE_THREAD
static  // In case we are using a thread, this function should not be part of the public API
#endif
TouchScreen_return_code_t TouchScreen_spin(void)
{
#if CONFIG_TOUCHSCREEN_USE_THREAD
    _lock();
#endif
    TouchScreen_dre_t snapshot = TouchScreen_dre;
#if CONFIG_TOUCHSCREEN_USE_THREAD
    _unlock();
#endif

    if (!snapshot.enabled)
    {
        return TouchScreen_ret_ok;
    }

    touchscreen_update(&snapshot);

    TouchScreen_nvs_spin();

    return TouchScreen_ret_ok;
}

TouchScreen_return_code_t TouchScreen_enable(void)
{
#if CONFIG_TOUCHSCREEN_USE_THREAD
    _lock();
#endif
    TouchScreen_dre.enabled = true;
    TouchScreen_dre.last_return_code = TouchScreen_ret_ok;
#if CONFIG_TOUCHSCREEN_USE_THREAD
    _unlock();
#endif
    return TouchScreen_ret_ok;
}

TouchScreen_return_code_t TouchScreen_disable(void)
{
#if CONFIG_TOUCHSCREEN_USE_THREAD
    _lock();
#endif
    TouchScreen_dre.enabled = false;
    TouchScreen_dre.last_return_code = TouchScreen_ret_ok;
#if CONFIG_TOUCHSCREEN_USE_THREAD
    _unlock();
#endif
    return TouchScreen_ret_ok;
}

// BEGIN ------------------ Public API (COMMON)------------------
