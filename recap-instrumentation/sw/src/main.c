#include "control.h"
#include "error.h"
#include "hwid.h"

#include <FreeRTOS.h>
#include <freertos-port.h>
#include <task.h>

int main(void)
{
    error_check_reset_reason();
    hwid_check();
    freertos_port_init();
    control_init();
    vTaskStartScheduler();
    error("Exited from scheduler");
}
