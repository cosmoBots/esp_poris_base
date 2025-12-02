#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// BEGIN --- Standard C headers section ---
#define max(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})

#define min(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})

#include <stdbool.h>

// END   --- Standard C headers section ---

// BEGIN --- SDK config section---
#include <sdkconfig.h>
// END   --- SDK config section---

// BEGIN --- FreeRTOS headers section ---

// END   --- FreeRTOS headers section ---


// BEGIN --- ESP-IDF headers section ---
#include <esp_netif.h>
#include <driver/uart.h>
#include <driver/gpio.h>
// END   --- ESP-IDF headers section ---


// ------------------ BEGIN Return code ------------------
typedef enum {
    PrjCfg_ret_error = -1,
    PrjCfg_ret_ok    = 0
} PrjCfg_return_code_t;
// ------------------ END   Return code ------------------

// ------------------ BEGIN Datatypes ------------------

#define PRJCFG_MAC_LEN (6)
#define PRJCFG_UNIQUEID_BASE_LEN (0)
#define PRJCFG_UNIQUEID_MAX_LEN (PRJCFG_UNIQUEID_BASE_LEN + (PRJCFG_MAC_LEN * 2) + 1)
#define PRJCFG_IPADDRESS_MAX_LEN ((4 * 4) + 1)
#define PRJCFG_SSID_MAX_LEN (32 + 1)

#ifdef CONFIG_PRJCFG_VARIANT_KINCONY_BOARD 
#define UART_PORT_TO_USER 0
#define UART_PORT_TO_USER_RXD 3
#define UART_PORT_TO_USER_TXD 1
#define UART_PORT_TO_DEVICE 2
#define UART_PORT_TO_DEVICE_RXD 16
#define UART_PORT_TO_DEVICE_TXD 17
#else
#ifdef CFG_USE_FTDI_CENTRAL
#define UART_PORT_TO_USER UART_NUM_1
#define UART_PORT_TO_USER_RXD 22
#define UART_PORT_TO_USER_TXD 23
#else
#define UART_PORT_TO_USER UART_NUM_0
#define UART_PORT_TO_USER_RXD UART_PIN_NO_CHANGE
#define UART_PORT_TO_USER_TXD UART_PIN_NO_CHANGE
#endif
#ifdef CFG_USE_FTDI_PERIPHERAL
#define UART_PORT_TO_DEVICE UART_NUM_1
#define UART_PORT_TO_DEVICE_RXD 22
#define UART_PORT_TO_DEVICE_TXD 23
#else
#define UART_PORT_TO_DEVICE UART_NUM_1
#define UART_PORT_TO_DEVICE_RXD 22
#define UART_PORT_TO_DEVICE_TXD 23
#endif
#endif

#define UART_BRIDGE_PORT_TO_USER UART_NUM_0
#define UART_BRIDGE_PORT_TO_USER_RXD UART_PIN_NO_CHANGE
#define UART_BRIDGE_PORT_TO_USER_TXD UART_PIN_NO_CHANGE

#define UART_BRIDGE_PORT_TO_DEVICE UART_NUM_1
#define UART_BRIDGE_PORT_TO_DEVICE_RXD 22
#define UART_BRIDGE_PORT_TO_DEVICE_TXD 23

#define UART_PORT_TO_USER_BAUDRATE 38400
#define UART_PORT_TO_DEVICE_BAUDRATE 38400

#define UART_PORT_TO_USER_STOP_BITS UART_STOP_BITS_2
#define UART_PORT_TO_DEVICE_STOP_BITS UART_STOP_BITS_2

#define UART_CFG_DATA_BITS UART_DATA_8_BITS
#define UART_CFG_PARITY UART_PARITY_DISABLE
#define UART_CFG_FLOWCTRL UART_HW_FLOWCTRL_DISABLE

#define MQTT_DATA_PERIODICITY_MS 60000

#define UDP_SERVER_PORT 3001

#ifdef CONFIG_IDF_TARGET_ESP32C6
#define PROV_FORGET_GPIO (9)
#else
#ifdef CONFIG_PRJCFG_VARIANT_ESP32S3_LCD_147
#define PROV_FORGET_GPIO (0)
#define CFG_USE_LCD1_47
#else
#ifdef CONFIG_IDF_TARGET_ESP32C5
#define PROV_FORGET_GPIO (28)
#else
#error "Reprovisioning pin for this target has not been set"
#endif
#endif
#endif

#ifdef CFG_USE_LCD1_47
//#define UART_BRIDGE_USE_TINYUSB
#endif

// Blink driver default values
#define BLINK_DEFAULT_ENABLED false
#define BLINK_DEFAULT_PIN 8
#define BLINK_DEFAULT_ON_MS 500
#define BLINK_DEFAULT_OFF_MS 1000
#define BLINK_DEFAULT_ON_VALUE 1

// HoverWheels driver default values
#define HOVER_DEFAULT_ENABLED false
#define UART_PORT_TO_HOVER UART_NUM_1
#define UART_PORT_TO_HOVER_RXD 5
#define UART_PORT_TO_HOVER_TXD 4

// Func modules
// #define CFG_FUNC_TEST_HOVER

// Debug config
// #define CONFIG_FORCE_CENTRAL_ROLE
// #define CONFIG_FORCE_PERIPHERAL_ROLE
// ------------------ END   Datatypes ------------------

// ------------------ BEGIN DRE ------------------
typedef struct {
    bool enabled;
    PrjCfg_return_code_t last_return_code;

    uint8_t eth_mac[PRJCFG_MAC_LEN];
    char unique_id[PRJCFG_UNIQUEID_MAX_LEN];
    esp_ip4_addr_t ip;
    char ssid[PRJCFG_SSID_MAX_LEN];
    bool wifi_connected;

#include "PrjCfg_netvar_types_fragment.h_"

} PrjCfg_dre_t;

extern PrjCfg_dre_t PrjCfg_dre;
// ------------------ END   DRE ------------------

// ------------------ BEGIN Public API (MULTITASKING)--------------------
#if CONFIG_PRJCFG_USE_THREAD
/**
 *  Start background task that calls spin() every period.
 *  Idempotent. Returns error if task creation fails.
 */
PrjCfg_return_code_t PrjCfg_start(void);

/**
 *  Stop background task gracefully.
 *  Idempotent. Safe to call if not running.
 */
PrjCfg_return_code_t PrjCfg_stop(void);

/**
 *  Thread-safe clone of current DRE state.
 */
PrjCfg_return_code_t PrjCfg_get_dre_clone(PrjCfg_dre_t *dst);

/**
 *  Change the periodic interval at runtime (ms).
 *  Clamped internamente a >= 10 ms.
 */
PrjCfg_return_code_t PrjCfg_set_period_ms(uint32_t period_ms);

/**
 *  Get current period in ms.
 */
uint32_t PrjCfg_get_period_ms(void);

/**
 *  Execute a function wrapped with locks so you can access the DRE variables in thread-safe mode
*/
void PrjCfg_execute_function_safemode(void (*callback)());


// ------------------ END   Public API (MULTITASKING)--------------------
#else
// ------------------ BEGIN Public API (SPIN)--------------------
/**
 *  Non-blocking step of this module (call it from your scheduler when
 *  CONFIG_PRJCFG_USE_THREAD = n).
 */
PrjCfg_return_code_t PrjCfg_spin(void);

// ------------------ END   Public API (SPIN)--------------------
#endif // CONFIG_PRJCFG_USE_THREAD

// ------------------ BEGIN Public API (COMMON)--------------------

/**
 *  Called at initialization time. Does minimal setup.
 */
PrjCfg_return_code_t PrjCfg_setup(void);

/**
 *  Enable/disable from user code (thread-safe if internal thread is enabled).
 */
PrjCfg_return_code_t PrjCfg_enable(void);
PrjCfg_return_code_t PrjCfg_disable(void);

// ------------------ BEGIN Public API (COMMON)--------------------

#ifdef __cplusplus
}
#endif
