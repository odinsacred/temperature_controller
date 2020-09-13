#define F_CPU 16000000
#include "assert.h"
#include <avr/io.h>
#include <util/delay.h>

void assert(bool value, uint8_t blink){
	
	if(!value){
		for(uint8_t i=0; i < blink; i++){
			PORTC = 0b00000001;
			_delay_ms(500);
			PORTC = 0;
			_delay_ms(500);
		}
		while(1);
	}
}
