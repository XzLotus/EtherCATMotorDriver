#include "motor_backend.h"
#include <stddef.h>

/* New portfolio adapter. All algorithm and hardware output code is omitted. */
static MotorStatus status = {0U, MOTOR_BACKEND_REDACTED};

void motor_backend_init(void)
{
    status.output_enabled = 0U;
    status.result = MOTOR_BACKEND_REDACTED;
}

MotorResult motor_backend_apply(const MotorCommand *command)
{
    status.output_enabled = 0U;
    status.result = (command == NULL)
        ? MOTOR_BACKEND_INVALID_ARGUMENT : MOTOR_BACKEND_REDACTED;
    return status.result;
}

MotorStatus motor_backend_status(void)
{
    return status;
}
