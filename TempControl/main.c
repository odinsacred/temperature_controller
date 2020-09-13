/*
* TempControl.c
*
* Created: 07.02.2020 20:35:08
* Author : odins
*/
#define MAX_SENSOR 10
#define F_CPU 16000000
#include <avr/io.h>
#include <string.h>
#include <avr/wdt.h>
#include<util/delay.h>
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

item_t temp4_whole = {0};
item_t temp4_frac = {0};
item_t temp4_conn = {0};

item_t temp5_whole = {0};
item_t temp5_frac = {0};
item_t temp5_conn = {0};

item_t temp6_whole = {0};
item_t temp6_frac = {0};
item_t temp6_conn = {0};

item_t temp7_whole = {0};
item_t temp7_frac = {0};
item_t temp7_conn = {0};

item_t temp8_whole = {0};
item_t temp8_frac = {0};
item_t temp8_conn = {0};

item_t temp9_whole = {0};
item_t temp9_frac = {0};
item_t temp9_conn = {0};

item_t temp10_whole = {0};
item_t temp10_frac = {0};
item_t temp10_conn = {0};
ds18b20_t sensor[MAX_SENSOR]={0};


void refresh_sensor_data(ds18b20_t *sensor, uint8_t number);
uint8_t get_frac(uint16_t *value);
void search_device();
void save_sensors_data();
void watchdog_off(void);

int main(void)
{
	asm("cli");
	
	display_usart = usart_create(1,9600);
	pc_usart = usart_create(0,9600);
	nextion_display_init(&display_usart);
	
	nextion_display_create_menu(&menu);
	nextion_display_create_page(&page_1);
	
	nextion_display_create_item(&temp1_whole, "n0.val=");
	nextion_display_create_item(&temp1_frac, "n1.val=");
	nextion_display_create_item(&temp1_conn, "p0.pic=");
	nextion_display_create_item(&temp2_whole, "n2.val=");
	nextion_display_create_item(&temp2_frac, "n3.val=");
	nextion_display_create_item(&temp2_conn, "p1.pic=");
	nextion_display_create_item(&temp3_whole, "n4.val=");
	nextion_display_create_item(&temp3_frac, "n5.val=");
	nextion_display_create_item(&temp3_conn, "p2.pic=");

	nextion_display_create_item(&temp4_whole, "n6.val=");
	nextion_display_create_item(&temp4_frac, "n7.val=");
	nextion_display_create_item(&temp4_conn, "p3.pic=");
	
	nextion_display_create_item(&temp5_whole, "n8.val=");
	nextion_display_create_item(&temp5_frac, "n9.val=");
	nextion_display_create_item(&temp5_conn, "p4.pic=");
	
	nextion_display_create_item(&temp6_whole, "n10.val=");
	nextion_display_create_item(&temp6_frac, "n11.val=");
	nextion_display_create_item(&temp6_conn, "p5.pic=");
	
	nextion_display_create_item(&temp7_whole, "n12.val=");
	nextion_display_create_item(&temp7_frac, "n13.val=");
	nextion_display_create_item(&temp7_conn, "p6.pic=");
	
	nextion_display_create_item(&temp8_whole, "n14.val=");
	nextion_display_create_item(&temp8_frac, "n15.val=");
	nextion_display_create_item(&temp8_conn, "p7.pic=");
	
	nextion_display_create_item(&temp9_whole, "n16.val=");
	nextion_display_create_item(&temp9_frac, "n17.val=");
	nextion_display_create_item(&temp9_conn, "p8.pic=");
	
	nextion_display_create_item(&temp10_whole, "n18.val=");
	nextion_display_create_item(&temp10_frac, "n19.val=");
	nextion_display_create_item(&temp10_conn, "p9.pic=");
	
	nextion_display_add_item(&page_1,&temp1_whole);
	nextion_display_add_item(&page_1,&temp1_frac);
	nextion_display_add_item(&page_1,&temp1_conn);
	nextion_display_add_item(&page_1,&temp2_whole);
	nextion_display_add_item(&page_1,&temp2_frac);
	nextion_display_add_item(&page_1,&temp2_conn);
	nextion_display_add_item(&page_1,&temp3_whole);
	nextion_display_add_item(&page_1,&temp3_frac);
	nextion_display_add_item(&page_1,&temp3_conn);
	
	nextion_display_add_item(&page_1,&temp4_whole);
	nextion_display_add_item(&page_1,&temp4_frac);
	nextion_display_add_item(&page_1,&temp4_conn);
	
	nextion_display_add_item(&page_1,&temp5_whole);
	nextion_display_add_item(&page_1,&temp5_frac);
	nextion_display_add_item(&page_1,&temp5_conn);
	
	nextion_display_add_item(&page_1,&temp6_whole);
	nextion_display_add_item(&page_1,&temp6_frac);
	nextion_display_add_item(&page_1,&temp6_conn);
	
	nextion_display_add_item(&page_1,&temp7_whole);
	nextion_display_add_item(&page_1,&temp7_frac);
	nextion_display_add_item(&page_1,&temp7_conn);
	
	nextion_display_add_item(&page_1,&temp8_whole);
	nextion_display_add_item(&page_1,&temp8_frac);
	nextion_display_add_item(&page_1,&temp8_conn);
	
	nextion_display_add_item(&page_1,&temp9_whole);
	nextion_display_add_item(&page_1,&temp9_frac);
	nextion_display_add_item(&page_1,&temp9_conn);
	
	nextion_display_add_item(&page_1,&temp10_whole);
	nextion_display_add_item(&page_1,&temp10_frac);
	nextion_display_add_item(&page_1,&temp10_conn);
	nextion_display_add_page(&menu,&page_1);
	usart_reset(display_usart);
	usart_reset(pc_usart);
	watchdog_off();
	asm("sei");
	DDRC = 0b00000001;
	uint8_t cmd = 0;
	uint8_t test = 0xBB;
	/*while(1){
	char greetings[] = "hello";
	usart_write(display_usart, greetings, sizeof(greetings));
	_delay_ms(500);
	}*/
	while (1)
	{
		//usart_write(display_usart,&test, sizeof(test));
		//_delay_ms(1000);
		if(usart_available_bytes(display_usart)>=sizeof(cmd)){
			usart_read(display_usart,&cmd,sizeof(cmd));
		}
		switch(cmd)
		{
			case 1:
			search_device();
			cmd = 0x0;
			break;
			case 2:
			save_sensors_data();
			default:
			break;
		}
		
		for(uint8_t i=0; i<MAX_SENSOR; i++)
		{
			
			if(sensor[i].status == DEVICE_FOUND)
			{
				PORTC = 0b00000001;
				device_state state = ds18b20_get_temperature(&sensor[i]);
				if(state == DEVICE_OK)
				{
					refresh_sensor_data(&sensor[i],i);
				}
			}
		}
		nextion_display_refresh(&menu);
		_delay_ms(1000);
		PORTC = 0;
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


void refresh_sensor_data(ds18b20_t *sensor, uint8_t number)
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
		case 3:
		temp4_whole.value = sensor->temperature >>4;
		temp4_frac.value = get_frac(&sensor->temperature);
		if(sensor->status == DEVICE_FOUND)
		temp4_conn.value = 1;
		else
		temp4_conn.value = 0;
		break;
		case 4:
		temp5_whole.value = sensor->temperature >>4;
		temp5_frac.value = get_frac(&sensor->temperature);
		if(sensor->status == DEVICE_FOUND)
		temp5_conn.value = 1;
		else
		temp5_conn.value = 0;
		break;
		case 5:
		temp6_whole.value = sensor->temperature >>4;
		temp6_frac.value = get_frac(&sensor->temperature);
		if(sensor->status == DEVICE_FOUND)
		temp6_conn.value = 1;
		else
		temp6_conn.value = 0;
		break;
		case 6:
		temp7_whole.value = sensor->temperature >>4;
		temp7_frac.value = get_frac(&sensor->temperature);
		if(sensor->status == DEVICE_FOUND)
		temp7_conn.value = 1;
		else
		temp7_conn.value = 0;
		break;
		case 7:
		temp8_whole.value = sensor->temperature >>4;
		temp8_frac.value = get_frac(&sensor->temperature);
		if(sensor->status == DEVICE_FOUND)
		temp8_conn.value = 1;
		else
		temp8_conn.value = 0;
		break;
		case 8:
		temp9_whole.value = sensor->temperature >>4;
		temp9_frac.value = get_frac(&sensor->temperature);
		if(sensor->status == DEVICE_FOUND)
		temp9_conn.value = 1;
		else
		temp9_conn.value = 0;
		break;
		case 9:
		temp10_whole.value = sensor->temperature >>4;
		temp10_frac.value = get_frac(&sensor->temperature);
		if(sensor->status == DEVICE_FOUND)
		temp10_conn.value = 1;
		else
		temp10_conn.value = 0;
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

void save_sensors_data(){
	
}

void watchdog_off(void){
	wdt_reset();
	WDTCR |= (1<<WDCE) | (1<<WDE);
	WDTCR = 0x0;
}