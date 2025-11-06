#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef enum {
    OtCoap_ret_error = -1,
    OtCoap_ret_ok = 0
}OtCoap_return_code;

typedef struct {
    bool enabled;
    OtCoap_return_code last_return_code;
}OtCoap_dre;

/**
 *  This is to be called on initialization
 */
OtCoap_return_code OtCoap_setup(void);

/**
 * This is not blocking, an execution step for this module
 */
OtCoap_return_code OtCoap_spin(void);

/**
 * In case you want to use a separate thread, use this function
 */
OtCoap_return_code OtCoap_start(void);

/**
 * This is a thread-safe function to get a clone of the OtCoap_dre.
 */
OtCoap_return_code OtCoap_get_dre_clone(OtCoap_dre *OtCoap_dre_destination);

/**
 * This is a thread-safe function to enable OtCoap.
 */
OtCoap_return_code OtCoap_enable(void);

/**
 * This is a thread-safe function to disable OtCoap.
 */
OtCoap_return_code OtCoap_disable(void);


#ifdef __cplusplus
}
#endif