#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include <mqtt_client.h>

// ------------------ BEGIN Return code ------------------
typedef enum {
    MQTTComm_ret_error = -1,
    MQTTComm_ret_ok    = 0
} MQTTComm_return_code_t;
// ------------------ END   Return code ------------------

// ------------------ BEGIN Datatypes ------------------
#define MAX_MQTT_COMM_TOPIC_LEN 256
#define MAX_MQTT_COMM_PAYLOAD_LEN 512

typedef struct mqtt_comm_cfg_t
{
    char cfg_topic[MAX_MQTT_COMM_TOPIC_LEN];
    void (*f_cfg_cb)(const char *, int);    // Callback function for processing the requests
    char req_topic[MAX_MQTT_COMM_TOPIC_LEN];
    void (*f_req_cb)(const char *, int);    // Callback function for processing the configurations
    char data_topic[MAX_MQTT_COMM_TOPIC_LEN];
    void (*f_data_cb)(char *, int *);    // Callback function for creating the data payload for publishing
    char *cfg_client_id;
} mqtt_comm_cfg_t;

// ------------------ END   Datatypes ------------------

// ------------------ BEGIN DRE ------------------
typedef struct {
    bool enabled;
    bool initialized;
    bool started;
    mqtt_comm_cfg_t cfg;
    MQTTComm_return_code_t last_return_code;
    esp_mqtt_client_handle_t main_client;
} MQTTComm_dre_t;

extern MQTTComm_dre_t MQTTComm_dre;
// ------------------ END   DRE ------------------

// ------------------ BEGIN Public API (MULTITASKING)--------------------
#if CONFIG_MQTTCOMM_USE_THREAD
/**
 *  Start background task that calls spin() every period.
 *  Idempotent. Returns error if task creation fails.
 */
MQTTComm_return_code_t MQTTComm_start(void);

/**
 *  Stop background task gracefully.
 *  Idempotent. Safe to call if not running.
 */
MQTTComm_return_code_t MQTTComm_stop(void);

/**
 *  Thread-safe clone of current DRE state.
 */
MQTTComm_return_code_t MQTTComm_get_dre_clone(MQTTComm_dre_t *dst);

/**
 *  Change the periodic interval at runtime (ms).
 *  Clamped internamente a >= 10 ms.
 */
MQTTComm_return_code_t MQTTComm_set_period_ms(uint32_t period_ms);

/**
 *  Get current period in ms.
 */
uint32_t MQTTComm_get_period_ms(void);

/**
 *  Execute a function wrapped with locks so you can access the DRE variables in thread-safe mode
*/
void MQTTComm_execute_function_safemode(void (*callback)());


// ------------------ END   Public API (MULTITASKING)--------------------
#else
// ------------------ BEGIN Public API (SPIN)--------------------
/**
 *  Non-blocking step of this module (call it from your scheduler when
 *  CONFIG_MQTTCOMM_USE_THREAD = n).
 */
MQTTComm_return_code_t MQTTComm_spin(void);

// ------------------ END   Public API (SPIN)--------------------
#endif // CONFIG_MQTTCOMM_USE_THREAD

// ------------------ BEGIN Public API (COMMON)--------------------

/**
 *  Called at initialization time. Does minimal setup.
 */
MQTTComm_return_code_t MQTTComm_setup(mqtt_comm_cfg_t *cfg);

/**
 *  Enable/disable from user code (thread-safe if internal thread is enabled).
 */
MQTTComm_return_code_t MQTTComm_enable(void);
MQTTComm_return_code_t MQTTComm_disable(void);

// ------------------ BEGIN Public API (COMMON)--------------------

#ifdef __cplusplus
}
#endif
