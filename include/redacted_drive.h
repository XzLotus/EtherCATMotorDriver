#ifndef HUB_REDACTED_DRIVE_H
#define HUB_REDACTED_DRIVE_H

#include <stdbool.h>

typedef enum {
    DRIVE_IMPLEMENTATION_REDACTED = 1
} DriveResult;

typedef struct {
    bool output_enabled;
    DriveResult result;
} DriveDemoStatus;

void drive_demo_init(void);
DriveResult drive_demo_request_run(bool requested);
DriveDemoStatus drive_demo_status(void);

#endif
