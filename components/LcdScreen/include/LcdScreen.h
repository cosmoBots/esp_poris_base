#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include <PrjCfg.h>

// ------------------ BEGIN Return code ------------------
typedef enum {
    LcdScreen_ret_error = -1,
    LcdScreen_ret_ok    = 0
} LcdScreen_return_code_t;
// ------------------ END   Return code ------------------

// ------------------ BEGIN Datatypes ------------------

// ------------------ END   Datatypes ------------------

// ------------------ BEGIN DRE ------------------
typedef struct {
    bool enabled;
    LcdScreen_return_code_t last_return_code;

#include "LcdScreen_netvar_types_fragment.h_"
} LcdScreen_dre_t;

extern LcdScreen_dre_t LcdScreen_dre;
// ------------------ END   DRE ------------------

// ------------------ BEGIN Public API (MULTITASKING)--------------------
#if CONFIG_LCDSCREEN_USE_THREAD
/**
 *  Start background task that calls spin() every period.
 *  Idempotent. Returns error if task creation fails.
 */
LcdScreen_return_code_t LcdScreen_start(void);

/**
 *  Stop background task gracefully.
 *  Idempotent. Safe to call if not running.
 */
LcdScreen_return_code_t LcdScreen_stop(void);

/**
 *  Thread-safe clone of current DRE state.
 */
LcdScreen_return_code_t LcdScreen_get_dre_clone(LcdScreen_dre_t *dst);

/**
 *  Change the periodic interval at runtime (ms).
 *  Clamped internamente a >= 10 ms.
 */
LcdScreen_return_code_t LcdScreen_set_period_ms(uint32_t period_ms);

/**
 *  Get current period in ms.
 */
uint32_t LcdScreen_get_period_ms(void);

/**
 *  Execute a function wrapped with locks so you can access the DRE variables in thread-safe mode
*/
void LcdScreen_execute_function_safemode(void (*callback)());


// ------------------ END   Public API (MULTITASKING)--------------------
#else
// ------------------ BEGIN Public API (SPIN)--------------------
/**
 *  Non-blocking step of this module (call it from your scheduler when
 *  CONFIG_LCDSCREEN_USE_THREAD = n).
 */
LcdScreen_return_code_t LcdScreen_spin(void);

// ------------------ END   Public API (SPIN)--------------------
#endif // CONFIG_LCDSCREEN_USE_THREAD

// ------------------ BEGIN Public API (COMMON)--------------------

/**
 *  Called at initialization time. Does minimal setup.
 */
LcdScreen_return_code_t LcdScreen_setup(void);

/**
 *  Enable/disable from user code (thread-safe if internal thread is enabled).
 */
LcdScreen_return_code_t LcdScreen_enable(void);
LcdScreen_return_code_t LcdScreen_disable(void);

// ------------------ BEGIN Public API (COMMON)--------------------

#ifdef __cplusplus
}
#endif
