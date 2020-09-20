/*
* nextion_display.c
*
* Created: 10.02.2020 13:43:21
*  Author: odins
*/
#define F_CPU 16000000
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "nextion_display.h"

usart_t _usart = NULL;
void send_ffffff(void);
void send_item(item_t *item);

void nextion_display_init(usart_t *usart)
{
	_usart = *usart;
	usart_reset(_usart);
}

void nextion_display_create_row(display_row *row, const char *whole_id, const char *frac_id, const char *conn_id){
	row->whole.string_id = whole_id;
	row->frac.string_id = frac_id;
	row->conn.string_id = conn_id;
}

void nextion_display_refresh_row(display_row *row){
	send_item(&row->whole);
	send_item(&row->frac);
	send_item(&row->conn);
}

void send_item(item_t *item){
	char value[3];
	if(abs(item->value - item->prev_value)>DEATHBAND){
		item->prev_value = item->value;
		if(item->value>=0)
		sprintf(value,"%d",item->value);
		else
		sprintf(value,"%+d",item->value);
		char ch = 0;
		uint8_t i = 0;
		while((ch = pgm_read_byte(item->string_id+i)) != '\0'){
			usart_write(_usart, &ch, sizeof(char));
			i++;
		}
		//usart_write(_usart, item->string_id, strlen(item->string_id));
		usart_write(_usart, value, strlen(value));
		send_ffffff();
	}	
}

void send_ffffff(void)
{
	uint32_t var = 0xFFFFFF;
	usart_write(_usart,&var,3);
}
