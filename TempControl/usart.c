/*
 * usart.c
 *
 * Created: 12.09.2020 21:08:36
 *  Author: odins
 */ 

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <util/atomic.h>
#include "usart.h"

struct _usart_t {
	bool initilized;
	uint8_t index;
	buffer_t rx_buffer;
	buffer_t tx_buffer;
	void (*init)(uint32_t baud);
	void (*ensure_write)();
};

static void usart_0_init(uint32_t baud);
static void usart_1_init(uint32_t baud);
static void usart_0_ensure_write();
static void usart_1_ensure_write();

static struct _usart_t usarts[USART_COUNT] = {
	{ false, 0, NULL, NULL, &usart_0_init, &usart_0_ensure_write, NULL},
	{ false, 1, NULL, NULL, &usart_1_init, &usart_1_ensure_write, NULL}
};

static usart_t USART_0 = &usarts[0];
static usart_t USART_1 = &usarts[1];


usart_t usart_create(uint8_t index, uint32_t baud) {
	assert(index < USART_COUNT);
	usart_t usart = &usarts[index];
	
	assert(!usart->initilized);
	switch(index){
		case 0: 
		usart->rx_buffer = buffer_create(0);
		usart->tx_buffer = buffer_create(1);
		break;
		case 1: 
		usart->rx_buffer = buffer_create(2);
		usart->tx_buffer = buffer_create(3);
		break;
	}	
	usart->index = index;
	usart->init(baud);
	usart->initilized = true;
	return usart;
}

void usart_reset(usart_t usart)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		buffer_clear(usart->tx_buffer);
		buffer_clear(usart->rx_buffer);
	}
}

void usart_write(usart_t usart, const void * data, size_t length)
{
	
	assert(length > 0);
	
	// копируем все данные во внутренний буфер usart
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		buffer_write_bytes(usart->tx_buffer, data, length);

	}
	usart->ensure_write();
}

size_t usart_available_bytes(usart_t port)
{
	size_t avail;
	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		avail = buffer_count(port->rx_buffer);
	}

	return avail;
}

bool usart_rx_empty(usart_t usart)
{
	bool result;
	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		result = buffer_is_empty(usart->rx_buffer);
	}
	
	return result;
}

void usart_read(usart_t port, void *data, size_t length)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		buffer_read_bytes(port->rx_buffer, data, length);
	}
}


#define XTAL 16000000UL
#define HI(x) ((x)>>8)
#define LO(x) ((x)&0xFF)

static void usart_0_init(uint32_t baud){
	long bauddivider = XTAL/(16UL*baud)-1;
	UBRR0L = LO(bauddivider);
	UBRR0H = HI(bauddivider);
	UCSR0A = 0;
	UCSR0B = 1<<RXEN0|1<<TXEN0|1<<RXCIE0|1<<TXCIE0;
	/* Set frame format: 8data, 1stop bit */
	UCSR0C = 1<<UCSZ00|1<<UCSZ01;
}

static void usart_1_init(uint32_t baud){
	long bauddivider = XTAL/(16UL*baud)-1;
	UBRR1L = LO(bauddivider);
	UBRR1H = HI(bauddivider);
	UCSR1B = (1<<RXEN1)|(1<<TXEN1)|(1<<RXCIE1)|(1<<TXCIE1);
	UCSR1C = (1<<URSEL1)|1<<UCSZ10|1<<UCSZ11;
	
}

#undef XTAL
#undef HI
#undef LO

static void usart_0_ensure_write()
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if(buffer_is_empty(USART_0->tx_buffer))
		return;
		if(UCSR0A & (1<<UDRE0))
		{
			UDR0 = buffer_read(USART_0->tx_buffer);
		}
	}
}

static void usart_1_ensure_write()
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if(buffer_is_empty(USART_1->tx_buffer))
		return;
		if(UCSR1A & (1<<UDRE1))
		{
			UDR1 = buffer_read(USART_1->tx_buffer);
		}
	}
}

ISR(USART1_RXC_vect){
	buffer_write(USART_1->rx_buffer, UDR1);
}

ISR(USART1_TXC_vect){
	if(!buffer_is_empty(USART_1->tx_buffer))
	{
		UDR1 = buffer_read(USART_1->tx_buffer);
	}
}

ISR(USART0_RXC_vect){
	buffer_write(USART_0->rx_buffer, UDR0);
}

ISR(USART0_TXC_vect){

	if(!buffer_is_empty(USART_0->tx_buffer))
	{
		UDR0 = buffer_read(USART_0->tx_buffer);
	}
}

