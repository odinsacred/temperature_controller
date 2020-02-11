/*
* ds18b20.c
*
* Created: 08.02.2020 15:05:54
*  Author: odins
*/


#define F_CPU 16000000
#include <util/delay.h>
#include "crc_8_dallas.h"
#include "ds18b20.h"

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

uint8_t memory[9];

struct
{
	uint64_t rom;				// очередной найденный ROM
	uint8_t slave_bit: 1;		// очередной бит
	uint8_t inv_slave_bit: 1;	// инверсный бит
	uint8_t last_dev: 1;		// найден последний девайс
	uint8_t master_bit: 1;		// ответ от мастера - дальнейшее направление поиска
	uint8_t: 4;
	uint8_t step;				// шаг поиска (бит какого разряда мы ищем)
	uint8_t last_fork;			// уровень, на котором расположена последняя развилка, из которой мы поворачивали влево при предыдущем проходе по дереву (в этот раз на ней нужно повернуть направо);
	uint8_t last_family_fork;	// здесь хранится уровень последней развилки, относящейся к семейству устройств (младший байт ROM)
	int8_t zero_fork;			// сюда пишем уровни всех развилок, в которых мастер при текущем проходе по дереву отвечал нулём (поворачивал налево), в конце прохода отсюда мы узнаем уровень последней такой развилки.
}bin_search_info;

void send_master_bit();


device_state ds18b20_search_device(ds18b20_t *sensor){
	device_state state = one_wire_reset();
	if(state != DEVICE_OK)
	return state;
	if(bin_search_info.last_dev != 0)
	return LAST_DEVICE;
	bin_search_info.zero_fork = -1;
	bin_search_info.step = 0;
	write_byte(SEARCH_ROM);
	while(bin_search_info.step<64)
	{
		bin_search_info.slave_bit = read_bit(); // возможно тут надо if
		bin_search_info.inv_slave_bit = read_bit();
		if(bin_search_info.slave_bit == bin_search_info.inv_slave_bit)
		{
			if(bin_search_info.slave_bit != 0)
			{
				//это ошибка
				return FAULT;
			}
			// Это развилка
			if(bin_search_info.step == bin_search_info.last_fork)
			{
				bin_search_info.master_bit = 1;
				send_master_bit();
			}
			else
			{
				if(bin_search_info.step>bin_search_info.last_fork)
				{
					// в этой развилке мы еще не были - идем влево
					bin_search_info.master_bit = 0;
				}
				else
				{
					// развилка нам пока не интересна
					bin_search_info.master_bit = (bin_search_info.rom >> bin_search_info.step) & 0x1;
				}
				if(bin_search_info.master_bit == 0)
				{
					// Запомнили развилку в которой поворачивали налево
					bin_search_info.zero_fork = bin_search_info.step;
					if(bin_search_info.step<8)
					{
						bin_search_info.last_family_fork = bin_search_info.step;
					}
					send_master_bit();
				}
				else
				{
					// повернули по развилке вправо
					send_master_bit();
				}
			}
		}
		else
		{ // Это не развилка - продолжаем движение по дереву
			bin_search_info.master_bit = bin_search_info.slave_bit;
			send_master_bit();
		}
	}
	
	bin_search_info.last_fork = bin_search_info.zero_fork; // запоминаем последнюю развилку, где мы повернули налево
	if(bin_search_info.last_fork<0)
	{
		bin_search_info.last_dev = 1;
	}
	if(crc_8_checkSum(&bin_search_info.rom, sizeof(bin_search_info.rom))==0)
	{
		sensor->rom_code = bin_search_info.rom;
		return DEVICE_FOUND;
	}
	else
	{
		return CRC_ERROR;
	}
}

device_state ds18b20_get_temperature(ds18b20_t *sensor)
{
	device_state state = one_wire_reset();
	if(state == DEVICE_OK)
	{
		write_byte(SKIP_ROM);
		write_byte(TEMP_MEASURE);
		_delay_us(750);
		state = one_wire_reset();
		if(state == DEVICE_OK)
		{
			write_byte(SKIP_ROM);
			write_byte(READ_SCRATCHPAD);
			memory[0] = read_byte();
			memory[1] = read_byte();
			memory[2] = read_byte();
			memory[3] = read_byte();
			memory[4] = read_byte();
			memory[5] = read_byte();
			memory[6] = read_byte();
			memory[7] = read_byte();
			memory[8] = read_byte();
			if(crc_8_checkSum(memory,9)==0){
				sensor->temperature = memory[1];
				sensor->temperature <<= 8;
				sensor->temperature = sensor->temperature|memory[0];
				sensor->high_level_temp = memory[2];
				sensor->low_level_temp = memory[3];
				sensor->config = memory[4];
				}else{
				state = CRC_ERROR;
			}
		}
		
	}
	return(state);
}

// посылаем master_bit в шину
void send_master_bit()
{
	if(bin_search_info.master_bit == 1){
		bin_search_info.rom |= 0x8000000000000000;
		write_1();
	}
	else
	{
		write_0();
	}
	bin_search_info.rom>>=1;
	bin_search_info.step++;
}