#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include <PrjCfg.h>
#include <freertos/FreeRTOS.h>

// ------------------ BEGIN Return code ------------------
typedef enum {
    DualLED_ret_error = -1,
    DualLED_ret_ok    = 0
} DualLED_return_code_t;
// ------------------ END   Return code ------------------

// ------------------ BEGIN Datatypes ------------------
typedef enum {
    DUALLED_OFF,
    DUALLED_GREEN,
    DUALLED_RED,
    DUALLED_BLINK_GREEN,
    DUALLED_BLINK_RED,
    DUALLED_ALTERNATE_START_GREEN,
    DUALLED_ALTERNATE_START_RED,
    // Los opcionales al final, por conveniencia.  Aún podemos hacerlo ya que no tenemos nada en producción
#if CONFIG_DUALLED_ALLOW_BOTH
    DUALLED_BOTH_COLORS,
    DUALLED_BLINK_BOTH,
#endif    
    DUALLED_STATE_TERMINATOR
} dual_led_state_t;

// ------------------ END   Datatypes ------------------

// ------------------ BEGIN DRE ------------------
typedef struct {
    bool enabled;
#include "DualLED_netvar_types_fragment.h_"
    dual_led_state_t prev_state;
    TickType_t last_toggle;
    bool hw_init;
    DualLED_return_code_t last_return_code;

} DualLED_dre_t;

extern DualLED_dre_t DualLED_dre;
// ------------------ END   DRE ------------------

// ------------------ BEGIN Public API (MULTITASKING)--------------------
#if CONFIG_DUALLED_USE_THREAD
/**
 *  Start background task that calls spin() every period.
 *  Idempotent. Returns error if task creation fails.
 */
DualLED_return_code_t DualLED_start(void);

/**
 *  Stop background task gracefully.
 *  Idempotent. Safe to call if not running.
 */
DualLED_return_code_t DualLED_stop(void);

/**
 *  Thread-safe clone of current DRE state.
 */
DualLED_return_code_t DualLED_get_dre_clone(DualLED_dre_t *dst);

/**
 *  Change the periodic interval at runtime (ms).
 *  Clamped internamente a >= 10 ms.
 */
DualLED_return_code_t DualLED_set_period_ms(uint32_t period_ms);

/**
 *  Get current period in ms.
 */
uint32_t DualLED_get_period_ms(void);

/**
 *  Execute a function wrapped with locks so you can access the DRE variables in thread-safe mode
*/
void DualLED_execute_function_safemode(void (*callback)());


// ------------------ END   Public API (MULTITASKING)--------------------
#else
// ------------------ BEGIN Public API (SPIN)--------------------
/**
 *  Non-blocking step of this module (call it from your scheduler when
 *  CONFIG_DUALLED_USE_THREAD = n).
 */
DualLED_return_code_t DualLED_spin(void);

// ------------------ END   Public API (SPIN)--------------------
#endif // CONFIG_DUALLED_USE_THREAD

// ------------------ BEGIN Public API (COMMON)--------------------

/**
 *  Called at initialization time. Does minimal setup.
 */
DualLED_return_code_t DualLED_setup(void);

/**
 *  Enable/disable from user code (thread-safe if internal thread is enabled).
 */
DualLED_return_code_t DualLED_enable(void);
DualLED_return_code_t DualLED_disable(void);

/**
 * Set current LED state/behavior.
 */
void DualLED_set_state(dual_led_state_t newstate);

/**
 * Configure duty/periods for blinking/alternating states.
 * - For blinking states, on_duration_ms is the time LEDs stay ON.
 * - For alternating states, on_duration_ms is the time the starting LED stays ON.
 * - In DUALLED_ALTERNATE_START_GREEN the starting LED is GREEN; in DUALLED_ALTERNATE_START_RED it is RED.
 */
void DualLED_set_duty(uint32_t on_duration_ms, uint32_t off_alternate_duration_ms);

// ------------------ BEGIN Public API (COMMON)--------------------

#ifdef __cplusplus
}
#endif
