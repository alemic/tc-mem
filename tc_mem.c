
#include "tc_mem.h"

static int tc_mem_slabs_init(tc_mpool_t *pool);
static void tc_mem_chunks_init(char *p, int n, int size);
static tc_slab_t *tc_mem_slab_find(tc_mpool_t *pool, size_t size);
static void *tc_mem_block_alloc(size_t size);

static tc_slab_conf_t slabs[] = {
    { 8, 10 },
    { 16, 10 },
    { 32, 10 },
    { 64, 10 },
    { 128, 10 },
    { 256, 10 },
    { 512, 10 },
    { 1024, 10 },
    { 2048, 10 },
    { 3072, 10 },
    { 4096, 10 },
    { 0, 0 }
};

int
tc_mem_init(tc_mpool_t *pool)
{
    int        i, n;
    tc_slab_t *p;

    for (i = 0, n = 0; slabs[i].max; i++) {
        n++;
    }

    p = malloc(n * sizeof(tc_slab_t));
    if (p == NULL) {
        return TC_MEM_ERROR;
    }

    for (i = 0; i < n; i++) {
        p[i].size = slabs[i].max;
        p[i].n = slabs[i].num;
        p[i].free = NULL;
        p[i].head = NULL;
    }

    pool->head = p;
    pool->min = 8;
    pool->max = 4096;
    pool->slabs = n;

    if (tc_mem_slabs_init(pool) == TC_MEM_ERROR) {
        return TC_MEM_ERROR;
    }

    return TC_MEM_OK;
}

int
tc_mem_destroy(tc_mpool_t *pool)
{
    int         n, i;
    tc_slab_t  *s;
    tc_block_t *b;

    n = pool->slabs;

    for (i = 0; i < n; i++) {
        s = pool->head + i;
        while ((b = s->head)) {
            s->head = tc_mem_block_head(b->next);
            free(b);
        }
    }

    free(pool->head);

    return TC_MEM_OK;
}

void *
tc_mem_alloc(tc_mpool_t *pool, size_t size)
{
    char       *p;
    tc_slab_t  *s;
    tc_chunk_t *c;
    tc_block_t *b;

    s = tc_mem_slab_find(pool, size);

    /* size is too large */
    if (s == NULL) {
        return malloc(size);
    }

    /* no free chunk */
    if (s->free == NULL) {
        p = tc_mem_block_alloc(s->n * s->size);
        if (p == NULL) {
            return NULL;
        }

        s->block++;
        s->free = tc_mem_chunk_begin(p);
        b = tc_mem_block_head(p);
        b->next = s->head;
        s->head = b;


        tc_mem_chunks_init(tc_mem_chunk_begin(p), s->n, s->size);
    }

    c = (tc_chunk_t *) s->free;
    s->free = c->next;

    return c;
}

void
tc_mem_free(tc_mpool_t *pool, void *p, size_t size)
{
    tc_slab_t  *s;
    tc_chunk_t *c;

    s = tc_mem_slab_find(pool, size);

    /* size is too large */
    if (s == NULL) {
        free(p);
        return;
    }

    c = (tc_chunk_t *) p;
    c->next = s->free;
    s->free = c;
}

void
tc_mem_info(tc_mpool_t *pool)
{
    int        i, n;
    tc_slab_t *s;

    printf("slab num: %d    min slab: %d    max slab: %d\n",
            pool->slabs, pool->min, pool->max);

    n = pool->slabs;

    for (i = 0; i < n; i++) {
        s = pool->head + i;

        printf("slab <= %4d - blocks: %d, chunks: %d, size: %d\n",
                s->size, s->block, s->n, s->size);
    }
}

static int
tc_mem_slabs_init(tc_mpool_t *pool)
{
    int         i, j;
    char       *p;
    tc_slab_t  *s;
    tc_chunk_t *c;

    for (i = 0; i < pool->slabs; i++) {
        s = pool->head + i;

        p = tc_mem_block_alloc(s->n * s->size);
        if (p == NULL) {
            return TC_MEM_ERROR;
        }

        s->block = 1;
        s->free = tc_mem_chunk_begin(p);
        s->head = tc_mem_block_head(p);

        tc_mem_chunks_init(tc_mem_chunk_begin(p), s->n, s->size);
    }

    return TC_MEM_OK;
}

static void
tc_mem_chunks_init(char *p, int n, int size)
{
    int         i;
    tc_chunk_t *c;

    memset(p, 0, n * size);

    for (i = 0; i < n - 1; i++) {
        c = (tc_chunk_t *) p;
        p += size;
        c->next = p;
    }
}

/**
 * binary search
 */
static tc_slab_t *
tc_mem_slab_find(tc_mpool_t *pool, size_t size)
{
    int        first, last, m1, m2;
    tc_slab_t *h, *p1, *p2;

    h = pool->head;

    if (size <= pool->min) {
        return h;
    }

    if (size > pool->max) {
        return NULL;
    }

    first = 0;
    last = pool->slabs - 1;

    for ( ;; ) {
        m2 = (last - first + 1) / 2;
        m1 = m2 - 1;

        p1 = h + m1;
        p2 = h + m2;

        if (size > p1->size && size <= p2->size) {
            return p2;
        }

        if (size == p1->size) {
            return p1;
        }

        if (size < p1->size) {
            last = m1;
            continue;
        }

        if (size > p2->size) {
            first = m2;
            continue;
        }
    }
}


static void *
tc_mem_block_alloc(size_t size)
{
    char       *p;
    tc_block_t *b;

    p = malloc(size + sizeof(tc_block_t));
    if (p == NULL) {
        return NULL;
    }

    b = tc_mem_block_head(p);
    b->next = NULL;

    return p;
}
