/*
 * pc_connection.c
 *
 * Created: 12.10.2020 19:23:47
 *  Author: odins
 */ 
#include "pc_connection.h"

static usart_t _usart = NULL;

void pc_connection_init(usart_t *usart)
{
	_usart = *usart;
	usart_reset(_usart);
}

void send_message(uint8_t *message, uint8_t length){
	usart_write(_usart, message, length);
}
