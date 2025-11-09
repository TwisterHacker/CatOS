#ifndef MEMORY_MANAGMENT_H_
#define MEMORY_MANAGMENT_H_

#define HEAP_START 0x100000  // Heap Start
#define HEAP_SIZE  0x1000000  // 10 MiB

void* kmalloc(size_t size);
void kfree(void* ptr);

#endif