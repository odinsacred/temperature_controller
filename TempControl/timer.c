/*
* softTimer.c
*
* Created: 27.05.2019 18:11:08
*  Author: MIK
*/
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>
#include "timer.h"
#include "assert.h"

#define XTAL 16000000
#define MAX_TIMERS 3

#define divider 64
#define ticks XTAL/divider

#define INIT 256 - (ticks/1000) // 1000 - чтобы получить 1 мс, 256 - число тиков таймера до переполнения

static int current_timer = 0;

static bool initilized = false;

// значение больше нуля - таймер работает, значение ноль - таймер сработал, значение меньше нуля - таймер выключен
volatile static int32_t timers[MAX_TIMERS];

static const int TIMEOUT = 0;
static const int DISABLED = -1;

void timer_init()
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		assert(!initilized,1);

		current_timer = 0;
		for (size_t i = 0; i < MAX_TIMERS; i++)
		timers[i] = DISABLED;

		initilized = true;

		// настроить таймер на 1 мс тики
		// установить делитель на 64
		//GTCCR |= 1<<PSRSYNC; // сброс предделителя
		TCNT0 = INIT;
		TCCR0 |=1<<CS00|1<<CS01|0<<CS02;
		TIMSK |= 1<<TOIE0; // разрешение прерывания
	}
}

timer_t timer_create()
{
	timer_t result;
	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		assert(current_timer < MAX_TIMERS,2);
		
		timers[current_timer] = DISABLED;

		result = current_timer++;
	}
	
	return result;
}

void timer_restart(timer_t timer, uint16_t ms)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		timers[timer] = ms;
	}
}

void timer_stop(timer_t timer)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		timers[timer] = DISABLED;
	}
}

bool timer_poll(timer_t timer)
{
	bool result = false;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if (timers[timer] == TIMEOUT) {
			timers[timer] = DISABLED;
			result = true;
		}
	}
	return result;
}

ISR(TIMER0_OVF_vect){
	TCNT0 = INIT;
	for (size_t i = 0; i < current_timer; i++)
		if(timers[i] > 0) timers[i]--;
}
