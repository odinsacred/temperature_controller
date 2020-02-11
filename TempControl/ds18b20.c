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

struct
{
	uint64_t rom;				// ��������� ��������� ROM
	uint8_t slave_bit: 1;		// ��������� ���
	uint8_t inv_slave_bit: 1;	// ��������� ���
	uint8_t last_dev: 1;		// ������ ��������� ������
	uint8_t master_bit: 1;		// ����� �� ������� - ���������� ����������� ������
	uint8_t: 4;
	uint8_t step;				// ��� ������ (��� ������ ������� �� ����)
	uint8_t last_fork;			// �������, �� ������� ����������� ��������� ��������, �� ������� �� ������������ ����� ��� ���������� ������� �� ������ (� ���� ��� �� ��� ����� ��������� �������);
	uint8_t last_family_fork;	// ����� �������� ������� ��������� ��������, ����������� � ��������� ��������� (������� ���� ROM)
	int8_t zero_fork;			// ���� ����� ������ ���� ��������, � ������� ������ ��� ������� ������� �� ������ ������� ���� (����������� ������), � ����� ������� ������ �� ������ ������� ��������� ����� ��������.
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
		bin_search_info.slave_bit = read_bit(); // �������� ��� ���� if
		bin_search_info.inv_slave_bit = read_bit();
		if(bin_search_info.slave_bit == bin_search_info.inv_slave_bit)
		{
			if(bin_search_info.slave_bit != 0)
			{
				//��� ������
				return FAULT;
			}
			// ��� ��������
			if(bin_search_info.step == bin_search_info.last_fork)
			{
				bin_search_info.master_bit = 1;
				send_master_bit();
			}
			else
			{
				if(bin_search_info.step>bin_search_info.last_fork)
				{
					// � ���� �������� �� ��� �� ���� - ���� �����
					bin_search_info.master_bit = 0;
				}
				else
				{
					// �������� ��� ���� �� ���������
					bin_search_info.master_bit = (bin_search_info.rom >> bin_search_info.step) & 0x1;
				}
				if(bin_search_info.master_bit == 0)
				{
					// ��������� �������� � ������� ������������ ������
					bin_search_info.zero_fork = bin_search_info.step;
					if(bin_search_info.step<8)
					{
						bin_search_info.last_family_fork = bin_search_info.step;
					}
					send_master_bit();
				}
				else
				{
					// ��������� �� �������� ������
					send_master_bit();
				}
			}
		}
		else
		{ // ��� �� �������� - ���������� �������� �� ������
			bin_search_info.master_bit = bin_search_info.slave_bit;
			send_master_bit();
		}
	}
	
	bin_search_info.last_fork = bin_search_info.zero_fork; // ���������� ��������� ��������, ��� �� ��������� ������
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

// �������� master_bit � ����
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