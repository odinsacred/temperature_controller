/*
* ringBuffer.c
*
* Created: 20.06.2019 13:51:19
*  Author: MIK
*/
#include <stdlib.h>
#include "buffer.h"
#include "assert.h"

struct _buffer_t {
	size_t tail;
	size_t head;
	size_t size;
	uint8_t bytes[MAX_BUFFER_SIZE];
};

struct _buffer_t buffers[BUFFER_COUNT] = 
{
    {0,0,MAX_BUFFER_SIZE,{0}},{0,0,MAX_BUFFER_SIZE,{0}},{0,0,MAX_BUFFER_SIZE,{0}},{0,0,MAX_BUFFER_SIZE,{0}}
};

buffer_t buffer_create(uint8_t index)
{
    assert(index<BUFFER_COUNT,1);
	return &buffers[index];
}

void buffer_write(buffer_t buffer, uint8_t value)
{
	assert(buffer->tail != (buffer->head - 1 + buffer->size) % buffer->size,2);
			
	buffer->bytes[buffer->tail] = value;
    buffer->tail++;
    buffer->tail &= MAX_BUFFER_SIZE - 1;
}

void buffer_write_bytes(buffer_t buffer, const void * data, size_t length)
{
	assert(length < buffer->size - buffer_count(buffer),3);
	uint8_t * bytes = (uint8_t *) data;
	
	for (size_t i = 0; i < length; i++, bytes++)
	{
		buffer_write(buffer, *bytes);
	}
}

uint8_t buffer_read(buffer_t buffer)
{
	assert(buffer_count(buffer) > 0,4);
	
	uint8_t value = buffer->bytes[buffer->head];

	buffer->head++;
    
    buffer->head &= MAX_BUFFER_SIZE - 1;

	return value;
}

size_t buffer_count(buffer_t buffer)
{
	
	if (buffer->tail >= buffer->head)
	{
		return (buffer->tail - buffer->head);
	}

	return (buffer->size - buffer->head) + buffer->tail;
}

void buffer_read_bytes(buffer_t buffer, void * data, uint8_t count)
{
	uint8_t available = buffer_count(buffer);

	assert(count <= available,5);
    
	uint8_t * bytes = (uint8_t *) data;
	
	count = available > count ? count : available;
	for (size_t i = 0; i < count; i++, bytes++)
	{
		*bytes = buffer_read(buffer);
	}
}

void buffer_clear(buffer_t buffer)
{
	buffer->tail = buffer->head = 0;
}

bool buffer_is_empty(buffer_t buffer)
{
	return buffer->tail == buffer->head;
}
