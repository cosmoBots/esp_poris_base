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
    RelaysTest_ret_error = -1,
    RelaysTest_ret_ok    = 0
} RelaysTest_return_code_t;
// ------------------ END   Return code ------------------

// ------------------ BEGIN Datatypes ------------------

// ------------------ END   Datatypes ------------------

// ------------------ BEGIN DRE ------------------
typedef struct {
    bool enabled;
    uint32_t dwell_ms;
    uint32_t relay_index;
    TickType_t last_change;
    RelaysTest_return_code_t last_return_code;
} RelaysTest_dre_t;

extern RelaysTest_dre_t RelaysTest_dre;
// ------------------ END   DRE ------------------

// ------------------ BEGIN Public API (MULTITASKING)--------------------
#if CONFIG_RELAYSTEST_USE_THREAD
/**
 *  Start background task that calls spin() every period.
 *  Idempotent. Returns error if task creation fails.
 */
RelaysTest_return_code_t RelaysTest_start(void);

/**
 *  Stop background task gracefully.
 *  Idempotent. Safe to call if not running.
 */
RelaysTest_return_code_t RelaysTest_stop(void);

/**
 *  Thread-safe clone of current DRE state.
 */
RelaysTest_return_code_t RelaysTest_get_dre_clone(RelaysTest_dre_t *dst);

/**
 *  Change the periodic interval at runtime (ms).
 *  Clamped internamente a >= 10 ms.
 */
RelaysTest_return_code_t RelaysTest_set_period_ms(uint32_t period_ms);

/**
 *  Get current period in ms.
 */
uint32_t RelaysTest_get_period_ms(void);

/**
 *  Execute a function wrapped with locks so you can access the DRE variables in thread-safe mode
*/
void RelaysTest_execute_function_safemode(void (*callback)());


// ------------------ END   Public API (MULTITASKING)--------------------
#else
// ------------------ BEGIN Public API (SPIN)--------------------
/**
 *  Non-blocking step of this module (call it from your scheduler when
 *  CONFIG_RELAYSTEST_USE_THREAD = n).
 */
RelaysTest_return_code_t RelaysTest_spin(void);

// ------------------ END   Public API (SPIN)--------------------
#endif // CONFIG_RELAYSTEST_USE_THREAD

// ------------------ BEGIN Public API (COMMON)--------------------

/**
 *  Called at initialization time. Does minimal setup.
 */
RelaysTest_return_code_t RelaysTest_setup(void);

/**
 *  Enable/disable from user code (thread-safe if internal thread is enabled).
 */
RelaysTest_return_code_t RelaysTest_enable(void);
RelaysTest_return_code_t RelaysTest_disable(void);

/**
 * Configure dwell time (ms) per relay in the test cycle.
 */
void RelaysTest_set_dwell_ms(uint32_t dwell_ms);

// ------------------ BEGIN Public API (COMMON)--------------------

#ifdef __cplusplus
}
#endif
