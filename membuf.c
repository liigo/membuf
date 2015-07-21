#include "membuf.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

// https://github.com/liigo/membuf
// by Liigo.

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

void membuf_init_move_from(membuf_t* buf, membuf_t* other) {
    if(other->uses_local_buffer) {
        membuf_init(buf, 0);
        if(other->size > 0)
			membuf_append_data(buf, other->data, other->size);
    } else {
        *buf = *other;
    }
    memset(other, 0, sizeof(membuf_t)); // other is hollowed now
}

void membuf_uninit(membuf_t* buf) {
	if(!buf->uses_local_buffer && buf->data)
		free(buf->data);
    memset(buf, 0, sizeof(membuf_t));
}

void membuf_reserve(membuf_t* buf, unsigned int extra_size) {
	if(extra_size > buf->buffer_size - buf->size) {
		//calculate new buffer size
		unsigned int new_buffer_size = buf->buffer_size == 0 ? extra_size : buf->buffer_size << 1;
		unsigned int new_data_size = buf->size + extra_size;
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
	membuf_reserve(buf, size);
	memmove(buf->data + buf->size, data, size);
	buf->size += size;
	return (buf->size - size);
}

unsigned int membuf_append_zeros(membuf_t* buf, unsigned int size) {
	membuf_reserve(buf, size);
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
    membuf_reserve(buf, len + 1);
	offset = membuf_append_data(buf, (void*)str, len);
	membuf_append_zeros(buf, 1);
	return offset;
}

void membuf_insert(membuf_t* buf, unsigned int offset, void* data, unsigned int size) {
	assert(offset < buf->size);
	membuf_reserve(buf, size);
	memcpy(buf->data + offset + size, buf->data + offset, buf->size - offset);
	memcpy(buf->data + offset, data, size);
	buf->size += size;
}

void membuf_remove(membuf_t* buf, unsigned int offset, unsigned int size) {
	assert(offset < buf->size);
	if (offset + size >= buf->size) {
		buf->size = offset;
	} else {
		memmove(buf->data + offset, buf->data + offset + size, buf->size - offset - size);
		buf->size -= size;
	}
}

void* membuf_offset(membuf_t* buf, unsigned int offset) {
	assert(offset < buf->size);
	return (buf->size == 0) ? NULL : buf->data + offset;
}

void* membuf_detach(membuf_t* buf, unsigned int* psize) {
	void* result = buf->data;
	if(psize) *psize = buf->size;
	if(buf->uses_local_buffer && buf->size > 0) {
		result = malloc(buf->size);
		memcpy(result, buf->data, buf->size);
	}
	memset(buf, 0, sizeof(membuf_t));
	return result;
}

unsigned int membuf_save_to_file(membuf_t* buf, const char* file, const void* bom, unsigned int bomlen) {
	FILE* pfile = fopen(file, "wb+");
	if (pfile) {
		unsigned int bytes = 0;
		if (bom)
			bytes += fwrite(bom, 1, bomlen, pfile);
		if (buf->size > 0)
			bytes += fwrite(buf->data, 1, buf->size, pfile);
		fclose(pfile);
		return bytes;
	}
	return -1;
}

unsigned int membuf_append_to_file(membuf_t* buf, const char* file) {
	FILE* pfile = fopen(file, "ab+");
	if (pfile) {
		unsigned int bytes = fwrite(buf->data, 1, buf->size, pfile);
		fclose(pfile);
		return bytes;
	}
	return -1;
}

unsigned int membuf_load_from_file(membuf_t* buf, const char* file, int append_zero_char) {
	if (file == NULL) return 0;
	FILE* pfile = fopen(file, "rb");
	if (pfile) {
		unsigned int read_bytes = 0;
		long filelen;
		fseek(pfile, 0, SEEK_END);
		filelen = ftell(pfile);
		fseek(pfile, 0, SEEK_SET);
		if (filelen > 0) {
			membuf_reserve(buf, filelen);
			read_bytes = fread(membuf_offset_uncheck(buf, buf->size), 1, filelen, pfile);
			assert(read_bytes == filelen);
			buf->size += read_bytes;
		}
		if (append_zero_char) {
			membuf_append_zeros(buf, 1);
			read_bytes++;
		}
		fclose(pfile);
		return read_bytes;
	}
	return -1;
}
