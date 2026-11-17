/* Extracted from the readable project UART module; null-input guard added. */
#include "demo_modbus.h"
#include <stddef.h>

uint16_t demo_crc16(const uint8_t *data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    uint16_t i, j;
    if (data == NULL) return crc;

    for (i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i];
        for (j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}
