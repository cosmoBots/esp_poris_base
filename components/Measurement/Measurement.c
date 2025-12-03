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
#if CONFIG_MEASUREMENT_USE_THREAD
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
#include "Measurement.h"

// END --- Self-includes section ---

// BEGIN --- Logging related variables
static const char *TAG = "Measurement";
// END --- Logging related variables

// BEGIN --- Internal variables (DRE)
Measurement_dre_t Measurement_dre = {
    .enabled = true,
    .last_return_code = Measurement_ret_ok
};
// END   --- Internal variables (DRE)

// BEGIN --- Multitasking variables and handlers

#if CONFIG_MEASUREMENT_USE_THREAD
static TaskHandle_t s_task = NULL;
static volatile bool s_run = false;
static uint32_t s_period_ms =
    #ifdef CONFIG_MEASUREMENT_PERIOD_MS
      CONFIG_MEASUREMENT_PERIOD_MS
    #else
      1000
    #endif
;
static SemaphoreHandle_t s_mutex = NULL;

static inline void _lock(void)   { if (s_mutex) xSemaphoreTake(s_mutex, portMAX_DELAY); }
static inline void _unlock(void) { if (s_mutex) xSemaphoreGive(s_mutex); }

#ifdef CONFIG_MEASUREMENT_MINIMIZE_JITTER
    static TickType_t xLastWakeTime;
    static TickType_t xFrequency;
#endif

static Measurement_return_code_t Measurement_spin(void);  // In case we are using a thread, this function should not be part of the public API

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
    #if CONFIG_MEASUREMENT_PIN_CORE_ANY
        return tskNO_AFFINITY;
    #elif CONFIG_MEASUREMENT_PIN_CORE_0
        return 0;
    #elif CONFIG_MEASUREMENT_PIN_CORE_1
        return 1;
    #else
        return tskNO_AFFINITY;
    #endif
}

static void Measurement_task(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "task started (period=%u ms)", (unsigned)s_period_ms);
#ifdef CONFIG_MEASUREMENT_MINIMIZE_JITTER
    xLastWakeTime = xTaskGetTickCount();
    xFrequency = (s_period_ms / portTICK_PERIOD_MS);
#endif
    while (s_run) {
        Measurement_return_code_t ret = Measurement_spin();
        if (ret != Measurement_ret_ok)
        {
            ESP_LOGW(TAG, "Error in spin");
        }
#ifdef CONFIG_MEASUREMENT_MINIMIZE_JITTER
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
#else
        vTaskDelay(pdMS_TO_TICKS(s_period_ms));
#endif
    }
    ESP_LOGI(TAG, "task exit");
    vTaskDelete(NULL);
}

#endif // CONFIG_MEASUREMENT_USE_THREAD

// END   --- Multitasking variables and handlers

// BEGIN ------------------ Public API (MULTITASKING)------------------


#if CONFIG_MEASUREMENT_USE_THREAD

Measurement_return_code_t Measurement_start(void)
{
    if (_create_mutex_once() != pdPASS) {
        ESP_LOGE(TAG, "mutex creation failed");
        return Measurement_ret_error;
    }
    if (s_task) {
        // idempotente
        return Measurement_ret_ok;
    }
    s_run = true;

    BaseType_t core = _get_core_affinity();
    BaseType_t ok = xTaskCreatePinnedToCore(
        Measurement_task,
        "Measurement",
        CONFIG_MEASUREMENT_TASK_STACK,
        NULL,
        CONFIG_MEASUREMENT_TASK_PRIO,
        &s_task,
        core
    );
    if (ok != pdPASS) {
        s_task = NULL;
        s_run = false;
        ESP_LOGE(TAG, "xTaskCreatePinnedToCore failed");
        return Measurement_ret_error;
    }
    return Measurement_ret_ok;
}

Measurement_return_code_t Measurement_stop(void)
{
    if (!s_task) return Measurement_ret_ok; // idempotente
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
    return Measurement_ret_ok;
}

Measurement_return_code_t Measurement_get_dre_clone(Measurement_dre_t *dst)
{
    if (!dst) return Measurement_ret_error;
    _lock();
    *dst = Measurement_dre;
    _unlock();
    return Measurement_ret_ok;
}

Measurement_return_code_t Measurement_set_period_ms(uint32_t period_ms)
{
    if (period_ms < 10) period_ms = 10;
    _lock();
    s_period_ms = period_ms;
#ifdef CONFIG_MEASUREMENT_MINIMIZE_JITTER    
    xFrequency = (s_period_ms / portTICK_PERIOD_MS);
#endif
    _unlock();
    ESP_LOGI(TAG, "period set to %u ms", (unsigned)period_ms);
    return Measurement_ret_ok;
}

uint32_t Measurement_get_period_ms(void)
{
    _lock();
    uint32_t v = s_period_ms;
    _unlock();
    return v;
}

/**
 *  Execute a function wrapped with locks so you can access the DRE variables in thread-safe mode
*/
void Measurement_execute_function_safemode(void (*callback)())
{
    _lock();
    callback();
    _unlock();
}

#endif // CONFIG_MEASUREMENT_USE_THREAD

// END   ------------------ Public API (MULTITASKING)------------------

// BEGIN ------------------ Public API (COMMON + SPIN)------------------

Measurement_return_code_t Measurement_setup(void)
{
    // Init liviano; no arranca tarea.
    ESP_LOGD(TAG, "setup()");
    // Set the constant data
    Measurement_dre.ai1_desc = AI1_DESC;
    Measurement_dre.ai2_desc = AI2_DESC;
    Measurement_dre.bi0_desc = BI0_DESC;

    // Set the initial data values
    Measurement_dre.ai1 = 15.0;
    Measurement_dre.ai2 = 20.0;
    Measurement_dre.bi0 = false;

#if CONFIG_MEASUREMENT_USE_THREAD
    if (_create_mutex_once() != pdPASS) {
        ESP_LOGE(TAG, "mutex creation failed");
        return Measurement_ret_error;
    }
#endif
    Measurement_dre.last_return_code = Measurement_ret_ok;
    return Measurement_ret_ok;
}

#if CONFIG_MEASUREMENT_USE_THREAD
static  // In case we are using a thread, this function should not be part of the public API
#endif
Measurement_return_code_t Measurement_spin(void)
{
#if CONFIG_MEASUREMENT_USE_THREAD
    _lock();
#endif
    bool en = Measurement_dre.enabled;
    if (!en) 
    {
#if CONFIG_MEASUREMENT_USE_THREAD
        _unlock();
#endif
        return Measurement_ret_ok;
    }
    else
    {
        // Implement your spin here
        // this area is protected, so concentrate here
        // the stuff which needs protection against
        // concurrency issues

        ESP_LOGI(TAG, "Hello world! %d",Measurement_dre.enabled);
#if CONFIG_MEASUREMENT_USE_THREAD
        _unlock();
#endif
        // Communicate results, do stuff which 
        // does not need protection
        // ...
        ESP_LOGI(TAG, "Finishing!");
        return Measurement_ret_ok;
    }
}

Measurement_return_code_t Measurement_enable(void)
{
#if CONFIG_MEASUREMENT_USE_THREAD
    _lock();
#endif
    Measurement_dre.enabled = true;
    Measurement_dre.last_return_code = Measurement_ret_ok;
#if CONFIG_MEASUREMENT_USE_THREAD
    _unlock();
#endif
    return Measurement_ret_ok;
}

Measurement_return_code_t Measurement_disable(void)
{
#if CONFIG_MEASUREMENT_USE_THREAD
    _lock();
#endif
    Measurement_dre.enabled = false;
    Measurement_dre.last_return_code = Measurement_ret_ok;
#if CONFIG_MEASUREMENT_USE_THREAD
    _unlock();
#endif
    return Measurement_ret_ok;
}

// BEGIN ------------------ Public API (COMMON)------------------
