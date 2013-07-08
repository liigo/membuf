#include <stdio.h>
#include <string.h>
#include <assert.h>

#define N 16

#include "membuf.h"

void assert_equal(int value, int expect, const char* msg) {
	if(value != expect) {
		printf("assertion fails: %s, got %d instead of %d\n", msg, value, expect);
	}
}

void assert_equal_str(const char* value, const char* expect, const char* msg) {
	if(strcmp(value, expect) != 0) {
		printf("assertion fails: %s, got %s instead of %s\n", msg, value, expect);
	}
}

void assert_not_equal(int value, int expect, const char* msg) {
	if(value == expect) {
		printf("assertion fails: %s, both are %d\n", msg, value);
	}
}

void simple() {
	membuf_t buf;
	char dummy[1024];
	strcpy(dummy, "dummy");
	membuf_init(&buf, 0);
	membuf_append_zeros(&buf, 8);
	membuf_uninit(&buf);
}

void append() {
	membuf_t buf;
	unsigned char* data_bak;

	membuf_init(&buf, 0);
	assert_equal(buf.data, buf.inline_buffer, "uses inline buffer");
	assert_equal(buf.buffer_size, N, "inline buffer size");

	data_bak = buf.data;
	membuf_append_byte(&buf, 'a');
	membuf_append_text(&buf, "bcd", 3);
	assert_equal(buf.data, data_bak, "no realloc");
	membuf_append_text(&buf, "1234567890AB", -1);
	assert_equal(buf.size, N, "full size");
	assert_equal(buf.data, data_bak, "no realloc");
	membuf_append_byte(&buf, 'x');
	assert_not_equal(buf.data, data_bak, "did realloc");
	assert_not_equal(buf.data, buf.inline_buffer, "out of inline buffer");
	assert_equal(buf.buffer_size, N*2, "expand buffer size");
	membuf_append_byte(&buf, 0);
	assert_equal_str(buf.data, "abcd1234567890ABx", "check data");
	assert_equal(buf.size, 18, "check size");
	
	membuf_uninit(&buf);
}


void exchange() {
	membuf_t buf1, buf2;
	membuf_init(&buf1, 0);
	membuf_init(&buf2, 0);

	membuf_append_text(&buf1, "12345678", -1);
	membuf_append_text(&buf2, "abcdefgh", -1);
	membuf_exchange(&buf1, &buf2);

	membuf_append_text(&buf2, "1234567890", -1);
	membuf_exchange(&buf1, &buf2);
	membuf_exchange(&buf1, &buf2);

	membuf_append_text(&buf1, "abcdefghijklmn", -1);
	membuf_exchange(&buf1, &buf2);
	membuf_exchange(&buf1, &buf2);	

	membuf_uninit(&buf1);
	membuf_uninit(&buf2);
}

int main() {
	simple();
	append();
	exchange();
	return 0;
}
