#include "membuf.h"
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <assert.h>

void membuf_init(membuf_t* buf, unsigned int init_buffer_size) {
	memset(buf, 0, sizeof(membuf_t));
	if(init_buffer_size <= MEMBUF_INLINE_CAPACITY) {
		buf->data = buf->inline_buffer;
		buf->buffer_size = MEMBUF_INLINE_CAPACITY;
	} else {
		buf->data = malloc(init_buffer_size);
		assert(buf->data);
		buf->buffer_size = init_buffer_size;
	}
}

void membuf_uninit(membuf_t* buf) {
	if(buf->data && buf->buffer_size > MEMBUF_INLINE_CAPACITY) {
		free(buf->data);
	}
	memset(buf, 0, sizeof(membuf_t));
}

void membuf_ensure_new_size(membuf_t* buf, unsigned int new_size) {
	unsigned int new_data_size = buf->size + new_size;
	if(new_data_size > buf->buffer_size) {
		//calculate new buffer size
		unsigned int new_buffer_size = buf->buffer_size * 2;
		while(new_buffer_size < new_data_size)
			new_buffer_size *= 2;

		// malloc/realloc new buffer and 
		assert(buf->buffer_size >= MEMBUF_INLINE_CAPACITY);
		if(buf->buffer_size == MEMBUF_INLINE_CAPACITY) {
			buf->data = malloc(new_buffer_size);
			memcpy(buf->data, buf->inline_buffer, buf->size);
			memset(buf->data + buf->size, 0, new_buffer_size - buf->size);
			memset(buf->inline_buffer, 0, MEMBUF_INLINE_CAPACITY);
		} else {
			buf->data = realloc(buf->data, new_buffer_size);
			memset(buf->data + buf->size, 0, new_buffer_size - buf->size);
		}
		buf->buffer_size = new_buffer_size;
	}
}

unsigned int membuf_append_data(membuf_t* buf, void* data, unsigned int size) {
	assert(data && size > 0);
	membuf_ensure_new_size(buf, size);
	memmove(buf->data + buf->size, data, size);
	buf->size += size;
	return (buf->size - size);
}

unsigned int membuf_append_zeros(membuf_t* buf, unsigned int size) {
	membuf_ensure_new_size(buf, size); //have done zero buffer
	memset(buf->data + buf->size, 0, size);
	buf->size += size;
	return (buf->size - size);
}

unsigned int membuf_append_text(membuf_t* buf, const char* str, unsigned int len) {
	if(str && (len == (unsigned int)(-1)))
		len = strlen(str);
	return membuf_append_data(buf, (void*)str, len);
}

void membuf_exchange(membuf_t* buf1, membuf_t* buf2) {
	assert(buf1 && buf2);
	unsigned char* tmp_data = buf1->data;
	unsigned int tmp_size = buf1->size, tmp_buffer_size = buf1->buffer_size;

	//exchange data
	assert((buf1->buffer_size >= MEMBUF_INLINE_CAPACITY) && (buf2->buffer_size >= MEMBUF_INLINE_CAPACITY));
	if(buf1->buffer_size == MEMBUF_INLINE_CAPACITY) {
		if(buf2->buffer_size == MEMBUF_INLINE_CAPACITY) {
			//both use inline buffer
			if(buf1->size <= buf2->size) {
				// #1
				unsigned char tmp_buffer[MEMBUF_INLINE_CAPACITY];
				memcpy(tmp_buffer, buf1->inline_buffer, buf1->size);
				memcpy(buf1->inline_buffer, buf2->inline_buffer, buf2->size);
				memcpy(buf2->inline_buffer, tmp_buffer, buf1->size);
				buf1->data = buf1->inline_buffer;
				buf2->data = buf2->inline_buffer;
			} else {
				membuf_exchange(buf2, buf1); //goto #1
				return;
			}
		} else {
			//buf1 uses inline buffer, buf2 not
			buf1->data = buf2->data; buf2->data = buf2->inline_buffer;
			memcpy(buf2->inline_buffer, buf1->inline_buffer, buf1->size);
		}
	} else {
		if(buf2->buffer_size > MEMBUF_INLINE_CAPACITY) {
			//both not use inline buffer
			buf1->data = buf2->data; buf2->data = tmp_data;
		} else {
			//buf2 uses inline buffer, buf1 not
			membuf_exchange(buf2, buf1); //goto #2
			return;
		}
	}

	//exchange size and buffer_size
	buf1->size = buf2->size; buf2->size = tmp_size;
	buf1->buffer_size = buf2->buffer_size; buf2->buffer_size = tmp_buffer_size;
}
