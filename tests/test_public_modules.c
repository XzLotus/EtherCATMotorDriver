#include "demo_telemetry.h"
#include "redacted_drive.h"
#include <stdio.h>
#include <string.h>

int main(void)
{
    static const float channels[] = {1.0F, -2.0F, 0.0F, 42.0F};
    static const uint8_t expected[] = {
        0U, 0U, 0x80U, 0x3FU, 0U, 0U, 0U, 0xC0U,
        0U, 0U, 0U, 0U, 0U, 0U, 0x28U, 0x42U,
        0U, 0U, 0x80U, 0x7FU
    };
    uint8_t output[20];
    unsigned int i;
    if (demo_telemetry_pack(channels, 4U, output, sizeof(output)) != sizeof(output)
        || memcmp(output, expected, sizeof(output)) != 0)
        return 1;
    memset(output, 0xAA, sizeof(output));
    if (demo_telemetry_pack(channels, 4U, output, 19U) != 0U || output[0] != 0xAAU)
        return 1;
    if (demo_telemetry_pack(NULL, 4U, output, sizeof(output)) != 0U
        || demo_telemetry_pack(channels, 0U, output, sizeof(output)) != 0U
        || demo_telemetry_pack(channels, 5U, output, sizeof(output)) != 0U
        || demo_telemetry_pack(channels, 1U, NULL, sizeof(output)) != 0U)
        return 1;
    drive_demo_init();
    for (i = 0U; i < 100U; ++i) {
        if (drive_demo_request_run((i & 1U) != 0U) != DRIVE_IMPLEMENTATION_REDACTED
            || drive_demo_status().output_enabled)
            return 1;
    }
    drive_demo_init();
    if (drive_demo_status().output_enabled)
        return 1;
    puts("PASS: synthetic telemetry serialization and redacted drive output checks");
    return 0;
}
