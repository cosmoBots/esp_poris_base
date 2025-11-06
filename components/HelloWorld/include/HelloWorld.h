#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef enum {
    HelloWorld_ret_error = -1,
    HelloWorld_ret_ok = 0
}HelloWorld_return_code;

typedef struct {
    bool enabled;
    HelloWorld_return_code last_return_code;
}HelloWorld_dre;

/**
 *  This is to be called on initialization
 */
HelloWorld_return_code HelloWorld_setup(void);

/**
 * This is not blocking, an execution step for this module
 */
HelloWorld_return_code HelloWorld_spin(void);

/**
 * In case you want to use a separate thread, use this function
 */
HelloWorld_return_code HelloWorld_start(void);

/**
 * This is a thread-safe function to get a clone of the HelloWorld_dre.
 */
HelloWorld_return_code HelloWorld_get_dre_clone(HelloWorld_dre *HelloWorld_dre_destination);

/**
 * This is a thread-safe function to enable HelloWorld.
 */
HelloWorld_return_code HelloWorld_enable(void);

/**
 * This is a thread-safe function to disable HelloWorld.
 */
HelloWorld_return_code HelloWorld_disable(void);


#ifdef __cplusplus
}
#endif