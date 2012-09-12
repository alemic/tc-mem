#ifndef __TC_MEM_H__
#define __TC_MEM_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TC_MEM_ERROR -1
#define TC_MEM_OK     0

typedef struct tc_block_s tc_block_t;

#define tc_mem_chunk_begin(p) ((char *) (p) + sizeof(tc_block_t))
#define tc_mem_block_head(p) ((tc_block_t *) (p))


typedef struct {
    int max;
    int num;
} tc_slab_conf_t;

typedef struct {
    int         size;     /* size of a chunk */
    int         n;        /* number of chunks */
    int         block;    /* number of blocks */
    void       *free;
    tc_block_t *head;
} tc_slab_t;

struct tc_block_s {
    void *next;
};

typedef struct {
    void *next;
} tc_chunk_t;

typedef struct {
    int        slabs; /* number of slabs */
    int        min;
    int        max;
    tc_slab_t *head;
} tc_mpool_t;

int tc_mem_init(tc_mpool_t *pool);
int tc_mem_destroy(tc_mpool_t *pool);
void *tc_mem_malloc(tc_mpool_t *pool, size_t size);
void tc_mem_free(tc_mpool_t *pool, void *p, size_t size);
void tc_mem_info(tc_mpool_t *pool);

#endif /* __TC_MEM_H__ */
