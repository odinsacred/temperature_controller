/*
 * pc_connection.h
 *
 * Created: 12.10.2020 19:24:02
 *  Author: odins
 */ 


#ifndef PC_CONNECTION_H_
#define PC_CONNECTION_H_
#include <stdint.h>
#include "usart.h"

void pc_connection_init(usart_t *usart);
void send_message(uint8_t *message, uint8_t length);

#endif /* PC_CONNECTION_H_ */