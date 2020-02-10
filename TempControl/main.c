/*
* TempControl.c
*
* Created: 07.02.2020 20:35:08
* Author : odins
*/
#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>
#include "ds18b20.h"
#include "usart.h"

void send_ffffff();
void out_number(char *component, uint16_t number);
void print_string(char *string);
void print_dec(uint16_t data);
usart_t display_usart;
usart_t pc_usart;
uint8_t test[3] = {0xFF,0xAA,0xEA};
	
int main(void)
{
	DDRD = 0xFF;
	ds18b20_t sensor = {0};
	display_usart = usart_create(1,9600,128);
	
	pc_usart = usart_create(0,9600,128);
	asm("sei");
	while (1)
	{
		//device_state state = get_temperature(&sensor);
		//out_number("n0.val", sensor.temperature>>4);
		//PORTD = (sensor.temperature>>4);
		//print_string("hello", sizeof("hello"));
		
		usart_write(display_usart, test, 3);
		_delay_ms(2000);
	}
}


void out_number(char *component, uint16_t number){
	print_string(component);
	print_string("=");
	print_dec(number);
	send_ffffff();
}

void print_dec(uint16_t data)
{
	//uint8_t num;
	//for(num=0; data>=10000;num++) data-=10000;
	//usart_write(display_usart, num+'0', sizeof(uint8_t));
	//for(num=0; data>=1000;num++) data-=1000;
	//usart_write(display_usart, num+'0', sizeof(uint8_t));
	//for(num=0; data>=100;num++) data-=100;
	//usart_write(display_usart, num+'0', sizeof(uint8_t));
	//for(num=0; data>=10;num++) data-=10;
	//usart_write(display_usart, num+'0', sizeof(uint8_t));
	//usart_write(display_usart, data+'0', sizeof(uint8_t));
}

void print_string(char *string)
{
	
	//while(*string) {
		//usart_write(display_usart, string, len);
	//	string++;
	//}
}

static uint8_t var = 0xFF;
void send_ffffff()
{
	
	usart_write(display_usart,&var,sizeof(uint8_t));
	usart_write(display_usart,&var,sizeof(uint8_t));
	usart_write(display_usart,&var,sizeof(uint8_t));
}