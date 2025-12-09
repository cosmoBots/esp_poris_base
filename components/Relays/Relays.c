// BEGIN --- Standard C headers section ---
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// END   --- Standard C headers section ---

// BEGIN --- SDK config section---
#include <sdkconfig.h>
// END   --- SDK config section---

// BEGIN --- FreeRTOS headers section ---
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#if CONFIG_RELAYS_USE_THREAD
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
#include "Relays.h"
#include "Relays_netvars.h"

// END --- Self-includes section ---

// BEGIN --- Logging related variables
static const char *TAG = "Relays";
// END --- Logging related variables

// BEGIN --- Internal variables (DRE)
Relays_dre_t Relays_dre = {
    .enabled = true,
    .count = 0,
    .gpios = {0},
    .states = {0},
    .hw_init = false,
    .last_return_code = Relays_ret_ok
};
// END   --- Internal variables (DRE)



// BEGIN --- Multitasking variables and handlers

#if CONFIG_RELAYS_USE_THREAD
static TaskHandle_t s_task = NULL;
static volatile bool s_run = false;
static uint32_t s_period_ms =
    #ifdef CONFIG_RELAYS_PERIOD_MS
      CONFIG_RELAYS_PERIOD_MS
    #else
      1000
    #endif
;
static SemaphoreHandle_t s_mutex = NULL;

static inline void _lock(void)   { if (s_mutex) xSemaphoreTake(s_mutex, portMAX_DELAY); }
static inline void _unlock(void) { if (s_mutex) xSemaphoreGive(s_mutex); }

#ifdef CONFIG_RELAYS_MINIMIZE_JITTER
    static TickType_t xLastWakeTime;
    static TickType_t xFrequency;
#endif

static Relays_return_code_t Relays_spin(void);  // In case we are using a thread, this function should not be part of the public API

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
    #if CONFIG_RELAYS_PIN_CORE_ANY
        return tskNO_AFFINITY;
    #elif CONFIG_RELAYS_PIN_CORE_0
        return 0;
    #elif CONFIG_RELAYS_PIN_CORE_1
        return 1;
    #else
        return tskNO_AFFINITY;
    #endif
}

static void Relays_task(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "task started (period=%u ms)", (unsigned)s_period_ms);
#ifdef CONFIG_RELAYS_MINIMIZE_JITTER
    xLastWakeTime = xTaskGetTickCount();
    xFrequency = (s_period_ms / portTICK_PERIOD_MS);
#endif
    while (s_run) {
        Relays_return_code_t ret = Relays_spin();
        if (ret != Relays_ret_ok)
        {
            ESP_LOGW(TAG, "Error in spin");
        }
#ifdef CONFIG_RELAYS_MINIMIZE_JITTER
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
#else
        vTaskDelay(pdMS_TO_TICKS(s_period_ms));
#endif
    }
    ESP_LOGI(TAG, "task exit");
    vTaskDelete(NULL);
}

#endif // CONFIG_RELAYS_USE_THREAD

// END   --- Multitasking variables and handlers

// BEGIN ------------------ Public API (MULTITASKING)------------------


#if CONFIG_RELAYS_USE_THREAD

Relays_return_code_t Relays_start(void)
{
    if (_create_mutex_once() != pdPASS) {
        ESP_LOGE(TAG, "mutex creation failed");
        return Relays_ret_error;
    }
    if (s_task) {
        // idempotente
        return Relays_ret_ok;
    }
    s_run = true;

    BaseType_t core = _get_core_affinity();
    BaseType_t ok = xTaskCreatePinnedToCore(
        Relays_task,
        "Relays",
        CONFIG_RELAYS_TASK_STACK,
        NULL,
        CONFIG_RELAYS_TASK_PRIO,
        &s_task,
        core
    );
    if (ok != pdPASS) {
        s_task = NULL;
        s_run = false;
        ESP_LOGE(TAG, "xTaskCreatePinnedToCore failed");
        return Relays_ret_error;
    }
    return Relays_ret_ok;
}

Relays_return_code_t Relays_stop(void)
{
    if (!s_task) return Relays_ret_ok; // idempotente
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
    return Relays_ret_ok;
}

Relays_return_code_t Relays_get_dre_clone(Relays_dre_t *dst)
{
    if (!dst) return Relays_ret_error;
    _lock();
    *dst = Relays_dre;
    _unlock();
    return Relays_ret_ok;
}

Relays_return_code_t Relays_set_period_ms(uint32_t period_ms)
{
    if (period_ms < 10) period_ms = 10;
    _lock();
    s_period_ms = period_ms;
#ifdef CONFIG_RELAYS_MINIMIZE_JITTER    
    xFrequency = (s_period_ms / portTICK_PERIOD_MS);
#endif
    _unlock();
    ESP_LOGI(TAG, "period set to %u ms", (unsigned)period_ms);
    return Relays_ret_ok;
}

uint32_t Relays_get_period_ms(void)
{
    _lock();
    uint32_t v = s_period_ms;
    _unlock();
    return v;
}

/**
 *  Execute a function wrapped with locks so you can access the DRE variables in thread-safe mode
*/
void Relays_execute_function_safemode(void (*callback)())
{
    _lock();
    callback();
    _unlock();
}

#endif // CONFIG_RELAYS_USE_THREAD

// END   ------------------ Public API (MULTITASKING)------------------

// BEGIN ------------------ Public API (COMMON + SPIN)------------------

Relays_return_code_t Relays_setup(void)
{
    // Init liviano; no arranca tarea.
    ESP_LOGD(TAG, "setup()");
#if CONFIG_RELAYS_USE_THREAD
    if (_create_mutex_once() != pdPASS) {
        ESP_LOGE(TAG, "mutex creation failed");
        return Relays_ret_error;
    }
#endif
    Relays_dre.count = CONFIG_RELAYS_COUNT;
    if (Relays_dre.count > RELAYS_MAX_COUNT)
    {
        ESP_LOGE(TAG, "RELAYS_COUNT too high (%u > %u)", (unsigned)Relays_dre.count, (unsigned)RELAYS_MAX_COUNT);
        Relays_dre.count = RELAYS_MAX_COUNT;
    }

    // Parse GPIO list
    const char *list = CONFIG_RELAYS_GPIO_LIST;
    size_t idx = 0;
    const char *p = list;
    while (*p && idx < Relays_dre.count)
    {
        char *endp = NULL;
        long v = strtol(p, &endp, 10);
        if (endp == p)
        {
            ESP_LOGE(TAG, "Invalid GPIO list entry at position %zu in \"%s\"", idx, list);
            return Relays_ret_error;
        }
        Relays_dre.gpios[idx++] = (int)v;
        while (*endp == ' ' || *endp == ',') endp++;
        p = endp;
    }
    if (idx < Relays_dre.count)
    {
        ESP_LOGE(TAG, "Not enough GPIOs provided (%zu < %u)", idx, (unsigned)Relays_dre.count);
        return Relays_ret_error;
    }
    // init GPIOs
    for (size_t i = 0; i < Relays_dre.count; ++i)
    {
        gpio_reset_pin(Relays_dre.gpios[i]);
        gpio_set_direction(Relays_dre.gpios[i], GPIO_MODE_OUTPUT);
        gpio_set_level(Relays_dre.gpios[i], 0);
        Relays_dre.states[i] = RELAY_STATE_OFF;
    }
    Relays_dre.hw_init = true;
    Relays_dre.last_return_code = Relays_ret_ok;
    return Relays_ret_ok;
}

#if CONFIG_RELAYS_USE_THREAD
static  // In case we are using a thread, this function should not be part of the public API
#endif
Relays_return_code_t Relays_spin(void)
{
#if CONFIG_RELAYS_USE_THREAD
    _lock();
#endif
    bool en = Relays_dre.enabled;

    if (!en)
    {
#if CONFIG_RELAYS_USE_THREAD        
        _unlock();
#endif
        return Relays_ret_ok;
    }
    else
    {
#if CONFIG_RELAYS_USE_THREAD
        // Unlocking after the protected data has been managed for this cycle
        _unlock();
#endif
        Relays_nvs_spin();

        // Communicate results, do stuff which 
        // does not need protection
        // ...
        return Relays_ret_ok;
    }
}

Relays_return_code_t Relays_enable(void)
{
#if CONFIG_RELAYS_USE_THREAD
    _lock();
#endif
    Relays_dre.enabled = true;
    Relays_dre.last_return_code = Relays_ret_ok;
#if CONFIG_RELAYS_USE_THREAD
    _unlock();
#endif
    return Relays_ret_ok;
}

Relays_return_code_t Relays_disable(void)
{
#if CONFIG_RELAYS_USE_THREAD
    _lock();
#endif
    Relays_dre.enabled = false;
    Relays_dre.last_return_code = Relays_ret_ok;
#if CONFIG_RELAYS_USE_THREAD
    _unlock();
#endif
    return Relays_ret_ok;
}

Relays_return_code_t Relays_set_state(uint32_t relay_idx, relay_state_t state)
{
    if (state != RELAY_STATE_ON && state != RELAY_STATE_OFF)
    {
        ESP_LOGE(TAG, "Invalid state %d", state);
        return Relays_ret_error;
    }
    if (!Relays_dre.hw_init)
    {
        if (Relays_setup() != Relays_ret_ok)
        {
            return Relays_ret_error;
        }
    }
    if (relay_idx >= Relays_dre.count)
    {
        ESP_LOGE(TAG, "Relay index out of range (%u >= %u)", (unsigned)relay_idx, (unsigned)Relays_dre.count);
        return Relays_ret_error;
    }
    bool any_on = false;
    if (!CONFIG_RELAYS_ALLOW_MULTIPLE && state == RELAY_STATE_ON)
    {
        // ensure no other relay is ON
        for (uint32_t i = 0; i < Relays_dre.count; ++i)
        {
            if (Relays_dre.states[i] == RELAY_STATE_ON && i != relay_idx)
            {
                any_on = true;
                break;
            }
        }
        if (any_on)
        {
            ESP_LOGE(TAG, "Multiple relays ON not allowed (another relay is already ON)");
            return Relays_ret_error;
        }
    }
    gpio_set_level(Relays_dre.gpios[relay_idx], state == RELAY_STATE_ON ? 1 : 0);
    Relays_dre.states[relay_idx] = state;
    return Relays_ret_ok;
}

uint32_t Relays_get_count(void)
{
    return Relays_dre.count;
}

// BEGIN ------------------ Public API (COMMON)------------------
