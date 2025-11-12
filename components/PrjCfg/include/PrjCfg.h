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

// ------------------ END   Datatypes ------------------

// ------------------ BEGIN DRE ------------------
typedef struct {
    bool enabled;
    PrjCfg_return_code_t last_return_code;

    uint8_t eth_mac[PRJCFG_MAC_LEN];
    char unique_id[PRJCFG_UNIQUEID_MAX_LEN];
    char ip_address[PRJCFG_IPADDRESS_MAX_LEN];
    esp_ip4_addr_t ip;
    char ssid[PRJCFG_SSID_MAX_LEN];
    bool wifi_connected;

#ifdef CONFIG_PORIS_ENABLE_BLECENTRAL
    bool central_role;
#endif
#ifdef CONFIG_PORIS_ENABLE_BLEPERIPHERAL
    bool peripheral_role;
    bool echo;
#endif
#ifdef CONFIG_PORIS_ENABLE_OTA
    bool skip_ota;
#endif
#ifdef CONFIG_PORIS_ENABLE_UARTBRIDGE
    bool uart_bridge;
#endif

#ifdef CONFIG_PORIS_ENABLE_UARTUSER
    // Uart to User configuration
    uart_port_t uart_user_port;             // porn number
    int uart_user_baudrate;                 // baud rate
    uart_stop_bits_t uart_user_stop_bits;   /*!< UART stop bits*/
    gpio_num_t uart_user_tx_io_num;                // -1 for keeping hardware defaults
    gpio_num_t uart_user_rx_io_num;                // -1 for keeping hardware defaults
#endif

#ifdef CONFIG_PORIS_ENABLE_UARTPERIPH
    // Uart to Peripheral configuration
    uart_port_t uart_periph_port;           // porn number
    int uart_periph_baudrate;
    uart_stop_bits_t uart_periph_stop_bits; /*!< UART stop bits*/
    gpio_num_t uart_periph_tx_io_num;              // -1 for keeping hardware defaults
    gpio_num_t uart_periph_rx_io_num;              // -1 for keeping hardware defaults
#endif

#ifdef CONFIG_PORIS_ENABLE_BLINK
    // Blink functionality
    bool blink_enabled;                     // true if blink shall execute
    gpio_num_t blink_io_num;                       // pin number for blink pin
    bool blink_on_value;                    // LOW or HIGH for lighting the LED
    uint32_t blink_on_ms;                        // blinking time in ON
    uint32_t blink_off_ms;                       // blinking time in OFF
#endif

#ifdef CONFIG_PORIS_ENABLE_BUTTON
    // Button functionality
    bool button_enabled;                    // true if button read shall execute
    gpio_num_t button_io_num;                      // pin number for reading the button
    bool button_pullup;                     // true if button read uses internal pullup
    uint32_t button_filter_ms;                   // debouncing time for button
    gpio_num_t button_notification_io_num;              // pin number to notify when the button is pressed
    bool button_notification_value;         // LOW or HIGH for notifying button pressed
#endif

#ifdef CONFIG_PORIS_ENABLE_SERVO
    // Servo driver functionality
    bool servo_enabled;                     // true if servo shall execute
#endif

#ifdef CONFIG_PORIS_ENABLE_HOVER
    // HoverWheels driver functionality
    bool hover_enabled;                             // true if hoverwheels shall execute
    uart_port_t uart_hover_port;                    // porn number
    gpio_num_t uart_hover_tx_io_num;                // -1 for keeping hardware defaults
    gpio_num_t uart_hover_rx_io_num;                // -1 for keeping hardware defaults
#endif

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
