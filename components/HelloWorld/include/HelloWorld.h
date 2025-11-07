#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// ------------------ BEGIN Return code ------------------
typedef enum {
    HelloWorld_ret_error = -1,
    HelloWorld_ret_ok    = 0
} HelloWorld_return_code_t;
// ------------------ END   Return code ------------------

// ------------------ BEGIN Datatypes ------------------

// ------------------ END   Datatypes ------------------

// ------------------ BEGIN DRE ------------------
typedef struct {
    bool enabled;
    HelloWorld_return_code_t last_return_code;
} HelloWorld_dre_t;

extern HelloWorld_dre_t HelloWorld_dre;
// ------------------ END   DRE ------------------

// ------------------ BEGIN Public API (MULTITASKING)--------------------
#if CONFIG_HELLOWORLD_USE_THREAD
/**
 *  Start background task that calls spin() every period.
 *  Idempotent. Returns error if task creation fails.
 */
HelloWorld_return_code_t HelloWorld_start(void);

/**
 *  Stop background task gracefully.
 *  Idempotent. Safe to call if not running.
 */
HelloWorld_return_code_t HelloWorld_stop(void);

/**
 *  Thread-safe clone of current DRE state.
 */
HelloWorld_return_code_t HelloWorld_get_dre_clone(HelloWorld_dre_t *dst);

/**
 *  Change the periodic interval at runtime (ms).
 *  Clamped internamente a >= 10 ms.
 */
HelloWorld_return_code_t HelloWorld_set_period_ms(uint32_t period_ms);

/**
 *  Get current period in ms.
 */
uint32_t HelloWorld_get_period_ms(void);

/**
 *  Execute a function wrapped with locks so you can access the DRE variables in thread-safe mode
*/
void HelloWorld_execute_function_safemode(void (*callback)());


// ------------------ END   Public API (MULTITASKING)--------------------
#else
// ------------------ BEGIN Public API (SPIN)--------------------
/**
 *  Non-blocking step of this module (call it from your scheduler when
 *  CONFIG_HELLOWORLD_USE_THREAD = n).
 */
HelloWorld_return_code_t HelloWorld_spin(void);

// ------------------ END   Public API (SPIN)--------------------
#endif // CONFIG_HELLOWORLD_USE_THREAD

// ------------------ BEGIN Public API (COMMON)--------------------

/**
 *  Called at initialization time. Does minimal setup.
 */
HelloWorld_return_code_t HelloWorld_setup(void);

/**
 *  Enable/disable from user code (thread-safe if internal thread is enabled).
 */
HelloWorld_return_code_t HelloWorld_enable(void);
HelloWorld_return_code_t HelloWorld_disable(void);

// ------------------ BEGIN Public API (COMMON)--------------------

#ifdef __cplusplus
}
#endif
