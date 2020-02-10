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

// ������� �������� ������
#define SKIP_ROM        0xCC // ��� ����, ����� �������� ����� �� ����� ���������
#define TEMP_MEASURE    0x44
#define READ_SCRATCHPAD 0xBE
#define READ_ROM		0x33 // ������ ���� �� ����� ���� ����������
#define MATCH_ROM		0x55 // ��� ������ � ���������� �����������, ��� ������� ��������
#define ALARM_SEARCH	0xEC
#define SEARCH_ROM		0xF0

//�������������� ������� (������������ �������)
#define CONVERT_T 0x44; // ������������� ��������������� �����������
#define WRITE_SCRATCHPAD 0x4E // ����� ���� ������� ���������� ��� �����: Th Tl � ������������
#define READ_SCRATCHPAD 0xBE
#define COPY_SCRATCHPAD 0x48 // ���������� �������� � EEPROM
#define READ_POWER_SUPPLY 0xB4 // ����������� ������� ����������� �������

#define NORMAL_SEARCH 0xF0 // ���������� ����� - � �������� ���� ���������
#define CONDITIONAL_SEARCH 0xEC // �������� ����� - ������ � ������������, ������� ��������� � ���������� ���������

uint8_t memory[9];
//static uint16_t t = 0;

device_state get_temperature(ds18b20_t *sensor)
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
				//t>>=4;
				//return(t);
			}else{
				state = CRC_ERROR;
			}
		}
		
	}
	return(state);
}