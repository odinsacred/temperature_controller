/*
 * events.h
 *
 * Created: 20.07.2019 13:58:53
 *  Author: NPashenkov
 */ 


#ifndef EVENTS_H_
#define EVENTS_H_

#include <stdlib.h>
#include <stdint.h>

typedef enum{
	STATE_CONFIG,
	STATE_RUN,
	STATE_MAX
}state_t;

typedef enum{
	EV_NONE=0,
	EV_SEARCH_PRESSED,
	EV_RUN_PRESSED,
	EV_SAVE_PRESSED,
	EV_FORGET_PRESSED,
	EV_MODBUS_REQUEST_RECIEVED,
	EV_ONE_WIRE_REQUEST_SENDED,
	EV_ONE_WIRE_REQUEST_RECIEVED,
	EVENT_MAX
}event_t;

void events_init(size_t size);
void events_put(event_t event);
event_t events_get();
size_t events_count();

#endif /* EVENTS_H_ */