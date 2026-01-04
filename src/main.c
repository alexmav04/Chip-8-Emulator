#include <stdio.h>
#include <stdbool.h>
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

    struct chip8 chip8;
    chip8_init(&chip8);
    
    chip8.registers.SP = 0;

    // Test stack functions
    chip8_stack_push(&chip8, 0xff);
    chip8_stack_push(&chip8, 0xaa);
    printf("Popped value: %x\n", chip8_stack_pop(&chip8));
    printf("Popped value: %x\n", chip8_stack_pop(&chip8));

    // Test memory functions
    chip8_memory_set(&chip8.memory, 0x400, 'Z');
    printf("Value at memory index 0x400: %c\n", chip8_memory_get(&chip8.memory, 0x400));

    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow(
        EMULATOR_WINDOW_TITLE, 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        CHIP8_WIDTH * CHIP8_WINDOW_MULTIPLIER, 
        CHIP8_HEIGHT * CHIP8_WINDOW_MULTIPLIER, 
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_TEXTUREACCESS_TARGET);

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
                        char vkey = chip8_keyboard_map(keyboard_map, key);
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
                        char vkey = chip8_keyboard_map(keyboard_map, key);
                        if (vkey != -1)
                        {
                            chip8_keyboard_up(&chip8.keyboard, vkey);
                            printf("Key is up.\n");
                        }
                    }
                    break;
                default:
                    break;
            };

        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
        
        SDL_Rect r;
        r.x = 0;
        r.y = 0;
        r.w = 40;
        r.h = 40;

        SDL_RenderFillRect(renderer, &r);
        SDL_RenderPresent(renderer);
    }

out:
    SDL_DestroyWindow(window);

    return 0;
}