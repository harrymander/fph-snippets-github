#include "build-id.h"

#include "error.h"

#include <stdbool.h>
#include <stdint.h>

struct build_id {
    uint32_t namesz;
    uint32_t descsz;
    uint32_t type;
    uint8_t data[];
};

// This is declared in overlay.ld
extern const struct build_id ext_gnu_build_id;

static char digest[BUILD_ID_DIGEST_STR_LEN + 1];
static bool init = false;

const uint8_t *build_id_get(void)
{
    return &ext_gnu_build_id.data[ext_gnu_build_id.namesz];
}

static char hex(uint8_t nybble)
{
    ASSERT(nybble <= 0xf, "Nybble must be <= 0xf");
    return nybble < 10 ? nybble + '0' : nybble - 10 + 'a';
}

static void assert_valid_build_id(void)
{
    static const char expected_name[] = "GNU";

    ASSERT(
        ext_gnu_build_id.namesz == sizeof(expected_name),
        "Unexpected build ID name size"
    );
    for (size_t i = 0; i < sizeof(expected_name); i++) {
        ASSERT(
            ext_gnu_build_id.data[i] == expected_name[i],
            "Unexpected build ID name"
        );
    }
    ASSERT(
        ext_gnu_build_id.descsz == BUILD_ID_SIZE, "Unexpected build ID size"
    );
    ASSERT(ext_gnu_build_id.type == 3, "Unexpected build ID type");
}

void build_id_init(void)
{
    if (init) {
        return;
    }

    assert_valid_build_id();
    const uint8_t *data = build_id_get();
    for (uint32_t i = 0; i < ext_gnu_build_id.descsz; i++) {
        digest[i * 2] = hex(data[i] >> 4);
        digest[i * 2 + 1] = hex(data[i] & 0xf);
    }
    digest[BUILD_ID_DIGEST_STR_LEN] = 0;
    init = true;
}

const char *build_id_digest_str(void)
{
    return digest;
}
