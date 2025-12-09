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
#if CONFIG_MQTTCOMM_USE_THREAD
#include <freertos/semphr.h>
#endif

// END   --- FreeRTOS headers section ---

// BEGIN --- ESP-IDF headers section ---
#include <esp_log.h>
#include <esp_system.h>
#include <esp_mac.h>

// END   --- ESP-IDF headers section ---

// BEGIN --- Project configuration section ---
#include <PrjCfg.h> // Including project configuration module
// END   --- Project configuration section ---

// BEGIN --- Project configuration section ---

// end   --- Project configuration section ---

// BEGIN --- Self-includes section ---
#include "MQTTComm.h"
#include "MQTTComm_netvars.h"
#include "MqttSecrets.h"
// END --- Self-includes section ---

#ifndef CONFIG_MQTT_TOPIC_DEVICE_NAME
#define CONFIG_MQTT_TOPIC_DEVICE_NAME ""
#endif

#ifndef CONFIG_MQTT_CLIENT_ID_CUSTOM
#define CONFIG_MQTT_CLIENT_ID_CUSTOM ""
#endif

// BEGIN --- Logging related variables
static const char *TAG = "MQTTComm";
// END --- Logging related variables

// BEGIN --- Internal variables (DRE)
MQTTComm_dre_t MQTTComm_dre = {
    .enabled = false,
    .initialized = false,
    .started = false,
    .last_return_code = MQTTComm_ret_ok};
// END   --- Internal variables (DRE)



// BEGIN --- Functional variables and handlers

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
#ifdef CONFIG_DEBUG_SEND_DATA_ON_CONNECTION
        msg_id = esp_mqtt_client_publish(client, MQTTComm_dre.data_topic, "connected!", 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
#endif
        msg_id = esp_mqtt_client_subscribe(client, MQTTComm_dre.req_topic, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d %s", msg_id, MQTTComm_dre.req_topic);

        msg_id = esp_mqtt_client_subscribe(client, MQTTComm_dre.cfg_topic, 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d %s", msg_id, MQTTComm_dre.cfg_topic);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
#ifdef CONFIG_DEBUG_SEND_DATA_ON_SUBSCRIPTION
        msg_id = esp_mqtt_client_publish(client, MQTTComm_dre.data_topic, "subscribed!", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
#endif
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d %.*s", event->msg_id, event->topic_len, event->topic);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        ESP_LOGD(TAG, "Compare %.*s =? %s", event->topic_len, event->topic, MQTTComm_dre.cfg_topic);
        if (strncmp(event->topic, MQTTComm_dre.cfg_topic, event->topic_len) == 0)
        {
            MQTTComm_dre.cfg.f_cfg_cb(event->data, event->data_len);
        }
        else
        {
            if (strncmp(event->topic, MQTTComm_dre.req_topic, event->topic_len) == 0)
            {
                MQTTComm_dre.cfg.f_req_cb(event->data, event->data_len);
            }
        }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_BROKER_URL,
        .credentials = {
            .username = MQTT_BROKER_USER,
            .authentication = {
                .password = MQTT_BROKER_PW,
            },
            .client_id = MQTTComm_dre.client_id}};
#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.broker.address.uri, "FROM_STDIN") == 0)
    {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128)
        {
            int c = fgetc(stdin);
            if (c == '\n')
            {
                line[count] = '\0';
                break;
            }
            else if (c > 0 && c < 127)
            {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt_cfg.broker.address.uri = line;
        printf("Broker url: %s\n", line);
    }
    else
    {
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

    MQTTComm_dre.main_client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(MQTTComm_dre.main_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(MQTTComm_dre.main_client);
}

// END --- Functional variables and handlers

// BEGIN --- Multitasking variables and handlers

#if CONFIG_MQTTCOMM_USE_THREAD
static TaskHandle_t s_task = NULL;
static volatile bool s_run = false;
static uint32_t s_period_ms =
#ifdef CONFIG_MQTTCOMM_PERIOD_MS
    CONFIG_MQTTCOMM_PERIOD_MS
#else
    1000
#endif
    ;
static SemaphoreHandle_t s_mutex = NULL;

static inline void _lock(void)
{
    if (s_mutex)
        xSemaphoreTake(s_mutex, portMAX_DELAY);
}
static inline void _unlock(void)
{
    if (s_mutex)
        xSemaphoreGive(s_mutex);
}

#ifdef CONFIG_MQTTCOMM_MINIMIZE_JITTER
static TickType_t xLastWakeTime;
static TickType_t xFrequency;
#endif

static MQTTComm_return_code_t MQTTComm_spin(void); // In case we are using a thread, this function should not be part of the public API

static inline BaseType_t _create_mutex_once(void)
{
    if (!s_mutex)
    {
        s_mutex = xSemaphoreCreateMutex();
        if (!s_mutex)
            return pdFAIL;
    }
    return pdPASS;
}

static inline BaseType_t _get_core_affinity(void)
{
#if CONFIG_MQTTCOMM_PIN_CORE_ANY
    return tskNO_AFFINITY;
#elif CONFIG_MQTTCOMM_PIN_CORE_0
    return 0;
#elif CONFIG_MQTTCOMM_PIN_CORE_1
    return 1;
#else
    return tskNO_AFFINITY;
#endif
}

static void MQTTComm_task(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "task started (period=%u ms)", (unsigned)s_period_ms);
#ifdef CONFIG_MQTTCOMM_MINIMIZE_JITTER
    xLastWakeTime = xTaskGetTickCount();
    xFrequency = (s_period_ms / portTICK_PERIOD_MS);
#endif
    while (s_run)
    {
        MQTTComm_return_code_t ret = MQTTComm_spin();
        if (ret != MQTTComm_ret_ok)
        {
            ESP_LOGW(TAG, "Error in spin");
        }
#ifdef CONFIG_MQTTCOMM_MINIMIZE_JITTER
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
#else
        vTaskDelay(pdMS_TO_TICKS(s_period_ms));
#endif
    }
    ESP_LOGI(TAG, "task exit");
    vTaskDelete(NULL);
}

#endif // CONFIG_MQTTCOMM_USE_THREAD

// END   --- Multitasking variables and handlers

// BEGIN ------------------ Public API (MULTITASKING)------------------

#if CONFIG_MQTTCOMM_USE_THREAD

MQTTComm_return_code_t MQTTComm_start(void)
{
    if (_create_mutex_once() != pdPASS)
    {
        ESP_LOGE(TAG, "mutex creation failed");
        return MQTTComm_ret_error;
    }
    if (s_task)
    {
        // idempotente
        return MQTTComm_ret_ok;
    }
    s_run = true;

    BaseType_t core = _get_core_affinity();
    BaseType_t ok = xTaskCreatePinnedToCore(
        MQTTComm_task,
        "MQTTComm",
        CONFIG_MQTTCOMM_TASK_STACK,
        NULL,
        CONFIG_MQTTCOMM_TASK_PRIO,
        &s_task,
        core);
    if (ok != pdPASS)
    {
        s_task = NULL;
        s_run = false;
        ESP_LOGE(TAG, "xTaskCreatePinnedToCore failed");
        return MQTTComm_ret_error;
    }
    return MQTTComm_ret_ok;
}

MQTTComm_return_code_t MQTTComm_stop(void)
{
    if (!s_task)
        return MQTTComm_ret_ok; // idempotente
    s_run = false;
    // Espera una vuelta de scheduler para que el loop salga y se autodelete
    vTaskDelay(pdMS_TO_TICKS(1));
    // Si aún vive por cualquier motivo, fuerza delete
    if (s_task)
    {
        TaskHandle_t t = s_task;
        s_task = NULL;
        vTaskDelete(t);
    }
    ESP_LOGI(TAG, "stopped");
    return MQTTComm_ret_ok;
}

MQTTComm_return_code_t MQTTComm_get_dre_clone(MQTTComm_dre_t *dst)
{
    if (!dst)
        return MQTTComm_ret_error;
    _lock();
    *dst = MQTTComm_dre;
    _unlock();
    return MQTTComm_ret_ok;
}

MQTTComm_return_code_t MQTTComm_set_period_ms(uint32_t period_ms)
{
    if (period_ms < 10)
        period_ms = 10;
    _lock();
    s_period_ms = period_ms;
#ifdef CONFIG_MQTTCOMM_MINIMIZE_JITTER
    xFrequency = (s_period_ms / portTICK_PERIOD_MS);
#endif
    _unlock();
    ESP_LOGI(TAG, "period set to %u ms", (unsigned)period_ms);
    return MQTTComm_ret_ok;
}

uint32_t MQTTComm_get_period_ms(void)
{
    _lock();
    uint32_t v = s_period_ms;
    _unlock();
    return v;
}

/**
 *  Execute a function wrapped with locks so you can access the DRE variables in thread-safe mode
 */
void MQTTComm_execute_function_safemode(void (*callback)())
{
    _lock();
    callback();
    _unlock();
}

#endif // CONFIG_MQTTCOMM_USE_THREAD

// END   ------------------ Public API (MULTITASKING)------------------

// BEGIN ------------------ Public API (COMMON + SPIN)------------------

MQTTComm_return_code_t MQTTComm_setup(mqtt_comm_cfg_t *cfg)
{
    // Init liviano; no arranca tarea.
    ESP_LOGD(TAG, "setup()");
    if (MQTTComm_dre.initialized)
    {
        ESP_LOGW(TAG, "MQTT already initialized");
        return MQTTComm_ret_error;
    }
    else
    {
        MQTTComm_dre.cfg.f_cfg_cb = cfg->f_cfg_cb;
        MQTTComm_dre.cfg.f_req_cb = cfg->f_req_cb;
        MQTTComm_dre.cfg.f_data_cb = cfg->f_data_cb;

        snprintf(MQTTComm_dre.org, sizeof(MQTTComm_dre.org), "%s", CONFIG_MQTT_TOPIC_ORGANIZATION);
        snprintf(MQTTComm_dre.api_version, sizeof(MQTTComm_dre.api_version), "%s", CONFIG_MQTT_TOPIC_APIVERSION);
        snprintf(MQTTComm_dre.site, sizeof(MQTTComm_dre.site), "%s", CONFIG_MQTT_TOPIC_SITE);

        if (CONFIG_MQTT_TOPIC_DEVICE_USE_WIFI_MAC)
        {
            uint8_t mac[6] = {0};
            if (esp_read_mac(mac, ESP_MAC_WIFI_STA) == ESP_OK)
            {
                snprintf(MQTTComm_dre.device, sizeof(MQTTComm_dre.device),
                         "%02X%02X%02X%02X%02X%02X",
                         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            }
            else
            {
                ESP_LOGW(TAG, "Could not read Wi-Fi MAC; falling back to configured device name");
                snprintf(MQTTComm_dre.device, sizeof(MQTTComm_dre.device), "%s", CONFIG_MQTT_TOPIC_DEVICE_NAME);
            }
        }
        else
        {
            snprintf(MQTTComm_dre.device, sizeof(MQTTComm_dre.device), "%s", CONFIG_MQTT_TOPIC_DEVICE_NAME);
        }

        if (CONFIG_MQTT_CLIENT_ID_USE_DEVICE)
        {
            snprintf(MQTTComm_dre.client_id, sizeof(MQTTComm_dre.client_id), "%s", MQTTComm_dre.device);
        }
        else
        {
            if (strlen(CONFIG_MQTT_CLIENT_ID_CUSTOM) > 0)
            {
                snprintf(MQTTComm_dre.client_id, sizeof(MQTTComm_dre.client_id), "%s", CONFIG_MQTT_CLIENT_ID_CUSTOM);
            }
            else
            {
                snprintf(MQTTComm_dre.client_id, sizeof(MQTTComm_dre.client_id), "%s", MQTTComm_dre.device);
            }
        }

        snprintf(MQTTComm_dre.cfg_topic, sizeof(MQTTComm_dre.cfg_topic), "%s/%s/%s/%s/cfg",
                 MQTTComm_dre.org, MQTTComm_dre.api_version, MQTTComm_dre.site, MQTTComm_dre.device);
        snprintf(MQTTComm_dre.req_topic, sizeof(MQTTComm_dre.req_topic), "%s/%s/%s/%s/cmd",
                 MQTTComm_dre.org, MQTTComm_dre.api_version, MQTTComm_dre.site, MQTTComm_dre.device);
        snprintf(MQTTComm_dre.data_topic, sizeof(MQTTComm_dre.data_topic), "%s/%s/%s/%s/data",
                 MQTTComm_dre.org, MQTTComm_dre.api_version, MQTTComm_dre.site, MQTTComm_dre.device);

#if CONFIG_MQTTCOMM_USE_THREAD
        if (_create_mutex_once() != pdPASS)
        {
            ESP_LOGE(TAG, "mutex creation failed");
            return MQTTComm_ret_error;
        }
#endif
        mqtt_app_start();
        MQTTComm_dre.initialized = true;
        MQTTComm_dre.last_return_code = MQTTComm_ret_ok;
    }

    return MQTTComm_ret_ok;
}

#if CONFIG_MQTTCOMM_USE_THREAD
static // In case we are using a thread, this function should not be part of the public API
#endif
    MQTTComm_return_code_t MQTTComm_spin(void)
{
    ESP_LOGI(TAG, "Spinning");
#if CONFIG_MQTTCOMM_USE_THREAD
    _lock();
#endif
    static char data_buffer[MAX_MQTT_COMM_PAYLOAD_LEN];
    static int len;

    bool en = MQTTComm_dre.enabled;

    if (!en)
    {
#if CONFIG_MQTTCOMM_USE_THREAD        
        _unlock();
#endif
        return MQTTComm_ret_ok;
    }
    else
    {
        MQTTComm_return_code_t ret;
        int msg_id = -1;
        // Do your stuff here
        // This is the periodic data publication loop
        if (MQTTComm_dre.cfg.f_cfg_cb != NULL)
        {
            MQTTComm_dre.cfg.f_data_cb(data_buffer, &len);
            data_buffer[len] = '\0';
            msg_id = esp_mqtt_client_publish(MQTTComm_dre.main_client, MQTTComm_dre.data_topic, data_buffer, 0, 1, 0);
            if (msg_id >= 0)
            {
                ret = MQTTComm_ret_ok;
            }
            else
            {
                ret = MQTTComm_ret_error;
            }
        }
        else
        {
            ret = MQTTComm_ret_error;
        }

#if CONFIG_MQTTCOMM_USE_THREAD        
        // Unlocking after the protected data has been managed for this cycle
        _unlock();
#endif
        MQTTComm_nvs_spin();
        // Process the output messages
        if (ret == MQTTComm_ret_ok)
        {
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        }
        else
        {
            ESP_LOGW(TAG, "callback function for loading the data is not given");
        }
        return ret;
    }
}

MQTTComm_return_code_t MQTTComm_enable(void)
{
#if CONFIG_MQTTCOMM_USE_THREAD
    _lock();
#endif
    MQTTComm_dre.enabled = true;
    MQTTComm_dre.last_return_code = MQTTComm_ret_ok;
#if CONFIG_MQTTCOMM_USE_THREAD
    _unlock();
#endif
    return MQTTComm_ret_ok;
}

MQTTComm_return_code_t MQTTComm_disable(void)
{
#if CONFIG_MQTTCOMM_USE_THREAD
    _lock();
#endif
    MQTTComm_dre.enabled = false;
    MQTTComm_dre.last_return_code = MQTTComm_ret_ok;
#if CONFIG_MQTTCOMM_USE_THREAD
    _unlock();
#endif
    return MQTTComm_ret_ok;
}

// BEGIN ------------------ Public API (COMMON)------------------
