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
#if CONFIG_UIDEMO_USE_THREAD
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
#include <TouchScreen.h>
// end   --- Project configuration section ---

// BEGIN --- Self-includes section ---
#include "UIDemo.h"
#include "UIDemo_netvars.h"

// END --- Self-includes section ---

// BEGIN --- Logging related variables
static const char *TAG = "UIDemo";
// END --- Logging related variables

// BEGIN --- Internal variables (DRE)
UIDemo_dre_t UIDemo_dre = {
    .enabled = true,
    .last_return_code = UIDemo_ret_ok,
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

lv_style_t style_header_bg;
lv_style_t style_btn_primary;
lv_style_t style_text_body;
lv_style_t style_text_invert;

// BEGIN --- UI helpers
#define TOUCHSCREEN_MAX_LINES 10
#define TOUCHSCREEN_LINE_MAX_CHARS 128

static lv_obj_t *s_screen = NULL;
static bool s_ui_ready = false;

//#define UIDEMO_FIRST_DEMO
#ifdef UIDEMO_FIRST_DEMO

static lv_obj_t *s_labels[TOUCHSCREEN_MAX_LINES] = {0};
static char s_prev_lines[TOUCHSCREEN_MAX_LINES][TOUCHSCREEN_LINE_MAX_CHARS + 1] = {{0}};
static lv_obj_t *s_button = NULL;
static lv_obj_t *s_slider = NULL;
static lv_obj_t *s_controls = NULL;
static lv_obj_t *s_chart = NULL;
static lv_chart_series_t *s_chart_series = NULL;
static lv_timer_t *s_chart_timer = NULL;
static lv_obj_t *s_alert = NULL;
static int s_last_slider_value = 0;
static void touchscreen_update(void);
static void UIDemo_ensure_ui(void);

#if CONFIG_UIDEMO_USE_THREAD
static inline void _lock(void);
static inline void _unlock(void);
#endif


static inline void set_line_text(size_t idx, const char *text)
{
    if (!text) return;
#if CONFIG_UIDEMO_USE_THREAD
    _lock();
#endif
    switch (idx) {
    case 0: strlcpy(UIDemo_dre.line1, text, sizeof(UIDemo_dre.line1)); break;
    case 1: strlcpy(UIDemo_dre.line2, text, sizeof(UIDemo_dre.line2)); break;
    case 2: strlcpy(UIDemo_dre.line3, text, sizeof(UIDemo_dre.line3)); break;
    case 3: strlcpy(UIDemo_dre.line4, text, sizeof(UIDemo_dre.line4)); break;
    case 4: strlcpy(UIDemo_dre.line5, text, sizeof(UIDemo_dre.line5)); break;
    case 5: strlcpy(UIDemo_dre.line6, text, sizeof(UIDemo_dre.line6)); break;
    case 6: strlcpy(UIDemo_dre.line7, text, sizeof(UIDemo_dre.line7)); break;
    case 7: strlcpy(UIDemo_dre.line8, text, sizeof(UIDemo_dre.line8)); break;
    case 8: strlcpy(UIDemo_dre.line9, text, sizeof(UIDemo_dre.line9)); break;
    case 9: strlcpy(UIDemo_dre.line10, text, sizeof(UIDemo_dre.line10)); break;
    default: break;
    }
#if CONFIG_UIDEMO_USE_THREAD
    _unlock();
#endif
    touchscreen_update();
}

static void btn_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        set_line_text(0, "boton pulsado");
    }
}

static void slider_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *slider = lv_event_get_target(e);
        int val = lv_slider_get_value(slider);
        s_last_slider_value = val;
        char buf[32];
        snprintf(buf, sizeof(buf), "slider: %d", val);
        set_line_text(1, buf);
    }
}

static void chart_timer_cb(lv_timer_t *t)
{
    (void)t;
    if (!s_ui_ready || !s_chart || !s_chart_series) return;
    if (TouchScreen_lvgl_lock(-1)) {
        lv_chart_set_next_value(s_chart, s_chart_series, s_last_slider_value);
        TouchScreen_lvgl_unlock();
    }
}
#endif

static void touchscreen_update(void)
{
#if CONFIG_UIDEMO_USE_THREAD
    _lock();
#endif
    UIDemo_dre_t snapshot = UIDemo_dre;
#if CONFIG_UIDEMO_USE_THREAD
    _unlock();
#endif

    if (!s_ui_ready) return;
    if (!TouchScreen_lvgl_lock(-1)) return;

#ifdef UIDEMO_FIRST_DEMO
    for (size_t i = 0; i < TOUCHSCREEN_MAX_LINES; ++i) {
        if (!s_labels[i]) continue;
        const char *src = NULL;
        switch (i) {
        case 0: src = snapshot.line1; break;
        case 1: src = snapshot.line2; break;
        case 2: src = snapshot.line3; break;
        case 3: src = snapshot.line4; break;
        case 4: src = snapshot.line5; break;
        case 5: src = snapshot.line6; break;
        case 6: src = snapshot.line7; break;
        case 7: src = snapshot.line8; break;
        case 8: src = snapshot.line9; break;
        case 9: src = snapshot.line10; break;
        default: break;
        }
        if (!src) continue;
        if (strncmp(src, s_prev_lines[i], TOUCHSCREEN_LINE_MAX_CHARS) != 0) {
            lv_label_set_text(s_labels[i], src);
            strlcpy(s_prev_lines[i], src, sizeof(s_prev_lines[i]));
        }
    }
    // Alert: show only when line10 == "1"
    if (s_alert) {
        if (strncmp(snapshot.line10, "1", 1) == 0 && snapshot.line10[1] == '\0') {
            lv_obj_clear_flag(s_alert, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(s_alert, LV_OBJ_FLAG_HIDDEN);
        }
    }
#endif
    TouchScreen_lvgl_unlock();
}

// END   --- Internal variables (DRE)




// BEGIN --- Multitasking variables and handlers

#if CONFIG_UIDEMO_USE_THREAD
static TaskHandle_t s_task = NULL;
static volatile bool s_run = false;
static uint32_t s_period_ms =
    #ifdef CONFIG_UIDEMO_PERIOD_MS
      CONFIG_UIDEMO_PERIOD_MS
    #else
      1000
    #endif
;
static SemaphoreHandle_t s_mutex = NULL;

static inline void _lock(void)   { if (s_mutex) xSemaphoreTake(s_mutex, portMAX_DELAY); }
static inline void _unlock(void) { if (s_mutex) xSemaphoreGive(s_mutex); }

#ifdef CONFIG_UIDEMO_MINIMIZE_JITTER
    static TickType_t xLastWakeTime;
    static TickType_t xFrequency;
#endif

static UIDemo_return_code_t UIDemo_spin(void);  // In case we are using a thread, this function should not be part of the public API

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
    #if CONFIG_UIDEMO_PIN_CORE_ANY
        return tskNO_AFFINITY;
    #elif CONFIG_UIDEMO_PIN_CORE_0
        return 0;
    #elif CONFIG_UIDEMO_PIN_CORE_1
        return 1;
    #else
        return tskNO_AFFINITY;
    #endif
}

static void UIDemo_task(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "task started (period=%u ms)", (unsigned)s_period_ms);
#ifdef CONFIG_UIDEMO_MINIMIZE_JITTER
    xLastWakeTime = xTaskGetTickCount();
    xFrequency = (s_period_ms / portTICK_PERIOD_MS);
#endif
    while (s_run) {
        UIDemo_return_code_t ret = UIDemo_spin();
        if (ret != UIDemo_ret_ok)
        {
            ESP_LOGW(TAG, "Error in spin");
        }
#ifdef CONFIG_UIDEMO_MINIMIZE_JITTER
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
#else
        vTaskDelay(pdMS_TO_TICKS(s_period_ms));
#endif
    }
    ESP_LOGI(TAG, "task exit");
    vTaskDelete(NULL);
}

#endif // CONFIG_UIDEMO_USE_THREAD

// END   --- Multitasking variables and handlers

// BEGIN ------------------ Public API (MULTITASKING)------------------


#if CONFIG_UIDEMO_USE_THREAD

UIDemo_return_code_t UIDemo_start(void)
{
    if (_create_mutex_once() != pdPASS) {
        ESP_LOGE(TAG, "mutex creation failed");
        return UIDemo_ret_error;
    }
    if (s_task) {
        // idempotente
        return UIDemo_ret_ok;
    }
    s_run = true;

    BaseType_t core = _get_core_affinity();
    BaseType_t ok = xTaskCreatePinnedToCore(
        UIDemo_task,
        "UIDemo",
        CONFIG_UIDEMO_TASK_STACK,
        NULL,
        CONFIG_UIDEMO_TASK_PRIO,
        &s_task,
        core
    );
    if (ok != pdPASS) {
        s_task = NULL;
        s_run = false;
        ESP_LOGE(TAG, "xTaskCreatePinnedToCore failed");
        return UIDemo_ret_error;
    }
    return UIDemo_ret_ok;
}

UIDemo_return_code_t UIDemo_stop(void)
{
    if (!s_task) return UIDemo_ret_ok; // idempotente
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
    return UIDemo_ret_ok;
}

UIDemo_return_code_t UIDemo_get_dre_clone(UIDemo_dre_t *dst)
{
    if (!dst) return UIDemo_ret_error;
    _lock();
    memcpy(dst, &UIDemo_dre, sizeof(UIDemo_dre));
    _unlock();
    return UIDemo_ret_ok;
}

UIDemo_return_code_t UIDemo_set_period_ms(uint32_t period_ms)
{
    if (period_ms < 10) period_ms = 10;
    _lock();
    s_period_ms = period_ms;
#ifdef CONFIG_UIDEMO_MINIMIZE_JITTER    
    xFrequency = (s_period_ms / portTICK_PERIOD_MS);
#endif
    _unlock();
    ESP_LOGI(TAG, "period set to %u ms", (unsigned)period_ms);
    return UIDemo_ret_ok;
}

uint32_t UIDemo_get_period_ms(void)
{
    _lock();
    uint32_t v = s_period_ms;
    _unlock();
    return v;
}

/**
 *  Execute a function wrapped with locks so you can access the DRE variables in thread-safe mode
*/
void UIDemo_execute_function_safemode(void (*callback)())
{
    _lock();
    callback();
    _unlock();
}

#endif // CONFIG_UIDEMO_USE_THREAD

// END   ------------------ Public API (MULTITASKING)------------------

// BEGIN ------------------ Public API (COMMON + SPIN)------------------
static void UIDemo_ensure_ui(void)
{
    if (s_ui_ready) return;
    if (!TouchScreen_lvgl_ready()) return;
    if (!TouchScreen_lvgl_lock(1000)) return;

    s_screen = lv_obj_create(NULL);
    lv_obj_set_size(s_screen, LV_HOR_RES, LV_VER_RES);
    lv_obj_clear_flag(s_screen, LV_OBJ_FLAG_SCROLLABLE);

#ifdef UIDEMO_FIRST_DEMO
    // Labels container
    lv_obj_t *label_cont = lv_obj_create(s_screen);
    lv_obj_set_size(label_cont, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(label_cont, 0, 0);
    lv_obj_set_style_bg_opa(label_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_flow(label_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(label_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    for (size_t i = 0; i < TOUCHSCREEN_MAX_LINES; ++i) {
        s_labels[i] = lv_label_create(label_cont);
        lv_obj_set_width(s_labels[i], LV_PCT(100));
        lv_label_set_long_mode(s_labels[i], LV_LABEL_LONG_WRAP);
        lv_label_set_text(s_labels[i], s_prev_lines[i]);
    }

    // Controls container (floating top-right)
    s_controls = lv_obj_create(s_screen);
    lv_obj_set_size(s_controls, LV_PCT(40), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(s_controls, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(s_controls, 4, 0);
    lv_obj_clear_flag(s_controls, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(s_controls, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(s_controls, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(s_controls, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_START);
    lv_obj_align(s_controls, LV_ALIGN_TOP_RIGHT, -8, 8);

    s_button = lv_btn_create(s_controls);
    lv_obj_add_event_cb(s_button, btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn_label = lv_label_create(s_button);
    lv_label_set_text(btn_label, "BTN");
    lv_obj_center(btn_label);

    // Slider
    s_slider = lv_slider_create(s_controls);
    lv_slider_set_range(s_slider, 0, 100);
    lv_obj_set_width(s_slider, LV_PCT(100));
    lv_obj_add_event_cb(s_slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_slider_set_value(s_slider, 0, LV_ANIM_OFF);

    // Chart
    s_chart = lv_chart_create(s_screen);
    lv_obj_set_width(s_chart, LV_PCT(95));
    lv_obj_set_height(s_chart, LV_PCT(30));
    lv_obj_align(s_chart, LV_ALIGN_BOTTOM_MID, 0, -8);
    lv_chart_set_type(s_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(s_chart, 60);
    lv_chart_set_range(s_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    s_chart_series = lv_chart_add_series(s_chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);
    for (uint16_t i = 0; i < lv_chart_get_point_count(s_chart); ++i) {
        lv_chart_set_value_by_id(s_chart, s_chart_series, i, LV_CHART_POINT_NONE);
    }
    s_chart_timer = lv_timer_create(chart_timer_cb, 1000, NULL);

    // Alert indicator (placed after controls so it's on top)
    s_alert = lv_obj_create(s_screen);
    lv_obj_set_size(s_alert, 24, 24);
    lv_obj_set_style_bg_color(s_alert, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_set_style_radius(s_alert, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_color(s_alert, lv_palette_darken(LV_PALETTE_RED, 3), 0);
    lv_obj_set_style_border_width(s_alert, 2, 0);
    lv_obj_align(s_alert, LV_ALIGN_TOP_RIGHT, -4, 4);
    lv_obj_add_flag(s_alert, LV_OBJ_FLAG_HIDDEN);

#else
    lv_style_init(&style_header_bg);
    lv_style_set_bg_color(&style_header_bg, lv_color_hex(0x033875));
    lv_style_set_bg_opa(&style_header_bg, LV_OPA_COVER);

    lv_style_init(&style_btn_primary);
    lv_style_set_bg_color(&style_btn_primary, lv_color_hex(0x437dce));
    lv_style_set_bg_opa(&style_btn_primary, LV_OPA_COVER);
    lv_style_set_radius(&style_btn_primary, 8);
    lv_style_set_pad_all(&style_btn_primary, 10);

    lv_style_init(&style_text_body);
    lv_style_set_text_color(&style_text_body, lv_color_hex(0x170632));

    lv_style_init(&style_text_invert);
    lv_style_set_text_color(&style_text_invert, lv_color_hex(0xFFFFFF));

    // Header
    lv_obj_t *hdr = lv_obj_create(s_screen);
    lv_obj_remove_style_all(hdr);
    lv_obj_add_style(hdr, &style_header_bg, 0);
    lv_obj_set_size(hdr, 800, 70);
    lv_obj_set_pos(hdr, 0, 0);

    // Header title
    lv_obj_t *title = lv_label_create(hdr);
    lv_label_set_text(title, "Control principal");
    lv_obj_add_style(title, &style_text_invert, 0);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 20, 0);

    // Body status
    lv_obj_t *st = lv_label_create(s_screen);
    lv_label_set_text(st, "Estado: OK");
    lv_obj_add_style(st, &style_text_body, 0);
    lv_obj_align(st, LV_ALIGN_CENTER, 0, -20);

    // Primary button
    lv_obj_t *btn = lv_btn_create(s_screen);
    lv_obj_remove_style_all(btn);
    lv_obj_add_style(btn, &style_btn_primary, 0);
    lv_obj_set_size(btn, 220, 64);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 80);

    lv_obj_t *btn2_label = lv_label_create(btn);
    lv_label_set_text(btn2_label, "START");
    lv_obj_add_style(btn2_label, &style_text_invert, 0);
    lv_obj_center(btn2_label);
#endif

    lv_scr_load(s_screen);
    s_ui_ready = true;
    TouchScreen_lvgl_unlock();

    touchscreen_update();
}

UIDemo_return_code_t UIDemo_setup(void)
{
    // Init liviano; no arranca tarea.
    ESP_LOGD(TAG, "setup()");
    // Loading values from NVS
    UIDemo_netvars_nvs_load();    
#if CONFIG_UIDEMO_USE_THREAD
    if (_create_mutex_once() != pdPASS) {
        ESP_LOGE(TAG, "mutex creation failed");
        return UIDemo_ret_error;
    }
#endif

    UIDemo_ensure_ui();

    UIDemo_dre.last_return_code = UIDemo_ret_ok;
    return UIDemo_ret_ok;
}

#if CONFIG_UIDEMO_USE_THREAD
static  // In case we are using a thread, this function should not be part of the public API
#endif
UIDemo_return_code_t UIDemo_spin(void)
{
    UIDemo_ensure_ui();

#if CONFIG_UIDEMO_USE_THREAD
    _lock();
#endif
    bool en = UIDemo_dre.enabled;
#if CONFIG_UIDEMO_USE_THREAD
    _unlock();
#endif

    if (!en)
    {
#if CONFIG_UIDEMO_USE_THREAD        
        _unlock();
#endif
        return UIDemo_ret_ok;
    }
    else
    {
        // Implement your spin here
        // this area is protected, so concentrate here
        // the stuff which needs protection against
        // concurrency issues

        ESP_LOGI(TAG, "Doing protected stuff %d", UIDemo_dre.enabled);
        //vTaskDelay(pdMS_TO_TICKS(120));

#if CONFIG_UIDEMO_USE_THREAD
        // Unlocking after the protected data has been managed for this cycle
        _unlock();
#endif
        UIDemo_nvs_spin();

        // Communicate results, do stuff which 
        // does not need protection
        // ...
        ESP_LOGI(TAG, "Hello world!");
        return UIDemo_ret_ok;
    }
}

UIDemo_return_code_t UIDemo_enable(void)
{
#if CONFIG_UIDEMO_USE_THREAD
    _lock();
#endif
    UIDemo_dre.enabled = true;
    UIDemo_dre.last_return_code = UIDemo_ret_ok;
#if CONFIG_UIDEMO_USE_THREAD
    _unlock();
#endif
    return UIDemo_ret_ok;
}

UIDemo_return_code_t UIDemo_disable(void)
{
#if CONFIG_UIDEMO_USE_THREAD
    _lock();
#endif
    UIDemo_dre.enabled = false;
    UIDemo_dre.last_return_code = UIDemo_ret_ok;
#if CONFIG_UIDEMO_USE_THREAD
    _unlock();
#endif
    return UIDemo_ret_ok;
}

// BEGIN ------------------ Public API (COMMON)------------------
