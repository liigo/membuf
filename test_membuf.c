#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "membuf.h"

void assert_equal_ptr(const void* value, const void* expect, const char* msg) {
	if(value == expect) {
		printf("assertion ok: %s\n", msg);
	} else {
		printf("*** assertion FAIL: %s, got %d instead of %d\n", msg, value, expect);
	}
}

void assert_equal_int(int value, int expect, const char* msg) {
	if(value == expect) {
		printf("assertion ok: %s\n", msg);
	} else {
		printf("*** assertion FAIL: %s, got %d instead of %d\n", msg, value, expect);
	}
}

void assert_equal_str(const char* value, const char* expect, const char* msg) {
	if(strcmp(value, expect) == 0) {
		printf("assertion ok: %s\n", msg);
	} else {
		printf("*** assertion FAIL: %s, got %s instead of %s\n", msg, value, expect);
	}
}

void assert_not_equal_ptr(const void* value, const void* expect, const char* msg) {
	if(value == expect) {
		printf("*** assertion FAIL: %s, both are %d\n", msg, value);
	} else {
		printf("assertion ok: %s\n", msg);
	}
}

void assert_not_equal_int(int value, int expect, const char* msg) {
	if(value == expect) {
		printf("*** assertion FAIL: %s, both are %d\n", msg, value);
	} else {
		printf("assertion ok: %s\n", msg);
	}
}


static void simple() {
	membuf_t buf;
	membuf_init(&buf, 0);
	membuf_append_text(&buf, "dummy", -1);
	membuf_append_zeros(&buf, 1);
	membuf_uninit(&buf);
}

static void append() {
	membuf_t buf;
	unsigned char* data_bak;

	membuf_init(&buf, 0);
	assert_equal_ptr(buf.data, buf.inline_buffer, "uses inline buffer");
	assert_equal_int(buf.buffer_size, MEMBUF_INLINE_CAPACITY, "uses inline buffer");

	data_bak = buf.data;
	membuf_append_byte(&buf, 'a');
	membuf_append_text(&buf, "bcd", 3);
	assert_equal_ptr(buf.data, data_bak, "no realloc");
	membuf_append_text(&buf, "1234567890AB", -1);
	assert_equal_ptr(buf.data, data_bak, "no realloc");
	membuf_append_zeros(&buf, MEMBUF_INLINE_CAPACITY);
	assert_not_equal_ptr(buf.data, data_bak, "did realloc");
	assert_not_equal_ptr(buf.data, buf.inline_buffer, "out of inline buffer");
	membuf_append_byte(&buf, 0);
	membuf_uninit(&buf);
}

static void exchange() {
	membuf_t buf1, buf2;
	unsigned int longlen = MEMBUF_INLINE_CAPACITY + 16;
	const char* str1 = "12345678";
	const char* str2 = "abcdefg";
	const char* longstr;
	char* tmp = (char*) malloc(longlen+1);
	memset(tmp, 'A', longlen);
	tmp[longlen] = '\0';
	longstr = tmp;

	//both use inline buffer
	membuf_init(&buf1, 0);
	membuf_init(&buf2, 0);
	membuf_append_text(&buf1, str1, strlen(str1)+1);
	membuf_append_text(&buf2, str2, strlen(str2)+1);
	assert_equal_ptr(buf1.data, buf1.inline_buffer, "buf1 use inline buffer");
	assert_equal_ptr(buf2.data, buf2.inline_buffer, "buf2 use inline buffer");
	membuf_exchange(&buf1, &buf2);
	assert_equal_str((const char*)buf1.data, str2, "exchange1, check data");
	assert_equal_str((const char*)buf2.data, str1, "exchange1, check data");
	membuf_uninit(&buf1);
	membuf_uninit(&buf2);

	//one use inline buffer, another not
	membuf_init(&buf1, 0);
	membuf_init(&buf2, 0);
	membuf_append_text(&buf1, str1, strlen(str1)+1);
	membuf_append_text(&buf2, longstr, longlen+1);
	membuf_exchange(&buf1, &buf2);
	assert_equal_str((const char*)buf1.data, longstr, "exchange2, check data");
	assert_equal_str((const char*)buf2.data, str1, "exchange2, check data");
	membuf_exchange(&buf1, &buf2);
	assert_equal_str((const char*)buf1.data, str1, "exchange2, check data");
	assert_equal_str((const char*)buf2.data, longstr, "exchange2, check data");
	membuf_uninit(&buf1);
	membuf_uninit(&buf2);
	
	//both not use inline buffer
	membuf_init(&buf1, 0);
	membuf_init(&buf2, 0);
	membuf_append_text(&buf1, longstr, longlen);
	membuf_append_text(&buf1, "A", 2);
	membuf_append_text(&buf2, longstr, longlen);
	membuf_append_text(&buf2, "BC", 3);
	membuf_exchange(&buf1, &buf2);
	assert_equal_str((const char*)buf1.data+longlen, "BC", "exchange3, check data");
	assert_equal_int(buf1.size, longlen + 3, "exchange3, check size");
	assert_equal_str((const char*)buf2.data+longlen, "A", "exchange3, check data");
	assert_equal_int(buf2.size, longlen + 2, "exchange3, check size");
	membuf_exchange(&buf1, &buf2);
	assert_equal_str((const char*)buf1.data+longlen, "A", "exchange3, check data");
	assert_equal_int(buf1.size, longlen + 2, "exchange3, check size");
	assert_equal_str((const char*)buf2.data+longlen, "BC", "exchange3, check data");
	assert_equal_int(buf2.size, longlen + 3, "exchange3, check size");
	membuf_uninit(&buf1);
	membuf_uninit(&buf2);

	free(tmp);
}

int main() {
	simple();
	append();
	exchange();
	return 0;
}
