#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef enum {
    $$1_ret_error = -1,
    $$1_ret_ok = 0
}$$1_return_code;

typedef struct {
    bool enabled;
    $$1_return_code last_return_code;
}$$1_dre;

/**
 *  This is to be called on initialization
 */
$$1_return_code $$1_setup(void);

/**
 * This is not blocking, an execution step for this module
 */
$$1_return_code $$1_spin(void);

/**
 * In case you want to use a separate thread, use this function
 */
$$1_return_code $$1_start(void);

/**
 * This is a thread-safe function to get a clone of the $$1_dre.
 */
$$1_return_code $$1_get_dre_clone($$1_dre *$$1_dre_destination);

/**
 * This is a thread-safe function to enable $$1.
 */
$$1_return_code $$1_enable(void);

/**
 * This is a thread-safe function to disable $$1.
 */
$$1_return_code $$1_disable(void);


#ifdef __cplusplus
}
#endif