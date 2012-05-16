/*
 * ProgSkeet utility functions
 */

#include "progskeet.h"

int progskeet_wait_ns(struct progskeet_handle* handle, const uint32_t ns)
{
    static const double ratio = 20.833333333333333333333333333333;
    return progskeet_nop(handle, (ns / ratio) + 1); /* Better 1 more than one less */
}

int progskeet_wait_us(struct progskeet_handle* handle, const uint32_t us)
{
    return progskeet_nop(handle, us * 48);
}

int progskeet_wait_ms(struct progskeet_handle* handle, const uint32_t ms)
{
    return progskeet_nop(handle, ms * 47940);
}

int progskeet_wait(struct progskeet_handle* handle, const uint32_t seconds)
{
    return progskeet_nop(handle, seconds * 47940000);
}
