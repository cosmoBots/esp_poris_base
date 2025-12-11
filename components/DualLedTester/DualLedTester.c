// BEGIN --- Standard C headers section ---
#include <stdio.h>
#include <string.h>
#include <stddef.h>

// END   --- Standard C headers section ---

// BEGIN --- SDK config section---
#include <sdkconfig.h>
// END   --- SDK config section---

// BEGIN --- FreeRTOS headers section ---
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#if CONFIG_DUALLEDTESTER_USE_THREAD
  #include <freertos/semphr.h>
#endif

// END   --- FreeRTOS headers section ---


// BEGIN --- ESP-IDF headers section ---
#include <esp_log.h>

// END   --- ESP-IDF headers section ---

// BEGIN --- Project configuration section ---
#include <PrjCfg.h> // Including project configuration module 
// END   --- Project configuration section ---

// BEGIN --- Project configuration section ---

// end   --- Project configuration section ---

// BEGIN --- Self-includes section ---
#include "DualLedTester.h"
#include "DualLedTester_netvars.h"
#include "DualLED.h"

// END --- Self-includes section ---

// BEGIN --- Logging related variables
static const char *TAG = "DualLedTester";
// END --- Logging related variables

#if CONFIG_DUALLEDTESTER_VERBOSE_MODE
static const char *k_dualled_state_names[] = {
    "OFF",
    "GREEN",
    "RED",
    "BLINK_GREEN",
    "BLINK_RED",
    "ALTERNATE_START_GREEN",
    "ALTERNATE_START_RED",
#if CONFIG_DUALLED_ALLOW_BOTH
    "BOTH_COLORS",
    "BLINK_BOTH",
#endif
};

static const char *DualLedTester_state_name_safe(dual_led_state_t st)
{
    size_t idx = (size_t)st;
    if (idx < (sizeof(k_dualled_state_names) / sizeof(k_dualled_state_names[0])))
    {
        return k_dualled_state_names[idx];
    }
    return "UNKNOWN";
}
#endif

// BEGIN --- Internal variables (DRE)
DualLedTester_dre_t DualLedTester_dre = {
    .enabled = true,
    .dwell_ms = 10000,
    .duty_on_ms = 500,
    .duty_off_ms = 500,
    .seq_index = 0,
    .last_change = 0,
    .last_return_code = DualLedTester_ret_ok
};
// END   --- Internal variables (DRE)



// BEGIN --- Multitasking variables and handlers

#if CONFIG_DUALLEDTESTER_USE_THREAD
static TaskHandle_t s_task = NULL;
static volatile bool s_run = false;
static uint32_t s_period_ms =
    #ifdef CONFIG_DUALLEDTESTER_PERIOD_MS
      CONFIG_DUALLEDTESTER_PERIOD_MS
    #else
      1000
    #endif
;
static SemaphoreHandle_t s_mutex = NULL;

static inline void _lock(void)   { if (s_mutex) xSemaphoreTake(s_mutex, portMAX_DELAY); }
static inline void _unlock(void) { if (s_mutex) xSemaphoreGive(s_mutex); }

#ifdef CONFIG_DUALLEDTESTER_MINIMIZE_JITTER
    static TickType_t xLastWakeTime;
    static TickType_t xFrequency;
#endif

static DualLedTester_return_code_t DualLedTester_spin(void);  // In case we are using a thread, this function should not be part of the public API

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
    #if CONFIG_DUALLEDTESTER_PIN_CORE_ANY
        return tskNO_AFFINITY;
    #elif CONFIG_DUALLEDTESTER_PIN_CORE_0
        return 0;
    #elif CONFIG_DUALLEDTESTER_PIN_CORE_1
        return 1;
    #else
        return tskNO_AFFINITY;
    #endif
}

static void DualLedTester_task(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "task started (period=%u ms)", (unsigned)s_period_ms);
#ifdef CONFIG_DUALLEDTESTER_MINIMIZE_JITTER
    xLastWakeTime = xTaskGetTickCount();
    xFrequency = (s_period_ms / portTICK_PERIOD_MS);
#endif
    while (s_run) {
        DualLedTester_return_code_t ret = DualLedTester_spin();
        if (ret != DualLedTester_ret_ok)
        {
            ESP_LOGW(TAG, "Error in spin");
        }
#ifdef CONFIG_DUALLEDTESTER_MINIMIZE_JITTER
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
#else
        vTaskDelay(pdMS_TO_TICKS(s_period_ms));
#endif
    }
    ESP_LOGI(TAG, "task exit");
    vTaskDelete(NULL);
}

#endif // CONFIG_DUALLEDTESTER_USE_THREAD

// END   --- Multitasking variables and handlers

// BEGIN ------------------ Public API (MULTITASKING)------------------


#if CONFIG_DUALLEDTESTER_USE_THREAD

DualLedTester_return_code_t DualLedTester_start(void)
{
    if (_create_mutex_once() != pdPASS) {
        ESP_LOGE(TAG, "mutex creation failed");
        return DualLedTester_ret_error;
    }
    if (s_task) {
        // idempotente
        return DualLedTester_ret_ok;
    }
    s_run = true;

    BaseType_t core = _get_core_affinity();
    BaseType_t ok = xTaskCreatePinnedToCore(
        DualLedTester_task,
        "DualLedTester",
        CONFIG_DUALLEDTESTER_TASK_STACK,
        NULL,
        CONFIG_DUALLEDTESTER_TASK_PRIO,
        &s_task,
        core
    );
    if (ok != pdPASS) {
        s_task = NULL;
        s_run = false;
        ESP_LOGE(TAG, "xTaskCreatePinnedToCore failed");
        return DualLedTester_ret_error;
    }
    return DualLedTester_ret_ok;
}

DualLedTester_return_code_t DualLedTester_stop(void)
{
    if (!s_task) return DualLedTester_ret_ok; // idempotente
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
    return DualLedTester_ret_ok;
}

DualLedTester_return_code_t DualLedTester_get_dre_clone(DualLedTester_dre_t *dst)
{
    if (!dst) return DualLedTester_ret_error;
    _lock();
    memcpy(dst, &DualLedTester_dre, sizeof(DualLedTester_dre));
    _unlock();
    return DualLedTester_ret_ok;
}

DualLedTester_return_code_t DualLedTester_set_period_ms(uint32_t period_ms)
{
    if (period_ms < 10) period_ms = 10;
    _lock();
    s_period_ms = period_ms;
#ifdef CONFIG_DUALLEDTESTER_MINIMIZE_JITTER    
    xFrequency = (s_period_ms / portTICK_PERIOD_MS);
#endif
    _unlock();
    ESP_LOGI(TAG, "period set to %u ms", (unsigned)period_ms);
    return DualLedTester_ret_ok;
}

uint32_t DualLedTester_get_period_ms(void)
{
    _lock();
    uint32_t v = s_period_ms;
    _unlock();
    return v;
}

/**
 *  Execute a function wrapped with locks so you can access the DRE variables in thread-safe mode
*/
void DualLedTester_execute_function_safemode(void (*callback)())
{
    _lock();
    callback();
    _unlock();
}

#endif // CONFIG_DUALLEDTESTER_USE_THREAD

// END   ------------------ Public API (MULTITASKING)------------------

// BEGIN ------------------ Public API (COMMON + SPIN)------------------

static const dual_led_state_t k_sequence[] = {
    DUALLED_RED,
    DUALLED_GREEN,
#if CONFIG_DUALLED_ALLOW_BOTH
    DUALLED_BOTH_COLORS,
#endif
    DUALLED_BLINK_RED,
    DUALLED_BLINK_GREEN,
#if CONFIG_DUALLED_ALLOW_BOTH
    DUALLED_BLINK_BOTH,
#endif
    DUALLED_OFF,                    // spacer before alternates
    DUALLED_ALTERNATE_START_GREEN,
    DUALLED_OFF,                    // spacer between alternates
    DUALLED_ALTERNATE_START_RED,
};

DualLedTester_return_code_t DualLedTester_setup(void)
{
    // Init liviano; no arranca tarea.
    ESP_LOGD(TAG, "setup()");
#if CONFIG_DUALLEDTESTER_USE_THREAD
    if (_create_mutex_once() != pdPASS) {
        ESP_LOGE(TAG, "mutex creation failed");
        return DualLedTester_ret_error;
    }
#endif
    DualLedTester_dre.dwell_ms = 10000;   // 10s per state
    DualLedTester_dre.duty_on_ms = 500;
    DualLedTester_dre.duty_off_ms = 500;
    DualLedTester_dre.seq_index = 0;
    DualLedTester_dre.last_change = xTaskGetTickCount();
    DualLED_set_duty(DualLedTester_dre.duty_on_ms, DualLedTester_dre.duty_off_ms);
    DualLED_set_state(k_sequence[DualLedTester_dre.seq_index]);
    DualLedTester_dre.last_return_code = DualLedTester_ret_ok;
    return DualLedTester_ret_ok;
}

#if CONFIG_DUALLEDTESTER_USE_THREAD
static  // In case we are using a thread, this function should not be part of the public API
#endif
DualLedTester_return_code_t DualLedTester_spin(void)
{
#if CONFIG_DUALLEDTESTER_USE_THREAD
    _lock();
#endif
    bool en = DualLedTester_dre.enabled;

    if (!en)
    {
#if CONFIG_DUALLEDTESTER_USE_THREAD
        _unlock();
#endif
        return DualLedTester_ret_ok;
    }
    else
    {

        const size_t nseq = sizeof(k_sequence) / sizeof(k_sequence[0]);

        TickType_t now = xTaskGetTickCount();
        TickType_t elapsed = now - DualLedTester_dre.last_change;
        if (elapsed >= pdMS_TO_TICKS(DualLedTester_dre.dwell_ms))
        {
            DualLedTester_dre.seq_index = (DualLedTester_dre.seq_index + 1) % nseq;
            dual_led_state_t next = k_sequence[DualLedTester_dre.seq_index];
            DualLedTester_dre.last_change = now;
            DualLED_set_state(next);
#if CONFIG_DUALLEDTESTER_VERBOSE_MODE
            ESP_LOGI(TAG, "DualLED state -> %d (%s)", (int)next, DualLedTester_state_name_safe(next));
#endif
        }

#if CONFIG_DUALLEDTESTER_USE_THREAD
        // Unlocking after the protected data has been managed for this cycle
        _unlock();
#endif

        DualLedTester_nvs_spin();
        return DualLedTester_ret_ok;
    }
}

DualLedTester_return_code_t DualLedTester_enable(void)
{
#if CONFIG_DUALLEDTESTER_USE_THREAD
    _lock();
#endif
    DualLedTester_dre.enabled = true;
    DualLedTester_dre.last_return_code = DualLedTester_ret_ok;
#if CONFIG_DUALLEDTESTER_USE_THREAD
    _unlock();
#endif
    return DualLedTester_ret_ok;
}

DualLedTester_return_code_t DualLedTester_disable(void)
{
#if CONFIG_DUALLEDTESTER_USE_THREAD
    _lock();
#endif
    DualLedTester_dre.enabled = false;
    DualLedTester_dre.last_return_code = DualLedTester_ret_ok;
#if CONFIG_DUALLEDTESTER_USE_THREAD
    _unlock();
#endif
    return DualLedTester_ret_ok;
}

// BEGIN ------------------ Public API (COMMON)------------------
