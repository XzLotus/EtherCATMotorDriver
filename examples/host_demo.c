#include "demo_modbus.h"
#include "demo_telemetry.h"
#include "redacted_drive.h"
#include <stdio.h>

int main(void)
{
    static const uint8_t request[] = {1U, 3U, 0U, 0U, 0U, 1U, 0x84U, 0x0AU};
    static const float channels[] = {0.0F, 1.0F, 2.0F, 3.0F};
    DemoModbus context;
    uint8_t response[DEMO_MAX_FRAME], packet[20];
    size_t length, i;

    demo_modbus_init(&context, 1U);
    context.registers[0] = 42U; /* Synthetic data, independent of the product. */
    length = demo_modbus_handle(&context, request, sizeof(request), response, sizeof(response));
    if (length != 7U)
        return 1;
    printf("Modbus response:");
    for (i = 0U; i < length; ++i)
        printf(" %02X", (unsigned int)response[i]);
    printf("\nSynthetic register 0 = 42\n");
    length = demo_telemetry_pack(channels, 4U, packet, sizeof(packet));
    if (length != sizeof(packet))
        return 1;
    printf("Synthetic telemetry packet: %u bytes\n", (unsigned int)length);
    drive_demo_init();
    (void)drive_demo_request_run(true);
    printf("Drive implementation: REDACTED; output_enabled=%u\n",
           (unsigned int)drive_demo_status().output_enabled);
    return drive_demo_status().output_enabled ? 1 : 0;
}
