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
    DualLedTester_ret_error = -1,
    DualLedTester_ret_ok    = 0
} DualLedTester_return_code_t;
// ------------------ END   Return code ------------------

// ------------------ BEGIN Datatypes ------------------

// ------------------ END   Datatypes ------------------

// ------------------ BEGIN DRE ------------------
typedef struct {
    bool enabled;
    uint32_t dwell_ms;
    uint32_t duty_on_ms;
    uint32_t duty_off_ms;
    uint32_t seq_index;
    TickType_t last_change;
    DualLedTester_return_code_t last_return_code;
} DualLedTester_dre_t;

extern DualLedTester_dre_t DualLedTester_dre;
// ------------------ END   DRE ------------------

// ------------------ BEGIN Public API (MULTITASKING)--------------------
#if CONFIG_DUALLEDTESTER_USE_THREAD
/**
 *  Start background task that calls spin() every period.
 *  Idempotent. Returns error if task creation fails.
 */
DualLedTester_return_code_t DualLedTester_start(void);

/**
 *  Stop background task gracefully.
 *  Idempotent. Safe to call if not running.
 */
DualLedTester_return_code_t DualLedTester_stop(void);

/**
 *  Thread-safe clone of current DRE state.
 */
DualLedTester_return_code_t DualLedTester_get_dre_clone(DualLedTester_dre_t *dst);

/**
 *  Change the periodic interval at runtime (ms).
 *  Clamped internamente a >= 10 ms.
 */
DualLedTester_return_code_t DualLedTester_set_period_ms(uint32_t period_ms);

/**
 *  Get current period in ms.
 */
uint32_t DualLedTester_get_period_ms(void);

/**
 *  Execute a function wrapped with locks so you can access the DRE variables in thread-safe mode
*/
void DualLedTester_execute_function_safemode(void (*callback)());


// ------------------ END   Public API (MULTITASKING)--------------------
#else
// ------------------ BEGIN Public API (SPIN)--------------------
/**
 *  Non-blocking step of this module (call it from your scheduler when
 *  CONFIG_DUALLEDTESTER_USE_THREAD = n).
 */
DualLedTester_return_code_t DualLedTester_spin(void);

// ------------------ END   Public API (SPIN)--------------------
#endif // CONFIG_DUALLEDTESTER_USE_THREAD

// ------------------ BEGIN Public API (COMMON)--------------------

/**
 *  Called at initialization time. Does minimal setup.
 */
DualLedTester_return_code_t DualLedTester_setup(void);

/**
 *  Enable/disable from user code (thread-safe if internal thread is enabled).
 */
DualLedTester_return_code_t DualLedTester_enable(void);
DualLedTester_return_code_t DualLedTester_disable(void);

// ------------------ BEGIN Public API (COMMON)--------------------

#ifdef __cplusplus
}
#endif
