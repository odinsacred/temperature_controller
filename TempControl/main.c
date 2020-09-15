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
#include <util/delay.h>
#include "nextion_display.h"
#include "ds18b20.h"
#include "usart.h"
#include "crc_8_dallas.h"
#include "hw.h"


static hw_device_t _hw_device = {0};
		
void refresh_sensor_data(ds18b20_t *sensor, uint8_t number);
uint8_t get_frac(uint16_t *value);
void watchdog_off(void);

int main(void)
{
	asm("cli");
	_hw_device.display_baud = 9600;
	_hw_device.modbus_address = 1;
	_hw_device.modbus_baud = 9600;
	_hw_device.num_sensors = 10;
	watchdog_off();
	hw_init(&_hw_device);
	asm("sei");
	DDRC = 0b00000001;
	while (1)
	{
		//if(usart_available_bytes(display_usart)>=sizeof(cmd)){
			//usart_read(display_usart,&cmd,sizeof(cmd));
		//}
		//switch(cmd)
		//{
			//case 1:
			//search_device();
			//cmd = 0x0;
			//break;
			//case 2:
			//save_sensors_data();
			//default:
			//break;
		//}
		
		//for(uint8_t i=0; i<MAX_SENSOR; i++)
		//{
			//
			//if(sensor[i].status == DEVICE_FOUND)
			//{
				//PORTC = 0b00000001;
				//device_state state = ds18b20_get_temperature(&sensor[i]);
				//if(state == DEVICE_OK)
				//{
					//refresh_sensor_data(&sensor[i],i);
				//}
			//}
		//}
		//nextion_display_refresh(&menu);
		_delay_ms(1000);
		PORTC = 0;
	}
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

//uint8_t get_frac(uint16_t *value)
//{
	//uint8_t frac = *value & 0x000F;
	//float temp = (float)frac;
	//temp /=16.0f;
	//temp*=100.0;
	//frac = (uint8_t)temp;
	//return frac;
//}

void watchdog_off(void){
	wdt_reset();
	WDTCR |= (1<<WDCE) | (1<<WDE);
	WDTCR = 0x0;
}