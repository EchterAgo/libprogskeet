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

/* Configuration (PROGSKEET_CMD_SET_CONFIG) */
#define PROGSKEET_CFG_DELAY_MASK    0x0F
#define PROGSKEET_CFG_WORD          (1 << 4)
#define PROGSKEET_CFG_TRISTATE      (1 << 5)
#define PROGSKEET_CFG_WAIT_RDY      (1 << 6)
#define PROGSKEET_CFG_BYTESWAP      (1 << 7)

#define PROGSKEET_ADDR_AUTO_INC (1 << 23)

int progskeet_set_gpio_dir(struct progskeet_handle* handle, const uint16_t dir)
{
    char cmdbuf[3];
    int res;

    if (!handle)
        return -1;

    if (handle->cur_gpio_dir == dir)
        return 0;

    cmdbuf[0] = PROGSKEET_CMD_SET_GPIO_DIR;
    cmdbuf[1] = (dir & 0x00FF) >> 0;
    cmdbuf[2] = (dir & 0xFF00) >> 8;

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

    if (handle->cur_gpio == gpio)
        return 0;

    cmdbuf[0] = PROGSKEET_CMD_SET_GPIO;
    cmdbuf[1] = (gpio & 0x00FF) >> 0;
    cmdbuf[2] = (gpio & 0xFF00) >> 8;

    if ((res = progskeet_enqueue_tx_buf(handle, cmdbuf, sizeof(cmdbuf))) < 0)
        return res;

    handle->cur_gpio = gpio;

    return 0;
}

int progskeet_get_gpio(struct progskeet_handle* handle, uint16_t* gpio)
{
    int res;

    if (!handle)
        return -1;

    if ((res = progskeet_enqueue_tx(handle, PROGSKEET_CMD_GET_GPIO)) < 0)
        return res;

    if ((res = progskeet_enqueue_rxloc(handle, gpio, sizeof(uint16_t))) < 0)
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
    cmdbuf[1] = (value & 0x00FF) >> 0;
    cmdbuf[2] = (value & 0xFF00) >> 8;
    cmdbuf[3] = (mask  & 0x00FF) >> 0;
    cmdbuf[4] = (mask  & 0xFF00) >> 8;

    return progskeet_enqueue_tx_buf(handle, cmdbuf, sizeof(cmdbuf));
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

    if (handle->cur_addr == maddr)
        return 0;

    cmdbuf[0] = PROGSKEET_CMD_SET_ADDR;
    cmdbuf[1] = (maddr & 0x0000FF) >>  0;
    cmdbuf[2] = (maddr & 0x00FF00) >>  8;
    cmdbuf[3] = (maddr & 0xFF0000) >> 16;

    if ((res = progskeet_enqueue_tx_buf(handle, cmdbuf, sizeof(cmdbuf))) < 0)
        return res;

    handle->cur_addr = maddr;

    return 0;
}

int progskeet_set_data(struct progskeet_handle* handle, const uint16_t data)
{
    char cmdbuf[5];

    if (!handle)
        return -1;

    cmdbuf[0] = PROGSKEET_CMD_WRITE_CYCLE;
    cmdbuf[1] = 0x01;
    cmdbuf[2] = 0x00;
    cmdbuf[3] = (data & 0x00FF) >> 0;
    cmdbuf[4] = (data & 0xFF00) >> 8;

    return progskeet_enqueue_tx_buf(handle, cmdbuf, sizeof(cmdbuf));
}

int progskeet_set_config(struct progskeet_handle* handle, const uint8_t delay, const int word)
{
    char cmdbuf[2];
    uint8_t config;
    int res;

    if (!handle)
        return -1;

    config = delay;
    config &= PROGSKEET_CFG_DELAY_MASK;

    if (word)
        config |= PROGSKEET_CFG_WORD;

    if (handle->cur_config == config)
        return 0;

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
    size_t blocksize;
    const char* cur;
    const char* end;
    int res;

    cur = buf;
    end = buf + len;

    blocksize = 0x1FFFE;
    if ((handle->cur_config & PROGSKEET_CFG_WORD) == 0)
	blocksize /= 2; /* 0xFFFF */

    while((end - cur) >= blocksize) {
	cmdbuf[0] = PROGSKEET_CMD_WRITE_CYCLE;
	cmdbuf[1] = 0xFF;
	cmdbuf[2] = 0xFF;

	if ((res = progskeet_enqueue_tx_buf(handle, cmdbuf, sizeof(cmdbuf))) < 0)
	    return res;

	if ((res = progskeet_enqueue_tx_buf(handle, cur, blocksize)) < 0)
	    return res;

	cur += blocksize;
    }

    if ((end - cur) > 0) {
        cmdbuf[0] = PROGSKEET_CMD_WRITE_CYCLE;
        cmdbuf[1] = (uint8_t)(((end - cur) >> 0) & 0xFF);
	cmdbuf[2] = (uint8_t)(((end - cur) >> 8) & 0xFF);

        if ((res = progskeet_enqueue_tx_buf(handle, cmdbuf, sizeof(cmdbuf))) < 0)
            return res;

        if ((res = progskeet_enqueue_tx_buf(handle, cur, blocksize)) < 0)
            return res;
    }

    return 0;
}

int progskeet_read(struct progskeet_handle* handle, char* buf, const size_t len)
{
    char cmdbuf[3];
    size_t blocksize;
    size_t remaining;
    int res;

    remaining = len;

    blocksize = 0x1FFFE;
    if ((handle->cur_config & PROGSKEET_CFG_WORD) == 0)
	blocksize /= 2; /* 0xFFFF */

    while(remaining >= blocksize) {
        cmdbuf[0] = PROGSKEET_CMD_READ_CYCLE;
        cmdbuf[1] = 0xFF;
        cmdbuf[2] = 0xFF;

        if ((res = progskeet_enqueue_tx_buf(handle, cmdbuf, sizeof(cmdbuf))) < 0)
            return res;

	remaining -= blocksize;
    }

    if (remaining > 0) {
        cmdbuf[0] = PROGSKEET_CMD_READ_CYCLE;
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

    if ((res = progskeet_set_addr(handle, addr, 0)) < 0)
        return res;

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

    while (remaining >= 0xFF) {
	cmdbuf[0] = PROGSKEET_CMD_NOP;
	cmdbuf[1] = 0xFF;

	if ((res = progskeet_enqueue_tx_buf(handle, cmdbuf, sizeof(cmdbuf))) < 0)
	    return res;

	remaining -= 0xFF;
    }

    if (remaining > 0) {
        cmdbuf[0] = PROGSKEET_CMD_NOP;
        cmdbuf[1] = (uint8_t)(remaining & 0xFF);

        if ((res = progskeet_enqueue_tx_buf(handle, cmdbuf, sizeof(cmdbuf))) < 0)
            return res;
    }

    return 0;
}

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