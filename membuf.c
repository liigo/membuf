#include "membuf.h"
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

void membuf_init(membuf_t* buf, unsigned int initial_buffer_size) {
	memset(buf, 0, sizeof(membuf_t));
	buf->data = initial_buffer_size > 0 ? (unsigned char*) malloc(initial_buffer_size) : NULL;
	buf->buffer_size = initial_buffer_size;
	buf->uses_local_buffer = 0;
}

void membuf_init_local(membuf_t* buf, void* local_buffer, unsigned int local_buffer_size) {
	memset(buf, 0, sizeof(membuf_t));
	buf->data = (local_buffer && (local_buffer_size > 0)) ? local_buffer : NULL;
	buf->buffer_size = local_buffer_size;
	buf->uses_local_buffer = 1;
}

void membuf_uninit(membuf_t* buf) {
	if(!buf->uses_local_buffer && buf->data)
		free(buf->data);
}

void membuf_ensure_new_size(membuf_t* buf, unsigned int new_size) {
	if(new_size > buf->buffer_size - buf->size) {
		//calculate new buffer size
		unsigned int new_buffer_size = buf->buffer_size == 0 ? new_size : buf->buffer_size << 1;
		unsigned int new_data_size = buf->size + new_size;
		while(new_buffer_size < new_data_size)
			new_buffer_size <<= 1;

		// malloc/realloc new buffer
		if(buf->uses_local_buffer) {
            void* local = buf->data;
			buf->data = realloc(NULL, new_buffer_size); // alloc new buffer
			memcpy(buf->data, local, buf->size); // copy local bufer to new buffer
			buf->uses_local_buffer = 0;
		} else {
			buf->data = realloc(buf->data, new_buffer_size); // realloc new buffer
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
	membuf_ensure_new_size(buf, size);
	memset(buf->data + buf->size, 0, size);
	buf->size += size;
	return (buf->size - size);
}

unsigned int membuf_append_text(membuf_t* buf, const char* str, unsigned int len) {
	if(str && (len == (unsigned int)(-1)))
		len = strlen(str);
	return membuf_append_data(buf, (void*)str, len);
}

unsigned int membuf_append_text_zero(membuf_t* buf, const char* str, unsigned int len) {
	unsigned int offset;
	if(str && (len == (unsigned int)(-1)))
		len = strlen(str);
	offset = membuf_append_data(buf, (void*)str, len);
	membuf_append_zeros(buf, 1);
	return offset;
}

void* membuf_detach(membuf_t* buf, unsigned int* psize) {
	void* result = buf->data;
	if(psize) *psize = buf->size;
	if(buf->uses_local_buffer) {
		result = buf->size > 0 ? malloc(buf->size) : NULL;
		memcpy(result, buf->data, buf->size);
	} else {
		buf->buffer_size = 0;
	}
	buf->data = NULL;
	buf->size = 0;
	return result;
}

MEMBUF_INLINE void swap_data(membuf_t* buf1, membuf_t* buf2) {
	unsigned char* tmp_data = buf1->data;
	buf1->data = buf2->data; buf2->data = tmp_data;
}

MEMBUF_INLINE void swap_size(membuf_t* buf1, membuf_t* buf2) {
	unsigned int tmp_size = buf1->size;
	buf1->size = buf2->size; buf2->size = tmp_size;
}

MEMBUF_INLINE void swap_buffer_size(membuf_t* buf1, membuf_t* buf2) {
	unsigned int tmp_buffer_size = buf1->buffer_size;
	buf1->buffer_size = buf2->buffer_size; buf2->buffer_size = tmp_buffer_size;
}

void membuf_exchange_data(membuf_t* buf1, membuf_t* buf2) {
	assert(buf1 && buf2);

	//exchange data
	if(buf1->uses_local_buffer == 0 && buf2->uses_local_buffer == 0) {
		swap_data(buf1, buf2);
		swap_size(buf1, buf2);
		swap_buffer_size(buf1, buf2);
		return;
	}

	if(buf1->uses_local_buffer) {
		if(buf2->uses_local_buffer) {
			//both use local buffer
			// #1
			if(buf1->buffer_size >= buf2->size && buf2->buffer_size >= buf1->size) {
				if(buf1->size >= buf2->size) {
					// #1.1
					unsigned char tmp_data[USHRT_MAX]; // uses const-sized array for compatibility
					memcpy(tmp_data, buf2->data, buf2->size); // buf2->size is smaller than buf1->size
					memcpy(buf2->data, buf1->data, buf1->size);
					memcpy(buf1->data, tmp_data, buf2->size);
					swap_size(buf1, buf2);
				} else {
					membuf_exchange_data(buf2, buf1); //goto #1.1
				}
				return;
			} else if(buf1->buffer_size < buf2->size && buf2->buffer_size < buf1->size) {
				// #1.2
				buf1->uses_local_buffer = buf2->uses_local_buffer = 0;
				buf1->data = (unsigned char*) malloc(buf2->size);
				memcpy(buf1->data, buf2->data, buf2->size);
				buf1->buffer_size = buf2->size;
				buf2->data = (unsigned char*) malloc(buf1->size);
				memcpy(buf2->data, buf1->data, buf1->size);
				buf2->buffer_size = buf1->size;
				swap_size(buf1, buf2);
				return;
			} else {
				if(buf1->buffer_size < buf2->size) {
					// #1.3
                    void* buf1_local = buf1->data;
					buf1->uses_local_buffer = 0;
					buf1->data = (unsigned char*) malloc(buf2->size);
					memcpy(buf1->data, buf2->data, buf2->size);
					buf1->buffer_size = buf2->size;
					memcpy(buf2->data, buf1_local, buf1->size);
					swap_size(buf1, buf2);
				} else {
					membuf_exchange_data(buf2, buf1); //goto #1.3
				}
				return;
			}
		} else {
			// #2 
			//buf1 uses local buffer, buf2 not
			unsigned char* buf1_data = buf1->data;
            buf1->uses_local_buffer = 0;
            buf1->data = buf2->data;
            buf1->buffer_size = buf2->buffer_size;
            buf2->data = (unsigned char*) malloc(buf1->size);
            memcpy(buf2->data, buf1_data, buf1->size);
            buf2->buffer_size = buf1->size;
			swap_size(buf1, buf2);
			return;
		}
	} else {
		if(buf2->uses_local_buffer) {
			//buf2 uses local buffer, buf1 not
			membuf_exchange_data(buf2, buf1); //goto #2
			return;
		}
	}
}
