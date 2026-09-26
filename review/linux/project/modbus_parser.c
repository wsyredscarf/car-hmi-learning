#include "modbus_parser.h"
#include <string.h>

uint16_t crc16_modbus(const uint8_t *buf, int len) {
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < len; i++) {
        crc ^= buf[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) crc = (crc >> 1) ^ 0xA001;
            else crc >>= 1;
        }
    }
    return crc;
}

void modbus_parse_byte(ModbusParser *p, uint8_t byte) {
    switch (p->state) {
        case MB_STATE_IDLE:
            p->addr = byte;
            p->state = MB_STATE_FUNC;
            break;

         case MB_STATE_ADDR:
            break;
        case MB_STATE_FUNC:
            p->func = byte;
            p->data_len = 0;
            if (byte == 0x03 || byte == 0x04) p->expected_len = 1;
            else if (byte == 0x06) p->expected_len = 4;
            else { p->state = MB_STATE_IDLE; return; }
            p->state = MB_STATE_DATA;
            break;
        case MB_STATE_DATA:
            p->data[p->data_len++] = byte;
            if (p->data_len >= p->expected_len) {
                p->state = MB_STATE_CRC;
                p->crc_index = 0;
            }
            break;
        case MB_STATE_CRC:
            if (p->crc_index == 0) {
                p->data[p->data_len++] = byte;
                p->crc_index = 1;
            } else {
                p->data[p->data_len++] = byte;
                uint16_t calc_crc = crc16_modbus(p->data, p->data_len - 2);
                uint16_t recv_crc = p->data[p->data_len - 2] | (p->data[p->data_len - 1] << 8);
                if (calc_crc == recv_crc) {
                    printf("CRC校验通过!\n"); // 
                } else {
                    printf("CRC校验失败!\n");
                }
                /* 校验通过则提交给上层 */
                p->state = MB_STATE_IDLE;
            }
            break;
            
            default:
                break;
    }
}