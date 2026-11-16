#include "motor_backend.h"
#include <limits.h>
#include <stddef.h>
#include <stdio.h>

int main(void)
{
    MotorCommand command = {1U, INT32_MAX};
    MotorStatus status;
    motor_backend_init();
    if (motor_backend_apply(&command) != MOTOR_BACKEND_REDACTED)
        return 1;
    status = motor_backend_status();
    if (status.output_enabled != 0U || status.result != MOTOR_BACKEND_REDACTED)
        return 1;
    command.target_velocity = INT32_MIN;
    (void)motor_backend_apply(&command);
    if (motor_backend_status().output_enabled != 0U)
        return 1;
    if (motor_backend_apply(NULL) != MOTOR_BACKEND_INVALID_ARGUMENT)
        return 1;
    if (motor_backend_status().output_enabled != 0U)
        return 1;
    motor_backend_init();
    status = motor_backend_status();
    if (status.output_enabled != 0U || status.result != MOTOR_BACKEND_REDACTED)
        return 1;
    puts("PASS: redacted backend keeps all outputs disabled");
    return 0;
}
