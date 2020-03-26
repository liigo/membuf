#include "nodebuf.h"
#include <stdio.h>
#include <memory.h>
#include <assert.h>

static size_t *count_addr(struct nodebuf_t *buf) {
    return (size_t *) (buf->data + buf->bytes - sizeof(size_t));
}

static void **first_ptr_addr(struct nodebuf_t *buf) {
    size_t *count = count_addr(buf);
    return (void**)count - *count;
}

void nodebuf_init(struct nodebuf_t *buf,
                  void *data, size_t bytes, size_t node_bytes)
{
    int i;
    buf->data = (unsigned char*) data;
    buf->bytes = bytes;
    buf->node_bytes = node_bytes;

    // the data's memory layout:
    // __________________________________________________
    // | node1 node2 ... nodeN | ptr1 ptr2 ... ptrN | N |
    // | first_node            | first_ptr          |   |
    // by liigo 20200214.
    //
    assert(bytes >= sizeof(size_t));
    // N: the node count
    size_t N = (bytes - sizeof(size_t)) / (node_bytes + sizeof(void*));
    *count_addr(buf) = N;
    unsigned char *node = data;
    void **ptr = first_ptr_addr(buf);
    for (i = 0; i < N; i++) {
        *(ptr + i) = node;
        node += node_bytes;
    }

    // just for test
    /*
    printf("nodebuf_init: data=%p bytes=%d node_bytes=%d node_count=%d\n",
           data, bytes, node_bytes, N);
    for(i = 0; i < N; i++) {
        printf("  %d %p\n", i, *(ptr + i));
    }
    */
}

void nodebuf_fini(struct nodebuf_t *buf, void (*free_fn)(void*)) {
    if (free_fn && buf->data)
        free_fn(buf->data);
    memset(buf, 0, sizeof(*buf));
}

size_t nodebuf_count(struct nodebuf_t *buf) {
    return *count_addr(buf);
}

void* nodebuf_malloc(struct nodebuf_t *buf, int zeromem) {
    void *p;
    size_t *count = count_addr(buf);
    if (*count > 0) {
        p = *first_ptr_addr(buf);
        (*count)--;
    } else {
        p = malloc(buf->node_bytes);
        printf("nodebuf_malloc: real malloc(%d) call, returns %p\n",
               buf->node_bytes, p);
    }

    if(p && zeromem) {
        memset(p, 0, buf->node_bytes);
    }

    return p;
}

void nodebuf_free(struct nodebuf_t *buf, void *node) {
    unsigned char *p = node;
    if (p >= buf->data && p < buf->data + buf->bytes) {
        (*count_addr(buf))++;
        *first_ptr_addr(buf) = node;
    } else {
        free(node); // TODO: cache it (in buf->malloced) for future use
        printf("nodebuf_free: real free(%p) call\n", node);
    }
}
