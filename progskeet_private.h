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

#ifndef _PROGSKEET_PRIVATE_H
#define _PROGSKEET_PRIVATE_H

/*
 * PRIVATE HANDLE
 */

struct progskeet_handle
{
    /* LibUSB device handle */
    void* hdev;

    /* Transmit buffers */
    char* txbuf;
    size_t txlen;

    /* Receive list */
    struct progskeet_rxloc* rxlist;
    size_t rxlen;

    /* Set to 1 to cancel any running operations */
    int cancel;

    /*
     * Device state cache
     */
    uint16_t cur_gpio;
    uint16_t cur_gpio_dir;
    uint8_t cur_config;

    /*
     * This gets used to mask and add to the address.
     * It's used for such things as the virtual chip enable
     * on Samsung K8Q.
     */
    uint32_t addr_mask;
    uint32_t addr_add;

    progskeet_log_target log_target;

    struct progskeet_config def_config;
};

/*
 * LOG FUNCTIONS
 */

int progskeet_log(struct progskeet_handle* handle, const enum progskeet_log_level level, const char* fmt, ...);

int progskeet_log_global(const enum progskeet_log_level level, const char* fmt, ...);

/*
 * COMMUNICATION FUNCTIONS
 */

/* Sends until the TX buffer is empty */
int DLL_API progskeet_sync(struct progskeet_handle* handle);

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

/* Configuration (progskeet_config_set) */
#define PROGSKEET_CFG_NONE          0x00
#define PROGSKEET_CFG_16BIT         (1 << 4)
#define PROGSKEET_CFG_TRISTATE      (1 << 5)
#define PROGSKEET_CFG_WAIT_RDY      (1 << 6)
/* #define PROGSKEET_CFG_BYTESWAP      (1 << 7) This should be handled in the library */
#define PROGSKEET_CFG_ALL           0xFF

uint8_t DLL_API progskeet_config_from_struct(struct progskeet_config* config);

int DLL_API progskeet_config_set(struct progskeet_handle* handle, struct progskeet_config* config, uint8_t add, uint8_t rem);

int DLL_API progskeet_config_set_default(struct progskeet_handle* handle, struct progskeet_config* config);

int DLL_API progskeet_config_set_byte(struct progskeet_handle* handle, uint8_t config);

int DLL_API progskeet_write(struct progskeet_handle* handle, const char* buf, const size_t len);

int DLL_API progskeet_read(struct progskeet_handle* handle, char* buf, const size_t len);

int DLL_API progskeet_write_addr(struct progskeet_handle* handle, uint32_t addr, uint16_t data);

int DLL_API progskeet_read_addr(struct progskeet_handle* handle, uint32_t addr, uint16_t *data);

/* Does nothing by the specified amount, 48 nops are 1us */
int DLL_API progskeet_nop(struct progskeet_handle* handle, const uint32_t amount);

/*
 * UTILITY FUNCTIONS
 */

/* Waits for the specified amount of nanoseconds */
int DLL_API progskeet_wait_ns(struct progskeet_handle* handle, const uint32_t ns);

/* Waits for the specified amount of microseconds */
int DLL_API progskeet_wait_us(struct progskeet_handle* handle, const uint32_t us);

/* Waits for the specified amount of milliseconds */
int DLL_API progskeet_wait_ms(struct progskeet_handle* handle, const uint32_t ms);

/* Waits for the specified amount of seconds */
int DLL_API progskeet_wait(struct progskeet_handle* handle, const uint32_t seconds);

#endif /* _PROGSKEET_PRIVATE_H */
