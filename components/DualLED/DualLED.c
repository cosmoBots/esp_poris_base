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
#if CONFIG_DUALLED_USE_THREAD
  #include <freertos/semphr.h>
#endif

// END   --- FreeRTOS headers section ---


// BEGIN --- ESP-IDF headers section ---
#include <esp_log.h>
#include <driver/gpio.h>

// END   --- ESP-IDF headers section ---

// BEGIN --- Project configuration section ---
#include <PrjCfg.h> // Including project configuration module 
// END   --- Project configuration section ---

// BEGIN --- Project configuration section ---

// end   --- Project configuration section ---

// BEGIN --- Self-includes section ---
#include "DualLED.h"
#include "DualLED_netvars.h"

// END --- Self-includes section ---

// BEGIN --- Logging related variables
static const char *TAG = "DualLED";
// END --- Logging related variables

// BEGIN --- Internal variables (DRE)
DualLED_dre_t DualLED_dre = {
    .enabled = true,
    .prev_state = DUALLED_OFF,
    .last_toggle = 0,
    .hw_init = false,
    .last_return_code = DualLED_ret_ok,
    .state = DUALLED_OFF,
    .phase_on = true,
    .on_ms = 500,
    .off_ms = 500
};
// END   --- Internal variables (DRE)

// Netvars dirty tracking
static bool s_nvs_dirty = false;
static TickType_t s_nvs_dirty_since = 0;

static inline void DualLED_apply_outputs(bool red_on, bool green_on)
{
    gpio_set_level(CONFIG_DUALLED_RED_GPIO, red_on ? 1 : 0);
    gpio_set_level(CONFIG_DUALLED_GREEN_GPIO, green_on ? 1 : 0);
}

static void DualLED_hw_init(void)
{
    if (DualLED_dre.hw_init) return;
    DualLED_dre.state = DUALLED_OFF;
    DualLED_dre.prev_state = DUALLED_OFF;
    DualLED_dre.phase_on = true;
    DualLED_dre.on_ms = 500;
    DualLED_dre.off_ms = 500;
    DualLED_dre.last_toggle = xTaskGetTickCount();

    gpio_reset_pin(CONFIG_DUALLED_RED_GPIO);
    gpio_reset_pin(CONFIG_DUALLED_GREEN_GPIO);
    gpio_set_direction(CONFIG_DUALLED_RED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(CONFIG_DUALLED_GREEN_GPIO, GPIO_MODE_OUTPUT);

    DualLED_apply_outputs(false, false);
    DualLED_dre.hw_init = true;
}

static void DualLED_update_phase(void)
{
    TickType_t now = xTaskGetTickCount();
    uint32_t current_ms = DualLED_dre.phase_on ? DualLED_dre.on_ms : DualLED_dre.off_ms;
    if ((TickType_t)(now - DualLED_dre.last_toggle) >= pdMS_TO_TICKS(current_ms))
    {
        DualLED_dre.phase_on = !DualLED_dre.phase_on;
        DualLED_dre.last_toggle = now;
    }
}

static void DualLED_render_state(void)
{
    switch (DualLED_dre.state)
    {
    case DUALLED_OFF:
        DualLED_apply_outputs(false, false);
        break;
    case DUALLED_GREEN:
        DualLED_apply_outputs(false, true);
        break;
    case DUALLED_RED:
        DualLED_apply_outputs(true, false);
        break;
    case DUALLED_BLINK_GREEN:
        DualLED_update_phase();
        DualLED_apply_outputs(false, DualLED_dre.phase_on);
        break;
    case DUALLED_BLINK_RED:
        DualLED_update_phase();
        DualLED_apply_outputs(DualLED_dre.phase_on, false);
        break;
    case DUALLED_ALTERNATE_START_GREEN:
        DualLED_update_phase();
        if (DualLED_dre.phase_on)
            DualLED_apply_outputs(false, true);
        else
            DualLED_apply_outputs(true, false);
        break;
    case DUALLED_ALTERNATE_START_RED:
        DualLED_update_phase();
        if (DualLED_dre.phase_on)
            DualLED_apply_outputs(true, false);
        else
            DualLED_apply_outputs(false, true);
        break;
#if CONFIG_DUALLED_ALLOW_BOTH
    case DUALLED_BOTH_COLORS:
        DualLED_apply_outputs(true, true);
        break;
    case DUALLED_BLINK_BOTH:
        DualLED_update_phase();
        DualLED_apply_outputs(DualLED_dre.phase_on, DualLED_dre.phase_on);
        break;
#endif
    default:
        DualLED_apply_outputs(false, false);
        break;
    }
}

void DualLED_set_state(dual_led_state_t newstate)
{
    if (newstate >= DUALLED_STATE_TERMINATOR)
    {
        ESP_LOGE(TAG, "Invalid state %d (out of range)", (int)newstate);
        DualLED_dre.last_return_code = DualLED_ret_error;
        return;
    }
    DualLED_hw_init();
    DualLED_dre.state = newstate;
    DualLED_dre.prev_state = (dual_led_state_t)(-1); // force refresh
    DualLED_dre.phase_on = true;
    DualLED_dre.last_toggle = xTaskGetTickCount();
    DualLED_render_state();
}

void DualLED_set_duty(uint32_t on_duration_ms, uint32_t off_alternate_duration_ms)
{
    DualLED_hw_init();
    if (on_duration_ms == 0) on_duration_ms = 1;
    if (off_alternate_duration_ms == 0) off_alternate_duration_ms = 1;
    DualLED_dre.on_ms = on_duration_ms;
    DualLED_dre.off_ms = off_alternate_duration_ms;
    DualLED_dre.last_toggle = xTaskGetTickCount();
}
// END --- LED state machine

// BEGIN --- Multitasking variables and handlers

#if CONFIG_DUALLED_USE_THREAD
static TaskHandle_t s_task = NULL;
static volatile bool s_run = false;
static uint32_t s_period_ms =
    #ifdef CONFIG_DUALLED_PERIOD_MS
      CONFIG_DUALLED_PERIOD_MS
    #else
      1000
    #endif
;
static SemaphoreHandle_t s_mutex = NULL;

static inline void _lock(void)   { if (s_mutex) xSemaphoreTake(s_mutex, portMAX_DELAY); }
static inline void _unlock(void) { if (s_mutex) xSemaphoreGive(s_mutex); }

#ifdef CONFIG_DUALLED_MINIMIZE_JITTER
    static TickType_t xLastWakeTime;
    static TickType_t xFrequency;
#endif

static DualLED_return_code_t DualLED_spin(void);  // In case we are using a thread, this function should not be part of the public API

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
    #if CONFIG_DUALLED_PIN_CORE_ANY
        return tskNO_AFFINITY;
    #elif CONFIG_DUALLED_PIN_CORE_0
        return 0;
    #elif CONFIG_DUALLED_PIN_CORE_1
        return 1;
    #else
        return tskNO_AFFINITY;
    #endif
}

static void DualLED_task(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "task started (period=%u ms)", (unsigned)s_period_ms);
#ifdef CONFIG_DUALLED_MINIMIZE_JITTER
    xLastWakeTime = xTaskGetTickCount();
    xFrequency = (s_period_ms / portTICK_PERIOD_MS);
#endif
    while (s_run) {
        DualLED_return_code_t ret = DualLED_spin();
        if (ret != DualLED_ret_ok)
        {
            ESP_LOGW(TAG, "Error in spin");
        }
#ifdef CONFIG_DUALLED_MINIMIZE_JITTER
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
#else
        vTaskDelay(pdMS_TO_TICKS(s_period_ms));
#endif
    }
    ESP_LOGI(TAG, "task exit");
    vTaskDelete(NULL);
}

#endif // CONFIG_DUALLED_USE_THREAD

// END   --- Multitasking variables and handlers

// BEGIN ------------------ Public API (MULTITASKING)------------------


#if CONFIG_DUALLED_USE_THREAD

DualLED_return_code_t DualLED_start(void)
{
    if (_create_mutex_once() != pdPASS) {
        ESP_LOGE(TAG, "mutex creation failed");
        return DualLED_ret_error;
    }
    if (s_task) {
        // idempotente
        return DualLED_ret_ok;
    }
    s_run = true;

    BaseType_t core = _get_core_affinity();
    BaseType_t ok = xTaskCreatePinnedToCore(
        DualLED_task,
        "DualLED",
        CONFIG_DUALLED_TASK_STACK,
        NULL,
        CONFIG_DUALLED_TASK_PRIO,
        &s_task,
        core
    );
    if (ok != pdPASS) {
        s_task = NULL;
        s_run = false;
        ESP_LOGE(TAG, "xTaskCreatePinnedToCore failed");
        return DualLED_ret_error;
    }
    return DualLED_ret_ok;
}

DualLED_return_code_t DualLED_stop(void)
{
    if (!s_task) return DualLED_ret_ok; // idempotente
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
    return DualLED_ret_ok;
}

DualLED_return_code_t DualLED_get_dre_clone(DualLED_dre_t *dst)
{
    if (!dst) return DualLED_ret_error;
    _lock();
    *dst = DualLED_dre;
    _unlock();
    return DualLED_ret_ok;
}

DualLED_return_code_t DualLED_set_period_ms(uint32_t period_ms)
{
    if (period_ms < 10) period_ms = 10;
    _lock();
    s_period_ms = period_ms;
#ifdef CONFIG_DUALLED_MINIMIZE_JITTER    
    xFrequency = (s_period_ms / portTICK_PERIOD_MS);
#endif
    _unlock();
    ESP_LOGI(TAG, "period set to %u ms", (unsigned)period_ms);
    return DualLED_ret_ok;
}

uint32_t DualLED_get_period_ms(void)
{
    _lock();
    uint32_t v = s_period_ms;
    _unlock();
    return v;
}

/**
 *  Execute a function wrapped with locks so you can access the DRE variables in thread-safe mode
*/
void DualLED_execute_function_safemode(void (*callback)())
{
    _lock();
    callback();
    _unlock();
}

#endif // CONFIG_DUALLED_USE_THREAD

// END   ------------------ Public API (MULTITASKING)------------------

// BEGIN ------------------ Public API (COMMON + SPIN)------------------

DualLED_return_code_t DualLED_setup(void)
{
    // Init liviano; no arranca tarea.
    ESP_LOGD(TAG, "setup()");
#if CONFIG_DUALLED_USE_THREAD
    if (_create_mutex_once() != pdPASS) {
        ESP_LOGE(TAG, "mutex creation failed");
        return DualLED_ret_error;
    }
#endif
    DualLED_hw_init();
    DualLED_dre.last_return_code = DualLED_ret_ok;
    return DualLED_ret_ok;
}

#if CONFIG_DUALLED_USE_THREAD
static  // In case we are using a thread, this function should not be part of the public API
#endif
DualLED_return_code_t DualLED_spin(void)
{
#if CONFIG_DUALLED_USE_THREAD
    _lock();
#endif
    bool en = DualLED_dre.enabled;

    if (!en)
    {
        // keep outputs as-is, just exit
#if CONFIG_DUALLED_USE_THREAD
        _unlock();
#endif
        return DualLED_ret_ok;
    }
    else
    {
        if (DualLED_dre.state != DualLED_dre.prev_state)
        {
            DualLED_dre.prev_state = DualLED_dre.state;
            DualLED_dre.phase_on = true;
            DualLED_dre.last_toggle = xTaskGetTickCount();
        }
        DualLED_render_state();
        TickType_t now_ticks = xTaskGetTickCount();
        if (s_nvs_dirty &&
            (TickType_t)(now_ticks - s_nvs_dirty_since) >= pdMS_TO_TICKS(5000))
        {
            s_nvs_dirty = false;
#if CONFIG_DUALLED_USE_THREAD
            _unlock();
#endif
            DualLED_netvars_nvs_save();
        }
        else
        {
#if CONFIG_DUALLED_USE_THREAD
            _unlock();
#endif
        }

        return DualLED_ret_ok;
    }
}

DualLED_return_code_t DualLED_enable(void)
{
#if CONFIG_DUALLED_USE_THREAD
    _lock();
#endif
    DualLED_dre.enabled = true;
    DualLED_dre.last_return_code = DualLED_ret_ok;
#if CONFIG_DUALLED_USE_THREAD
    _unlock();
#endif
    return DualLED_ret_ok;
}

DualLED_return_code_t DualLED_disable(void)
{
#if CONFIG_DUALLED_USE_THREAD
    _lock();
#endif
    DualLED_dre.enabled = false;
    DualLED_dre.last_return_code = DualLED_ret_ok;
#if CONFIG_DUALLED_USE_THREAD
    _unlock();
#endif
    return DualLED_ret_ok;
}

// BEGIN ------------------ Public API (COMMON)------------------
