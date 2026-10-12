#ifndef RECAP_INSTRUMENT_BUILD_ID_H
#define RECAP_INSTRUMENT_BUILD_ID_H

#include <stdint.h>

#define BUILD_ID_SIZE 20
#define BUILD_ID_DIGEST_STR_LEN (BUILD_ID_SIZE * 2)

// Validates the build ID and initialises the digest string.
void build_id_init(void);

// Returns build ID of length `BUILD_ID_SIZE`.
const uint8_t *build_id_get(void);

/**
 * Returns null-terminated hexadecimal representation of build ID of length
 * `BUILD_ID_DIGEST_STR_LEN`. Must have called `build_id_init` first.
 */
const char *build_id_digest_str(void);

#endif // RECAP_INSTRUMENT_BUILD_ID_H
