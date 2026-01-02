#include "chip8_memory.h"
#include <assert.h>

static void chip8_is_memory_index_valid(int index)
{
    assert(index >= 0 && index < CHIP8_MEMORY_SIZE);
}

void chip8_memory_set(struct chip_memory* memory, int index, unsigned char value)
{
    chip8_is_memory_index_valid(index);
    memory->memory[index] = value;
}

unsigned char chip8_memory_get(struct chip_memory* memory, int index)
{
    chip8_is_memory_index_valid(index);
    return memory->memory[index];
}