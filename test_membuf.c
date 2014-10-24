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

static void exchange() {
    // exchange bufs both on heap 
    {
        membuf_t buf1, buf2, buf3;
        membuf_init(&buf1, 0); membuf_append_text(&buf1, "123", 4);
        membuf_init(&buf2, 8); membuf_append_text(&buf2, "abcde", 6);
        membuf_exchange(&buf1, &buf2);
        assert_membuf(&buf1, "abcde", 6, 8, "exchange both on heap 1");
        assert_membuf(&buf2, "123", 4, 4, "exchange both on heap 1");
        membuf_exchange(&buf1, &buf2); // exchange back
        assert_membuf(&buf1, "123", 4, 4, "exchange both on heap 2");
        assert_membuf(&buf2, "abcde", 6, 8, "exchange both on heap 2");
        membuf_init(&buf3, 0); // no buffer allocated
        membuf_exchange(&buf2, &buf3);
        assert_membuf(&buf3, "abcde", 6, 8, "exchange both on heap 3");
        assert_equal_int(buf2.size, 0, "exchange both on heap 3");
        membuf_uninit(&buf1); membuf_uninit(&buf2); membuf_uninit(&buf3);
    }
    
    // exchange bufs both on stack
    {
        char data1[8]; char data2[32];
        membuf_t buf1, buf2, buf3;
        membuf_init_local(&buf1, data1, 8); membuf_append_text_zero(&buf1, "abc", 3);
        membuf_init_local(&buf2, data2, 32); membuf_append_text_zero(&buf2, "1", 1);
        membuf_exchange(&buf1, &buf2);
        assert_membuf(&buf1, "1", 2, 8, "exchange both on stack 1");
        assert_membuf(&buf2, "abc", 4, 32, "exchange both on stack 1");
        membuf_init_local(&buf3, data2, 32); membuf_append_text_zero(&buf3, "1234567890", -1);
        membuf_exchange(&buf1, &buf3);
        assert_membuf(&buf1, "1234567890", 11, 11, "exchange both on stack 2"); // now it use heap buffer
        assert_membuf(&buf3, "1", 2, 32, "exchange both on stack 2");
        membuf_uninit(&buf1); membuf_uninit(&buf2); membuf_uninit(&buf3);
    }

    // exchange bufs one on stack and one on heap
    {
        char data1[8];
        membuf_t buf1, buf2;
        membuf_init_local(&buf1, data1, 8); membuf_append_text_zero(&buf1, "abc", -1);
        membuf_init(&buf2, 0); membuf_append_text_zero(&buf2, "12345", -1);
        membuf_exchange(&buf1, &buf2);
        assert_membuf(&buf1, "12345", 6, 6, "exchange bufs one on stack and one on heap 1");
        assert_membuf(&buf2, "abc", 4, 4, "exchange bufs one on stack and one on heap 1");
        assert_equal_int(buf1.uses_local_buffer, 0, "buf1 uses buf2' buffer");
        membuf_exchange(&buf1, &buf2); // exchange back
        assert_membuf(&buf1, "abc", 4, 4, "exchange bufs one on stack and one on heap 2");
        assert_membuf(&buf2, "12345", 6, 6, "exchange bufs one on stack and one on heap 2");
        buf2.size = 0; membuf_append_text_zero(&buf2, "123456789", -1);
        membuf_exchange(&buf1, &buf2);
        assert_membuf(&buf1, "123456789", 10, 12, "exchange bufs one on stack and one on heap 3");
        assert_membuf(&buf2, "abc", 4, 4, "exchange bufs one on stack and one on heap 3");
        membuf_uninit(&buf1); membuf_uninit(&buf2);
    }
}

int main() {
    readme();
	simple();
	append();
	stack();
	exchange();
	return 0;
}
