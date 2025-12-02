#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// ------------------ BEGIN Return code ------------------
typedef enum {
    Provisioning_ret_error = -1,
    Provisioning_ret_ok    = 0
} Provisioning_return_code_t;
// ------------------ END   Return code ------------------

// ------------------ BEGIN Datatypes ------------------

// ------------------ END   Datatypes ------------------

// ------------------ BEGIN DRE ------------------
typedef struct {
    bool enabled;
    Provisioning_return_code_t last_return_code;
} Provisioning_dre_t;

extern Provisioning_dre_t Provisioning_dre;
// ------------------ END   DRE ------------------

// ------------------ BEGIN Public API (MULTITASKING)--------------------
#if CONFIG_PROVISIONING_USE_THREAD
/**
 *  Start background task that calls spin() every period.
 *  Idempotent. Returns error if task creation fails.
 */
Provisioning_return_code_t Provisioning_start(void);

/**
 *  Stop background task gracefully.
 *  Idempotent. Safe to call if not running.
 */
Provisioning_return_code_t Provisioning_stop(void);

/**
 *  Thread-safe clone of current DRE state.
 */
Provisioning_return_code_t Provisioning_get_dre_clone(Provisioning_dre_t *dst);

/**
 *  Change the periodic interval at runtime (ms).
 *  Clamped internamente a >= 10 ms.
 */
Provisioning_return_code_t Provisioning_set_period_ms(uint32_t period_ms);

/**
 *  Get current period in ms.
 */
uint32_t Provisioning_get_period_ms(void);

/**
 *  Execute a function wrapped with locks so you can access the DRE variables in thread-safe mode
*/
void Provisioning_execute_function_safemode(void (*callback)());


// ------------------ END   Public API (MULTITASKING)--------------------
#else
// ------------------ BEGIN Public API (SPIN)--------------------
/**
 *  Non-blocking step of this module (call it from your scheduler when
 *  CONFIG_PROVISIONING_USE_THREAD = n).
 */
Provisioning_return_code_t Provisioning_spin(void);

// ------------------ END   Public API (SPIN)--------------------
#endif // CONFIG_PROVISIONING_USE_THREAD

// ------------------ BEGIN Public API (COMMON)--------------------

/**
 *  Called at initialization time. Does minimal setup.
 */
Provisioning_return_code_t Provisioning_setup(void);

/**
 *  Enable/disable from user code (thread-safe if internal thread is enabled).
 */
Provisioning_return_code_t Provisioning_enable(void);
Provisioning_return_code_t Provisioning_disable(void);

// ------------------ BEGIN Public API (COMMON)--------------------

#ifdef __cplusplus
}
#endif
