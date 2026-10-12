#include "usbio.h"

#include "error.h"

#include <appconfig.h>
#include <udp.h>
#include <usb_cdc.h>

#include <FreeRTOS.h>
#include <printf/printf.h>
#include <task.h>

#include <string.h>

static usb_cdc_t usb_cdc = NULL;

void usbio_init(void)
{
    static const usb_cdc_cfg_t cfg = {0, 0};

    if (!usb_cdc) {
        vTaskSuspendAll();
        usb_cdc = usb_cdc_init(&cfg);
        if (!usb_cdc) {
            error("USB serial initialisation error");
        }
        xTaskResumeAll();
    }
}

bool usbio_read_char_nonblocking(char *c)
{
    bool ret = false;

    if (usb_cdc_read_ready_p(usb_cdc)) {
        if (usb_cdc_read(usb_cdc, c, 1) != 1) {
            error("error reading from USB CDC");
        }
        ret = true;
    }
    return ret;
}

ssize_t usbio_write(const void *vdata, const size_t size)
{
    return usb_cdc_write(usb_cdc, vdata, size);
}

// Required by printf.h
void putchar_(char c)
{
    if (usbio_write(&c, 1) != 1) {
        error("error writing character");
    }
}

int usbio_printf(const char *fmt, ...)
{
    int ret;
    va_list ap;
    va_start(ap, fmt);
    ret = vprintf_(fmt, ap);
    va_end(ap);
    return ret;
}
