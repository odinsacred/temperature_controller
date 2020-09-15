/*
 * events.c
 *
 * Created: 20.07.2019 14:03:55
 *  Author: NPashenkov
 */ 

#include "events.h"

#include <stdlib.h>
#include <string.h>
#include "assert.h"

static struct events_t
{
	size_t size;
	size_t in;
	size_t out;
	event_t * items;
} events = {0};

void events_init(size_t size)
{
	events.size = size;
	events.items = calloc(size, sizeof(event_t));
}

void events_put(event_t event)
{
	assert(events.in != (events.out - 1 + events.size) % events.size, 1);
	events.items[events.in] = event;
	events.in = (events.in + 1) % events.size;
}

event_t events_get()
{
	event_t e = EV_NONE;

	if (events.in != events.out)
	{
		e = events.items[events.out];
	
		events.out = (events.out + 1) % events.size;
	}

	return e;
}

size_t events_count()
{
	if (events.in >= events.out)
		return events.in - events.out;
	
	return events.size - events.in + events.out;
}
