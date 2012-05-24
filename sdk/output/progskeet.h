#ifndef _PROGSKEET_H
#define _PROGSKEET_H

#include <stddef.h>
#include <stdint.h>

/*
 * DLL API
 */
#ifdef _WIN32
#ifdef DLL_EXPORTS
#define DLL_API __declspec(dllexport)
#else /* !DLL_EXPORTS */
#define DLL_API __declspec(dllimport)
#endif /* DLL_EXPORTS */
#else /* !_WIN32 */
#define DLL_API
#endif /* _WIN32 */

/*
 * CAUTION: ANYTHING IS POSSIBLE WITH ... THE HANDLE!
 */

struct progskeet_handle;

/*
 * LOGGING FUNCTIONS
 */

enum progskeet_log_level {
    progskeet_log_level_none = 0,
    progskeet_log_level_error,
    progskeet_log_level_info,
    progskeet_log_level_verbose,
    progskeet_log_level_debug,
};

typedef void (*progskeet_log_target)(struct progskeet_handle* handle, const char* string, const enum progskeet_log_level level);

int DLL_API progskeet_log_set_target(struct progskeet_handle* handle, progskeet_log_target target);

int DLL_API progskeet_log_set_global_target(progskeet_log_target target);

const char* DLL_API progskeet_log_get_level_name(const enum progskeet_log_level level);

/*
 * COMMUNICATION FUNCTIONS
 */

int DLL_API progskeet_init();

int DLL_API progskeet_open(struct progskeet_handle** handle);

int DLL_API progskeet_close(struct progskeet_handle* handle);

int DLL_API progskeet_reset(struct progskeet_handle* handle);

/* Sends until the TX buffer is empty */
int DLL_API progskeet_sync(struct progskeet_handle* handle);

/* Cancels any running operation */
int DLL_API progskeet_cancel(struct progskeet_handle* handle);

int DLL_API progskeet_enqueue_tx(struct progskeet_handle* handle, char data);

int DLL_API progskeet_enqueue_tx_buf(struct progskeet_handle* handle, const char* buf, const size_t len);

int DLL_API progskeet_enqueue_rx_buf(struct progskeet_handle* handle, void* addr, size_t len);

/*
 * LOWLEVEL FUNCTIONS
 */

int DLL_API progskeet_set_gpio_dir(struct progskeet_handle* handle, const uint16_t dir);

int DLL_API progskeet_set_gpio(struct progskeet_handle* handle, const uint16_t gpio);

int DLL_API progskeet_get_gpio(struct progskeet_handle* handle, uint16_t* gpio);

/* Waits for the the trigger conditions to be satisfied on the GPIO inputs */
int DLL_API progskeet_wait_gpio(struct progskeet_handle* handle, const uint16_t mask, const uint16_t value);

int DLL_API progskeet_assert_gpio(struct progskeet_handle* handle, const uint16_t gpio);

int DLL_API progskeet_deassert_gpio(struct progskeet_handle* handle, const uint16_t gpio);

int DLL_API progskeet_set_addr(struct progskeet_handle* handle, const uint32_t addr, int auto_incr);

int DLL_API progskeet_set_data(struct progskeet_handle* handle, const uint16_t data);

int DLL_API progskeet_set_config(struct progskeet_handle* handle, const uint8_t delay, const int word);

int DLL_API progskeet_write(struct progskeet_handle* handle, const char* buf, const size_t len);

int DLL_API progskeet_read(struct progskeet_handle* handle, char* buf, const size_t len);

int DLL_API progskeet_write_addr(struct progskeet_handle* handle, uint32_t addr, uint16_t data);

int DLL_API progskeet_read_addr(struct progskeet_handle* handle, uint32_t addr, uint16_t *data);

/* Does nothing by the specified ammount, 48 nops are 1us */
int DLL_API progskeet_nop(struct progskeet_handle* handle, const uint32_t amount);

/*
 * UTILITY FUNCTIONS
 */

/* Waits for the specified ammount of nanoseconds */
int DLL_API progskeet_wait_ns(struct progskeet_handle* handle, const uint32_t ns);

/* Waits for the specified ammount of microseconds */
int DLL_API progskeet_wait_us(struct progskeet_handle* handle, const uint32_t us);

/* Waits for the specified ammount of milliseconds */
int DLL_API progskeet_wait_ms(struct progskeet_handle* handle, const uint32_t ms);

/* Waits for the specified ammount of seconds */
int DLL_API progskeet_wait(struct progskeet_handle* handle, const uint32_t seconds);

/*
 * TODO: NOR / NAND FUNCTIONS
 */

#endif /* _PROGSKEET_H */
