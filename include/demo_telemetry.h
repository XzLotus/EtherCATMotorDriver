#ifndef HUB_DEMO_TELEMETRY_H
#define HUB_DEMO_TELEMETRY_H

#include <stddef.h>
#include <stdint.h>

/* Serialize 1..4 synthetic IEEE-754 binary32 channels in little-endian order,
 * followed by the four-byte JustFloat frame tail. */
size_t demo_telemetry_pack(const float *channels, size_t count,
                           uint8_t *output, size_t capacity);

#endif
