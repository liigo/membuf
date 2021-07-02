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
    // __________________________________________________
    // | node1 node2 ... nodeN | N | idxN ... idx2 idx1 |
    // | first_node            pn  |          first_idx |
    // N: the node count
    // by liigo 20200214, 20210702 redesigned.
    
    if (node_bytes == 0) {
        fprintf(stderr, "nodebuf_init error: node_bytes cannot be zero!\n");
        abort();
    }
    if (bytes < sizeof(uint16_t) + sizeof(uint16_t) + node_bytes) {
        fprintf(stderr, "nodebuf_init error: the buffer data is too small!\n");
        abort();
    }

    int N = (bytes - sizeof(uint16_t)) / (node_bytes + sizeof(uint16_t));
    if (N > UINT16_MAX) {
        fprintf(stderr, "nodebuf_init error: too many node count (%d) to manage!\n", N);
        abort();
    }

    buf->pn = (uint16_t*) (buf->data + N * node_bytes);
    buf->pn[0] = N;
    uint16_t *pi = buf->pn + N;
    for (int i = 0; i < N; i++) {
        *pi-- = i;
    }
}

void nodebuf_fini(struct nodebuf_t *buf, void (*free_fn)(void*)) {
    if (free_fn && buf->data)
        free_fn(buf->data);
    memset(buf, 0, sizeof(*buf));
}

int nodebuf_count(struct nodebuf_t *buf) {
    return buf->pn[0];
}

void* nodebuf_malloc(struct nodebuf_t *buf, int zeromem) {
    void *p;
    uint16_t *pn = buf->pn;
    int n = *pn;
    if (n > 0) {
        p = buf->data + pn[n] * buf->node_bytes;
        (*pn)--;
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
    uint16_t *pn = buf->pn;
    if (p >= buf->data && p < (uint8_t*)pn) {
        int n = ++(*pn);
        pn[n] = (p - buf->data) / buf->node_bytes;
    } else {
        free(node);
        printf("nodebuf_free: real free(%p)\n", node);
    }
}
