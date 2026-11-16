#ifndef PORTFOLIO_MOCK_RS485_H
#define PORTFOLIO_MOCK_RS485_H

#include <stddef.h>
#include <stdint.h>

int mock_rs485_feed(const uint8_t *data, size_t length);
size_t mock_rs485_take_tx(uint8_t *data, size_t capacity);
unsigned int mock_rs485_tx_count(void);
void mock_rs485_set_busy(uint8_t busy);

#endif
