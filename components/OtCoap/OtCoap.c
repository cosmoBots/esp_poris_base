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
#if CONFIG_OTCOAP_USE_THREAD
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
#include "OtCoap.h"

// END --- Self-includes section ---

// BEGIN --- Logging related variables
static const char *TAG = "OtCoap";
// END --- Logging related variables

// BEGIN --- Internal variables (DRE)
OtCoap_dre_t OtCoap_dre = {
    .enabled = true,
    .last_return_code = OtCoap_ret_ok
};
// END   --- Internal variables (DRE)

// BEGIN --- Multitasking variables and handlers

#if CONFIG_OTCOAP_USE_THREAD
static TaskHandle_t s_task = NULL;
static volatile bool s_run = false;
static uint32_t s_period_ms =
    #ifdef CONFIG_OTCOAP_PERIOD_MS
      CONFIG_OTCOAP_PERIOD_MS
    #else
      1000
    #endif
;
static SemaphoreHandle_t s_mutex = NULL;

static inline void _lock(void)   { if (s_mutex) xSemaphoreTake(s_mutex, portMAX_DELAY); }
static inline void _unlock(void) { if (s_mutex) xSemaphoreGive(s_mutex); }

#ifdef CONFIG_OTCOAP_MINIMIZE_JITTER
    static TickType_t xLastWakeTime;
    static TickType_t xFrequency;
#endif

static OtCoap_return_code_t OtCoap_spin(void);  // In case we are using a thread, this function should not be part of the public API

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
    #if CONFIG_OTCOAP_PIN_CORE_ANY
        return tskNO_AFFINITY;
    #elif CONFIG_OTCOAP_PIN_CORE_0
        return 0;
    #elif CONFIG_OTCOAP_PIN_CORE_1
        return 1;
    #else
        return tskNO_AFFINITY;
    #endif
}

static void OtCoap_task(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "task started (period=%u ms)", (unsigned)s_period_ms);
#ifdef CONFIG_OTCOAP_MINIMIZE_JITTER
    xLastWakeTime = xTaskGetTickCount();
    xFrequency = (s_period_ms / portTICK_PERIOD_MS);
#endif
    while (s_run) {
        OtCoap_return_code_t ret = OtCoap_spin();
        if (ret != OtCoap_ret_ok)
        {
            ESP_LOGW(TAG, "Error in spin");
        }
#ifdef CONFIG_OTCOAP_MINIMIZE_JITTER
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
#else
        vTaskDelay(pdMS_TO_TICKS(s_period_ms));
#endif
    }
    ESP_LOGI(TAG, "task exit");
    vTaskDelete(NULL);
}

#endif // CONFIG_OTCOAP_USE_THREAD

// END   --- Multitasking variables and handlers

// BEGIN ------------------ Public API (MULTITASKING)------------------


#if CONFIG_OTCOAP_USE_THREAD

OtCoap_return_code_t OtCoap_start(void)
{
    if (_create_mutex_once() != pdPASS) {
        ESP_LOGE(TAG, "mutex creation failed");
        return OtCoap_ret_error;
    }
    if (s_task) {
        // idempotente
        return OtCoap_ret_ok;
    }
    s_run = true;

    BaseType_t core = _get_core_affinity();
    BaseType_t ok = xTaskCreatePinnedToCore(
        OtCoap_task,
        "OtCoap",
        CONFIG_OTCOAP_TASK_STACK,
        NULL,
        CONFIG_OTCOAP_TASK_PRIO,
        &s_task,
        core
    );
    if (ok != pdPASS) {
        s_task = NULL;
        s_run = false;
        ESP_LOGE(TAG, "xTaskCreatePinnedToCore failed");
        return OtCoap_ret_error;
    }
    return OtCoap_ret_ok;
}

OtCoap_return_code_t OtCoap_stop(void)
{
    if (!s_task) return OtCoap_ret_ok; // idempotente
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
    return OtCoap_ret_ok;
}

OtCoap_return_code_t OtCoap_get_dre_clone(OtCoap_dre_t *dst)
{
    if (!dst) return OtCoap_ret_error;
    _lock();
    *dst = OtCoap_dre;
    _unlock();
    return OtCoap_ret_ok;
}

OtCoap_return_code_t OtCoap_set_period_ms(uint32_t period_ms)
{
    if (period_ms < 10) period_ms = 10;
    _lock();
    s_period_ms = period_ms;
#ifdef CONFIG_OTCOAP_MINIMIZE_JITTER    
    xFrequency = (s_period_ms / portTICK_PERIOD_MS);
#endif
    _unlock();
    ESP_LOGI(TAG, "period set to %u ms", (unsigned)period_ms);
    return OtCoap_ret_ok;
}

uint32_t OtCoap_get_period_ms(void)
{
    _lock();
    uint32_t v = s_period_ms;
    _unlock();
    return v;
}

/**
 *  Execute a function wrapped with locks so you can access the DRE variables in thread-safe mode
*/
void OtCoap_execute_function_safemode(void (*callback)())
{
    _lock();
    callback();
    _unlock();
}

#endif // CONFIG_OTCOAP_USE_THREAD

// END   ------------------ Public API (MULTITASKING)------------------

// BEGIN ------------------ Public API (COMMON + SPIN)------------------

OtCoap_return_code_t OtCoap_setup(void)
{
    // Init liviano; no arranca tarea.
    ESP_LOGD(TAG, "setup()");
#if CONFIG_OTCOAP_USE_THREAD
    if (_create_mutex_once() != pdPASS) {
        ESP_LOGE(TAG, "mutex creation failed");
        return OtCoap_ret_error;
    }
#endif
    OtCoap_dre.last_return_code = OtCoap_ret_ok;
    return OtCoap_ret_ok;
}

#if CONFIG_OTCOAP_USE_THREAD
static  // In case we are using a thread, this function should not be part of the public API
#endif
OtCoap_return_code_t OtCoap_spin(void)
{
#if CONFIG_OTCOAP_USE_THREAD
    _lock();
#endif
    bool en = OtCoap_dre.enabled;
#if CONFIG_OTCOAP_USE_THREAD
    _unlock();
#endif
    if (!en) return OtCoap_ret_ok;

    ESP_LOGI(TAG, "Hello world!");
    vTaskDelay(pdMS_TO_TICKS(120));
    return OtCoap_ret_ok;
}

OtCoap_return_code_t OtCoap_enable(void)
{
#if CONFIG_OTCOAP_USE_THREAD
    _lock();
#endif
    OtCoap_dre.enabled = true;
    OtCoap_dre.last_return_code = OtCoap_ret_ok;
#if CONFIG_OTCOAP_USE_THREAD
    _unlock();
#endif
    return OtCoap_ret_ok;
}

OtCoap_return_code_t OtCoap_disable(void)
{
#if CONFIG_OTCOAP_USE_THREAD
    _lock();
#endif
    OtCoap_dre.enabled = false;
    OtCoap_dre.last_return_code = OtCoap_ret_ok;
#if CONFIG_OTCOAP_USE_THREAD
    _unlock();
#endif
    return OtCoap_ret_ok;
}

// BEGIN ------------------ Public API (COMMON)------------------
