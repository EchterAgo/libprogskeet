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

int progskeet_testshorts(struct progskeet_handle* handle, uint32_t* result)
{
    uint16_t g_result[16];
    uint16_t d_result[16];
    int i;

    if (!handle || !result)
        return -1;

    progskeet_set_config(handle, 10, 1);

    for(i = 0; i < 16; i++) {
        progskeet_write_addr(handle, 0, 1 << i);
        progskeet_wait_us(handle, 20);
        progskeet_read(handle, (char*)&d_result[i], 2);
        progskeet_set_gpio_dir(handle, 0xFFFF);
        progskeet_set_gpio(handle, 0);
        progskeet_set_gpio(handle, 0);
        progskeet_set_gpio(handle, 0);
        progskeet_set_gpio(handle, 0);
        progskeet_set_gpio(handle, 0);
        progskeet_set_gpio(handle, 0);
        progskeet_set_gpio(handle, 0);
        progskeet_set_gpio(handle, 0);
        progskeet_set_gpio(handle, 0);
        progskeet_set_gpio(handle, 0);
        progskeet_set_gpio(handle, 0);
        progskeet_set_gpio(handle, 0);
        progskeet_set_gpio(handle, 0);
        progskeet_set_gpio(handle, 0);
        progskeet_set_gpio(handle, 0);
        progskeet_set_gpio(handle, 0);
        progskeet_set_gpio(handle, 0);
        progskeet_set_gpio(handle, 0);
        progskeet_set_gpio(handle, 0);
        progskeet_set_gpio(handle, 0);
        progskeet_set_gpio(handle, 0);
        progskeet_set_gpio(handle, 0);

        progskeet_nop(handle, 255);
        progskeet_nop(handle, 255);

        progskeet_set_gpio(handle, 1 << i);
        progskeet_set_gpio_dir(handle, 1 << i);
        progskeet_wait_us(handle, 20);
        progskeet_set_gpio_dir(handle, 0);
        progskeet_get_gpio(handle, &g_result[i]);
    }

    progskeet_sync(handle);

    for (i = 0; i < 16; i++) {
        if (d_result[i] != (1 << i))
            *result |= (1 << i);
        if (g_result[i] != (1 << i))
            *result |= (1 << (i+16));
    }

    return 0;
}
