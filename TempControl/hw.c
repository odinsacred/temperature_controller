/*
* hw.c
*
* Created: 15.09.2020 20:27:29
*  Author: odins
*/
#include <stdint.h>
#include <stdlib.h>
#include "hw.h"
#include "ds18b20.h"
#include "crc_8_dallas.h"
#include "nextion_display.h"
#include "events.h"

state_t _state = STATE_CONFIG;
event_t _event = EV_NONE;

static hw_device_t* _device_ptr=NULL;
static ds18b20_t *_sensors = NULL;
static usart_t display_usart;
static usart_t pc_usart;

display_row *_rows = NULL;

static void init_display_rows(void);
	
static void device_run(void);
static void device_config(void);
static void search_device(void);
static void save_founded_devices(void);
static void select_device(void);
static void forget_selected_devices(void);
static void ignored(void);
static void wait_for_command(void);
static void sensor_polling(void);

void (* const transition_table[STATE_MAX][EVENT_MAX])(void)={
	[STATE_CONFIG][EV_NONE]=wait_for_command,
	[STATE_CONFIG][EV_RUN_PRESSED]=device_run,
	[STATE_CONFIG][EV_SEARCH_PRESSED]=search_device,
	[STATE_CONFIG][EV_SAVE_PRESSED]=save_founded_devices,
	[STATE_CONFIG][EV_DEVICE_SELECTED]=select_device,
	[STATE_CONFIG][EV_FORGET_PRESSED]=forget_selected_devices,
	[STATE_CONFIG][EV_MODBUS_REQUEST_RECIEVED]=ignored,
	[STATE_CONFIG][EV_ONE_WIRE_REQUEST_SENDED]=ignored,
	[STATE_CONFIG][EV_ONE_WIRE_REQUEST_RECIEVED]=ignored,
	
	[STATE_RUN][EV_NONE]=sensor_polling,
	[STATE_RUN][EV_RUN_PRESSED]=device_config,
	[STATE_RUN][EV_SEARCH_PRESSED]=ignored,
	[STATE_RUN][EV_SAVE_PRESSED]=ignored,
	[STATE_RUN][EV_DEVICE_SELECTED]=ignored,
	[STATE_RUN][EV_FORGET_PRESSED]=ignored,
	[STATE_RUN][EV_MODBUS_REQUEST_RECIEVED]=ignored,
	[STATE_RUN][EV_ONE_WIRE_REQUEST_SENDED]=ignored,
	[STATE_RUN][EV_ONE_WIRE_REQUEST_RECIEVED]=ignored,
};



void hw_init(hw_device_t* device){
	_device_ptr = device;
	display_usart = usart_create(1,_device_ptr->display_baud);
	pc_usart = usart_create(0,_device_ptr->modbus_baud);
	nextion_display_init(&display_usart);
	events_init(16);
	_sensors = calloc(device->num_sensors, sizeof(ds18b20_t));
	init_display_rows();
}

void hw_run(void){
	while(1){
		_event = events_get();
		transition_table[_state][_event]();
	}
}

static void device_run(void){
	_state = STATE_RUN;
}

static void device_config(void){
	_state = STATE_CONFIG;
}

static void save_founded_devices(void){
	
}

static void select_device(void){
	
}

static void forget_selected_devices(void){
	
}

static void ignored(void){
	
}

static void wait_for_command(void){
	uint8_t cmd=0;
	if(usart_available_bytes(display_usart)>=sizeof(cmd)){
		usart_read(display_usart,&cmd,sizeof(cmd));
	}
	events_put((event_t)cmd);
}

static void sensor_polling(void){
	for(uint8_t i=0; i < _device_ptr.num_sensors; i++)
	{
		
		if(_sensors[i].status == DEVICE_FOUND)
		{
			PORTC = 0b00000001;
			device_state state = ds18b20_get_temperature(&_sensors[i]);
			if(state == DEVICE_OK)
			{
				refresh_sensor_data(&_sensors[i],&_rows[i]);
			}
		}
	}
}

static void search_device(void){
	static uint8_t i=0;
	if(_sensors[i].status != DEVICE_FOUND){
		device_state state = ds18b20_read_rom(&_sensors[i]);
		if(state == DEVICE_OK)
		{
			_sensors[i].status = DEVICE_FOUND;
		}
	}
	if(i < _device_ptr->num_sensors)
	i++;
	else
	i=0;
}

static void init_display_rows(void){
	_rows = calloc(_device_ptr->num_sensors, sizeof(display_row));
	nextion_display_create_row(_rows[0], "n0.val=","n1.val=","p0.pic=");
	nextion_display_create_row(_rows[1], "n2.val=","n3.val=","p1.pic=");
	nextion_display_create_row(_rows[2], "n4.val=","n5.val=","p2.pic=");
	nextion_display_create_row(_rows[3], "n6.val=","n7.val=","p3.pic=");
	nextion_display_create_row(_rows[4], "n8.val=","n9.val=","p4.pic=");
	nextion_display_create_row(_rows[5], "n10.val=","n11.val=","p5.pic=");
	nextion_display_create_row(_rows[6], "n12.val=","n13.val=","p6.pic=");
	nextion_display_create_row(_rows[7], "n14.val=","n15.val=","p7.pic=");
	nextion_display_create_row(_rows[8], "n16.val=","n17.val=","p8.pic=");
	nextion_display_create_row(_rows[9], "n18.val=","n19.val=","p9.pic=");
	
}
void refresh_sensor_data(ds18b20_t *sensor, display_row* row){
	row->whole.value = sensor->temperature >> 4;
	row->frac.value = get_frac(&sensor->temperature);
	if(sensor->status == DEVICE_FOUND)
		row->conn.value = 1;
	else
		row->conn.value = 0;
}
uint8_t get_frac(uint16_t *value)
{
	uint8_t frac = *value & 0x000F;
	float temp = (float)frac;
	temp /=16.0f;
	temp*=100.0;
	frac = (uint8_t)temp;
	return frac;
}