/*
* ds18b20.c
*
* Created: 08.02.2020 15:05:54
*  Author: odins
*/


#define F_CPU 16000000
#include <util/delay.h>
#include <avr/io.h>
#include "crc_8_dallas.h"
#include "ds18b20.h"
#include "utils.h"

#define TRUE 1
#define FALSE 0

// Команды сетевого уровня
#define SKIP_ROM        0xCC // для того, чтобы работать сразу со всеми датчиками
#define TEMP_MEASURE    0x44
#define READ_SCRATCHPAD 0xBE
#define READ_ROM		0x33 // только если на линии одно устройство
#define MATCH_ROM		0x55 // для обмена с конкретной микросхемой, код которой известен
#define ALARM_SEARCH	0xEC
#define SEARCH_ROM		0xF0

//Функциональные команды (транспортный уровень)
#define CONVERT_T 0x44; // Инициализация преоытазоывенмя ткмпературы
#define WRITE_SCRATCHPAD 0x4E // после этой команды передаются три байта: Th Tl и конфигурация
#define READ_SCRATCHPAD 0xBE
#define COPY_SCRATCHPAD 0x48 // сохранение блокнота в EEPROM
#define READ_POWER_SUPPLY 0xB4 // определения наличия паразитного питания

#define NORMAL_SEARCH 0xF0 // нормальный поиск - с участием всех устройств
#define CONDITIONAL_SEARCH 0xEC // условный поиск - только с устройствами, которые находятся в сигнальном состоянии

uint8_t scratchpad[9];
uint8_t ROM_NO[8];//8

uint8_t last_discrepancy;
uint8_t last_family_discrepancy;
uint8_t last_dev_flag;
uint8_t crc8;

void rom_code_to_64bit(uint64_t *value);
void send_master_bit();
void reset_search();
void read_scratchpad();
void write_rom_code(uint64_t rom_code);

device_state ds18b20_read_rom(ds18b20_t *sensor){
	device_state state = one_wire_reset();
	if(state != DEVICE_OK)
	return state;
	write_byte(READ_ROM);
	for(uint8_t i=0; i<7; i++)
	{
		sensor->rom_code |= read_byte();
		sensor->rom_code<<=8;
	}
	sensor->rom_code |= read_byte();
	return state;
}


device_state ds18b20_search_device(ds18b20_t *sensor)
{
	int8_t id_bit_number;
	int8_t last_zero, rom_byte_number;
	device_state search_result;
	int8_t id_bit, cmp_id_bit;
	uint8_t rom_byte_mask, search_direction;
	// initialize for search
	id_bit_number = 1;
	last_zero = 0;
	rom_byte_number = 0;
	rom_byte_mask = 1;
	search_result = DEVICE_MISSED;
	crc8 = 0;
	last_zero = 0;
	id_bit_number = 0;
	

	if (!last_dev_flag)
	{
		search_result = one_wire_reset();
		if(search_result != DEVICE_OK)
		{
			// reset the search
			last_discrepancy = 0;
			last_dev_flag = FALSE;
			last_family_discrepancy = 0;
			return search_result;
		}
		
		write_byte(SEARCH_ROM);
		do
		{
			id_bit = read_bit()>>7;
			cmp_id_bit = read_bit()>>7;
			if((id_bit == 1) && (cmp_id_bit==1))
			{
				search_result = FAULT;
				break;
			}
			else
			{
				// all devices coupled have 0 or 1
				if(id_bit != cmp_id_bit)
				{
					search_direction = id_bit;
				}
				else
				{
					// if this discrepancy if before the Last Discrepancy
					// on a previous next then pick the same as last time
					if(id_bit_number < last_discrepancy)
					{
						search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
					}
					else
					{
						// if equal to last pick 1, if not then pick 0
						search_direction = (id_bit_number == last_discrepancy);
					}
					// if 0 was picked then record its position in LastZero
					if(search_direction == 0)
					{
						last_zero = id_bit_number;
						// check for Last discrepancy in family
						if (last_zero < 9)
						{
							last_family_discrepancy = last_zero;
						}
					}
				}
				// set or clear the bit in the ROM byte rom_byte_number
				// with mask rom_byte_mask
				if (search_direction == 1)
				{
					ROM_NO[rom_byte_number] |= rom_byte_mask;
				}
				else
				{
					ROM_NO[rom_byte_number] &= ~rom_byte_mask;
				}
				
				write_bit(search_direction);
				// increment the byte counter id_bit_number
				// and shift the mask rom_byte_mask
				id_bit_number++;
				rom_byte_mask <<= 1;
				
				// if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
				if (rom_byte_mask == 0)
				{
					
					rom_byte_number++;
					rom_byte_mask = 1;
				}
			}
		}while (rom_byte_number < 8); // loop until through all ROM bytes 0-7
		crc8 = crc_8_checkSum(ROM_NO,sizeof(ROM_NO));
		if (!((id_bit_number < 64) || (crc8 != 0)))
		{
			// search successful so set LastDiscrepancy,LastDeviceFlag,search_result
			last_discrepancy = last_zero;
			// check for last device
			if (last_discrepancy == 0)
			{
				last_dev_flag = TRUE;
			}
			// check for last family group
			if (last_family_discrepancy == last_discrepancy)
			{
				last_family_discrepancy = 0;
			}
			rom_code_to_64bit(&sensor->rom_code);
			search_result = DEVICE_FOUND;
		}
	}
	// if no device found then reset counters so next 'search' will be like a first
	if ((search_result!=DEVICE_FOUND) || !ROM_NO[0])
	{
		last_discrepancy = 0;
		last_dev_flag = FALSE;
		last_family_discrepancy = 0;
		search_result = DEVICE_MISSED;
	}
	return search_result;
}

device_state ds18b20_get_temperature(ds18b20_t *sensor)
{
	device_state state = one_wire_reset();
	if(state == DEVICE_OK)
	{
		write_rom_code(sensor->rom_code);
		write_byte(TEMP_MEASURE);
		_delay_us(750);
		state = one_wire_reset();
		if(state == DEVICE_OK)
		{
			write_rom_code(sensor->rom_code);
			read_scratchpad();
			if(crc_8_checkSum(scratchpad,sizeof(scratchpad))==0){
				sensor->temperature = scratchpad[1];
				sensor->temperature <<= 8;
				sensor->temperature = sensor->temperature|scratchpad[0];
				sensor->high_level_temp = scratchpad[2];
				sensor->low_level_temp = scratchpad[3];
				sensor->config = scratchpad[4];
			}
			else{
				state = CRC_ERROR;
			}
		}
		
	}
	return state;
}

void rom_code_to_64bit(uint64_t *value)
{
	for(uint8_t i=0; i<8; i++)
	{
		*value<<=8;
		*value|=ROM_NO[i];
	}
}

void read_scratchpad(){
	write_byte(READ_SCRATCHPAD);
	scratchpad[0] = read_byte();
	scratchpad[1] = read_byte();
	scratchpad[2] = read_byte();
	scratchpad[3] = read_byte();
	scratchpad[4] = read_byte();
	scratchpad[5] = read_byte();
	scratchpad[6] = read_byte();
	scratchpad[7] = read_byte();
	scratchpad[8] = read_byte();
}

void write_rom_code(uint64_t rom_code){
	write_byte(MATCH_ROM);
	for(uint8_t i=0; i<64;i+=8)
	{
		uint8_t byte = ((rom_code<<i) & 0xFF00000000000000)>>56;
		write_byte(byte);
	}
}