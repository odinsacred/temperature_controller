/*
* CFile1.c
*
* Created: 25.06.2019 18:32:00
*  Author: MIK
*/
//#include <avr/pgmspace.h>
#include "crc_8_dallas.h"
#include <stdlib.h>
#include <string.h>

uint8_t crc_8_checkSum( void *mas, uint8_t Len )
{
	uint8_t i,dat,crc,fb,st_byt;
	uint8_t * var = calloc(Len, sizeof(uint8_t));//(uint8_t*)mas;
	memcpy(var,mas,Len);
	st_byt=0; crc=0;
	do{
		dat= var[st_byt];
		for( i=0; i<8; i++) {  // счетчик битов в байте
			fb = crc ^ dat;
			fb &= 1;
			crc >>= 1;
			dat >>= 1;
			if( fb == 1 ) crc ^= 0x8c; // полином
		}
		st_byt++;
	} while( st_byt < Len ); // счетчик байтов в массиве
	return crc;
}

