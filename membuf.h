#ifndef __MEMBUF_H__
#define __MEMBUF_H__

// `membuf_t` is a growable continuous in-memory buffer.
// It also support "local buffer" to use stack memory efficiently.
// https://github.com/liigo/membuf
// by Liigo, 2013-7-5, 2014-8-16, 2014-10-21, 2014-11-18, 2015-3-7, 2015-7-21.

#ifdef __cplusplus
extern "C"	{
#endif

#include <stdlib.h>

typedef struct {
	unsigned char* data;
	unsigned int   size;
	unsigned int   buffer_size;
	unsigned char  uses_local_buffer;  // local buffer, e.g. on stack
} membuf_t;

#ifndef MEMBUF_INIT_LOCAL
	#define MEMBUF_INIT_LOCAL(buf,n) membuf_t buf; unsigned char buf##n[n]; membuf_init_local(&buf, &buf##n, n);
#endif

void membuf_init(membuf_t* buf, unsigned int initial_buffer_size);
void membuf_init_local(membuf_t* buf, void* local_buffer, unsigned int local_buffer_size);
void membuf_init_move_from(membuf_t* buf, membuf_t* other); // don't use other anymore
void membuf_uninit(membuf_t* buf);

// returns the offset of the new added data
unsigned int membuf_append_data(membuf_t* buf, void* data, unsigned int size);
unsigned int membuf_append_zeros(membuf_t* buf, unsigned int size);
unsigned int membuf_append_text(membuf_t* buf, const char* str, unsigned int len);
unsigned int membuf_append_text_zero(membuf_t* buf, const char* str, unsigned int len);

static void* membuf_get_data(membuf_t* buf) { return (buf->size == 0 ? NULL : buf->data); }
static unsigned int membuf_get_size(membuf_t* buf) { return buf->size; }
static unsigned int membuf_is_empty(membuf_t* buf) { return buf->size == 0; }
static void membuf_empty(membuf_t* buf) { buf->size = 0; }

void membuf_insert(membuf_t* buf, unsigned int offset, void* data, unsigned int size);
void membuf_remove(membuf_t* buf, unsigned int offset, unsigned int size);

void membuf_reserve(membuf_t* buf, unsigned int extra_size);
void* membuf_offset(membuf_t* buf, unsigned int offset);
static void* membuf_offset_uncheck(membuf_t* buf, unsigned int offset) { return buf->data + offset; }
void* membuf_detach(membuf_t* buf, unsigned int* psize); // need free() result if not NULL

#if defined(_MSC_VER)
	#define MEMBUF_INLINE static _inline
#else
	#define MEMBUF_INLINE static inline
#endif

MEMBUF_INLINE unsigned int membuf_append_byte(membuf_t* buf, unsigned char b) {
	return membuf_append_data(buf, &b, sizeof(b));
}
MEMBUF_INLINE unsigned int membuf_append_int(membuf_t* buf, int i) {
	return membuf_append_data(buf, &i, sizeof(i));
}
MEMBUF_INLINE unsigned int membuf_append_uint(membuf_t* buf, unsigned int ui) {
	return membuf_append_data(buf, &ui, sizeof(ui));
}
MEMBUF_INLINE unsigned int membuf_append_short(membuf_t* buf, short s) {
	return membuf_append_data(buf, &s, sizeof(s));
}
MEMBUF_INLINE unsigned int membuf_append_ushort(membuf_t* buf, unsigned short us) {
	return membuf_append_data(buf, &us, sizeof(us));
}
MEMBUF_INLINE unsigned int membuf_append_float(membuf_t* buf, float f) {
	return membuf_append_data(buf, &f, sizeof(f));
}
MEMBUF_INLINE unsigned int membuf_append_double(membuf_t* buf, double d) {
	return membuf_append_data(buf, &d, sizeof(d));
}
MEMBUF_INLINE unsigned int membuf_append_ptr(membuf_t* buf, void* ptr) {
	return membuf_append_data(buf, &ptr, sizeof(ptr));
}

// save membuf's data to file, returns data length in bytes written to file, or -1 if fails.
unsigned int membuf_save_to_file(membuf_t* buf, const char* file, const void* bom, unsigned int bomlen);
// append membuf's data to the end of file, returns data length in bytes written to file, or -1 if fails.
unsigned int membuf_append_to_file(membuf_t* buf, const char* file);
// load file's content and append to membuf, returns length in bytes of new added data, or -1 if fails.
unsigned int membuf_load_from_file(membuf_t* buf, const char* file, int append_zero_char);

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__MEMBUF_H__
