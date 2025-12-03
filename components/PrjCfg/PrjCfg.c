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
#if CONFIG_PRJCFG_USE_THREAD
  #include <freertos/semphr.h>
#endif

// END   --- FreeRTOS headers section ---


// BEGIN --- ESP-IDF headers section ---
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_mac.h>
#include <cJSON.h>

// END   --- ESP-IDF headers section ---

// BEGIN --- Project configuration section ---
#include <PrjCfg.h> // Including project configuration module 
// END   --- Project configuration section ---

// BEGIN --- Project configuration section ---

// end   --- Project configuration section ---

// BEGIN --- Self-includes section ---
#include "PrjCfg_nvs.h"

// END --- Self-includes section ---

// BEGIN --- Logging related variables
static const char *TAG = "PrjCfg";
// END --- Logging related variables

// BEGIN --- Internal variables (DRE)
PrjCfg_dre_t PrjCfg_dre = {
    .enabled = true,
    .last_return_code = PrjCfg_ret_ok,

    .eth_mac = {0,0,0,0,0,0},
    .unique_id = "",
    .ip_address = "",
    .ip.addr = 0,
    .ssid = "",
    .wifi_connected = false,
    
#ifdef CONFIG_PORIS_ENABLE_BLECENTRAL
    .central_role = false,
#endif
#ifdef CONFIG_PORIS_ENABLE_BLEPERIPHERAL
    .peripheral_role = false,
    .echo = false,
#endif
#ifdef CONFIG_PORIS_ENABLE_OTA
    .skip_ota = true,
#endif
#ifdef CONFIG_PORIS_ENABLE_UARTBRIDGE
    .uart_bridge = false,
#endif

#ifdef CONFIG_PORIS_ENABLE_UARTUSER
    .uart_user_port = UART_PORT_TO_USER,                 // porn number
    .uart_user_baudrate = UART_PORT_TO_USER_BAUDRATE,     // baud rate
    .uart_user_stop_bits = UART_PORT_TO_USER_STOP_BITS,   /*!< UART stop bits*/
    .uart_user_tx_io_num = UART_PORT_TO_USER_TXD,                // -1 for keeping hardware defaults
    .uart_user_rx_io_num = UART_PORT_TO_USER_RXD,                // -1 for keeping hardware defaults
#endif

#ifdef CONFIG_PORIS_ENABLE_UARTPERIPH
    // Uart to Peripheral configuration
    .uart_periph_port = UART_PORT_TO_DEVICE,           // porn number
    .uart_periph_baudrate = UART_PORT_TO_DEVICE_BAUDRATE,
    .uart_periph_stop_bits = UART_PORT_TO_DEVICE_STOP_BITS, /*!< UART stop bits*/
    .uart_periph_tx_io_num = UART_PORT_TO_DEVICE_TXD,              // -1 for keeping hardware defaults
    .uart_periph_rx_io_num = UART_PORT_TO_DEVICE_RXD,            // -1 for keeping hardware defaults
#endif

#ifdef CONFIG_PORIS_ENABLE_BLINK
    // Blink functionality
    .blink_enabled = BLINK_DEFAULT_ENABLED,                     // true if blink shall execute
    .blink_io_num = BLINK_DEFAULT_PIN,                       // pin number for blink pin
    .blink_on_value = BLINK_DEFAULT_ON_VALUE,                    // LOW or HIGH for lighting the LED
    .blink_on_ms = BLINK_DEFAULT_ON_MS,                        // blinking time in ON
    .blink_off_ms = BLINK_DEFAULT_OFF_MS,                       // blinking time in OFF
#endif

#ifdef CONFIG_PORIS_ENABLE_BUTTON
    // Button functionality
    .button_enabled = BUTTON_DEFAULT_ENABLED,                    // true if button read shall execute
    .button_io_num = BUTTON_DEFAULT_IO_NUM,                      // pin number for reading the button
    .button_pullup = BUTTON_DEFAULT_PULLUP,                     // true if button read uses internal pullup
    .button_filter_ms = BUTTON_DEFAULT_FILTER_MS,                   // debouncing time for button
    .button_notification_io_num = BUTTON_DEFAULT_NOTIF_IO_NUM,              // pin number to notify when the button is pressed
    .button_notification_value = BUTTON_DEFAULT_NOTIF_VALUE,         // LOW or HIGH for notifying button pressed
#endif

#ifdef CONFIG_PORIS_ENABLE_SERVO
    // Servo driver functionality
    .servo_enabled = false,                     // true if servo shall execute
#endif

#ifdef CONFIG_PORIS_ENABLE_HOVER
    // HoverWheels driver functionality
    .hover_enabled = HOVER_DEFAULT_ENABLED,                             // true if hoverwheels shall execute
    .uart_hover_port = UART_PORT_TO_HOVER,                    // porn number
    .uart_hover_tx_io_num = UART_PORT_TO_HOVER_TXD,                // -1 for keeping hardware defaults
    .uart_hover_rx_io_num = UART_PORT_TO_HOVER_RXD,                // -1 for keeping hardware defaults
#endif

};
// END   --- Internal variables (DRE)


// BEGIN --- Static business logic functions











static void get_identifiers(void)
{
    esp_read_mac(PrjCfg_dre.eth_mac, ESP_MAC_BASE);
    sprintf(PrjCfg_dre.unique_id, "%02X%02X%02X%02X%02X%02X", PrjCfg_dre.eth_mac[0], PrjCfg_dre.eth_mac[1], 
        PrjCfg_dre.eth_mac[2], PrjCfg_dre.eth_mac[3], PrjCfg_dre.eth_mac[4], PrjCfg_dre.eth_mac[5]);
}

// END --- Static business logic functions


// BEGIN --- Multitasking variables and handlers

#if CONFIG_PRJCFG_USE_THREAD
static TaskHandle_t s_task = NULL;
static volatile bool s_run = false;
static uint32_t s_period_ms =
    #ifdef CONFIG_PRJCFG_PERIOD_MS
      CONFIG_PRJCFG_PERIOD_MS
    #else
      1000
    #endif
;
static SemaphoreHandle_t s_mutex = NULL;

static inline void _lock(void)   { if (s_mutex) xSemaphoreTake(s_mutex, portMAX_DELAY); }
static inline void _unlock(void) { if (s_mutex) xSemaphoreGive(s_mutex); }

#ifdef CONFIG_PRJCFG_MINIMIZE_JITTER
    static TickType_t xLastWakeTime;
    static TickType_t xFrequency;
#endif

static PrjCfg_return_code_t PrjCfg_spin(void);  // In case we are using a thread, this function should not be part of the public API

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
    #if CONFIG_PRJCFG_PIN_CORE_ANY
        return tskNO_AFFINITY;
    #elif CONFIG_PRJCFG_PIN_CORE_0
        return 0;
    #elif CONFIG_PRJCFG_PIN_CORE_1
        return 1;
    #else
        return tskNO_AFFINITY;
    #endif
}

static void PrjCfg_task(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "task started (period=%u ms)", (unsigned)s_period_ms);
#ifdef CONFIG_PRJCFG_MINIMIZE_JITTER
    xLastWakeTime = xTaskGetTickCount();
    xFrequency = (s_period_ms / portTICK_PERIOD_MS);
#endif
    while (s_run) {
        PrjCfg_return_code_t ret = PrjCfg_spin();
        if (ret != PrjCfg_ret_ok)
        {
            ESP_LOGW(TAG, "Error in spin");
        }
#ifdef CONFIG_PRJCFG_MINIMIZE_JITTER
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
#else
        vTaskDelay(pdMS_TO_TICKS(s_period_ms));
#endif
    }
    ESP_LOGI(TAG, "task exit");
    vTaskDelete(NULL);
}

#endif // CONFIG_PRJCFG_USE_THREAD

// END   --- Multitasking variables and handlers

// BEGIN ------------------ Public API (MULTITASKING)------------------


#if CONFIG_PRJCFG_USE_THREAD

PrjCfg_return_code_t PrjCfg_start(void)
{
    if (_create_mutex_once() != pdPASS) {
        ESP_LOGE(TAG, "mutex creation failed");
        return PrjCfg_ret_error;
    }
    if (s_task) {
        // idempotente
        return PrjCfg_ret_ok;
    }
    s_run = true;

    BaseType_t core = _get_core_affinity();
    BaseType_t ok = xTaskCreatePinnedToCore(
        PrjCfg_task,
        "PrjCfg",
        CONFIG_PRJCFG_TASK_STACK,
        NULL,
        CONFIG_PRJCFG_TASK_PRIO,
        &s_task,
        core
    );
    if (ok != pdPASS) {
        s_task = NULL;
        s_run = false;
        ESP_LOGE(TAG, "xTaskCreatePinnedToCore failed");
        return PrjCfg_ret_error;
    }
    return PrjCfg_ret_ok;
}

PrjCfg_return_code_t PrjCfg_stop(void)
{
    if (!s_task) return PrjCfg_ret_ok; // idempotente
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
    return PrjCfg_ret_ok;
}

PrjCfg_return_code_t PrjCfg_get_dre_clone(PrjCfg_dre_t *dst)
{
    if (!dst) return PrjCfg_ret_error;
    _lock();
    *dst = PrjCfg_dre;
    _unlock();
    return PrjCfg_ret_ok;
}

PrjCfg_return_code_t PrjCfg_set_period_ms(uint32_t period_ms)
{
    if (period_ms < 10) period_ms = 10;
    _lock();
    s_period_ms = period_ms;
#ifdef CONFIG_PRJCFG_MINIMIZE_JITTER    
    xFrequency = (s_period_ms / portTICK_PERIOD_MS);
#endif
    _unlock();
    ESP_LOGI(TAG, "period set to %u ms", (unsigned)period_ms);
    return PrjCfg_ret_ok;
}

uint32_t PrjCfg_get_period_ms(void)
{
    _lock();
    uint32_t v = s_period_ms;
    _unlock();
    return v;
}

/**
 *  Execute a function wrapped with locks so you can access the DRE variables in thread-safe mode
*/
void PrjCfg_execute_function_safemode(void (*callback)())
{
    _lock();
    callback();
    _unlock();
}

#endif // CONFIG_PRJCFG_USE_THREAD

// END   ------------------ Public API (MULTITASKING)------------------

// BEGIN ------------------ Public API (COMMON + SPIN)------------------

PrjCfg_return_code_t PrjCfg_setup(void)
{
    // Init liviano; no arranca tarea.
    ESP_LOGD(TAG, "setup()");
    // Loading values from NVS
    PrjCfg_nvs_cfg_load();
#ifdef CONFIG_FORCE_CENTRAL_ROLE
    prjcfg.central_role = true;
    PrjCfg_nvs_cfg_save();
#endif
#ifdef CONFIG_FORCE_PERIPHERAL_ROLE
    prjcfg.peripheral_role = true;
    PrjCfg_nvs_cfg_save();
#endif
    get_identifiers();    
#if CONFIG_PRJCFG_USE_THREAD
    if (_create_mutex_once() != pdPASS) {
        ESP_LOGE(TAG, "mutex creation failed");
        return PrjCfg_ret_error;
    }
#endif
    PrjCfg_dre.last_return_code = PrjCfg_ret_ok;
    return PrjCfg_ret_ok;
}

#if CONFIG_PRJCFG_USE_THREAD
static  // In case we are using a thread, this function should not be part of the public API
#endif
PrjCfg_return_code_t PrjCfg_spin(void)
{
#if CONFIG_PRJCFG_USE_THREAD
    _lock();
#endif
    bool en = PrjCfg_dre.enabled;
    if (!en)
    {
#if CONFIG_PRJCFG_USE_THREAD
        _unlock();
#endif
        return PrjCfg_ret_ok;
    }
    else
    {
        // Implement your spin here
        // this area is protected, so concentrate here
        // the stuff which needs protection against
        // concurrency issues

        //ESP_LOGI(TAG, "Doing protected stuff %d", PrjCfg_dre.enabled);
        //vTaskDelay(pdMS_TO_TICKS(120));

#if CONFIG_PRJCFG_USE_THREAD
        _unlock();
#endif

        // Communicate results, do stuff which 
        // does not need protection
        // ...
        //ESP_LOGI(TAG, "Hello world!");
        return PrjCfg_ret_ok;
    }
}

PrjCfg_return_code_t PrjCfg_enable(void)
{
#if CONFIG_PRJCFG_USE_THREAD
    _lock();
#endif
    PrjCfg_dre.enabled = true;
    PrjCfg_dre.last_return_code = PrjCfg_ret_ok;
#if CONFIG_PRJCFG_USE_THREAD
    _unlock();
#endif
    return PrjCfg_ret_ok;
}

PrjCfg_return_code_t PrjCfg_disable(void)
{
#if CONFIG_PRJCFG_USE_THREAD
    _lock();
#endif
    PrjCfg_dre.enabled = false;
    PrjCfg_dre.last_return_code = PrjCfg_ret_ok;
#if CONFIG_PRJCFG_USE_THREAD
    _unlock();
#endif
    return PrjCfg_ret_ok;
}

// BEGIN ------------------ Public API (COMMON)------------------
