#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// ------------------ BEGIN Return code ------------------
typedef enum {
    PrjCfg_ret_error = -1,
    PrjCfg_ret_ok    = 0
} PrjCfg_return_code_t;
// ------------------ END   Return code ------------------

// ------------------ BEGIN Datatypes ------------------

// ------------------ END   Datatypes ------------------

// ------------------ BEGIN DRE ------------------
typedef struct {
    bool enabled;
    PrjCfg_return_code_t last_return_code;
} PrjCfg_dre_t;
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
