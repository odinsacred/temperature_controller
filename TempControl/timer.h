/*
* softTimer.h
*
* Created: 27.05.2019 19:12:40
*  Author: MIK
*/


#ifndef SOFTTIMER_H_
#define SOFTTIMER_H_
#include <stdint.h>
#include <stdbool.h>
#include "events.h"

typedef int8_t timer_t;

void timer_init();
timer_t timer_create();
bool timer_poll(timer_t timer);
void timer_restart(timer_t timer, uint16_t ms);
void timer_stop(timer_t timer);

#endif /* SOFTTIMER_H_ */