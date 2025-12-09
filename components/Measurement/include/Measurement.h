#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include <PrjCfg.h>

// ------------------ BEGIN Return code ------------------
typedef enum {
    Measurement_ret_error = -1,
    Measurement_ret_ok    = 0
} Measurement_return_code_t;
// ------------------ END   Return code ------------------

// ------------------ BEGIN Datatypes ------------------

// ------------------ END   Datatypes ------------------

// ------------------ BEGIN DRE ------------------
typedef struct {
    bool enabled;
    Measurement_return_code_t last_return_code;
#include "Measurement_netvar_types_fragment.h_"
} Measurement_dre_t;

extern Measurement_dre_t Measurement_dre;
// ------------------ END   DRE ------------------

// ------------------ BEGIN Public API (MULTITASKING)--------------------
#if CONFIG_MEASUREMENT_USE_THREAD
/**
 *  Start background task that calls spin() every period.
 *  Idempotent. Returns error if task creation fails.
 */
Measurement_return_code_t Measurement_start(void);

/**
 *  Stop background task gracefully.
 *  Idempotent. Safe to call if not running.
 */
Measurement_return_code_t Measurement_stop(void);

/**
 *  Thread-safe clone of current DRE state.
 */
Measurement_return_code_t Measurement_get_dre_clone(Measurement_dre_t *dst);

/**
 *  Change the periodic interval at runtime (ms).
 *  Clamped internamente a >= 10 ms.
 */
Measurement_return_code_t Measurement_set_period_ms(uint32_t period_ms);

/**
 *  Get current period in ms.
 */
uint32_t Measurement_get_period_ms(void);

/**
 *  Execute a function wrapped with locks so you can access the DRE variables in thread-safe mode
*/
void Measurement_execute_function_safemode(void (*callback)());


// ------------------ END   Public API (MULTITASKING)--------------------
#else
// ------------------ BEGIN Public API (SPIN)--------------------
/**
 *  Non-blocking step of this module (call it from your scheduler when
 *  CONFIG_MEASUREMENT_USE_THREAD = n).
 */
Measurement_return_code_t Measurement_spin(void);

// ------------------ END   Public API (SPIN)--------------------
#endif // CONFIG_MEASUREMENT_USE_THREAD

// ------------------ BEGIN Public API (COMMON)--------------------

/**
 *  Called at initialization time. Does minimal setup.
 */
Measurement_return_code_t Measurement_setup(void);

/**
 *  Enable/disable from user code (thread-safe if internal thread is enabled).
 */
Measurement_return_code_t Measurement_enable(void);
Measurement_return_code_t Measurement_disable(void);

// ------------------ BEGIN Public API (COMMON)--------------------

#ifdef __cplusplus
}
#endif
