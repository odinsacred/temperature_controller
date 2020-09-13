/*
* ringBuffer.h
*
* Created: 20.06.2019 13:55:34
*  Author: MIK
*/

#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_BUFFER_SIZE 64
#define BUFFER_COUNT 4

typedef struct _buffer_t * buffer_t;

buffer_t buffer_create(uint8_t index);
void buffer_write(buffer_t buffer, uint8_t byte);
void buffer_write_bytes(buffer_t buffer, const void * src, size_t length);
uint8_t buffer_read(buffer_t buffer);
void buffer_read_bytes(buffer_t buffer, void * dst, uint8_t length);
void buffer_clear(buffer_t buffer);
size_t buffer_count(buffer_t buffer);
bool buffer_is_empty(buffer_t buffer);

#endif /* RINGBUFFER_H_ */

