#ifndef HUB_DEMO_MODBUS_H
#define HUB_DEMO_MODBUS_H

#include <stddef.h>
#include <stdint.h>

#define DEMO_REGISTER_COUNT 128U
#define DEMO_MAX_FRAME 256U

typedef struct {
    uint8_t address;
    uint16_t registers[DEMO_REGISTER_COUNT];
} DemoModbus;

uint16_t demo_crc16(const uint8_t *data, uint16_t length);
void demo_modbus_init(DemoModbus *context, uint8_t address);

/* Accept one complete RTU ADU. The caller supplies frame timing/transport.
 * Registers are synthetic demo storage; they have no hardware side effects.
 * A response length of zero means no response was emitted. */
size_t demo_modbus_handle(DemoModbus *context, const uint8_t *request,
                         size_t length, uint8_t *response, size_t capacity);

#endif
