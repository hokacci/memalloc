#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// This is a simple memory allocator that uses the sbrk system call to allocate
// memory.
// https://arjunsreedharan.org/post/148675821737/memory-allocators-101-write-a-simple-memory


#define SBRK_ERROR     ((void*)(-1))
#define ALIGNMENT_SIZE 16

typedef char Align[ALIGNMENT_SIZE];

typedef union Header Header;


union Header {
    struct {
        size_t size;
        bool is_free;
        Header* next;
    } s;
    Align stub;
};


static Header* head = NULL;
static Header* tail = NULL;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


static Header*
find_available_block(size_t size)
{
    Header* curr = head;
    while (curr != NULL) {
        if (curr->s.is_free && curr->s.size >= size) {
            return curr;
        }
        curr = curr->s.next;
    }
    return NULL;
}


static Header*
find_previous_block(Header* block)
{
    Header* curr = head;
    while (curr != NULL) {
        if (curr->s.next == block) {
            return curr;
        }
        curr = curr->s.next;
    }
    return NULL;
}


void*
yhok_memalloc(size_t size)
{
    if (size == 0) {
        return NULL;
    }

    pthread_mutex_lock(&lock);
    Header* header = find_available_block(size);

    // If there is an available block, return it
    if (header != NULL) {
        header->s.is_free = false;
        pthread_mutex_unlock(&lock);
        return (void*)(header + 1);
    }

    // If there is no available block, allocate a new block
    size_t total_size = sizeof(Header) + size;
    void* block = sbrk(total_size);

    if (block == SBRK_ERROR) {
        pthread_mutex_unlock(&lock);
        return NULL;
    }

    // Initialize the new block
    header = (Header*)block;
    header->s.size = size;
    header->s.is_free = false;
    header->s.next = NULL;

    // Add the new block to the list
    if (head == NULL) {
        head = header;
    }
    if (tail != NULL) {
        tail->s.next = header;
    }
    tail = header;

    pthread_mutex_unlock(&lock);
    return (void*)(header + 1);
}


void
yhok_free(void* block)
{
    if (block == NULL) {
        return;
    }

    pthread_mutex_lock(&lock);
    Header* header = (Header*)block - 1;

    void* program_break = sbrk(0);

    // If the block is the last one, release it
    if ((char*)block + header->s.size == program_break) {
        // Remove the block from the list
        if (head == tail) {
            head = NULL;
            tail = NULL;
        } else {
            Header* prev_tail = find_previous_block(tail);
            if (prev_tail != NULL) {
                prev_tail->s.next = NULL;
                tail = prev_tail;
            }
        }

        // Release the block
        sbrk(0 - sizeof(Header) - header->s.size);
        pthread_mutex_unlock(&lock);
        return;
    }

    // Otherwise, mark the block as free
    header->s.is_free = true;
    pthread_mutex_unlock(&lock);
}


void*
yhok_calloc(size_t num, size_t size)
{
    if (num == 0 || size == 0) {
        return NULL;
    }

    size_t total_size = num * size;

    // Check for overflow
    if (total_size / num != size) {
        return NULL;
    }

    void* block = yhok_memalloc(size);
    if (block == NULL) {
        return NULL;
    }

    memset(block, 0, total_size);
    return block;
}


void*
yhok_realloc(void* block, size_t size)
{
    if (block == NULL || size == 0) {
        return NULL;
    }

    Header* header = (Header*)block - 1;

    // If the new size is smaller than the current size, return the block
    if (header->s.size >= size) {
        return block;
    }

    // Allocate a new block with the new size
    void* new_block = yhok_memalloc(size);
    if (new_block == NULL) {
        return NULL;
    }
    memcpy(new_block, block, header->s.size);
    yhok_free(block);

    return new_block;
}
