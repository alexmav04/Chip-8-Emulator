#include "chip8_stack.h"
#include "chip8.h"
#include <assert.h>

static void chip8_is_stack_in_bounds(struct chip8* chip8)
{
    assert(chip8->registers.SP < CHIP8_TOTAL_STACK_DEPTH);
}

void chip8_stack_push(struct chip8* chip8, unsigned short value)
{
    chip8_is_stack_in_bounds(chip8);
    chip8->stack.stack[chip8->registers.SP] = value;
    chip8->registers.SP++;
}

unsigned short chip8_stack_pop(struct chip8* chip8)
{
    chip8->registers.SP--;
    chip8_is_stack_in_bounds(chip8);
    unsigned short value = chip8->stack.stack[chip8->registers.SP];
    return value;
}