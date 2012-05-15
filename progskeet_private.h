#ifndef _PROGSKEET_PRIVATE_H
#define _PROGSKEET_PRIVATE_H

struct progskeet_handle
{
    /* LibUSB device handle */
    void* hdev;

    /* Transmit buffers */
    char* txbuf;
    size_t txlen;

    /* Receive list */
    struct progskeet_rxloc* rxlist;

    /* Set to 1 to cancel any running operations */
    int cancel;

    uint32_t cur_addr;
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
};

#endif /* _PROGSKEET_PRIVATE_H */
