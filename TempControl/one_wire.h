/*
 * one_wire.h
 *
 * Created: 07.02.2020 20:35:45
 *  Author: odins
 */ 


#ifndef ONE_WIRE_H_
#define ONE_WIRE_H_

#define IO_PORT PORTA
#define IO_DDR  DDRA
#define IO_PIN  PINA
#define IO_WIRE PA0

#define F_CPU 16000000
#include <util/delay.h>

typedef enum
{
	DEVICE_MISSED,
	DEVICE_OK,
	DEVICE_SHORT,
	CRC_ERROR,
	LAST_DEVICE,
	FAULT,
	DEVICE_FOUND
}device_state;
	
device_state one_wire_reset();
void write_byte(uint8_t data);
uint8_t read_byte();
uint8_t read_bit();
void write_1();
void write_0();
void write_bit(uint8_t data);
#endif /* ONE_WIRE_H_ */