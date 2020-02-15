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
page_t page_1 = {0};
item_t temp1_whole = {0};
item_t temp1_frac = {0};
item_t temp1_conn = {0};
item_t temp2_whole = {0};
item_t temp2_frac = {0};
item_t temp2_conn = {0};
item_t temp3_whole = {0};
item_t temp3_frac = {0};
item_t temp3_conn = {0};
ds18b20_t sensor[MAX_SENSOR]={0};


void show_sensor_data(ds18b20_t *sensor, uint8_t number);
uint8_t get_frac(uint16_t *value);
void search_device();

int main(void)
{
	DDRC = 0xFF;

	uint8_t cmd;
	display_usart = usart_create(1,9600,128);
	pc_usart = usart_create(0,9600,128);
	nextion_display_init(&display_usart);
    nextion_display_create_menu(&menu, 1);
	
	nextion_display_create_page(&page_1,9);
	
	
	nextion_display_create_item(&temp1_whole, "n0.val=");
	nextion_display_create_item(&temp1_frac, "n1.val=");
	nextion_display_create_item(&temp1_conn, "p0.pic=");
	nextion_display_create_item(&temp2_whole, "n2.val=");
	nextion_display_create_item(&temp2_frac, "n3.val=");
	nextion_display_create_item(&temp2_conn, "p1.pic=");
	nextion_display_create_item(&temp3_whole, "n4.val=");
	nextion_display_create_item(&temp3_frac, "n5.val=");
	nextion_display_create_item(&temp3_conn, "p2.pic=");

	nextion_display_add_element(&page_1,&temp1_whole);
	nextion_display_add_element(&page_1,&temp1_frac);
	nextion_display_add_element(&page_1,&temp1_conn);
	nextion_display_add_element(&page_1,&temp2_whole);
	nextion_display_add_element(&page_1,&temp2_frac);
	nextion_display_add_element(&page_1,&temp2_conn);
	nextion_display_add_element(&page_1,&temp3_whole);
	nextion_display_add_element(&page_1,&temp3_frac);
	nextion_display_add_element(&page_1,&temp3_conn);
	nextion_display_add_page(&menu,page_1);

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
				device_state state = ds18b20_get_temperature(&sensor[i]);
				if(state == DEVICE_OK)
				{
					show_sensor_data(&sensor[i],i);
					
				}
				else
				{
					usart_write(pc_usart,&state, sizeof(state));
				}				
			}			
		}
		nextion_display_refresh(&menu);
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
		temp1_whole.value = sensor->temperature >> 4;
		temp1_frac.value = get_frac(&sensor->temperature);
		if(sensor->status == DEVICE_FOUND)
			temp1_conn.value = 1;
		else
			temp1_conn.value = 0;
		break;
		case 1:
		temp2_whole.value = sensor->temperature >>4;
		temp2_frac.value = get_frac(&sensor->temperature);
		if(sensor->status == DEVICE_FOUND)
		temp2_conn.value = 1;
		else
		temp2_conn.value = 0;
		break;
		case 2:
		temp3_whole.value = sensor->temperature >>4;
		temp3_frac.value = get_frac(&sensor->temperature);
		if(sensor->status == DEVICE_FOUND)
		temp3_conn.value = 1;
		else
		temp3_conn.value = 0;
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



