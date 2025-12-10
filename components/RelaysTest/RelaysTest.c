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
#if CONFIG_RELAYSTEST_USE_THREAD
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
#include "RelaysTest.h"
#include "RelaysTest_netvars.h"
#include "Relays.h"

// END --- Self-includes section ---

// BEGIN --- Logging related variables
static const char *TAG = "RelaysTest";
// END --- Logging related variables

#if CONFIG_RELAYSTEST_VERBOSE_MODE
static const char *RelaysTest_state_name(relay_state_t st)
{
    return st == RELAY_STATE_ON ? "ON" : "OFF";
}
#endif

// BEGIN --- Internal variables (DRE)
RelaysTest_dre_t RelaysTest_dre = {
    .enabled = true,
    .dwell_ms = 0,
    .relay_index = 0,
    .last_change = 0,
    .last_return_code = RelaysTest_ret_ok
};
// END   --- Internal variables (DRE)



// BEGIN --- Multitasking variables and handlers

#if CONFIG_RELAYSTEST_USE_THREAD
static TaskHandle_t s_task = NULL;
static volatile bool s_run = false;
static uint32_t s_period_ms =
    #ifdef CONFIG_RELAYSTEST_PERIOD_MS
      CONFIG_RELAYSTEST_PERIOD_MS
    #else
      1000
    #endif
;
static SemaphoreHandle_t s_mutex = NULL;

static inline void _lock(void)   { if (s_mutex) xSemaphoreTake(s_mutex, portMAX_DELAY); }
static inline void _unlock(void) { if (s_mutex) xSemaphoreGive(s_mutex); }

#ifdef CONFIG_RELAYSTEST_MINIMIZE_JITTER
    static TickType_t xLastWakeTime;
    static TickType_t xFrequency;
#endif

static RelaysTest_return_code_t RelaysTest_spin(void);  // In case we are using a thread, this function should not be part of the public API

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
    #if CONFIG_RELAYSTEST_PIN_CORE_ANY
        return tskNO_AFFINITY;
    #elif CONFIG_RELAYSTEST_PIN_CORE_0
        return 0;
    #elif CONFIG_RELAYSTEST_PIN_CORE_1
        return 1;
    #else
        return tskNO_AFFINITY;
    #endif
}

static void RelaysTest_task(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "task started (period=%u ms)", (unsigned)s_period_ms);
#ifdef CONFIG_RELAYSTEST_MINIMIZE_JITTER
    xLastWakeTime = xTaskGetTickCount();
    xFrequency = (s_period_ms / portTICK_PERIOD_MS);
#endif
    while (s_run) {
        RelaysTest_return_code_t ret = RelaysTest_spin();
        if (ret != RelaysTest_ret_ok)
        {
            ESP_LOGW(TAG, "Error in spin");
        }
#ifdef CONFIG_RELAYSTEST_MINIMIZE_JITTER
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
#else
        vTaskDelay(pdMS_TO_TICKS(s_period_ms));
#endif
    }
    ESP_LOGI(TAG, "task exit");
    vTaskDelete(NULL);
}

#endif // CONFIG_RELAYSTEST_USE_THREAD

// END   --- Multitasking variables and handlers

// BEGIN ------------------ Public API (MULTITASKING)------------------


#if CONFIG_RELAYSTEST_USE_THREAD

RelaysTest_return_code_t RelaysTest_start(void)
{
    if (_create_mutex_once() != pdPASS) {
        ESP_LOGE(TAG, "mutex creation failed");
        return RelaysTest_ret_error;
    }
    if (s_task) {
        // idempotente
        return RelaysTest_ret_ok;
    }
    s_run = true;

    BaseType_t core = _get_core_affinity();
    BaseType_t ok = xTaskCreatePinnedToCore(
        RelaysTest_task,
        "RelaysTest",
        CONFIG_RELAYSTEST_TASK_STACK,
        NULL,
        CONFIG_RELAYSTEST_TASK_PRIO,
        &s_task,
        core
    );
    if (ok != pdPASS) {
        s_task = NULL;
        s_run = false;
        ESP_LOGE(TAG, "xTaskCreatePinnedToCore failed");
        return RelaysTest_ret_error;
    }
    return RelaysTest_ret_ok;
}

RelaysTest_return_code_t RelaysTest_stop(void)
{
    if (!s_task) return RelaysTest_ret_ok; // idempotente
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
    return RelaysTest_ret_ok;
}

RelaysTest_return_code_t RelaysTest_get_dre_clone(RelaysTest_dre_t *dst)
{
    if (!dst) return RelaysTest_ret_error;
    _lock();
    memcpy(dst, &RelaysTest_dre, sizeof(RelaysTest_dre));
    _unlock();
    return RelaysTest_ret_ok;
}

RelaysTest_return_code_t RelaysTest_set_period_ms(uint32_t period_ms)
{
    if (period_ms < 10) period_ms = 10;
    _lock();
    s_period_ms = period_ms;
#ifdef CONFIG_RELAYSTEST_MINIMIZE_JITTER    
    xFrequency = (s_period_ms / portTICK_PERIOD_MS);
#endif
    _unlock();
    ESP_LOGI(TAG, "period set to %u ms", (unsigned)period_ms);
    return RelaysTest_ret_ok;
}

uint32_t RelaysTest_get_period_ms(void)
{
    _lock();
    uint32_t v = s_period_ms;
    _unlock();
    return v;
}

/**
 *  Execute a function wrapped with locks so you can access the DRE variables in thread-safe mode
*/
void RelaysTest_execute_function_safemode(void (*callback)())
{
    _lock();
    callback();
    _unlock();
}

#endif // CONFIG_RELAYSTEST_USE_THREAD

// END   ------------------ Public API (MULTITASKING)------------------

// BEGIN ------------------ Public API (COMMON + SPIN)------------------

RelaysTest_return_code_t RelaysTest_setup(void)
{
    // Init liviano; no arranca tarea.
    ESP_LOGD(TAG, "setup()");
#if CONFIG_RELAYSTEST_USE_THREAD
    if (_create_mutex_once() != pdPASS) {
        ESP_LOGE(TAG, "mutex creation failed");
        return RelaysTest_ret_error;
    }
#endif
    RelaysTest_dre.dwell_ms = CONFIG_RELAYSTEST_DWELL_MS;
    RelaysTest_dre.relay_index = 0;
    RelaysTest_dre.last_change = xTaskGetTickCount();
    // Ensure relays are initialized and all off
    uint32_t count = Relays_get_count();
    if (count > 0)
    {
        for (uint32_t i = 0; i < count; ++i)
        {
            Relays_set_state(i, RELAY_STATE_OFF);
        }
    }
    RelaysTest_dre.last_return_code = RelaysTest_ret_ok;
    return RelaysTest_ret_ok;
}

#if CONFIG_RELAYSTEST_USE_THREAD
static  // In case we are using a thread, this function should not be part of the public API
#endif
RelaysTest_return_code_t RelaysTest_spin(void)
{
#if CONFIG_RELAYSTEST_USE_THREAD
    _lock();
#endif
    bool en = RelaysTest_dre.enabled;

    if (!en)
    {
#if CONFIG_RELAYSTEST_USE_THREAD        
        _unlock();
#endif
        return RelaysTest_ret_ok;
    }
    else
    {
        uint32_t count = Relays_get_count();
        if (count == 0)
        {
            ESP_LOGE(TAG, "No relays configured");
            return RelaysTest_ret_error;
        }

        TickType_t now = xTaskGetTickCount();
        TickType_t elapsed = now - RelaysTest_dre.last_change;
        if (elapsed >= pdMS_TO_TICKS(RelaysTest_dre.dwell_ms))
        {
            // turn current off
            Relays_set_state(RelaysTest_dre.relay_index, RELAY_STATE_OFF);
            // next relay
            RelaysTest_dre.relay_index = (RelaysTest_dre.relay_index + 1) % count;
            // turn next on
            Relays_set_state(RelaysTest_dre.relay_index, RELAY_STATE_ON);
            RelaysTest_dre.last_change = now;
#if CONFIG_RELAYSTEST_VERBOSE_MODE
            ESP_LOGI(TAG, "Relay %u -> %s", (unsigned)RelaysTest_dre.relay_index, RelaysTest_state_name(RELAY_STATE_ON));
#endif
        }

#if CONFIG_RELAYSTEST_USE_THREAD
        // Unlocking after the protected data has been managed for this cycle
        _unlock();
#endif
        RelaysTest_nvs_spin();

        return RelaysTest_ret_ok;
    }
}

RelaysTest_return_code_t RelaysTest_enable(void)
{
#if CONFIG_RELAYSTEST_USE_THREAD
    _lock();
#endif
    RelaysTest_dre.enabled = true;
    RelaysTest_dre.last_return_code = RelaysTest_ret_ok;
#if CONFIG_RELAYSTEST_USE_THREAD
    _unlock();
#endif
    return RelaysTest_ret_ok;
}

RelaysTest_return_code_t RelaysTest_disable(void)
{
#if CONFIG_RELAYSTEST_USE_THREAD
    _lock();
#endif
    RelaysTest_dre.enabled = false;
    RelaysTest_dre.last_return_code = RelaysTest_ret_ok;
#if CONFIG_RELAYSTEST_USE_THREAD
    _unlock();
#endif
    return RelaysTest_ret_ok;
}

void RelaysTest_set_dwell_ms(uint32_t dwell_ms)
{
    if (dwell_ms < 100) dwell_ms = 100;
    RelaysTest_dre.dwell_ms = dwell_ms;
}

// BEGIN ------------------ Public API (COMMON)------------------
