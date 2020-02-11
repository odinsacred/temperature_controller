/*
* one_wire.c
*
* Created: 07.02.2020 20:35:32
*  Author: odins
*/
#include "one_wire.h"
#include <util/atomic.h>



#define sbit(x,PORT) (PORT) |= (1<<x)
#define cbit(x,PORT) (PORT) &= ~(1<<x)
#define pin(x,PIN) (PIN) & (1<<x)

#define SET_IOPIN_TO_OUT IO_DDR|=1<<IO_WIRE //DQ_OUT
#define SET_IOPIN_TO_IN  IO_DDR&=~(1<<IO_WIRE) //DQ_IN

#define PULL_UP_IOPIN		sbit(IO_WIRE,IO_PORT) //S_DQ
#define PULL_DOWN_IOPIN		cbit(IO_WIRE,IO_PORT) //C_DQ
#define GET_IOPIN_STATE     pin(IO_WIRE,IO_PIN) //DQ

void write_0();
void write_1();

// Установить сигнал сброса на линии 1-wire
device_state one_wire_reset()
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		SET_IOPIN_TO_OUT;
		PULL_DOWN_IOPIN;
		_delay_us(700);
		PULL_UP_IOPIN;
		_delay_us(75);
		SET_IOPIN_TO_IN;
		if(GET_IOPIN_STATE)
		return DEVICE_MISSED;
		_delay_us(700);
		if(!GET_IOPIN_STATE)
		return DEVICE_SHORT;
	}
	return DEVICE_OK;
}

void write_byte(uint8_t data)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		for(uint16_t i=1;i<=128;i*=2){
			if(data & i)
			write_1();
			else
			write_0();
		}
	}
}

uint8_t read_byte()
{
	uint8_t data = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		SET_IOPIN_TO_OUT;
		for(uint8_t i=8;i>0;--i)
		{
			data>>=1; // если эту строку пеенести в конец цикла - функция перестает работать
			PULL_DOWN_IOPIN;
			
			_delay_us(3);
			PULL_UP_IOPIN;
			SET_IOPIN_TO_IN;
			_delay_us(12);
			
			if(GET_IOPIN_STATE)
			{
				data|=0x80;
			}
			SET_IOPIN_TO_OUT;
			PULL_UP_IOPIN;
			_delay_us(75);
			
		}
	}
	return data;
}

uint8_t read_bit(){
	uint8_t data = 0;
	SET_IOPIN_TO_OUT;
	PULL_DOWN_IOPIN;
	_delay_us(3);
	PULL_UP_IOPIN;
	SET_IOPIN_TO_IN;
	_delay_us(12);	
	if(GET_IOPIN_STATE)
	{
		data|=0x01;
	}
	SET_IOPIN_TO_OUT;
	PULL_UP_IOPIN;
	_delay_us(75);	
	return data;
}

void write_0(){
	SET_IOPIN_TO_OUT;
	PULL_DOWN_IOPIN;
	_delay_us(15);
	_delay_us(45);
	PULL_UP_IOPIN;
	_delay_us(2);
}

void write_1(){
	SET_IOPIN_TO_OUT;
	PULL_DOWN_IOPIN;
	_delay_us(15);
	PULL_UP_IOPIN;
	_delay_us(47);
}