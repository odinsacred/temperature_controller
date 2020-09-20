/*
* TempControl.c
*
* Created: 07.02.2020 20:35:08
* Author : odins
*/
#define MAX_SENSOR 10
#define F_CPU 16000000

#include <string.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "nextion_display.h"
#include "ds18b20.h"
#include "usart.h"
#include "crc_8_dallas.h"
#include "hw.h"


static hw_device_t _hw_device = {0};
		
void watchdog_off(void);

int main(void)
{
	asm("cli");
	_hw_device.display_baud = 19200;
	_hw_device.modbus_address = 1;
	_hw_device.modbus_baud = 9600;
	watchdog_off();
	_delay_ms(1000);
	hw_init(&_hw_device);	
	asm("sei");
	hw_run();
}

void watchdog_off(void){
	wdt_reset();
	WDTCR |= (1<<WDCE) | (1<<WDE);
	WDTCR = 0x0;
}