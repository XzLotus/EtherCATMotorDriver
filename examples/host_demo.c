#include "modbus.h"
#include "motor_backend.h"
#include "mock_rs485.h"
#include <stdio.h>

int main(void)
{
    /* Standard Modbus request: slave 1, read register 0, count 1. */
    static const uint8_t request[] = {1U, 3U, 0U, 0U, 0U, 1U, 0x84U, 0x0AU};
    uint8_t response[256];
    size_t length, i;
    MotorCommand command = {1U, 100};
    MotorStatus status;

    modbus_init(1U, 115200U);
    /* Synthetic register content for this demonstration. */
    (void)modbus_set_holding(0U, 42U);
    if (!mock_rs485_feed(request, sizeof(request)))
        return 1;
    modbus_process();
    length = mock_rs485_take_tx(response, sizeof(response));
    if (length != 7U)
        return 1;
    printf("Modbus response:");
    for (i = 0U; i < length; ++i)
        printf(" %02X", (unsigned int)response[i]);
    printf("\nRegister 0 = 42 (synthetic demo data)\n");

    motor_backend_init();
    (void)motor_backend_apply(&command);
    status = motor_backend_status();
    printf("Motor backend: REDACTED, output_enabled=%u\n",
           (unsigned int)status.output_enabled);
    return status.output_enabled == 0U ? 0 : 1;
}
