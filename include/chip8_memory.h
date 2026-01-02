#ifndef CHIP8_MEMORY_H
#define CHIP8_MEMORY_H

#include "config.h"

struct chip_memory
{
    char memory[CHIP8_MEMORY_SIZE];
};

void chip8_memory_set(struct chip_memory* memory, int index, unsigned char value);
unsigned char chip8_memory_get(struct chip_memory* memory, int index);

#endif // CHIP8_MEMORY_H