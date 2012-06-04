/*
 * ProgSkeet lowlevel functions
 */

#include "progskeet.h"
#include "progskeet_private.h"

/* Command codes */
#define PROGSKEET_CMD_GET_GPIO      0x01
#define PROGSKEET_CMD_SET_ADDR      0x02
#define PROGSKEET_CMD_WRITE_CYCLE   0x03
#define PROGSKEET_CMD_READ_CYCLE    0x04
#define PROGSKEET_CMD_SET_CONFIG    0x05
#define PROGSKEET_CMD_SET_GPIO      0x06
#define PROGSKEET_CMD_SET_GPIO_DIR  0x07
#define PROGSKEET_CMD_WAIT_GPIO     0x08
#define PROGSKEET_CMD_NOP           0x09

#define PROGSKEET_CFG_DELAY_MASK    0x0F

#define PROGSKEET_ADDR_AUTO_INC (1 << 23)

int progskeet_set_gpio_dir(struct progskeet_handle* handle, const uint16_t dir)
{
    char cmdbuf[3];
    int res;

    if (!handle)
        return -1;

    cmdbuf[0] = PROGSKEET_CMD_SET_GPIO_DIR;
    cmdbuf[1] = (dir >> 0) & 0xFF;
    cmdbuf[2] = (dir >> 8) & 0xFF;

    if ((res = progskeet_enqueue_tx_buf(handle, cmdbuf, sizeof(cmdbuf))) < 0)
        return res;

    handle->cur_gpio_dir = dir;

    return 0;
}

int progskeet_set_gpio(struct progskeet_handle* handle, const uint16_t gpio)
{
    char cmdbuf[3];
    int res;

    if (!handle)
        return -1;

    cmdbuf[0] = PROGSKEET_CMD_SET_GPIO;
    cmdbuf[1] = (gpio >> 0) & 0xFF;
    cmdbuf[2] = (gpio >> 8) & 0xFF;

    if ((res = progskeet_enqueue_tx_buf(handle, cmdbuf, sizeof(cmdbuf))) < 0)
        return res;

    handle->cur_gpio = gpio;

    return 0;
}

int progskeet_get_gpio(struct progskeet_handle* handle, uint16_t* gpio)
{
    int res;

    if (!handle || !gpio)
        return -1;

    if ((res = progskeet_enqueue_tx(handle, PROGSKEET_CMD_GET_GPIO)) < 0)
        return res;

    if ((res = progskeet_enqueue_rx_buf(handle, gpio, sizeof(uint16_t))) < 0)
        return res;

    return 0;
}

int progskeet_wait_gpio(struct progskeet_handle* handle, const uint16_t mask, const uint16_t value)
{
    char cmdbuf[5];

    if (!handle)
        return -1;

    if (!mask)
        return 0;

    cmdbuf[0] = PROGSKEET_CMD_WAIT_GPIO;
    cmdbuf[1] = (value >> 0) & 0xFF;
    cmdbuf[2] = (value >> 8) & 0xFF;
    cmdbuf[3] = (mask  >> 0) & 0xFF;
    cmdbuf[4] = (mask  >> 8) & 0xFF;

    return progskeet_enqueue_tx_buf(handle, cmdbuf, sizeof(cmdbuf));
}

int progskeet_assert_gpio(struct progskeet_handle* handle, const uint16_t gpio)
{
    return progskeet_set_gpio(handle, handle->cur_gpio | gpio);
}

int progskeet_deassert_gpio(struct progskeet_handle* handle, const uint16_t gpio)
{
    return progskeet_set_gpio(handle, handle->cur_gpio & ~gpio);
}

int progskeet_set_addr(struct progskeet_handle* handle, const uint32_t addr, int auto_incr)
{
    char cmdbuf[4];
    uint32_t maddr;
    int res;

    if (!handle)
        return -1;

    maddr = (addr & handle->addr_mask) | handle->addr_add;
    maddr |= auto_incr ? PROGSKEET_ADDR_AUTO_INC : 0;

    cmdbuf[0] = PROGSKEET_CMD_SET_ADDR;
    cmdbuf[1] = (maddr >>  0) & 0xFF;
    cmdbuf[2] = (maddr >>  8) & 0xFF;
    cmdbuf[3] = (maddr >> 16) & 0xFF;

    if ((res = progskeet_enqueue_tx_buf(handle, cmdbuf, sizeof(cmdbuf))) < 0)
        return res;

    return 0;
}

int progskeet_set_data(struct progskeet_handle* handle, const uint16_t data)
{
    char cmdbuf[5];
    size_t idx = 0;

    if (!handle)
        return -1;

    cmdbuf[idx++] = PROGSKEET_CMD_WRITE_CYCLE;
    cmdbuf[idx++] = 0x01;
    cmdbuf[idx++] = 0x00;
    cmdbuf[idx++] = (data >> 0) & 0xFF;

    if ((handle->cur_config & PROGSKEET_CFG_16BIT) == 0)
        cmdbuf[idx++] = (data >> 8) & 0xFF;

    return progskeet_enqueue_tx_buf(handle, cmdbuf, idx);
}

int progskeet_set_config(struct progskeet_handle* handle, uint8_t delay, uint8_t config)
{
    char cmdbuf[2];
    int res;

    if (!handle)
        return -1;

    config &= ~PROGSKEET_CFG_DELAY_MASK;
    config |= (delay & PROGSKEET_CFG_DELAY_MASK);

    cmdbuf[0] = PROGSKEET_CMD_SET_CONFIG;
    cmdbuf[1] = config;

    if ((res = progskeet_enqueue_tx_buf(handle, cmdbuf, sizeof(cmdbuf))) < 0)
        return res;

    handle->cur_config = config;

    return 0;
}

int progskeet_write(struct progskeet_handle* handle, const char* buf, const size_t len)
{
    char cmdbuf[3];
    size_t remaining;
    size_t blocksize;
    size_t len_words;

    if (!handle || !buf)
        return -1;

    len_words = len;
    blocksize = 0xFFFF;
    if ((handle->cur_config & PROGSKEET_CFG_16BIT) > 0) {
        len_words /= 2;
        blocksize *= 2;
    }

    remaining = len_words;

    cmdbuf[0] = PROGSKEET_CMD_WRITE_CYCLE;
    cmdbuf[1] = 0xFF;
    cmdbuf[2] = 0xFF;

    while(remaining >= 0xFFFF) {
        if (progskeet_enqueue_tx_buf(handle, cmdbuf, sizeof(cmdbuf)) < 0)
            return -2;

        if (progskeet_enqueue_tx_buf(handle, buf, blocksize) < 0)
            return -3;

        buf += blocksize;
        remaining -= 0xFFFF;
    }

    if (remaining > 0) {
        cmdbuf[1] = (uint8_t)((remaining >> 0) & 0xFF);
        cmdbuf[2] = (uint8_t)((remaining >> 8) & 0xFF);

        if (progskeet_enqueue_tx_buf(handle, cmdbuf, sizeof(cmdbuf)) < 0)
            return -4;

        if ((handle->cur_config & PROGSKEET_CFG_16BIT) > 0)
            remaining *= 2;

        if (progskeet_enqueue_tx_buf(handle, buf, remaining) < 0)
            return -5;
    }

    return 0;
}

int progskeet_read(struct progskeet_handle* handle, char* buf, const size_t len)
{
    char cmdbuf[3];
    size_t remaining;
    size_t len_words;
    int res;

    len_words = len;
    if ((handle->cur_config & PROGSKEET_CFG_16BIT) > 0)
        len_words /= 2;

    remaining = len_words;

    cmdbuf[0] = PROGSKEET_CMD_READ_CYCLE;
    cmdbuf[1] = 0xFF;
    cmdbuf[2] = 0xFF;

    while(remaining >= 0xFFFF) {
        if ((res = progskeet_enqueue_tx_buf(handle, cmdbuf, sizeof(cmdbuf))) < 0)
            return res;

        remaining -= 0xFFFF;
    }

    if (remaining > 0) {
        cmdbuf[1] = (uint8_t)((remaining >> 0) & 0xFF);
        cmdbuf[2] = (uint8_t)((remaining >> 8) & 0xFF);

        if ((res = progskeet_enqueue_tx_buf(handle, cmdbuf, sizeof(cmdbuf))) < 0)
            return res;
    }

    progskeet_enqueue_rx_buf(handle, buf, len);

    return 0;
}

int progskeet_write_addr(struct progskeet_handle* handle, uint32_t addr, uint16_t data)
{
    int res;

    /* FIXME: Is this check correct, why? */
    if (addr != 0) {
        if ((res = progskeet_set_addr(handle, addr, 0)) < 0)
            return res;
    }

    return progskeet_write(handle, (char*)&data, sizeof(uint16_t));
}

int progskeet_read_addr(struct progskeet_handle* handle, uint32_t addr, uint16_t *data)
{
    int res;

    if ((res = progskeet_set_addr(handle, addr, 0)) < 0)
        return res;

    return progskeet_read(handle, (char*)data, sizeof(uint16_t));
}

int progskeet_nop(struct progskeet_handle* handle, const uint32_t amount)
{
    char cmdbuf[2];
    uint32_t remaining;
    int res;

    remaining = amount;

    cmdbuf[0] = PROGSKEET_CMD_NOP;
    cmdbuf[1] = 0xFF;

    while (remaining >= 0xFF) {
        if ((res = progskeet_enqueue_tx_buf(handle, cmdbuf, sizeof(cmdbuf))) < 0)
            return res;

        remaining -= 0xFF;
    }

    if (remaining > 0) {
        cmdbuf[1] = (uint8_t)(remaining & 0xFF);

        if ((res = progskeet_enqueue_tx_buf(handle, cmdbuf, sizeof(cmdbuf))) < 0)
            return res;
    }

    return 0;
}
