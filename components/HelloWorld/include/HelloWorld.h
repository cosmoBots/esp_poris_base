#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// ------------------ Return codes ------------------
typedef enum {
    HelloWorld_ret_error = -1,
    HelloWorld_ret_ok    = 0
} HelloWorld_return_code;

// ------------------ DRE snapshot ------------------
typedef struct {
    bool enabled;
    HelloWorld_return_code last_return_code;
} HelloWorld_dre;

// ------------------ Public API --------------------

/**
 *  Called at initialization time. Does minimal setup.
 */
HelloWorld_return_code HelloWorld_setup(void);

/**
 *  Non-blocking step of this module (call it from your scheduler when
 *  CONFIG_HELLOWORLD_USE_THREAD = n).
 */
HelloWorld_return_code HelloWorld_spin(void);

/**
 *  Enable/disable from user code (thread-safe if internal thread is enabled).
 */
HelloWorld_return_code HelloWorld_enable(void);
HelloWorld_return_code HelloWorld_disable(void);

#if CONFIG_HELLOWORLD_USE_THREAD
/**
 *  Start background task that calls spin() every period.
 *  Idempotent. Returns error if task creation fails.
 */
HelloWorld_return_code HelloWorld_start(void);

/**
 *  Stop background task gracefully.
 *  Idempotent. Safe to call if not running.
 */
HelloWorld_return_code HelloWorld_stop(void);

/**
 *  Thread-safe clone of current DRE state.
 */
HelloWorld_return_code HelloWorld_get_dre_clone(HelloWorld_dre *dst);

/**
 *  Change the periodic interval at runtime (ms).
 *  Clamped internamente a >= 10 ms.
 */
HelloWorld_return_code HelloWorld_set_period_ms(uint32_t period_ms);

/**
 *  Get current period in ms.
 */
uint32_t HelloWorld_get_period_ms(void);
#endif // CONFIG_HELLOWORLD_USE_THREAD

#ifdef __cplusplus
}
#endif
