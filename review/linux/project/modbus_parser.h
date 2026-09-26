#ifndef MODBUS_PARSER_H
#define MODBUS_PARSER_H

#include <stdint.h>
#include <stdio.h>

typedef enum {
    MB_STATE_IDLE,
    MB_STATE_ADDR,
    MB_STATE_FUNC,
    MB_STATE_DATA,
    MB_STATE_CRC
} ModbusState;

typedef struct {
    ModbusState state;
    uint8_t addr;
    uint8_t func;
    uint8_t data[256];
    int data_len;
    int expected_len;
    int crc_index;
} ModbusParser;

/* 函数声明 */
uint16_t crc16_modbus(const uint8_t *buf, int len);
void modbus_parse_byte(ModbusParser *p, uint8_t byte);

#endif