/*
 * hw.h
 *
 * Created: 15.09.2020 20:27:45
 *  Author: odins
 */ 


#ifndef HW_H_
#define HW_H_

typedef struct{
	uint8_t modbus_address;
	uint8_t num_sensors;
	uint32_t display_baud;
	uint32_t modbus_baud;
}hw_device_t;

void hw_init(hw_device_t* device);
void hw_run(void);
#endif /* HW_H_ */