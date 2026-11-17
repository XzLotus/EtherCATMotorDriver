/* Adapted from the readable project's UART request-handling flow.
 * Hardware access and all product-specific register/parameter mappings removed.
 * Length, range, broadcast and output-capacity checks added for the public demo. */
#include "demo_modbus.h"
#include <string.h>

static uint16_t read_u16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8U) | p[1]);
}

static void write_u16(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)(value >> 8U);
    p[1] = (uint8_t)value;
}

static size_t finish(uint8_t *frame, size_t payload)
{
    uint16_t crc = demo_crc16(frame, (uint16_t)payload);
    frame[payload] = (uint8_t)crc;
    frame[payload + 1U] = (uint8_t)(crc >> 8U);
    return payload + 2U;
}

static size_t exception(uint8_t address, uint8_t function, uint8_t code,
                        uint8_t *response, size_t capacity)
{
    if (address == 0U || response == NULL || capacity < 5U)
        return 0U;
    response[0] = address;
    response[1] = (uint8_t)(function | 0x80U);
    response[2] = code;
    return finish(response, 3U);
}

static int valid_range(uint16_t address, uint16_t count)
{
    return address < DEMO_REGISTER_COUNT
        && count <= DEMO_REGISTER_COUNT - address;
}

void demo_modbus_init(DemoModbus *context, uint8_t address)
{
    if (context == NULL)
        return;
    memset(context, 0, sizeof(*context));
    context->address = address >= 1U && address <= 247U ? address : 1U;
}

size_t demo_modbus_handle(DemoModbus *context, const uint8_t *request,
                         size_t length, uint8_t *response, size_t capacity)
{
    uint8_t address, function;
    uint16_t crc, start, count, i;

    if (context == NULL || request == NULL || length < 4U || length > DEMO_MAX_FRAME)
        return 0U;
    crc = demo_crc16(request, (uint16_t)(length - 2U));
    if (request[length - 2U] != (uint8_t)crc
        || request[length - 1U] != (uint8_t)(crc >> 8U))
        return 0U;
    address = request[0];
    function = request[1];
    if (address != context->address && address != 0U)
        return 0U;

    switch (function) {
    case 0x03U:
        if (address == 0U)
            return 0U;
        if (length != 8U)
            return exception(address, function, 3U, response, capacity);
        start = read_u16(request + 2U);
        count = read_u16(request + 4U);
        if (count == 0U || count > 125U)
            return exception(address, function, 3U, response, capacity);
        if (!valid_range(start, count))
            return exception(address, function, 2U, response, capacity);
        if (response == NULL || capacity < 5U + 2U * count)
            return 0U;
        response[0] = address;
        response[1] = function;
        response[2] = (uint8_t)(count * 2U);
        for (i = 0U; i < count; ++i)
            write_u16(response + 3U + 2U * i, context->registers[start + i]);
        return finish(response, 3U + 2U * count);

    case 0x06U:
        if (length != 8U)
            return exception(address, function, 3U, response, capacity);
        start = read_u16(request + 2U);
        if (!valid_range(start, 1U))
            return exception(address, function, 2U, response, capacity);
        if (address != 0U && (response == NULL || capacity < 8U))
            return 0U;
        context->registers[start] = read_u16(request + 4U);
        if (address == 0U)
            return 0U;
        memmove(response, request, 6U);
        return finish(response, 6U);

    case 0x10U:
        if (length < 9U)
            return exception(address, function, 3U, response, capacity);
        start = read_u16(request + 2U);
        count = read_u16(request + 4U);
        if (count == 0U || count > 123U || request[6] != 2U * count
            || length != 9U + request[6])
            return exception(address, function, 3U, response, capacity);
        if (!valid_range(start, count))
            return exception(address, function, 2U, response, capacity);
        if (address != 0U && (response == NULL || capacity < 8U))
            return 0U;
        for (i = 0U; i < count; ++i)
            context->registers[start + i] = read_u16(request + 7U + 2U * i);
        if (address == 0U)
            return 0U;
        memmove(response, request, 6U);
        return finish(response, 6U);

    default:
        return exception(address, function, 1U, response, capacity);
    }
}
