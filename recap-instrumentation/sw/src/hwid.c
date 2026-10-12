#include "hwid.h"

#include <target.h>

#include "error.h"

#include <printf/printf.h>

#include <inttypes.h>

void hwid_check(void)
{
    const uint32_t hwid = CHIPID->CHIPID_CIDR & ~1UL;
    const uint32_t expected = CHIP_CIDR & ~1UL;

    // Ignore the least significant bit which contains the hardware revision
    if (expected != hwid) {
        char error_msg[128];
        sprintf_(
            error_msg,
            "Invalid chip ID: expected 0x%" PRIx32 ", got 0x%" PRIx32 "",
            expected,
            hwid
        );
        error(error_msg);
    }
}
