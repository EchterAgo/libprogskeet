/*
 * ProgSkeet communication functions
 */

#ifndef WIN32
#ifdef __APPLE__
#include <libusb-legacy/usb.h>
#else /* !__APPLE__ */
#include <libusb-1.0/libusb.h>
#endif /* __APPLE__ */
#else /* WIN32 */
#include <libusb.h>
#endif /* !WIN32 */

#include <stdlib.h>
#include <string.h>

#include "progskeet.h"
#include "progskeet_private.h"

/* usblib helpers */
#define USB_HANDLE(x) ((struct libusb_device_handle*)x->hdev)

/* USB defines */
#define PROGSKEET_USB_VID 0x1988
#define PROGSKEET_USB_PID 0x0001

#define PROGSKEET_USB_CFG 1
#define PROGSKEET_USB_INT 0

#define PROGSKEET_USB_EP_OUT 0x01
#define PROGSKEET_USB_EP_IN 0x82
#define PROGSKEET_USB_EP_CONTROL 0x03

#define PROGSKEET_USB_TIMEOUT 1000 /* 1 second timeout for USB transfers */

/* Other defines */
#define PROGSKEET_TXBUF_LEN (1024 * 1024)

static int g_inited = 0;

struct progskeet_rxloc
{
    char* addr;
    size_t len;

    struct progskeet_rxloc* next;
};

static int progskeet_free_rxlist(struct progskeet_rxloc* list)
{
    struct progskeet_rxloc* next;

    while (list) {
        next = list->next;
        free(list);
        list = next;
    }

    return 0;
}

int progskeet_init()
{
    if (g_inited == 0) {
        libusb_init(NULL);
        libusb_set_debug(NULL, 3);

        g_inited = 1;
    }

    return 0;
}

static int progskeet_alloc(struct progskeet_handle** handle, struct libusb_device* dev)
{
    struct libusb_device_handle* hdev;

    if (!handle || !dev)
        return -1;

    if (libusb_open(dev, &hdev) < 0)
        return -3;

    *handle = (struct progskeet_handle*)malloc(sizeof(struct progskeet_handle));
    memset(*handle, 0, sizeof(struct progskeet_handle));

    (*handle)->hdev = hdev;

    (*handle)->txbuf = (char*)malloc(PROGSKEET_TXBUF_LEN);

    progskeet_reset(*handle);

    return 0;
}

static int progskeet_open_int(struct progskeet_handle** handle, uint8_t bus, uint8_t addr)
{
    ssize_t numdevs;
    ssize_t i;
    struct libusb_device** devs = NULL;
    struct libusb_device_descriptor descr;
    struct libusb_device_handle* hdev;
    uint8_t cbus;
    uint8_t caddr;
    int found = 0;

    if (g_inited == 0) {
        progskeet_log_global(progskeet_log_level_error, "Library is not initialized, call progskeet_init\n");
        return -1;
    }

    if (!handle)
        return -2;

    *handle = NULL;

    progskeet_log_global(progskeet_log_level_info, "Enumerating USB devices\n");

    if ((numdevs = libusb_get_device_list(NULL, &devs)) < 0)
        return -3;

    for (i = 0; i < numdevs; i++) {
        if (libusb_get_device_descriptor(devs[i], &descr) < 0)
            continue;

        cbus = libusb_get_bus_number(devs[i]);
        caddr = libusb_get_device_address(devs[i]);

        progskeet_log_global(progskeet_log_level_verbose, "Bus %03d Device %03d: ID %04x:%04x\n",
                             cbus, caddr, descr.idVendor, descr.idProduct);

        if (descr.idVendor == PROGSKEET_USB_VID && descr.idProduct == PROGSKEET_USB_PID) {
            if ((bus != 0xFF && cbus != bus) || (addr != 0xFF && caddr != addr))
                continue;

            progskeet_log_global(progskeet_log_level_info, "Trying to open device on bus %d address %d\n", cbus, caddr);

            found++;

            /* Easy now, Skeeter */
            if (progskeet_alloc(handle, devs[i]) == 0) {
                progskeet_log_global(progskeet_log_level_info, "Successfully opened device on bus %d address %d\n", cbus, caddr);
                break;
            }
        }
    }

    libusb_free_device_list(devs, 1);

    if (!*handle) {
        if (found == 0) {
            progskeet_log_global(progskeet_log_level_error, "No matching device found\n");
        } else {
            progskeet_log_global(progskeet_log_level_error, "Found %d devices but none could be opened\n", found);
        }

        return -4;
    }

    return 0;
}

int progskeet_open(struct progskeet_handle** handle)
{
    return progskeet_open_int(handle, 0xFF, 0xFF);
}

int progskeet_open_specific(struct progskeet_handle** handle, uint8_t bus, uint8_t addr)
{
    return progskeet_open_int(handle, bus, addr);
}

int progskeet_close(struct progskeet_handle* handle)
{
    if (!handle)
        return -1;

    progskeet_log(handle, progskeet_log_level_info, "Closing device\n");

    libusb_release_interface(USB_HANDLE(handle), PROGSKEET_USB_INT);

    libusb_close(USB_HANDLE(handle));

    free(handle->txbuf);

    progskeet_free_rxlist(handle->rxlist);

    free(handle);

    return 0;
}

int progskeet_reset(struct progskeet_handle* handle)
{
    int res;

    if (!handle)
        return -1;

    progskeet_log(handle, progskeet_log_level_info, "Resetting device\n");

    /* USB reset */
    if ((res = libusb_reset_device(USB_HANDLE(handle))) < 0) {
        progskeet_log(handle, progskeet_log_level_error, "Reset failed\n");

        if (res == LIBUSB_ERROR_NOT_FOUND) {
            /* Needs reenumeration or device has been disconnected */
            progskeet_close(handle);
        }

        return -2;
    }

    if (libusb_set_configuration(USB_HANDLE(handle), PROGSKEET_USB_CFG) < 0) {
        progskeet_log(handle, progskeet_log_level_error, "Failed to set USB configuration\n");
        return -3;
    }

    /* Will return 0 even if interface is already claimed */
    if (libusb_claim_interface(USB_HANDLE(handle), PROGSKEET_USB_INT) < 0) {
        progskeet_log(handle, progskeet_log_level_error, "Failed to claim interface\n");
        return -4;
    }

    /* Handle reset */
    handle->txlen = 0;

    progskeet_free_rxlist(handle->rxlist);
    handle->rxlist = NULL;
    handle->rxlen = 0;

    handle->cancel = 0;

    progskeet_set_addr(handle, 0, 0);
    progskeet_set_gpio(handle, 0);
    progskeet_set_gpio_dir(handle, 0);

    /* Set the default configuration */
    handle->def_config.delay = 10;
    handle->def_config.is16bit = 1;
    progskeet_set_config(handle, 0, PROGSKEET_CFG_NONE, PROGSKEET_CFG_NONE);

    handle->addr_mask = ~0;
    handle->addr_add = 0;

    progskeet_sync(handle);

    return 0;
}

static int progskeet_rx(struct progskeet_handle* handle)
{
    int count, res, i;
    size_t offset;
    struct progskeet_rxloc* rxnext;
    char *buf;

    if (!handle)
        return -1;

    if (handle->rxlen < 1)
        return 0;

    buf = malloc(handle->rxlen);

    offset = 0;
    while (offset < handle->rxlen && !handle->cancel) {
        if ((res = libusb_bulk_transfer(USB_HANDLE(handle), PROGSKEET_USB_EP_IN,
                                        buf + offset, handle->rxlen, &count,
                                        PROGSKEET_USB_TIMEOUT)) < 0) {
            continue;
        }

        handle->rxlen -= count;
        offset += count;
    }

    offset = 0;
    while (handle->rxlist) {
        memcpy(handle->rxlist->addr, buf + offset, handle->rxlist->len);

        offset += handle->rxlist->len;

        rxnext = handle->rxlist->next;
        free(handle->rxlist);
        handle->rxlist = rxnext;
    }

    progskeet_free_rxlist(handle->rxlist);

    return 0;
}

static int progskeet_tx(struct progskeet_handle* handle)
{
    int count, ret;
    size_t sent;

    if (!handle)
        return -1;

    if (handle->txlen < 1)
        return 0;

    ret = 0;

    /* TODO: Handle timeout */
    sent = 0;
    while ((handle->txlen - sent) > 0 && !handle->cancel) {
        if (libusb_bulk_transfer(USB_HANDLE(handle),
                                 PROGSKEET_USB_EP_OUT,
                                 handle->txbuf + sent,
                                 handle->txlen - sent,
                                 &count,
                                 PROGSKEET_USB_TIMEOUT) < 0) {
            continue;
        }

        sent += count;
    }

    handle->txlen = 0;

    return 0;
}

int progskeet_sync(struct progskeet_handle* handle)
{
    int res;

    if ((res = progskeet_tx(handle)) < 0)
        return res;

    return progskeet_rx(handle);
}

int progskeet_cancel(struct progskeet_handle* handle)
{
    if (!handle)
        return -1;

    handle->cancel = 1;

    return 0;
}

int progskeet_enqueue_tx(struct progskeet_handle* handle, char data)
{
    if (handle->txlen + 1 > PROGSKEET_TXBUF_LEN)
        return -1;

    handle->txbuf[handle->txlen++] = data;

    return 0;
}

int progskeet_enqueue_tx_buf(struct progskeet_handle* handle, const char* buf, const size_t len)
{
    if (handle->txlen + len > PROGSKEET_TXBUF_LEN)
        return -1;

    memcpy(handle->txbuf + handle->txlen, buf, len);
    handle->txlen += len;

    return 0;
}

int progskeet_enqueue_rx_buf(struct progskeet_handle* handle, void* addr, size_t len)
{
    struct progskeet_rxloc* rxloc;
    struct progskeet_rxloc* rxloci;

    rxloc = (struct progskeet_rxloc*)malloc(sizeof(struct progskeet_rxloc));

    rxloc->next = NULL;
    rxloc->addr = addr;
    rxloc->len = len;

    handle->rxlen += len;

    if (handle->rxlist == NULL) {
        handle->rxlist = rxloc;
        return 0;
    }

    rxloci = handle->rxlist;
    while (rxloci->next != NULL)
        rxloci = rxloci->next;
    rxloci->next = rxloc;

    return 0;
}
