/*
 * CRC8.h
 *
 * Created: 25.06.2019 18:32:50
 *  Author: MIK
 */ 


#ifndef crc_8_H_
#define crc_8_H_
#include <stdint.h>

uint8_t crc_8_checkSum(const void* data, uint8_t length);


#endif /* crc8_H_ */