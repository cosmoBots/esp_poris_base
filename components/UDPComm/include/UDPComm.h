#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include <PrjCfg.h>

// ------------------ BEGIN Return code ------------------
typedef enum {
    UDPComm_ret_error = -1,
    UDPComm_ret_ok    = 0
} UDPComm_return_code_t;
// ------------------ END   Return code ------------------

// ------------------ BEGIN Datatypes ------------------

// ------------------ END   Datatypes ------------------

// ------------------ BEGIN DRE ------------------
typedef struct {
    bool enabled;
    UDPComm_return_code_t last_return_code;
} UDPComm_dre_t;

extern UDPComm_dre_t UDPComm_dre;
// ------------------ END   DRE ------------------

// ------------------ BEGIN Public API (MULTITASKING)--------------------
#if CONFIG_UDPCOMM_USE_THREAD
/**
 *  Start background task that calls spin() every period.
 *  Idempotent. Returns error if task creation fails.
 */
UDPComm_return_code_t UDPComm_start(void);

/**
 *  Stop background task gracefully.
 *  Idempotent. Safe to call if not running.
 */
UDPComm_return_code_t UDPComm_stop(void);

/**
 *  Thread-safe clone of current DRE state.
 */
UDPComm_return_code_t UDPComm_get_dre_clone(UDPComm_dre_t *dst);

/**
 *  Change the periodic interval at runtime (ms).
 *  Clamped internamente a >= 10 ms.
 */
UDPComm_return_code_t UDPComm_set_period_ms(uint32_t period_ms);

/**
 *  Get current period in ms.
 */
uint32_t UDPComm_get_period_ms(void);

/**
 *  Execute a function wrapped with locks so you can access the DRE variables in thread-safe mode
*/
void UDPComm_execute_function_safemode(void (*callback)());


// ------------------ END   Public API (MULTITASKING)--------------------
#else
// ------------------ BEGIN Public API (SPIN)--------------------
/**
 *  Non-blocking step of this module (call it from your scheduler when
 *  CONFIG_UDPCOMM_USE_THREAD = n).
 */
UDPComm_return_code_t UDPComm_spin(void);

// ------------------ END   Public API (SPIN)--------------------
#endif // CONFIG_UDPCOMM_USE_THREAD

// ------------------ BEGIN Public API (COMMON)--------------------

/**
 *  Called at initialization time. Does minimal setup.
 */
UDPComm_return_code_t UDPComm_setup(void);

/**
 *  Enable/disable from user code (thread-safe if internal thread is enabled).
 */
UDPComm_return_code_t UDPComm_enable(void);
UDPComm_return_code_t UDPComm_disable(void);

// ------------------ BEGIN Public API (COMMON)--------------------

#ifdef __cplusplus
}
#endif
