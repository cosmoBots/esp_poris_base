#include <stdio.h>
#include <string.h>
#include "PrjCfg.h"             // tu cabecera de proyecto
#include "HelloWorld.h"

#include "esp_log.h"
#include "sdkconfig.h"

#if CONFIG_HELLOWORLD_USE_THREAD
  #include "freertos/FreeRTOS.h"
  #include "freertos/task.h"
  #include "freertos/semphr.h"
#endif

// ------------------ Internals ------------------
static const char *TAG = "HelloWorld";

static HelloWorld_dre s_dre = {
    .enabled = true,
    .last_return_code = HelloWorld_ret_ok
};

#if CONFIG_HELLOWORLD_USE_THREAD
static TaskHandle_t s_task = NULL;
static volatile bool s_run = false;
static uint32_t s_period_ms =
    #ifdef CONFIG_HELLOWORLD_PERIOD_MS
      CONFIG_HELLOWORLD_PERIOD_MS
    #else
      1000
    #endif
;
static SemaphoreHandle_t s_mutex = NULL;

static inline void _lock(void)   { if (s_mutex) xSemaphoreTake(s_mutex, portMAX_DELAY); }
static inline void _unlock(void) { if (s_mutex) xSemaphoreGive(s_mutex); }

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
    #if CONFIG_HELLOWORLD_PIN_CORE_ANY
        return tskNO_AFFINITY;
    #elif CONFIG_HELLOWORLD_PIN_CORE_0
        return 0;
    #elif CONFIG_HELLOWORLD_PIN_CORE_1
        return 1;
    #else
        return tskNO_AFFINITY;
    #endif
}
#endif // CONFIG_HELLOWORLD_USE_THREAD

// ------------------ Public API ------------------

HelloWorld_return_code HelloWorld_setup(void)
{
    // Init liviano; no arranca tarea.
    ESP_LOGD(TAG, "setup()");
#if CONFIG_HELLOWORLD_USE_THREAD
    if (_create_mutex_once() != pdPASS) {
        ESP_LOGE(TAG, "mutex creation failed");
        return HelloWorld_ret_error;
    }
#endif
    s_dre.last_return_code = HelloWorld_ret_ok;
    return HelloWorld_ret_ok;
}

HelloWorld_return_code HelloWorld_spin(void)
{
    // Si está deshabilitado, no hace nada (pero retorna OK para no romper pipelines)
#if CONFIG_HELLOWORLD_USE_THREAD
    _lock();
#endif
    bool en = s_dre.enabled;
#if CONFIG_HELLOWORLD_USE_THREAD
    _unlock();
#endif
    if (!en) return HelloWorld_ret_ok;

    ESP_LOGI(TAG, "Hello world!");
    return HelloWorld_ret_ok;
}

HelloWorld_return_code HelloWorld_enable(void)
{
#if CONFIG_HELLOWORLD_USE_THREAD
    _lock();
#endif
    s_dre.enabled = true;
    s_dre.last_return_code = HelloWorld_ret_ok;
#if CONFIG_HELLOWORLD_USE_THREAD
    _unlock();
#endif
    return HelloWorld_ret_ok;
}

HelloWorld_return_code HelloWorld_disable(void)
{
#if CONFIG_HELLOWORLD_USE_THREAD
    _lock();
#endif
    s_dre.enabled = false;
    s_dre.last_return_code = HelloWorld_ret_ok;
#if CONFIG_HELLOWORLD_USE_THREAD
    _unlock();
#endif
    return HelloWorld_ret_ok;
}

#if CONFIG_HELLOWORLD_USE_THREAD
// ------------------ Threaded mode ------------------

static void helloworld_task(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "task started (period=%u ms)", (unsigned)s_period_ms);
    while (s_run) {
        (void)HelloWorld_spin();
        vTaskDelay(pdMS_TO_TICKS(s_period_ms));
    }
    ESP_LOGI(TAG, "task exit");
    vTaskDelete(NULL);
}

HelloWorld_return_code HelloWorld_start(void)
{
    if (_create_mutex_once() != pdPASS) {
        ESP_LOGE(TAG, "mutex creation failed");
        return HelloWorld_ret_error;
    }
    if (s_task) {
        // idempotente
        return HelloWorld_ret_ok;
    }
    s_run = true;

    BaseType_t core = _get_core_affinity();
    BaseType_t ok = xTaskCreatePinnedToCore(
        helloworld_task,
        "helloworld",
        CONFIG_HELLOWORLD_TASK_STACK,
        NULL,
        CONFIG_HELLOWORLD_TASK_PRIO,
        &s_task,
        core
    );
    if (ok != pdPASS) {
        s_task = NULL;
        s_run = false;
        ESP_LOGE(TAG, "xTaskCreatePinnedToCore failed");
        return HelloWorld_ret_error;
    }
    return HelloWorld_ret_ok;
}

HelloWorld_return_code HelloWorld_stop(void)
{
    if (!s_task) return HelloWorld_ret_ok; // idempotente
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
    return HelloWorld_ret_ok;
}

HelloWorld_return_code HelloWorld_get_dre_clone(HelloWorld_dre *dst)
{
    if (!dst) return HelloWorld_ret_error;
    _lock();
    *dst = s_dre;
    _unlock();
    return HelloWorld_ret_ok;
}

HelloWorld_return_code HelloWorld_set_period_ms(uint32_t period_ms)
{
    if (period_ms < 10) period_ms = 10;
    _lock();
    s_period_ms = period_ms;
    _unlock();
    ESP_LOGI(TAG, "period set to %u ms", (unsigned)period_ms);
    return HelloWorld_ret_ok;
}

uint32_t HelloWorld_get_period_ms(void)
{
    _lock();
    uint32_t v = s_period_ms;
    _unlock();
    return v;
}
#endif // CONFIG_HELLOWORLD_USE_THREAD
