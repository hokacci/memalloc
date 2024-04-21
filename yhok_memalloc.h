#pragma once

#include <stdlib.h>

void*
yhok_memalloc(size_t size);

void
yhok_free(void* block);

void*
yhok_calloc(size_t num, size_t size);

void*
yhok_realloc(void* block, size_t size);
