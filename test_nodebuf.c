#include <stdio.h>
#include <assert.h>

#include "nodebuf.h"

struct nodebuf_t buf;

void* check_malloc(int expect_value) {
    int *p = nodebuf_malloc(&buf, 0);
    if(*p != expect_value) {
        printf("test error: int value at %p is not %d\n", p, expect_value);
    } else {
        puts("test ok.");
    }
    return p;
}

int main() {
    void *p1, *p3, *p;
    int data[] = {100,101,102,103, 0,0};
    nodebuf_init(&buf, data, sizeof(data), sizeof(int));
    printf("nodebuf_avail() = %d\n", nodebuf_avail(&buf));
    assert(nodebuf_avail(&buf) == 4);

    p1 = check_malloc(100);
    check_malloc(101);
    check_malloc(102);
    p3 = check_malloc(103);
    assert(nodebuf_avail(&buf) == 0);
    nodebuf_free(&buf, p1);
    check_malloc(100);
    nodebuf_free(&buf, p3);
    check_malloc(103);
    assert(nodebuf_avail(&buf) == 0);
    p = check_malloc(999); // expect a test error here
    nodebuf_free(&buf, p);

    nodebuf_fini(&buf, NULL);
}
