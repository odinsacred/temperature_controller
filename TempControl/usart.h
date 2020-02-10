/*
 * usart.h
 *
 * Created: 10.07.2019 21:03:58
 *  Author: MIK
 */ 

#ifndef USART_H_
#define USART_H_

#include <stdio.h>
#include "buffer.h"
#define USART_COUNT 2

typedef struct _usart_t * usart_t;

usart_t usart_create(uint8_t index, uint16_t baud, size_t buffers_size);
void usart_reset(usart_t usart);
void usart_write(usart_t port, const void * data, size_t length);
size_t usart_available_bytes(usart_t port);
bool usart_rx_empty(usart_t port);
void usart_read(usart_t port, void * data, size_t length);

#endif /* USART_H_ */