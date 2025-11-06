#include <stdio.h>
#include <PrjCfg.h>

#include "include/HelloWorld.h"

#ifdef CONFIG_PORIS_ENABLE_HELLOWORLD
// Optional: code that depends on Kconfig switch

/**
 *  This is to be called on initialization
 */
HelloWorld_return_code HelloWorld_setup(void)
{
#error "Unimplemented"
    return HelloWorld_ret_error;
}

/**
 * This is not blocking, an execution step for this module
 */
HelloWorld_return_code HelloWorld_spin(void)
{
#error "Unimplemented"
    return HelloWorld_ret_error;
}

/**
 * In case you want to use a separate thread, use this function
 */
HelloWorld_return_code HelloWorld_start(void)
{
#error "Unimplemented"
    return HelloWorld_ret_error;
}

/**
 * This is a thread-safe function to get a clone of the HelloWorld_dre.
 */
HelloWorld_return_code HelloWorld_get_dre_clone(HelloWorld_dre *HelloWorld_dre_destination)
{
#error "Unimplemented"
    return HelloWorld_ret_error;
}

/**
 * This is a thread-safe function to enable HelloWorld.
 */
HelloWorld_return_code HelloWorld_enable(void)
{
#error "Unimplemented"
    return HelloWorld_ret_error;
}

/**
 * This is a thread-safe function to disable HelloWorld.
 */
HelloWorld_return_code HelloWorld_disable(void)
{
#error "Unimplemented"
    return HelloWorld_ret_error;
}

#endif