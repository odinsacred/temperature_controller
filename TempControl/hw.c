/*
* hw.c
*
* Created: 15.09.2020 20:27:29
*  Author: odins
*/
#define F_CPU 16000000
#include <util/delay.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include "hw.h"
#include "ds18b20.h"
#include "crc_8_dallas.h"
#include "crc_modbus.h"
#include "nextion_display.h"
#include "events.h"
#include "timer.h"

typedef struct {
	uint8_t slave_addr;
	uint8_t func;
	uint16_t start_reg;
	uint16_t reg_count;
	uint16_t crc;
}modbus_request_t;

/*typedef struct{
	uint8_t slave_addr;
	uint8_t func;
	uint8_t byte_count;
}modbus_payload_t;

typedef struct {
	modbus_payload_t payload;
	uint16_t crc;
}modbus_response_t;*/

modbus_request_t modbus_request = {0};

static const uint16_t SENSOR_POLL_TIMEOUT = 500;
#define NUM_OF_SENSORS 10
state_t _state = STATE_CONFIG;
event_t _event = EV_NONE;
timer_t sensor_poll_timer = 0;
timer_t send_message_timer = 0;

static hw_device_t* _device_ptr=NULL;
static ds18b20_t _sensors[NUM_OF_SENSORS] = {0};
static display_row _rows[NUM_OF_SENSORS] = {0};
static usart_t display_usart;
static usart_t pc_usart;



static void init_display_rows(void);

static void device_run(void);
static void device_config(void);
static void search_device(void);
static void save_founded_device(void);
static void forget_selected_device(void);
static void ignored(void);
static void wait_for_command(void);
static void sensors_polling(void);
static void modbus_request_processing(void);
static void refresh_sensor_data(ds18b20_t *sensor, display_row* row);
static void poll_founded_sensor(void);
static uint8_t get_frac(uint16_t *value);

void swap_bytes(uint16_t* value);

typedef struct {
	uint8_t cmd;
	uint8_t sensor_id;
}cmd_t;

cmd_t _recieved_cmd_id = {0};

void (* const transition_table[STATE_MAX][EVENT_MAX])(void)={
	[STATE_CONFIG][EV_NONE]=wait_for_command,
	[STATE_CONFIG][EV_RUN_PRESSED]=device_run,
	[STATE_CONFIG][EV_SEARCH_PRESSED]=search_device,
	[STATE_CONFIG][EV_SAVE_PRESSED]=save_founded_device,
	[STATE_CONFIG][EV_FORGET_PRESSED]=forget_selected_device,
	[STATE_CONFIG][EV_MODBUS_REQUEST_RECIEVED]=ignored,
	[STATE_CONFIG][EV_ONE_WIRE_REQUEST_SENDED]=ignored,
	[STATE_CONFIG][EV_ONE_WIRE_REQUEST_RECIEVED]=ignored,
	[STATE_CONFIG][EV_POLL_SENSOR]=ignored,
	
	[STATE_RUN][EV_NONE]=wait_for_command,
	[STATE_RUN][EV_POLL_SENSOR]=sensors_polling,
	[STATE_RUN][EV_RUN_PRESSED]=device_config,
	[STATE_RUN][EV_SEARCH_PRESSED]=ignored,
	[STATE_RUN][EV_SAVE_PRESSED]=ignored,
	[STATE_RUN][EV_FORGET_PRESSED]=ignored,
	[STATE_RUN][EV_MODBUS_REQUEST_RECIEVED]=modbus_request_processing,
	[STATE_RUN][EV_ONE_WIRE_REQUEST_SENDED]=ignored,
	[STATE_RUN][EV_ONE_WIRE_REQUEST_RECIEVED]=ignored,
};



void hw_init(hw_device_t* device){
	_device_ptr = device;
	display_usart = usart_create(1,_device_ptr->display_baud);
	pc_usart = usart_create(0,_device_ptr->modbus_baud);
	nextion_display_init(&display_usart);
	events_init(16);
	init_display_rows();
	timer_init();
	sensor_poll_timer = timer_create();
	send_message_timer = timer_create();
	DDRC = 0b00001111;
}

void hw_run(void){
	
	timer_stop(sensor_poll_timer);
	timer_restart(send_message_timer, 1000);
	//uint8_t greatings[7] = "Hello!";

	while(1){
		
		_event = events_get();
		transition_table[_state][_event]();
		if(timer_poll(sensor_poll_timer)){
			events_put(EV_POLL_SENSOR);
			timer_restart(sensor_poll_timer, SENSOR_POLL_TIMEOUT);
		}
		
		if(usart_available_bytes(pc_usart)>=sizeof(modbus_request)){			
			usart_read(pc_usart, &modbus_request, sizeof(modbus_request));
			events_put(EV_MODBUS_REQUEST_RECIEVED);
		}
	}
}

static void device_run(void){
	timer_restart(sensor_poll_timer, SENSOR_POLL_TIMEOUT);
	_state = STATE_RUN;
}

static void device_config(void){
	timer_stop(sensor_poll_timer);
	_state = STATE_CONFIG;
}

static void save_founded_device(void){
	
}

static void forget_selected_device(void){
	
}

static void ignored(void){
	
}

static void wait_for_command(void){
	if(usart_available_bytes(display_usart)>=sizeof(_recieved_cmd_id)){
		usart_read(display_usart, &_recieved_cmd_id, sizeof(_recieved_cmd_id));
		events_put((event_t)_recieved_cmd_id.cmd);
	}
}

union value{
	uint32_t whole;
	float fraction;
	};

static void modbus_request_processing(void){
	if(_device_ptr->modbus_address != modbus_request.slave_addr){
		return;
	}
	if(crc16(&modbus_request, sizeof(modbus_request)) == 0){
		uint16_t crc = 0;
		union value val;
		swap_bytes(&modbus_request.reg_count);
		swap_bytes(&modbus_request.start_reg);
		//if(modbus_request.reg_count == 2)
		//PORTC |= 1;
		uint8_t response[7] ={0}; 
		response[0] = _device_ptr->modbus_address;
		response[1] = 0x03;
		response[2] = modbus_request.reg_count * 2;
		for(uint8_t i=0,j=3; i<(modbus_request.reg_count>>1); i++,j+=4){
			if(_device_ptr->modbus_address + i < NUM_OF_SENSORS){
				val.fraction = _sensors[modbus_request.start_reg + i].temperature >> 4;
				val.fraction += (_sensors[modbus_request.start_reg + i].temperature & 0x000F)/16.0f;
				response[j+1] = (uint8_t)(val.whole);
				response[j] = (uint8_t)(val.whole>>8);
				response[j+3] = (uint8_t)(val.whole>>16);
				response[j+2] = (uint8_t)(val.whole>>24);
			}else{
				break;
			}
		}
		
		/*for(uint8_t i = 0, j=3; i<response[2]; i++,j+=2){
			if(_device_ptr->modbus_address + i < NUM_OF_SENSORS){
				PORTC |= 1;
				response[j] = (uint8_t)_sensors[modbus_request.start_reg + i].temperature;
				response[j+1] = (uint8_t)(_sensors[modbus_request.start_reg + i].temperature>>8);
			}else{
				break;
			}			
		}*/
		crc = crc16(response,response_size);
		usart_write(pc_usart,response, response_size);
		usart_write(pc_usart,&crc,sizeof(uint16_t));
	}
}

static void sensors_polling(void){
	for(uint8_t i=0; i < NUM_OF_SENSORS; i++) //
	{
		
		if(_sensors[i].status == DEVICE_FOUND)
		{
			
			device_state state = ds18b20_get_temperature(&_sensors[i]);
			if(state == DEVICE_OK)
			{
				
				refresh_sensor_data(&_sensors[i],&_rows[i]);
			}
		}
	}
}

static void poll_founded_sensor(void){
	device_state state = ds18b20_get_temperature(&_sensors[_recieved_cmd_id.sensor_id]);
	if(state == DEVICE_OK)
	{
		refresh_sensor_data(&_sensors[_recieved_cmd_id.sensor_id],&_rows[_recieved_cmd_id.sensor_id]);
	}
}

static void search_device(void){
	device_state state = ds18b20_read_rom(&_sensors[_recieved_cmd_id.sensor_id]);
	if(state == DEVICE_OK)
	{
		_sensors[_recieved_cmd_id.sensor_id].status = DEVICE_FOUND;
		poll_founded_sensor();
		}else{
		_sensors[_recieved_cmd_id.sensor_id].status = DEVICE_MISSED;
	}
}


static void init_display_rows(void){
	nextion_display_create_row(&_rows[0], PSTR("n0.val="),PSTR("n1.val="),PSTR("p0.pic="));
	nextion_display_create_row(&_rows[1], PSTR("n2.val="),PSTR("n3.val="),PSTR("p1.pic="));
	nextion_display_create_row(&_rows[2], PSTR("n4.val="),PSTR("n5.val="),PSTR("p2.pic="));
	nextion_display_create_row(&_rows[3], PSTR("n6.val="),PSTR("n7.val="),PSTR("p3.pic="));
	nextion_display_create_row(&_rows[4], PSTR("n8.val="),PSTR("n9.val="),PSTR("p4.pic="));
	nextion_display_create_row(&_rows[5], PSTR("n10.val="),PSTR("n11.val="),PSTR("p5.pic="));
	nextion_display_create_row(&_rows[6], PSTR("n12.val="),PSTR("n13.val="),PSTR("p6.pic="));
	nextion_display_create_row(&_rows[7], PSTR("n14.val="),PSTR("n15.val="),PSTR("p7.pic="));
	nextion_display_create_row(&_rows[8], PSTR("n16.val="),PSTR("n17.val="),PSTR("p8.pic="));
	nextion_display_create_row(&_rows[9], PSTR("n18.val="),PSTR("n19.val="),PSTR("p9.pic="));
	
}

static void refresh_sensor_data(ds18b20_t *sensor, display_row* row){
	
	if(sensor->status == DEVICE_FOUND){
		
		row->whole.value = sensor->temperature >> 4;
		row->frac.value = get_frac(&sensor->temperature);
		row->conn.value = 1;
		nextion_display_refresh_row(row);
	}
}

static uint8_t get_frac(uint16_t *value)
{
	uint8_t frac = *value & 0x000F;
	float temp = (float)frac;
	temp /=16.0f;
	temp*=100.0;
	frac = (uint8_t)temp;
	return frac;
}

void swap_bytes(uint16_t* value) {
	uint8_t temp = (*value)>>8;
	*value <<= 8;
	*value |= temp;
}