/*
 * utils.c
 *
 * Created: 11.02.2020 22:37:34
 *  Author: odins
 */ 
#include "utils.h"
//00 7C 00 00 00 AA 09 BF  i<8
//00 00 7C 00 00 00 AA 09 i<=8

//7C 00 00 00 AA 09 BF 28 - должно быть наоборот для верного расчета CRC
uint64_t ntohl(uint64_t value){
	uint64_t temp = 0;
	for(uint8_t i=0; i<8; i++)
	{
		temp<<=8;
		temp |= (value & 0xFF);		
		value>>=8;
	}
	return temp;
}

