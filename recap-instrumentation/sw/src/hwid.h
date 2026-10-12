#ifndef RECAP_INSTRUMENTATION_HWID_H
#define RECAP_INSTRUMENTATION_HWID_H

/**
 * Panics if the HWID register value does not match the value expected for the
 * MCU type. The value is checked against the `CHIP_CIDR` macro which should be
 * defined in the device header file in mat91lib. Ignores the least significant
 * bit which contains the silicon revision number (i.e. for revisions A/B).
 */
void hwid_check(void);

#endif // RECAP_INSTRUMENTATION_HWID_H
