#include "sfm3xxx.h"

#include "target.h"

#include <delay.h>
#include <twi-async.h>

static twi_async_t *twi;
static twi_async_read_fut_t *future = NULL;
static volatile uint8_t read_buf[3];

// clang-format off
#if defined(SFM3XXX_DEV_3000_200)
#define CONTINUOUS_MEASUREMENT_CMD {0x10, 0x00}
#elif defined(SFM3XXX_DEV_3003_300_CE_AIR)
#define CONTINUOUS_MEASUREMENT_CMD {0x36, 0x08}
#else
#error SFM3XXX device not defined
#endif
// clang-format on

static bool wait_for_sample(int timeout)
{
    if (sfm3xxx_async_read() == 0) {
        sfm3xxx_flow_t val;
        delay_ms(timeout);
        return sfm3xxx_async_poll(&val) > 0;
    }

    return false;
}

bool sfm3xxx_init(twi_async_t *interface)
{
    twi = interface;

    // start continuous measurement
    uint8_t start[] = CONTINUOUS_MEASUREMENT_CMD;
    if (twi_master_write(twi, SFM3XXX_ADDR, start, sizeof(start))
        == sizeof(start))
    {
        delay_ms(100);
        return wait_for_sample(10);
    }

    return false;
}

static inline void convert_buf(sfm3xxx_flow_t *flow)
{
    // have to swap the bytes around
    *((uint16_t *) flow) = (read_buf[0] << 8) | read_buf[1];
}

int sfm3xxx_async_read(void)
{
    future =
        twi_async_begin_read(twi, SFM3XXX_ADDR, read_buf, sizeof(read_buf));
    return future ? 0 : -1;
}

int sfm3xxx_async_poll(sfm3xxx_flow_t *flow)
{
    switch (twi_async_poll(future)) {
    case TWI_ASYNC_ERROR:
        return -1;
    case TWI_ASYNC_SUCCESS:
        convert_buf(flow);
        future = NULL;
        return 1;
    case TWI_ASYNC_PENDING:
        break;
    }

    return 0;
}
