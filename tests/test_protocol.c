#include "modbus.h"
#include "mock_rs485.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned int cases;
#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #condition); exit(1); \
} } while (0)

static void reset(void)
{
    modbus_init(1U, 115200U);
    ++cases;
}

static size_t append_crc(uint8_t *frame, size_t length)
{
    uint16_t crc = modbus_crc16(frame, (uint16_t)length);
    frame[length] = (uint8_t)crc;
    frame[length + 1U] = (uint8_t)(crc >> 8U);
    return length + 2U;
}

static void send_request(uint8_t *request, size_t payload_length)
{
    size_t length = append_crc(request, payload_length);
    CHECK(mock_rs485_feed(request, length));
    modbus_process();
}

static size_t response(uint8_t *data)
{
    size_t length = mock_rs485_take_tx(data, 256U);
    if (length != 0U) {
        uint16_t crc;
        CHECK(length >= 4U);
        crc = modbus_crc16(data, (uint16_t)(length - 2U));
        CHECK(data[length - 2U] == (uint8_t)crc);
        CHECK(data[length - 1U] == (uint8_t)(crc >> 8U));
    }
    return length;
}

static uint16_t holding(uint16_t address)
{
    uint16_t value = 0U;
    CHECK(modbus_get_holding(address, &value));
    return value;
}

static void fixed_request(uint8_t *frame, uint8_t slave, uint8_t function,
                          uint16_t address, uint16_t count_or_value)
{
    frame[0] = slave;
    frame[1] = function;
    frame[2] = (uint8_t)(address >> 8U);
    frame[3] = (uint8_t)address;
    frame[4] = (uint8_t)(count_or_value >> 8U);
    frame[5] = (uint8_t)count_or_value;
}

int main(void)
{
    uint8_t frame[256], out[256], second[8];
    size_t length;
    uint16_t i;
    unsigned int n;
    uint32_t random = 0x1234ABCDU;
    static const uint8_t known[] = {1U, 3U, 0U, 0U, 0U, 1U};

    reset();
    CHECK(modbus_crc16(known, sizeof(known)) == 0x0A84U);
    CHECK(modbus_crc16(NULL, 0U) == 0xFFFFU);
    CHECK(!modbus_get_holding(0U, NULL));
    CHECK(!modbus_set_holding(128U, 1U));
    CHECK(!modbus_get_holding(65535U, &i));

    reset();
    CHECK(modbus_set_holding(0U, 0x1234U));
    fixed_request(frame, 1U, 3U, 0U, 1U);
    send_request(frame, 6U);
    CHECK(response(out) == 7U);
    CHECK(out[2] == 2U && out[3] == 0x12U && out[4] == 0x34U);

    reset();
    fixed_request(frame, 1U, 6U, 127U, 0x5678U);
    send_request(frame, 6U);
    CHECK(response(out) == 8U && memcmp(frame, out, 8U) == 0);
    CHECK(holding(127U) == 0x5678U);

    reset();
    fixed_request(frame, 1U, 6U, 128U, 1U);
    send_request(frame, 6U);
    CHECK(response(out) == 5U && out[1] == 0x86U && out[2] == 2U);

    reset();
    fixed_request(frame, 1U, 3U, 127U, 2U);
    send_request(frame, 6U);
    CHECK(response(out) == 5U && out[2] == 2U);

    reset();
    fixed_request(frame, 1U, 3U, 0U, 0U);
    send_request(frame, 6U);
    CHECK(response(out) == 5U && out[2] == 3U);

    reset();
    fixed_request(frame, 1U, 3U, 0U, 126U);
    send_request(frame, 6U);
    CHECK(response(out) == 5U && out[2] == 3U);

    reset();
    CHECK(modbus_set_holding(124U, 0xABCDU));
    fixed_request(frame, 1U, 3U, 0U, 125U);
    send_request(frame, 6U);
    CHECK(response(out) == 255U && out[2] == 250U);
    CHECK(out[251] == 0xABU && out[252] == 0xCDU);

    reset();
    fixed_request(frame, 1U, 0x10U, 10U, 2U);
    frame[6] = 4U; frame[7] = 0x12U; frame[8] = 0x34U;
    frame[9] = 0x56U; frame[10] = 0x78U;
    send_request(frame, 11U);
    CHECK(response(out) == 8U && out[1] == 0x10U);
    CHECK(holding(10U) == 0x1234U && holding(11U) == 0x5678U);

    reset();
    fixed_request(frame, 1U, 0x10U, 0U, 123U);
    frame[6] = 246U;
    for (i = 0U; i < 123U; ++i) {
        frame[7U + 2U * i] = 0U;
        frame[8U + 2U * i] = (uint8_t)i;
    }
    send_request(frame, 253U);
    CHECK(response(out) == 8U && holding(122U) == 122U);

    reset();
    fixed_request(frame, 1U, 0x10U, 127U, 2U);
    frame[6] = 4U; memset(frame + 7U, 0x55, 4U);
    send_request(frame, 11U);
    CHECK(response(out) == 5U && out[2] == 2U && holding(127U) == 0U);

    reset();
    fixed_request(frame, 1U, 0x10U, 0U, 2U);
    frame[6] = 2U; frame[7] = 1U; frame[8] = 2U;
    send_request(frame, 9U);
    CHECK(response(out) == 5U && out[2] == 3U && holding(0U) == 0U);

    reset();
    fixed_request(frame, 0U, 6U, 1U, 77U);
    send_request(frame, 6U);
    CHECK(response(out) == 0U && holding(1U) == 77U);

    reset();
    fixed_request(frame, 0U, 0x10U, 0U, 1U);
    frame[6] = 2U; frame[7] = 0U; frame[8] = 99U;
    send_request(frame, 9U);
    CHECK(response(out) == 0U && holding(0U) == 99U);

    reset();
    fixed_request(frame, 0U, 3U, 0U, 1U);
    send_request(frame, 6U);
    CHECK(response(out) == 0U);

    reset();
    fixed_request(frame, 2U, 6U, 0U, 22U);
    send_request(frame, 6U);
    CHECK(response(out) == 0U && holding(0U) == 0U);

    reset();
    fixed_request(frame, 1U, 6U, 0U, 22U);
    length = append_crc(frame, 6U); frame[length - 1U] ^= 0x80U;
    CHECK(mock_rs485_feed(frame, length)); modbus_process();
    CHECK(response(out) == 0U && holding(0U) == 0U);
    send_request(frame, 6U);
    CHECK(response(out) == 8U && holding(0U) == 22U);

    reset();
    fixed_request(frame, 1U, 6U, 0U, 33U);
    length = append_crc(frame, 6U);
    CHECK(mock_rs485_feed(frame, 3U)); modbus_process();
    CHECK(response(out) == 0U);
    CHECK(mock_rs485_feed(frame + 3U, length - 3U)); modbus_process();
    CHECK(response(out) == 8U && holding(0U) == 33U);

    reset();
    fixed_request(frame, 1U, 6U, 0U, 1U); (void)append_crc(frame, 6U);
    fixed_request(second, 1U, 6U, 1U, 2U); (void)append_crc(second, 6U);
    memcpy(frame + 8U, second, sizeof(second));
    CHECK(mock_rs485_feed(frame, 16U)); modbus_process();
    CHECK(response(out) == 8U && holding(0U) == 1U);
    modbus_process();
    CHECK(response(out) == 8U && holding(1U) == 2U);

    reset();
    frame[0] = 1U; frame[1] = 0x7FU;
    send_request(frame, 2U);
    CHECK(response(out) == 5U && out[1] == 0xFFU && out[2] == 1U);

    reset();
    fixed_request(frame, 1U, 3U, 0U, 1U);
    mock_rs485_set_busy(1U); send_request(frame, 6U);
    CHECK(response(out) == 0U && mock_rs485_tx_count() == 0U);
    mock_rs485_set_busy(0U); send_request(frame, 6U);
    CHECK(response(out) == 7U && mock_rs485_tx_count() == 1U);

    reset();
    modbus_init(0U, 0U);
    fixed_request(frame, 1U, 3U, 0U, 1U); send_request(frame, 6U);
    CHECK(response(out) == 7U);

    reset();
    /* Oversized partial input is discarded; a new request can be parsed. */
    memset(frame, 0, sizeof(frame)); frame[0] = 1U; frame[1] = 0x10U; frame[6] = 250U;
    CHECK(mock_rs485_feed(frame, 200U)); modbus_process();
    CHECK(mock_rs485_feed(frame, 100U)); modbus_process();
    CHECK(response(out) == 0U);
    fixed_request(frame, 1U, 3U, 0U, 1U); send_request(frame, 6U);
    CHECK(response(out) == 7U);

    reset();
    /* Deterministic malformed-input smoke test, including maximum buffer size. */
    for (n = 1U; n <= 1024U; ++n) {
        modbus_init(1U, 115200U);
        length = 1U + (n % sizeof(frame));
        for (i = 0U; i < length; ++i) {
            random = random * 1664525U + 1013904223U;
            frame[i] = (uint8_t)(random >> 24U);
        }
        CHECK(mock_rs485_feed(frame, length)); modbus_process();
        (void)response(out);
    }
    printf("PASS: %u protocol scenarios, including 1024 malformed inputs\n", cases);
    return 0;
}
