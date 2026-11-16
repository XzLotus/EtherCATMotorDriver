#ifndef PORTFOLIO_MOTOR_BACKEND_H
#define PORTFOLIO_MOTOR_BACKEND_H

#include <stdint.h>

typedef struct {
    uint8_t enable_requested;
    int32_t target_velocity;
} MotorCommand;

typedef enum {
    MOTOR_BACKEND_REDACTED = 1,
    MOTOR_BACKEND_INVALID_ARGUMENT = 2
} MotorResult;

typedef struct {
    uint8_t output_enabled;
    MotorResult result;
} MotorStatus;

/* Public demonstration adapter: output_enabled is always zero. */
void motor_backend_init(void);
MotorResult motor_backend_apply(const MotorCommand *command);
MotorStatus motor_backend_status(void);

#endif
