/*
* ds18b20.h
*
* Created: 08.02.2020 15:06:07
*  Author: odins
*/


#ifndef DS18B20_H_
#define DS18B20_H_
#include "one_wire.h"

typedef enum {
	NINE_BITS	= 0,
	TEN_BITS	= 1,
	ELEVEN_BITS = 2,
	TWELVE_BITS = 3
}mode;

typedef struct _ds18b20_t
{
	uint64_t rom_code;
	uint16_t temperature;
	uint8_t high_level_temp;
	uint8_t low_level_temp;
	mode config;
} ds18b20_t;

void ds18b20_set_conversion_mode(uint8_t config); // настройка разрешающей способности датчика
device_state ds18b20_get_temperature(ds18b20_t *sensor);


#endif /* DS18B20_H_ */