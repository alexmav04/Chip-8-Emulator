#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <Windows.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_main.h"
#include "chip8.h"
#include "chip8_keyboard.h"

// Corresponds to CHIP-8 keys:
// 1 2 3 C
// 4 5 6 D
// 7 8 9 E
// A 0 B F
const char keyboard_map[CHIP8_TOTAL_KEYS] = {
    SDLK_x, SDLK_1, SDLK_2, SDLK_3, SDLK_q, SDLK_w, SDLK_e, SDLK_a, SDLK_s, SDLK_d, 
    SDLK_z, SDLK_c, SDLK_4, SDLK_r, SDLK_f, SDLK_v
};

int main(int argc, char** argv) 
{

    if (argc < 2)
    {
        printf("You must provide a CHIP-8 file to run.\n");
        return -1;
    }

    const char* filename = argv[1];
    printf("Loading CHIP-8 file: %s\n", filename);

    FILE* file = fopen(filename, "rb");
    if (!file)
    {
        printf("Failed to open the file: %s\n", filename);
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buf = malloc(size);
    size_t res = fread(buf, 1, size, file);
    fclose(file);
    if (res != (size_t)size)
    {
        printf("Failed to read the file: %s\n", filename);
        free(buf);
        return -1;
    }

    struct chip8 chip8;
    chip8_init(&chip8);
    chip8_load(&chip8, buf, size);
    free(buf);

    chip8_screen_draw_sprite(&chip8.screen, 62, 12, &chip8.memory.memory[0x00], 5);
    chip8.registers.SP = 0;

    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow(
        EMULATOR_WINDOW_TITLE, 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        CHIP8_WIDTH * CHIP8_WINDOW_MULTIPLIER, 
        CHIP8_HEIGHT * CHIP8_WINDOW_MULTIPLIER, 
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    while(1)
    {
        SDL_Event event;

        while(SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    goto out;
                    break;
                case SDL_KEYDOWN:
                    {
                        char key = event.key.keysym.sym;
                        int vkey = chip8_keyboard_map(keyboard_map, key);
                        if (vkey != -1)
                        {
                            chip8_keyboard_down(&chip8.keyboard, vkey);
                            printf("Key is down: %x\n", vkey);
                        }
                    }
                    break;
                case SDL_KEYUP:
                    {
                        char key = event.key.keysym.sym;
                        int vkey = chip8_keyboard_map(keyboard_map, key);
                        if (vkey != -1)
                        {
                            chip8_keyboard_up(&chip8.keyboard, vkey);
                            printf("Key is up.\n");
                        }
                    }
                    break;
                default:
                    break;
            }

        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);

        for (int x = 0; x < CHIP8_WIDTH; x++)
        {
            for (int y = 0; y < CHIP8_HEIGHT; y++)
            {   
                if (chip8_screen_is_set(&chip8.screen, x, y))
                {
                    SDL_Rect r;
                    r.x = x * CHIP8_WINDOW_MULTIPLIER;
                    r.y = y * CHIP8_WINDOW_MULTIPLIER;
                    r.w = CHIP8_WINDOW_MULTIPLIER;
                    r.h = CHIP8_WINDOW_MULTIPLIER;

                    SDL_RenderFillRect(renderer, &r);
                }
            }
        }
        SDL_RenderPresent(renderer);

        if (chip8.registers.delay_timer > 0)
        {
            Sleep(100);
            chip8.registers.delay_timer--;
            printf("Delay timer: %d\n", chip8.registers.delay_timer);
        }

        if (chip8.registers.sound_timer > 0)
        {
            Beep(1500, 100 * chip8.registers.sound_timer);
            chip8.registers.sound_timer = 0;
        }

        unsigned short opcode = chip8_memory_get_short(&chip8.memory, chip8.registers.PC);
        chip8_exec(&chip8, opcode);
        chip8.registers.PC += 2;
        printf("PC: %04x, Opcode: %04x\n", chip8.registers.PC, opcode);
    }

out:
    SDL_DestroyWindow(window);

    return 0;
}