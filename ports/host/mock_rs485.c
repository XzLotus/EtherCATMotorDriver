#include "mock_rs485.h"
#include "rs485_port.h"
#include <string.h>

#define MOCK_CAPACITY 256U
static uint8_t rx[MOCK_CAPACITY], tx[MOCK_CAPACITY];
static size_t rx_length, tx_length;
static unsigned int tx_count;
static uint8_t tx_busy;

void rs485_init(uint32_t baudrate)
{
    (void)baudrate;
    rx_length = tx_length = 0U;
    tx_count = 0U;
    tx_busy = 0U;
}

int mock_rs485_feed(const uint8_t *data, size_t length)
{
    if (data == NULL || length == 0U || length > sizeof(rx) || rx_length != 0U)
        return 0;
    memcpy(rx, data, length);
    rx_length = length;
    return 1;
}

uint16_t rs485_read(uint8_t *data, uint16_t capacity)
{
    size_t length = rx_length;
    if (data == NULL || length > capacity)
        return 0U;
    memcpy(data, rx, length);
    rx_length = 0U;
    return (uint16_t)length;
}

uint8_t rs485_send(const uint8_t *data, uint16_t length)
{
    if (data == NULL || length == 0U || length > sizeof(tx) || tx_busy)
        return 0U;
    memcpy(tx, data, length);
    tx_length = length;
    ++tx_count;
    return 1U;
}

size_t mock_rs485_take_tx(uint8_t *data, size_t capacity)
{
    size_t length = tx_length;
    if (data == NULL || capacity < length)
        return 0U;
    memcpy(data, tx, length);
    tx_length = 0U;
    return length;
}

unsigned int mock_rs485_tx_count(void)
{
    return tx_count;
}

void mock_rs485_set_busy(uint8_t busy)
{
    tx_busy = busy;
}
