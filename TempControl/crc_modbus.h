/*
 * crc_modbus.h
 *
 * Created: 22.10.2020 17:55:59
 *  Author: odins
 */ 


#ifndef CRC_MODBUS_H_
#define CRC_MODBUS_H_
#include <stdint.h>

uint16_t crc16(uint8_t* buffer, uint16_t buffer_length);
uint16_t crc16_once(uint16_t value);

#endif /* CRC_MODBUS_H_ */