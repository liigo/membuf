#include "nodebuf.h"
#include <stdio.h>
#include <memory.h>
#include <assert.h>

void nodebuf_init(struct nodebuf_t *buf,
                  void *data, size_t bytes, uint16_t node_bytes)
{
    buf->data = (unsigned char*) data;
    buf->node_bytes = node_bytes;

    // the data's memory layout:
    // ______________________________________________
    // | node1 node2 ... nodeN | idxN ... idx2 idx1 |
    // | first_node            | indexes  first_idx |
    // N: the node count
    // by liigo 20200214, 20210906 redesigned.
    
    if (node_bytes == 0) {
        fprintf(stderr, "nodebuf_init error: node_bytes cannot be zero!\n");
        abort();
    }
    if (bytes < sizeof(uint16_t) + node_bytes) {
        fprintf(stderr, "nodebuf_init error: the buffer data is too small!\n");
        abort();
    }

    int n = bytes / (node_bytes + sizeof(uint16_t));
    if (n > UINT16_MAX) {
        fprintf(stderr, "nodebuf_init error: too many node count (%d) to manage!\n", n);
        abort();
    }

    buf->avail = n;
    buf->indexes = (uint16_t*) (buf->data + n * node_bytes);
    uint16_t *pi = buf->indexes + n - 1;
    for (int i = 0; i < n; i++) {
        *pi-- = i;
    }
}

void nodebuf_fini(struct nodebuf_t *buf, void (*free_fn)(void*)) {
    if (free_fn && buf->data)
        free_fn(buf->data);
    memset(buf, 0, sizeof(*buf));
}

int nodebuf_avail(struct nodebuf_t *buf) {
    return buf->avail;
}

void* nodebuf_malloc(struct nodebuf_t *buf, int zeromem) {
    void *p;
    if (buf->avail > 0) {
        buf->avail--;
        p = buf->data + (buf->indexes[buf->avail] * buf->node_bytes);
    } else {
        p = malloc(buf->node_bytes);
        printf("nodebuf_malloc: real malloc(%d) returns %p\n",
               buf->node_bytes, p);
    }

    if(p && zeromem) {
        memset(p, 0, buf->node_bytes);
    }
    return p;
}

void nodebuf_free(struct nodebuf_t *buf, void *node) {
    uint8_t *p = (uint8_t*)node;
    if (p >= buf->data && p < (uint8_t*)buf->indexes) {
        buf->indexes[buf->avail] = (p - buf->data) / buf->node_bytes;
        buf->avail++;
    } else {
        free(node);
        printf("nodebuf_free: real free(%p)\n", node);
    }
}
