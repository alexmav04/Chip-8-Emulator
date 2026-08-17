#include "chip8.h"
#include <memory.h>
#include <assert.h>
#include <stdbool.h>

const char chip_8_default_character_set[] = {
    0xf0, 0x90, 0x90, 0x90, 0xf0,  // 0
    0x20, 0x60, 0x20, 0x20, 0x70,  // 1
    0xf0, 0x10, 0xf0, 0x80, 0xf0,  // 2
    0xf0, 0x10, 0xf0, 0x10, 0xf0,  // 3
    0x90, 0x90, 0xf0, 0x10, 0x10,  // 4
    0xf0, 0x80, 0xf0, 0x10, 0xf0,  // 5
    0xf0, 0x80, 0xf0, 0x90, 0xf0,  // 6
    0xf0, 0x10, 0x20, 0x40, 0x40,  // 7
    0xf0, 0x90, 0xf0, 0x90, 0xf0,  // 8
    0xf0, 0x90, 0xf0, 0x10, 0xf0,  // 9
    0xf0, 0x90, 0xf0, 0x90, 0x90,  // A
    0xe0, 0x90, 0xe0, 0x90, 0xe0,  // B
    0xf0, 0x80, 0x80, 0x80, 0xf0,  // C
    0xe0, 0x90, 0x90, 0x90, 0xe0,  // D
    0xf0, 0x80, 0xf0, 0x80, 0xf0,  // E
    0xf0, 0x80, 0xf0, 0x80, 0x80   // F
};

void chip8_init(struct chip8* chip8) 
{
    memset(chip8, 0, sizeof(struct chip8));
    memcpy(&chip8->memory.memory[CHIP8_CHARACTER_LOAD_ADDRESS], 
        chip_8_default_character_set, 
        sizeof(chip_8_default_character_set));
}

void chip8_load(struct chip8* chip8, const char* buf, size_t size)
{
    assert(size + CHIP8_PROGRAM_LOAD_ADDRESS <= CHIP8_MEMORY_SIZE);
    memcpy(&chip8->memory.memory[CHIP8_PROGRAM_LOAD_ADDRESS], buf, size);
    chip8->registers.PC = CHIP8_PROGRAM_LOAD_ADDRESS;
}

static void chip8_exec_8xy(struct chip8* chip8, unsigned short opcode) 
{
    unsigned char x = (opcode >> 8) & 0x000F;
    unsigned char y = (opcode >> 4) & 0x000F;
    unsigned char final_four_bits = opcode & 0x000F;
    unsigned short tmp = 0;

    switch (final_four_bits)
    {
        case 0x00: // 8xy0 - LD Vx, Vy
            chip8->registers.V[x] = chip8->registers.V[y];
            break;
        case 0x01: // 8xy1 - OR Vx, Vy
            chip8->registers.V[x] |= chip8->registers.V[y];
            break;
        case 0x02: // 8xy2 - AND Vx, Vy
            chip8->registers.V[x] &= chip8->registers.V[y];
            break;
        case 0x03: // 8xy3 - XOR Vx, Vy
            chip8->registers.V[x] ^= chip8->registers.V[y];
            break;
        case 0x04: // 8xy4 - ADD Vx, Vy
            tmp = chip8->registers.V[x] + chip8->registers.V[y];
            chip8->registers.V[0x0f] = false;
            if (tmp > 0xff)
            {
                chip8->registers.V[0x0f] = true;
            }
            chip8->registers.V[x] = tmp;
            break;
        case 0x05: // 8xy5 - SUB Vx, Vy
            chip8->registers.V[0x0f] = false;
            if (chip8->registers.V[x] > chip8->registers.V[y])
            {
                chip8->registers.V[0x0f] = true;
            }
            chip8->registers.V[x] = chip8->registers.V[x] - chip8->registers.V[y];
            break;
        case 0x06: // 8xy6 - SHR Vx {, Vy}
            chip8->registers.V[0x0f] = chip8->registers.V[x] & 0x01;
            chip8->registers.V[x] >>= 1;
            break;
        case 0x07: // 8xy7 - SUBN Vx, Vy
            chip8->registers.V[0x0f] = chip8->registers.V[y] > chip8->registers.V[x];
            chip8->registers.V[x] = chip8->registers.V[y] - chip8->registers.V[x];
            break;
        case 0x0E: // 8xyE - SHL Vx {, Vy}
            chip8->registers.V[0x0f] = (chip8->registers.V[x] & 0x80) != 0;
            chip8->registers.V[x] <<= 1;
    }

}

static void chip8_exec_extended(struct chip8* chip8, unsigned short opcode) 
{
    unsigned short nnn = opcode & 0x0FFF;
    unsigned char x = (opcode >> 8) & 0x000F;
    unsigned char y = (opcode >> 4) & 0x000F;
    unsigned char kk = opcode & 0x00FF;

    switch (opcode & 0xF000)
    {
        case 0x1000: // JP addr
            chip8->registers.PC = nnn;
            break;
        case 0x2000: // CALL addr
            chip8_stack_push(chip8, chip8->registers.PC);
            chip8->registers.PC = nnn;
            break;
        case 0x3000: // SE Vx, byte
            if (chip8->registers.V[x] == kk)
            {
                chip8->registers.PC += 2;
            }
            break;
        case 0x4000: // SNE Vx, byte
            if (chip8->registers.V[x] != kk)
            {
                chip8->registers.PC += 2;
            }
            break;
        case 0x5000: // 5xy0 - SE, Vx, Vy
            if (chip8->registers.V[x] == chip8->registers.V[y])
            {
                chip8->registers.PC += 2;
            }
            break;
        case 0x6000: // 6xkk - LD Vx, byte
            chip8->registers.V[x] = kk;
            break;
        case 0x7000: // 7xkk - ADD Vx, byte
            chip8->registers.V[x] += kk;
            break;
        case 0x8000: // 8xyN - LD/OR/AND/XOR/ADD/SUB/SHR/SUBN/SHL Vx, Vy
            chip8_exec_8xy(chip8, opcode);
            break;
    }

}

void chip8_exec(struct chip8* chip8, unsigned short opcode) 
{
    switch (opcode)
    {
        case 0x00E0: // CLS
            chip8_screen_clear(&chip8->screen);
            break;
        case 0x00EE: // RET
            chip8->registers.PC = chip8_stack_pop(chip8);
            break;
        default:
            chip8_exec_extended(chip8, opcode);
    }
}