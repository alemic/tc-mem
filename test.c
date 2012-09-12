
#include <stdio.h>
#include <stdlib.h>

#include "tc_mem.h"


int main()
{
    char *p;
    tc_mpool_t pool;

    if (tc_mem_init(&pool) == TC_MEM_ERROR) {
        fprintf(stderr, "mem pool init failed\n");
        return -1;
    }

    p = tc_mem_malloc(&pool, 7);

    memset(p, 0, 7);
    memcpy(p, "123456", 6);
    printf("%s\n", p);

    tc_mem_free(&pool, p, 7);


    p = tc_mem_malloc(&pool, 17);

    memset(p, 0, 17);
    memcpy(p, "1234567", 7);
    printf("%s\n", p);

    tc_mem_free(&pool, p, 17);

    tc_mem_info(&pool);

    tc_mem_destroy(&pool);

    return 0;
}
