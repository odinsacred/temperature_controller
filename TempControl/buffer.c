/*
* ringBuffer.c
*
* Created: 20.06.2019 13:51:19
*  Author: MIK
*/
#include <stdlib.h>
//#include "debug.h"
#include "buffer.h"

struct _buffer_t {
	size_t tail;
	size_t head;
	size_t size;
	uint8_t * bytes;
};

buffer_t buffer_create(size_t size)
{
	buffer_t buffer = calloc(1, sizeof(struct _buffer_t));
	buffer->bytes = calloc(1, size);
	buffer->size = size;
	
	return buffer;
}

void buffer_write(buffer_t buffer, uint8_t value)
{
	//debug_assert(buffer->tail != (buffer->head - 1 + buffer->size) % buffer->size, "buffer full");
			
	buffer->bytes[buffer->tail] = value;
	buffer->tail = (buffer->tail + 1) % buffer->size;
}

void buffer_write_bytes(buffer_t buffer, const void * data, size_t length)
{
	//debug_assert(length < buffer->size - buffer_count(buffer), "buffer to small");
	uint8_t * bytes = (uint8_t *) data;
	
	for (size_t i = 0; i < length; i++, bytes++)
	{
		buffer_write(buffer, *bytes);
	}
}

uint8_t buffer_read(buffer_t buffer)
{
	//debug_assert(buffer_count(buffer) > 0, "buffer is empty");
	
	uint8_t value = buffer->bytes[buffer->head];
			
	buffer->head = (buffer->head + 1) % buffer->size;

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

void buffer_read_bytes(buffer_t buffer, void * data, size_t count)
{
	size_t available = buffer_count(buffer);

	//assert(count <= available);

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