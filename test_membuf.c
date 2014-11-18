#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "membuf.h"

static void assert_equal_int(int value, int expect, const char* msg) {
	if(value == expect) {
		printf("assertion ok: %s\n", msg);
	} else {
		printf("*** assertion FAIL: %s, got %d instead of %d\n", msg, value, expect);
	}
}

static void assert_not_equal_int(int value, int expect, const char* msg) {
	if(value == expect) {
		printf("*** assertion FAIL: %s, both are %d\n", msg, value);
	} else {
		printf("assertion ok: %s\n", msg);
	}
}

static void assert_equal_str(const char* value, const char* expect, const char* msg) {
	if(strcmp(value, expect) == 0) {
		printf("assertion ok: %s\n", msg);
	} else {
		printf("*** assertion FAIL: %s, got %s instead of %s\n", msg, value, expect);
	}
}

static void assert_equal_ptr(const void* value, const void* expect, const char* msg) {
	if(value == expect) {
		printf("assertion ok: %s\n", msg);
	} else {
		printf("*** assertion FAIL: %s, got %d instead of %d\n", msg, value, expect);
	}
}

static void assert_not_equal_ptr(const void* value, const void* expect, const char* msg) {
	if(value == expect) {
		printf("*** assertion FAIL: %s, both are %d\n", msg, value);
	} else {
		printf("assertion ok: %s\n", msg);
	}
}

static void assert_membuf(membuf_t* buf, const char* data, unsigned int size, unsigned int buffer_size, const char* msg) {
    assert_equal_str(buf->data, data, msg);
    assert_equal_int(buf->size, size, msg);
    assert_equal_int(buf->buffer_size, buffer_size, msg);
}

static void readme() {
    membuf_t buf; char on_stack_buffer[16];
    membuf_init_local(&buf, on_stack_buffer, sizeof(on_stack_buffer));
    membuf_append_text(&buf, "0123456789", 10); // uses stack buffer
    membuf_append_text(&buf, "ABCDEF", 6); // still uses stack buffer
    membuf_append_zeros(&buf, 1); // now it uses heap buffer
    printf("%s\n", buf.data); // will prints "0123456789ABCDEF"
    membuf_uninit(&buf);
}

static void simple() {
	membuf_t buf;
	membuf_init(&buf, 0);
	membuf_append_text(&buf, "dummy", -1);
	membuf_append_zeros(&buf, 1);
	assert_equal_str(buf.data, "dummy", "simple1");
    assert_equal_int(buf.size, 6, "simple1");
    assert_equal_int(buf.buffer_size, 10, "simple1");
	membuf_uninit(&buf);
}

static void append() {
	membuf_t buf, buf2;
	membuf_init(&buf, 0);
    assert_equal_ptr(buf.data, NULL, "init NULL data");
	membuf_append_byte(&buf, 'a');
	membuf_append_text(&buf, "bcd", 3);
	membuf_append_text(&buf, "1234567890ABC", -1);
	membuf_append_byte(&buf, 0);
	assert_equal_str(buf.data, "abcd1234567890ABC", "append1");
	membuf_uninit(&buf);

    membuf_init(&buf2, 8);
    assert_equal_int(buf2.buffer_size, 8, "init initial buffer size");
    assert_not_equal_ptr(buf2.data, NULL, "init initial buffer");
    assert_equal_int(buf2.size, 0, "init initial data size");
    membuf_append_text(&buf2, "12345678000", 8);
    assert_equal_int(buf2.buffer_size, 8, "buffer unchanged");
    membuf_append_byte(&buf2, 0);
    assert_equal_int(buf2.buffer_size, 16, "buffer changed");
    assert_equal_int(buf2.size, 9, "data changed");
    assert_equal_str(buf2.data, "12345678", "expect new data");
}

static void stack() {
	membuf_t buf; char stack_buf[8];
	unsigned int offset;
	char* text;
	membuf_init_local(&buf, stack_buf, sizeof(stack_buf));
	membuf_append_text_zero(&buf, "liigo", -1);
	assert_equal_str(buf.data, "liigo", "text on stack");
	assert_equal_str(buf.data, stack_buf, "uses stack buffer");
	assert_equal_int(buf.buffer_size, sizeof(stack_buf), "uses stack buffer");
	assert_equal_int(buf.uses_local_buffer, 1, "uses stack buffer");

	offset = membuf_append_text(&buf, "23", 2);
	text = (char*) membuf_get_data(&buf);
	assert_equal_int(text[offset], '2', "offset of new appented data");
	text[offset - 1] = '1';
	assert_equal_ptr(buf.data, stack_buf, "still uses stack buffer");
	assert_equal_int(buf.uses_local_buffer, 1, "still uses stack buffer");

	membuf_append_zeros(&buf, 1);
	assert_equal_int((int)buf.size, 9, "now size is 9");
	assert_equal_str(buf.data, "liigo123", "now data is liigo123/0");
	assert_not_equal_ptr(buf.data, stack_buf, "now uses alloced buffer");
	assert_not_equal_int(buf.buffer_size, sizeof(stack_buf), "now uses alloced buffer");
	assert_equal_int(buf.uses_local_buffer, 0, "now uses alloced buffer");

	membuf_uninit(&buf);
}

static void init_from_other() {
	membuf_t buf1, buf2, buf3; char stack_buf[8]; unsigned char* buf2data;
	membuf_init_local(&buf1, stack_buf, sizeof(stack_buf));
	membuf_append_text_zero(&buf1, "1234567", -1);
	membuf_init_from_other(&buf2, &buf1);
	assert_equal_str(buf2.data, "1234567", "buf2 is init from buf1");
	assert_equal_int(buf2.size, 8, "buf2 is init from buf1");
	assert_equal_int(buf2.uses_local_buffer, 0, "buf2 always not uses other's local buffer");
	assert_equal_ptr(buf1.data, NULL, "buf1 is hollowed by buf2");
	buf2data = buf2.data;
	membuf_init_from_other(&buf3, &buf2);
	assert_equal_ptr(buf3.data, buf2data, "buf3 use buf2's data directly");
	assert_equal_int(buf3.size, 8, "buf3 is init from buf2");
	assert_equal_ptr(buf2.data, NULL, "buf2 is hollowed by buf3");
	membuf_uninit(&buf3);
}

int main() {
    readme();
	simple();
	append();
	stack();
	init_from_other();
	return 0;
}
