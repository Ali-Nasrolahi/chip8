/**
 * @file main.cpp
 * @author Ali Nasrolahi (a.nasrolahi01@gmail.com)
 * @date 2024-03-24
 */

#include "chip8.h"

#include <SDL2/SDL.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    struct chip8 chip;
    SDL_Rect squareRect;

    assert(argc > 1);

    FILE *f = fopen(argv[1], "r");
    assert(f);
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    char buf[size];
    assert(fread(buf, size, 1, f) != 1);

    chip8_init(&chip);
    chip8_load(&chip, buf, size);

    chip8_screen_draw_sprite(&chip, 62, 30, &chip.memory[0x0A], 5);
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window *window =
        SDL_CreateWindow(CHIP8_WINDOW_NAME, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                         CHIP8_SCALED_SCREEN_WIDTH, CHIP8_SCALED_SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_TEXTUREACCESS_TARGET);

    while (1) {

        SDL_Event e;
        SDL_WaitEvent(&e);

        switch (e.type) {
        case SDL_QUIT:
            goto quit;
        case SDL_KEYUP:
            chip8_keyboard_release(&chip, e.key.keysym.sym);
            break;
        case SDL_KEYDOWN:
            chip8_keyboard_hold(&chip, e.key.keysym.sym);
            break;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);

        for (int x = 0; x < CHIP8_SCREEN_WIDTH; ++x)
            for (int y = 0; y < CHIP8_SCREEN_HEIGHT; ++y)
                if (chip8_screen_is_set(&chip, x, y)) {
                    squareRect.x = x * CHIP8_WINDOW_SCALE_RATIO;
                    squareRect.y = y * CHIP8_WINDOW_SCALE_RATIO;
                    squareRect.w = CHIP8_WINDOW_SCALE_RATIO;
                    squareRect.h = CHIP8_WINDOW_SCALE_RATIO;
                    SDL_RenderFillRect(renderer, &squareRect);
                }
        SDL_RenderPresent(renderer);

        if (chip.dt)
            sleep(100), --chip.dt;

        chip.pc += 2;
        chip8_exec(&chip, chip8_mem_getw(&chip, chip.pc));
    }

quit:
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
