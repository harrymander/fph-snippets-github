#ifndef RECAP_INSTRUMENT_FREERTOS_PORT_H
#define RECAP_INSTRUMENT_FREERTOS_PORT_H

void freertos_port_init(void);

#define DECLARE_STATIC_TASK(name, stack_size)                                  \
    static StaticTask_t name##_task_obj;                                       \
    static const size_t name##_stack_size = stack_size;                        \
    static StackType_t name##_task_stack[stack_size]

#define INIT_STATIC_TASK(name, func, ident, arg, pri)                          \
    xTaskCreateStatic(                                                         \
        func,                                                                  \
        ident,                                                                 \
        name##_stack_size,                                                     \
        arg,                                                                   \
        pri,                                                                   \
        name##_task_stack,                                                     \
        &name##_task_obj                                                       \
    )

#define DECLARE_STATIC_QUEUE(name, size, num)                                  \
    static StaticQueue_t name##_queue_obj;                                     \
    static const size_t name##_elem_size = size;                               \
    static const size_t name##_num_elements = num;                             \
    static uint8_t name##_queue_storage[(size) * (num)]

#define INIT_STATIC_QUEUE(name)                                                \
    xQueueCreateStatic(                                                        \
        name##_num_elements,                                                   \
        name##_elem_size,                                                      \
        name##_queue_storage,                                                  \
        &name##_queue_obj                                                      \
    )

#endif // RECAP_INSTRUMENT_FREERTOS_PORT_H
