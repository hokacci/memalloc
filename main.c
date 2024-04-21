#include <stdio.h>

#include "yhok_memalloc.h"

int main(void) {
    void* mem = yhok_memalloc(10);
    printf("Allocated memory at: %p\n", mem);
    mem = yhok_realloc(mem, 5);
    printf("Reallocated memory at: %p\n", mem);
    mem = yhok_realloc(mem, 20);
    printf("Reallocated memory at: %p\n", mem);
    yhok_free(mem);
    printf("Freed memory at: %p\n", mem);
    mem = yhok_calloc(10, 10);
    printf("CAllocated memory at: %p\n", mem);
    yhok_free(mem);
    printf("Freed memory at: %p\n", mem);

    return 0;
}