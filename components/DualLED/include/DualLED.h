#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include <PrjCfg.h>

// ------------------ BEGIN Return code ------------------
typedef enum {
    DualLED_ret_error = -1,
    DualLED_ret_ok    = 0
} DualLED_return_code_t;
// ------------------ END   Return code ------------------

// ------------------ BEGIN Datatypes ------------------

// ------------------ END   Datatypes ------------------

// ------------------ BEGIN DRE ------------------
typedef struct {
    bool enabled;
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

// ------------------ BEGIN Public API (COMMON)--------------------

#ifdef __cplusplus
}
#endif
