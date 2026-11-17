/* Public adaptation of the project's waveform serialization idea.
 * Input channels are provided by the caller; private runtime structures omitted. */
#include "demo_telemetry.h"
#include <float.h>
#include <string.h>

#if FLT_RADIX != 2 || FLT_MANT_DIG != 24 || FLT_MAX_EXP != 128
#error This demo requires IEEE-754 binary32 float
#endif

size_t demo_telemetry_pack(const float *channels, size_t count,
                           uint8_t *output, size_t capacity)
{
    static const uint8_t tail[] = {0U, 0U, 0x80U, 0x7FU};
    size_t i;
    if (channels == NULL || output == NULL || count == 0U || count > 4U
        || sizeof(float) != sizeof(uint32_t) || capacity < count * 4U + sizeof(tail))
        return 0U;
    for (i = 0U; i < count; ++i) {
        uint32_t bits;
        memcpy(&bits, channels + i, sizeof(bits));
        output[4U * i] = (uint8_t)bits;
        output[4U * i + 1U] = (uint8_t)(bits >> 8U);
        output[4U * i + 2U] = (uint8_t)(bits >> 16U);
        output[4U * i + 3U] = (uint8_t)(bits >> 24U);
    }
    memcpy(output + 4U * count, tail, sizeof(tail));
    return 4U * count + sizeof(tail);
}
