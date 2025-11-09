#include <common.h>
#include <memory/memory_managment.h>
static uint8_t kernel_heap[HEAP_SIZE];
static uint8_t* heap = kernel_heap;

typedef struct block_header {
    size_t size;
    int free;
    struct block_header* next;
} block_header_t;

static block_header_t* heap_head = NULL;
static size_t used_memory = 0;

void* kmalloc(size_t size) {
    if (size == 0) return NULL;

    // Вирівнювання до 8 байтів
    size = (size + 7) & ~7;

    block_header_t* current = heap_head;

    // Пошук вільного блоку
    while (current != NULL) {
        if (current->free && current->size >= size) {
            current->free = 0;
            return (void*)((uint8_t*)current + sizeof(block_header_t));
        }
        current = current->next;
    }

    // Немає вільного блоку — створюємо новий
    size_t total_size = sizeof(block_header_t) + size;
    if (used_memory + total_size > HEAP_SIZE) return NULL;

    block_header_t* new_block = (block_header_t*)(heap + used_memory);
    new_block->size = size;
    new_block->free = 0;
    new_block->next = NULL;

    if (heap_head == NULL) {
        heap_head = new_block;
    } else {
        current = heap_head;
        while (current->next != NULL) current = current->next;
        current->next = new_block;
    }

    used_memory += total_size;
    return (void*)((uint8_t*)new_block + sizeof(block_header_t));
}

void kfree(void* ptr) {
    if (!ptr) return;

    block_header_t* block = (block_header_t*)((uint8_t*)ptr - sizeof(block_header_t));
    block->free = 1;

    // Об'єднання вільних блоків
    block_header_t* current = heap_head;
    while (current && current->next) {
        if (current->free && current->next->free) {
            current->size += sizeof(block_header_t) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}