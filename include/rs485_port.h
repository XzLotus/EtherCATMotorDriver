#ifndef PORTFOLIO_RS485_PORT_H
#define PORTFOLIO_RS485_PORT_H

#include <stdint.h>

/* Transport owns frame timing and copies TX data before returning. */
void rs485_init(uint32_t baudrate);
uint16_t rs485_read(uint8_t *data, uint16_t capacity);
uint8_t rs485_send(const uint8_t *data, uint16_t length);

#endif
