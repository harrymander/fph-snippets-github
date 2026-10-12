#include "freertos-port.h"

#include <irq.h>

#include <FreeRTOS.h>

#if configSUPPORT_STATIC_ALLOCATION
// From: https://www.freertos.org/a00110.html
void vApplicationGetIdleTaskMemory(
    StaticTask_t **ppxIdleTaskTCBBuffer,
    StackType_t **ppxIdleTaskStackBuffer,
    uint32_t *pulIdleTaskStackSize
)
{
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

#if configUSE_TIMERS
// From: https://www.freertos.org/a00110.html
void vApplicationGetTimerTaskMemory(
    StaticTask_t **ppxTimerTaskTCBBuffer,
    StackType_t **ppxTimerTaskStackBuffer,
    uint32_t *pulTimerTaskStackSize
)
{
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
#endif // configUSE_TIMERS
#endif // configSUPPORT_STATIC_ALLOCATION

void freertos_port_init(void)
{
    extern void xPortPendSVHandler(void);
    extern void xPortSysTickHandler(void);
    extern void vPortSVCHandler(void);

    extern irq_handler_t exception_table[];

    /*
       See mat91lib/sam4s/irq.h: cannot use irq_vector_set directly since it
       takes an unsigned integer and the system interrupt numbers are defined as
       < 0.
    */
    exception_table[16 + SysTick_IRQn] = xPortSysTickHandler;
    exception_table[16 + PendSV_IRQn] = xPortPendSVHandler;
    exception_table[16 + SVCall_IRQn] = vPortSVCHandler;
    asm volatile("" ::: "memory");
}
