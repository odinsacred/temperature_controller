/*
* TempControl.c
*
* Created: 07.02.2020 20:35:08
* Author : odins
*/
#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>
#include "nextion_display.h"
#include "ds18b20.h"
#include "usart.h"

//void send_ffffff();
usart_t display_usart;
usart_t pc_usart;
//uint8_t test[3] = {0xFF,0xAA,0xEA};
menu_t menu = {0};
ds18b20_t sensor;
int main(void)
{
	DDRD = 0xFF;
	PORTD = 0x0F;
	uint8_t count = 0;
	display_usart = usart_create(1,9600,128);
	pc_usart = usart_create(0,9600,128);
	nextion_display_init(&display_usart);
	menu = nextion_display_create_menu(1);
	nextion_display_add_page(&menu,2);
	nextion_display_add_element(&menu.pages[0],"n0.val=");
	nextion_display_add_element(&menu.pages[0],"n1.val=");
	asm("sei");
	while (1)
	{
		
		device_state state = get_temperature(&sensor);
		//PORTD = sensor.temperature>>4;
		uint16_t value = sensor.temperature;
		PORTD = count;
		uint8_t frac = value & 0x000F;		 
		float temp = (float)frac;
		temp /=16.0f;
		temp*=100.0;
		frac = (uint8_t)temp;
		
		menu.pages[0].items[0].value = value>>4;
		menu.pages[0].items[1].value = frac;
		//PORTD = menu.pages[0].items[0].value;
		nextion_display_refresh(&menu);	
		count++;
		_delay_ms(500);
	}
}

//void print_line(const char *line, const uint8_t length){
	//usart_write(display_usart, line, length);
//}



//void send_ffffff()
//{
	//uint32_t var = 0xFFFFFF;
	//usart_write(display_usart,&var,3);
//}