#ifndef _PROGSKEET_H
#define _PROGSKEET_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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
 * DEFINES
 */

#define PROGSKEET_VERSION "0.0.1"

/*
 * CAUTION: ANYTHING IS POSSIBLE WITH ... THE HANDLE!
 */

struct progskeet_handle;

struct progskeet_config
{
    /* 1/48 us */
    uint8_t delay;

    int is16bit;

    int differential;
    int verify;
    int byte_swap;
    int abort_on_error;
};

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

int DLL_API progskeet_open_specific(struct progskeet_handle** handle, uint8_t bus, uint8_t addr);

int DLL_API progskeet_close(struct progskeet_handle* handle);

int DLL_API progskeet_reset(struct progskeet_handle* handle);

/* Cancels any running operation */
int DLL_API progskeet_cancel(struct progskeet_handle* handle);

/*
 * UTILITY FUNCTIONS
 */

int DLL_API progskeet_testshorts(struct progskeet_handle* handle, uint32_t* result);

#ifdef __cplusplus
}
#endif

#endif /* _PROGSKEET_H */
