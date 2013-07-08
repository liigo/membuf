#ifndef __MEMBUF_H__
#define __MEMBUF_H__

// membuf is an auto-enlarging continuous in-memory buffer.
// it also has inline-buffer to use stack memory efficiently.
// by liigo, 2013-7-5.
// https://github.com/liigo/membuf

#ifndef MEMBUF_INLINE_CAPACITY
	#define MEMBUF_INLINE_CAPACITY  512
#endif

typedef struct {
	unsigned char* data;
	unsigned int   size;
	unsigned int   buffer_size;
	unsigned char  inline_buffer[MEMBUF_INLINE_CAPACITY];
} membuf_t;

void membuf_init(membuf_t* buf, unsigned int init_buffer_size);
void membuf_uninit(membuf_t* buf);

unsigned int membuf_append_data(membuf_t* buf, void* data, unsigned int size);
unsigned int membuf_append_zeros(membuf_t* buf, unsigned int size);
unsigned int membuf_append_text(membuf_t* buf, const char* str, unsigned int len);

void membuf_ensure_new_size(membuf_t* buf, unsigned int new_size);
void membuf_exchange(membuf_t* buf1, membuf_t* buf2);

static inline unsigned int membuf_append_byte(membuf_t* buf, unsigned char b) {
	return membuf_append_data(buf, &b, sizeof(b));
}

static inline unsigned int membuf_append_int(membuf_t* buf, int i) {
	return membuf_append_data(buf, &i, sizeof(i));
}

static inline unsigned int membuf_append_uint(membuf_t* buf, unsigned int ui) {
	return membuf_append_data(buf, &ui, sizeof(ui));
}

static inline unsigned int membuf_append_short(membuf_t* buf, short s) {
	return membuf_append_data(buf, &s, sizeof(s));
}

static inline unsigned int membuf_append_ushort(membuf_t* buf, short us) {
	return membuf_append_data(buf, &us, sizeof(us));
}


#endif //__MEMBUF_H__
