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
#if CONFIG_PROVISIONING_USE_THREAD
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
#include "Provisioning.h"
#include "Provisioning_netvars.h"

// END --- Self-includes section ---

// BEGIN --- Logging related variables
static const char *TAG = "Provisioning";
// END --- Logging related variables

// BEGIN --- Internal variables (DRE)
Provisioning_dre_t Provisioning_dre = {
    .enabled = true,
    .last_return_code = Provisioning_ret_ok
};
// END   --- Internal variables (DRE)
// Netvars dirty tracking
static bool s_nvs_dirty = false;
static TickType_t s_nvs_dirty_since = 0;



// BEGIN --- Multitasking variables and handlers

#if CONFIG_PROVISIONING_USE_THREAD
static TaskHandle_t s_task = NULL;
static volatile bool s_run = false;
static uint32_t s_period_ms =
    #ifdef CONFIG_PROVISIONING_PERIOD_MS
      CONFIG_PROVISIONING_PERIOD_MS
    #else
      1000
    #endif
;
static SemaphoreHandle_t s_mutex = NULL;

static inline void _lock(void)   { if (s_mutex) xSemaphoreTake(s_mutex, portMAX_DELAY); }
static inline void _unlock(void) { if (s_mutex) xSemaphoreGive(s_mutex); }

#ifdef CONFIG_PROVISIONING_MINIMIZE_JITTER
    static TickType_t xLastWakeTime;
    static TickType_t xFrequency;
#endif

static Provisioning_return_code_t Provisioning_spin(void);  // In case we are using a thread, this function should not be part of the public API

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
    #if CONFIG_PROVISIONING_PIN_CORE_ANY
        return tskNO_AFFINITY;
    #elif CONFIG_PROVISIONING_PIN_CORE_0
        return 0;
    #elif CONFIG_PROVISIONING_PIN_CORE_1
        return 1;
    #else
        return tskNO_AFFINITY;
    #endif
}

static void Provisioning_task(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "task started (period=%u ms)", (unsigned)s_period_ms);
#ifdef CONFIG_PROVISIONING_MINIMIZE_JITTER
    xLastWakeTime = xTaskGetTickCount();
    xFrequency = (s_period_ms / portTICK_PERIOD_MS);
#endif
    while (s_run) {
        Provisioning_return_code_t ret = Provisioning_spin();
        if (ret != Provisioning_ret_ok)
        {
            ESP_LOGW(TAG, "Error in spin");
        }
#ifdef CONFIG_PROVISIONING_MINIMIZE_JITTER
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
#else
        vTaskDelay(pdMS_TO_TICKS(s_period_ms));
#endif
    }
    ESP_LOGI(TAG, "task exit");
    vTaskDelete(NULL);
}

#endif // CONFIG_PROVISIONING_USE_THREAD

// END   --- Multitasking variables and handlers

// BEGIN ------------------ Public API (MULTITASKING)------------------


#if CONFIG_PROVISIONING_USE_THREAD

Provisioning_return_code_t Provisioning_start(void)
{
    if (_create_mutex_once() != pdPASS) {
        ESP_LOGE(TAG, "mutex creation failed");
        return Provisioning_ret_error;
    }
    if (s_task) {
        // idempotente
        return Provisioning_ret_ok;
    }
    s_run = true;

    BaseType_t core = _get_core_affinity();
    BaseType_t ok = xTaskCreatePinnedToCore(
        Provisioning_task,
        "Provisioning",
        CONFIG_PROVISIONING_TASK_STACK,
        NULL,
        CONFIG_PROVISIONING_TASK_PRIO,
        &s_task,
        core
    );
    if (ok != pdPASS) {
        s_task = NULL;
        s_run = false;
        ESP_LOGE(TAG, "xTaskCreatePinnedToCore failed");
        return Provisioning_ret_error;
    }
    return Provisioning_ret_ok;
}

Provisioning_return_code_t Provisioning_stop(void)
{
    if (!s_task) return Provisioning_ret_ok; // idempotente
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
    return Provisioning_ret_ok;
}

Provisioning_return_code_t Provisioning_get_dre_clone(Provisioning_dre_t *dst)
{
    if (!dst) return Provisioning_ret_error;
    _lock();
    *dst = Provisioning_dre;
    _unlock();
    return Provisioning_ret_ok;
}

Provisioning_return_code_t Provisioning_set_period_ms(uint32_t period_ms)
{
    if (period_ms < 10) period_ms = 10;
    _lock();
    s_period_ms = period_ms;
#ifdef CONFIG_PROVISIONING_MINIMIZE_JITTER    
    xFrequency = (s_period_ms / portTICK_PERIOD_MS);
#endif
    _unlock();
    ESP_LOGI(TAG, "period set to %u ms", (unsigned)period_ms);
    return Provisioning_ret_ok;
}

uint32_t Provisioning_get_period_ms(void)
{
    _lock();
    uint32_t v = s_period_ms;
    _unlock();
    return v;
}

/**
 *  Execute a function wrapped with locks so you can access the DRE variables in thread-safe mode
*/
void Provisioning_execute_function_safemode(void (*callback)())
{
    _lock();
    callback();
    _unlock();
}

#endif // CONFIG_PROVISIONING_USE_THREAD

// END   ------------------ Public API (MULTITASKING)------------------

// BEGIN ------------------ Public API (COMMON + SPIN)------------------

Provisioning_return_code_t Provisioning_setup(void)
{
    // Init liviano; no arranca tarea.
    ESP_LOGD(TAG, "setup()");
#if CONFIG_PROVISIONING_USE_THREAD
    if (_create_mutex_once() != pdPASS) {
        ESP_LOGE(TAG, "mutex creation failed");
        return Provisioning_ret_error;
    }
#endif
    Provisioning_dre.last_return_code = Provisioning_ret_ok;
    return Provisioning_ret_ok;
}

#if CONFIG_PROVISIONING_USE_THREAD
static  // In case we are using a thread, this function should not be part of the public API
#endif
Provisioning_return_code_t Provisioning_spin(void)
{
#if CONFIG_PROVISIONING_USE_THREAD
    _lock();
#endif
    bool en = Provisioning_dre.enabled;
#if CONFIG_PROVISIONING_USE_THREAD
    _unlock();
#endif

    if (!en)
    {
#if CONFIG_PROVISIONING_USE_THREAD        
        _unlock();
#endif
        return Provisioning_ret_ok;
    }
    else
    {
        // Implement your spin here
        // this area is protected, so concentrate here
        // the stuff which needs protection against
        // concurrency issues

        //ESP_LOGI(TAG, "Doing protected stuff %d", Provisioning_dre.enabled);
        //vTaskDelay(pdMS_TO_TICKS(120));

        TickType_t now_ticks = xTaskGetTickCount();
        if (s_nvs_dirty &&
            (TickType_t)(now_ticks - s_nvs_dirty_since) >= pdMS_TO_TICKS(5000))
        {
            s_nvs_dirty = false;
#if CONFIG_PROVISIONING_USE_THREAD
            _unlock();
#endif
            Provisioning_netvars_nvs_save();
        }
        else
        {
#if CONFIG_PROVISIONING_USE_THREAD
            _unlock();
#endif
        }

        // Communicate results, do stuff which 
        // does not need protection
        // ...
        //ESP_LOGI(TAG, "Hello world!");
        return Provisioning_ret_ok;
    }
}

Provisioning_return_code_t Provisioning_enable(void)
{
#if CONFIG_PROVISIONING_USE_THREAD
    _lock();
#endif
    Provisioning_dre.enabled = true;
    Provisioning_dre.last_return_code = Provisioning_ret_ok;
#if CONFIG_PROVISIONING_USE_THREAD
    _unlock();
#endif
    return Provisioning_ret_ok;
}

Provisioning_return_code_t Provisioning_disable(void)
{
#if CONFIG_PROVISIONING_USE_THREAD
    _lock();
#endif
    Provisioning_dre.enabled = false;
    Provisioning_dre.last_return_code = Provisioning_ret_ok;
#if CONFIG_PROVISIONING_USE_THREAD
    _unlock();
#endif
    return Provisioning_ret_ok;
}

// BEGIN ------------------ Public API (COMMON)------------------
