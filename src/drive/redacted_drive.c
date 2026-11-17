#include "redacted_drive.h"

/* New public adapter. Private control algorithms and physical outputs omitted. */
static DriveDemoStatus status = {false, DRIVE_IMPLEMENTATION_REDACTED};

void drive_demo_init(void)
{
    status.output_enabled = false;
    status.result = DRIVE_IMPLEMENTATION_REDACTED;
}

DriveResult drive_demo_request_run(bool requested)
{
    (void)requested;
    status.output_enabled = false;
    status.result = DRIVE_IMPLEMENTATION_REDACTED;
    return status.result;
}

DriveDemoStatus drive_demo_status(void)
{
    return status;
}
