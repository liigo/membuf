#ifndef ZZ_NODE_BUF_HEADER
#define ZZ_NODE_BUF_HEADER

#include <stdlib.h>
#include <stdint.h>

// manage a series of node inside the `data` memory space
struct nodebuf_t {
    unsigned char* data;
    size_t bytes;
    size_t node_bytes;
};

#ifdef __cplusplus
extern "C" {
#endif

void nodebuf_init(struct nodebuf_t *buf,
                   void *data, size_t bytes, size_t node_bytes);
void nodebuf_fini(struct nodebuf_t *buf, void (*free_fn)(void*));

size_t nodebuf_count(struct nodebuf_t *buf);
void* nodebuf_malloc(struct nodebuf_t *buf, int zeromem);
void nodebuf_free(struct nodebuf_t *buf, void *node);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ZZ_NODE_BUF_HEADER
