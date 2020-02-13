/*
* TempControl.c
*
* Created: 07.02.2020 20:35:08
* Author : odins
*/
#define MAX_SENSOR 3
#define F_CPU 16000000
#include <avr/io.h>
#include <string.h>
#include "nextion_display.h"
#include "ds18b20.h"
#include "usart.h"
#include "crc_8_dallas.h"
#include "utils.h"
usart_t display_usart;
usart_t pc_usart;
menu_t menu = {0};
ds18b20_t sensor[MAX_SENSOR]={0};


void show_sensor_data(ds18b20_t *sensor, uint8_t number);
uint8_t get_frac(uint16_t *value);
void search_device();

int main(void)
{
	DDRC = 0xFF;

	uint8_t cmd;
	uint8_t count = 0;
	display_usart = usart_create(1,9600,128);
	pc_usart = usart_create(0,9600,128);
	nextion_display_init(&display_usart);
	menu = nextion_display_create_menu(1);
	nextion_display_add_page(&menu,6);
	nextion_display_add_element(&menu.pages[0],"n0.val=");
	nextion_display_add_element(&menu.pages[0],"n1.val=");
	nextion_display_add_element(&menu.pages[0],"n2.val=");
	nextion_display_add_element(&menu.pages[0],"n3.val=");
	nextion_display_add_element(&menu.pages[0],"n4.val=");
	nextion_display_add_element(&menu.pages[0],"n5.val=");
	//nextion_display_add_element(&menu.pages[0],"p0.val=");
	//nextion_display_add_element(&menu.pages[0],"p1.val=");
	//nextion_display_add_element(&menu.pages[0],"p2.val=");
	asm("sei");
	while (1)
	{
		usart_read(display_usart,&cmd,sizeof(cmd));
		switch(cmd)
		{
			case 0x01: 
				search_device();
				cmd = 0x0;			
			 break;
			default: break;
		}

		for(uint8_t i=0; i<MAX_SENSOR; i++)
		{
			if(sensor[i].status == DEVICE_FOUND)
			{

				//for(uint8_t i=0; i<64;i+=8)
				//{
					//uint8_t byte = ((sensor->rom_code<<i) & 0xFF00000000000000)>>56; //8 16 24 32 40 48 56 64
					//usart_write(pc_usart,&byte, sizeof(byte));
				//}
				//uint8_t crc8 = crc_8_checkSum(&sensor->rom_code,sizeof(sensor->rom_code));
				//usart_write(pc_usart,&sensor[i].rom_code, sizeof(uint64_t));
				device_state state = ds18b20_get_temperature(&sensor[i]);
				if(state == DEVICE_OK)
				{
					show_sensor_data(&sensor[i],i);
					nextion_display_refresh(&menu);
				}				
			}
		}
		_delay_ms(1000);
	}
}

void search_device(){
	static uint8_t i=0;
	if(sensor[i].status != DEVICE_FOUND){
		device_state state = ds18b20_read_rom(&sensor[i]);
		if(state == DEVICE_OK)
		{
			sensor[i].status = DEVICE_FOUND;
		}
	}
	if(i<MAX_SENSOR)
		i++;
	else
		i=0;
}
void show_sensor_data(ds18b20_t *sensor, uint8_t number)
{
	switch(number)
	{
		case 0: 
		menu.pages[0].items[0].value = sensor->temperature >>4;
		menu.pages[0].items[1].value = get_frac(&sensor->temperature);
		break;
		case 1: 
		menu.pages[0].items[2].value = sensor->temperature >>4;
		menu.pages[0].items[3].value = get_frac(&sensor->temperature);
		break;
		case 2: 
		menu.pages[0].items[4].value = sensor->temperature >>4;
		menu.pages[0].items[5].value = get_frac(&sensor->temperature);
		break;
	}
}

uint8_t get_frac(uint16_t *value)
{
	uint8_t frac = *value & 0x000F;
	float temp = (float)frac;
	temp /=16.0f;
	temp*=100.0;
	frac = (uint8_t)temp;
	return frac;
}



