#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// ------------------ BEGIN Return code ------------------
typedef enum {
    OtCoap_ret_error = -1,
    OtCoap_ret_ok    = 0
} OtCoap_return_code_t;
// ------------------ END   Return code ------------------

// ------------------ BEGIN Datatypes ------------------

// ------------------ END   Datatypes ------------------

// ------------------ BEGIN DRE ------------------
typedef struct {
    bool enabled;
    OtCoap_return_code_t last_return_code;
} OtCoap_dre_t;
// ------------------ END   DRE ------------------

// ------------------ BEGIN Public API (MULTITASKING)--------------------
#if CONFIG_OTCOAP_USE_THREAD
/**
 *  Start background task that calls spin() every period.
 *  Idempotent. Returns error if task creation fails.
 */
OtCoap_return_code_t OtCoap_start(void);

/**
 *  Stop background task gracefully.
 *  Idempotent. Safe to call if not running.
 */
OtCoap_return_code_t OtCoap_stop(void);

/**
 *  Thread-safe clone of current DRE state.
 */
OtCoap_return_code_t OtCoap_get_dre_clone(OtCoap_dre_t *dst);

/**
 *  Change the periodic interval at runtime (ms).
 *  Clamped internamente a >= 10 ms.
 */
OtCoap_return_code_t OtCoap_set_period_ms(uint32_t period_ms);

/**
 *  Get current period in ms.
 */
uint32_t OtCoap_get_period_ms(void);


// ------------------ END   Public API (MULTITASKING)--------------------
#else
// ------------------ BEGIN Public API (SPIN)--------------------
/**
 *  Non-blocking step of this module (call it from your scheduler when
 *  CONFIG_OTCOAP_USE_THREAD = n).
 */
OtCoap_return_code_t OtCoap_spin(void);

// ------------------ END   Public API (SPIN)--------------------
#endif // CONFIG_OTCOAP_USE_THREAD

// ------------------ BEGIN Public API (COMMON)--------------------

/**
 *  Called at initialization time. Does minimal setup.
 */
OtCoap_return_code_t OtCoap_setup(void);

/**
 *  Enable/disable from user code (thread-safe if internal thread is enabled).
 */
OtCoap_return_code_t OtCoap_enable(void);
OtCoap_return_code_t OtCoap_disable(void);

// ------------------ BEGIN Public API (COMMON)--------------------

#ifdef __cplusplus
}
#endif
