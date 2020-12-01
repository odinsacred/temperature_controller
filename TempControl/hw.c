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
#include <avr/eeprom.h>
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
static void load_device_roms(void);
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
	[STATE_CONFIG][EV_POLL_SENSOR]=ignored,
	
	[STATE_RUN][EV_NONE]=wait_for_command,
	[STATE_RUN][EV_POLL_SENSOR]=sensors_polling,
	[STATE_RUN][EV_RUN_PRESSED]=device_config,
	[STATE_RUN][EV_SEARCH_PRESSED]=ignored,
	[STATE_RUN][EV_SAVE_PRESSED]=ignored,
	[STATE_RUN][EV_FORGET_PRESSED]=ignored,
	[STATE_RUN][EV_MODBUS_REQUEST_RECIEVED]=modbus_request_processing
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
	
	load_device_roms();
}

void hw_run(void){
	
	timer_stop(sensor_poll_timer);
	timer_restart(send_message_timer, 1000);
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
	uint8_t counter = 0;
	uint16_t addr=1;
	while(!eeprom_is_ready());		
	for(uint8_t i=0; i<NUM_OF_SENSORS; i++,addr+=8){
		if(_sensors[i].status == DEVICE_FOUND){
			eeprom_write_block((void*)&_sensors[i].rom_code,(void*)addr,sizeof(uint64_t));
			counter++;			
		}
	}
	if(counter>0){
		eeprom_write_byte(0,counter);
		PORTC |=1;
	}
}

static void load_device_roms(void){
	uint8_t counter = 0;
	uint16_t addr=1;
	while(!eeprom_is_ready());	
	counter = eeprom_read_byte(0);
	if(counter != 0xFF){
		PORTC |=1;
		for(uint8_t i=0; i<counter; i++,addr+=8){
			eeprom_read_block((void*)&_sensors[i].rom_code,(const void*)addr,sizeof(uint64_t));
			_sensors[i].status = DEVICE_FOUND;
		}
		poll_founded_sensor();
	}
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
	uint8_t bytes[4];
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
		uint8_t response[3] ={0}; 
		response[0] = _device_ptr->modbus_address;
		response[1] = 0x03;
		response[2] = modbus_request.reg_count * 2;
		crc = crc16_once(response[0],0xFF,0xFF);
		crc = crc16_once(response[1],crc,crc>>8);
		crc = crc16_once(response[2],crc,crc>>8);
		usart_write(pc_usart,response, sizeof(response));
		for(uint8_t i=0; i<(modbus_request.reg_count>>1); i++){
				val.fraction = _sensors[modbus_request.start_reg + i].temperature >> 4;
				val.fraction += (_sensors[modbus_request.start_reg + i].temperature & 0x000F)/16.0f;				
				crc = crc16_once(val.bytes[0],crc,crc>>8);
				crc = crc16_once(val.bytes[1],crc,crc>>8);
				crc = crc16_once(val.bytes[2],crc,crc>>8);
				crc = crc16_once(val.bytes[3],crc,crc>>8);
				usart_write(pc_usart,&val.whole, sizeof(val.whole));		
		}
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