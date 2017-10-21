/*
 * libprogskeet - ProgSkeet library
 * Copyright (C) 2012 Axel Gembe <axel@gembe.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/*
 * ProgSkeet lowlevel functions
 */

#include "progskeet.h"
#include "progskeet_private.h"

#include <stdio.h>
#include <stdarg.h>

static progskeet_log_target g_target = NULL;

int progskeet_log_set_target(struct progskeet_handle* handle, progskeet_log_target target)
{
    if (handle == NULL)
        return -1;

    handle->log_target = target;

    return 0;
}

int progskeet_log_set_global_target(progskeet_log_target target)
{
    g_target = target;
}

static progskeet_log_target progskeet_log_get_target(struct progskeet_handle* handle)
{
    if (handle == NULL || handle->log_target == NULL)
        return g_target;

    return handle->log_target;
}

static int progskeet_log_int(struct progskeet_handle* handle, const enum progskeet_log_level level, const char* fmt, va_list argp)
{
    static char buf[(1024 * 8)];
    progskeet_log_target target;

    if ((target = progskeet_log_get_target(handle)) == NULL)
        return -1;

    vsnprintf(buf, sizeof(buf), fmt, argp);

    target(handle, buf, level);

    return 0;
}

int progskeet_log(struct progskeet_handle* handle, const enum progskeet_log_level level, const char* fmt, ...)
{
    va_list argp;
    int res;

    if (handle == NULL)
        return -1;

    va_start(argp, fmt);
    res = progskeet_log_int(handle, level, fmt, argp);
    va_end(argp);

    return res;
}

int progskeet_log_global(const enum progskeet_log_level level, const char* fmt, ...)
{
    va_list argp;
    int res;

    if (g_target == NULL)
        return -1;

    va_start(argp, fmt);
    res = progskeet_log_int(NULL, level, fmt, argp);
    va_end(argp);

    return res;
}

const char* progskeet_log_get_level_name(const enum progskeet_log_level level)
{
    switch (level) {
    case progskeet_log_level_none:
        return "NONE";
    case progskeet_log_level_error:
        return "ERROR";
    case progskeet_log_level_info:
        return "INFO";
    case progskeet_log_level_verbose:
        return "VERBOSE";
    case progskeet_log_level_debug:
        return "DEBUG";
    default:
        return "UNKNOWN";
    }
}
