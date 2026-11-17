#include "demo_modbus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static DemoModbus context;
static uint8_t request[DEMO_MAX_FRAME], response[DEMO_MAX_FRAME];
static unsigned int cases;
#define CHECK(test) do { if (!(test)) { \
    fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #test); exit(1); \
} } while (0)

static void begin(void)
{
    demo_modbus_init(&context, 1U);
    memset(request, 0, sizeof(request));
    memset(response, 0, sizeof(response));
    ++cases;
}

static size_t crc_frame(size_t payload)
{
    uint16_t crc = demo_crc16(request, (uint16_t)payload);
    request[payload] = (uint8_t)crc;
    request[payload + 1U] = (uint8_t)(crc >> 8U);
    return payload + 2U;
}

static void fields(uint8_t address, uint8_t function, uint16_t start, uint16_t value)
{
    request[0] = address;
    request[1] = function;
    request[2] = (uint8_t)(start >> 8U);
    request[3] = (uint8_t)start;
    request[4] = (uint8_t)(value >> 8U);
    request[5] = (uint8_t)value;
}

static size_t execute(size_t payload)
{
    size_t result = demo_modbus_handle(&context, request, crc_frame(payload),
                                       response, sizeof(response));
    if (result != 0U) {
        uint16_t crc;
        CHECK(result >= 5U && result <= sizeof(response));
        crc = demo_crc16(response, (uint16_t)(result - 2U));
        CHECK(response[result - 2U] == (uint8_t)crc);
        CHECK(response[result - 1U] == (uint8_t)(crc >> 8U));
    }
    return result;
}

int main(void)
{
    static const uint8_t vector[] = {1U, 3U, 0U, 0U, 0U, 1U};
    uint16_t i;
    unsigned int n;
    size_t length;
    uint32_t random = 0x31ACE27BU;

    begin();
    CHECK(demo_crc16(vector, sizeof(vector)) == 0x0A84U);
    CHECK(demo_crc16(NULL, 0U) == 0xFFFFU);

    begin();
    demo_modbus_init(&context, 0U); CHECK(context.address == 1U);
    demo_modbus_init(&context, 248U); CHECK(context.address == 1U);
    demo_modbus_init(&context, 247U); CHECK(context.address == 247U);
    demo_modbus_init(NULL, 1U);

    begin();
    context.registers[0] = 42U;
    fields(1U, 3U, 0U, 1U);
    CHECK(execute(6U) == 7U && response[2] == 2U && response[4] == 42U);

    begin();
    fields(1U, 6U, 127U, 0x1234U);
    CHECK(execute(6U) == 8U && context.registers[127U] == 0x1234U);
    CHECK(memcmp(request, response, 8U) == 0);

    begin();
    fields(1U, 6U, 128U, 0x5678U);
    CHECK(execute(6U) == 5U && response[1] == 0x86U && response[2] == 2U);

    begin();
    fields(1U, 3U, 127U, 2U);
    CHECK(execute(6U) == 5U && response[2] == 2U);

    begin();
    fields(1U, 3U, 0U, 0U);
    CHECK(execute(6U) == 5U && response[2] == 3U);

    begin();
    fields(1U, 3U, 0U, 126U);
    CHECK(execute(6U) == 5U && response[2] == 3U);

    begin();
    context.registers[124U] = 0xABCDU;
    fields(1U, 3U, 0U, 125U);
    CHECK(execute(6U) == 255U && response[2] == 250U);
    CHECK(response[251U] == 0xABU && response[252U] == 0xCDU);

    begin();
    fields(1U, 0x10U, 10U, 2U);
    request[6] = 4U; request[7] = 0x12U; request[8] = 0x34U;
    request[9] = 0x56U; request[10] = 0x78U;
    CHECK(execute(11U) == 8U);
    CHECK(context.registers[10U] == 0x1234U && context.registers[11U] == 0x5678U);

    begin();
    fields(1U, 0x10U, 0U, 123U); request[6] = 246U;
    for (i = 0U; i < 123U; ++i) {
        request[7U + 2U * i] = 0U;
        request[8U + 2U * i] = (uint8_t)i;
    }
    CHECK(execute(253U) == 8U && context.registers[122U] == 122U);

    begin();
    fields(1U, 0x10U, 0U, 2U);
    request[6] = 2U; request[7] = 1U; request[8] = 2U;
    CHECK(execute(9U) == 5U && response[2] == 3U && context.registers[0] == 0U);

    begin();
    fields(1U, 0x10U, 127U, 2U);
    request[6] = 4U; memset(request + 7U, 0x55, 4U);
    CHECK(execute(11U) == 5U && response[2] == 2U && context.registers[127U] == 0U);

    begin();
    request[0] = 1U; request[1] = 0x10U;
    CHECK(execute(2U) == 5U && response[2] == 3U);

    begin();
    fields(0U, 6U, 1U, 77U);
    CHECK(execute(6U) == 0U && context.registers[1] == 77U);

    begin();
    fields(0U, 0x10U, 0U, 1U);
    request[6] = 2U; request[7] = 0U; request[8] = 99U;
    CHECK(execute(9U) == 0U && context.registers[0] == 99U);

    begin();
    fields(0U, 3U, 0U, 1U);
    CHECK(execute(6U) == 0U);

    begin();
    fields(2U, 6U, 0U, 22U);
    CHECK(execute(6U) == 0U && context.registers[0] == 0U);

    begin();
    fields(1U, 6U, 0U, 22U); length = crc_frame(6U);
    request[length - 1U] ^= 1U;
    CHECK(demo_modbus_handle(&context, request, length, response, sizeof(response)) == 0U);
    CHECK(context.registers[0] == 0U);

    begin();
    fields(1U, 6U, 0U, 22U); length = crc_frame(6U);
    CHECK(demo_modbus_handle(&context, request, length, response, 7U) == 0U);
    CHECK(context.registers[0] == 0U);
    request[0] = 0U; length = crc_frame(6U);
    CHECK(demo_modbus_handle(&context, request, length, NULL, 0U) == 0U);
    CHECK(context.registers[0] == 22U);

    begin();
    fields(1U, 3U, 0U, 1U); length = crc_frame(6U);
    CHECK(demo_modbus_handle(&context, request, length, response, 6U) == 0U);
    CHECK(demo_modbus_handle(NULL, request, length, response, sizeof(response)) == 0U);
    CHECK(demo_modbus_handle(&context, NULL, length, response, sizeof(response)) == 0U);
    CHECK(demo_modbus_handle(&context, request, 3U, response, sizeof(response)) == 0U);
    CHECK(demo_modbus_handle(&context, request, 257U, response, sizeof(response)) == 0U);

    begin();
    request[0] = 1U; request[1] = 0x7FU;
    CHECK(execute(2U) == 5U && response[1] == 0xFFU && response[2] == 1U);

    begin();
    for (n = 1U; n <= 512U; ++n) {
        demo_modbus_init(&context, 1U);
        length = 1U + n % sizeof(request);
        for (i = 0U; i < length; ++i) {
            random = random * 1664525U + 1013904223U;
            request[i] = (uint8_t)(random >> 24U);
        }
        CHECK(demo_modbus_handle(&context, request, length, response, sizeof(response))
              <= sizeof(response));
    }
    printf("PASS: %u Modbus scenarios, including 512 malformed inputs\n", cases);
    return 0;
}
