/**
 * @file main.cpp
 * @author Ali Nasrolahi (a.nasrolahi01@gmail.com)
 * @date 2024-03-24
 */
#include "chip8.h"

#include <SDL2/SDL.h>
#include <errno.h>
#include <unistd.h>

int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0) {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

char buf[CHIP8_MEM_SIZE + 1024];

struct chip8 chip;
int main(int argc, char **argv)
{
    SDL_Rect squareRect;

    assert(argc > 1);

run:
    FILE *f = fopen(argv[1], "rb");
    assert(f);
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    buf[size] = '\0';
    rewind(f);
    assert(fread(buf, size, 1, f) == 1);
    fclose(f);

    chip8_init(&chip);
    chip8_load(&chip, buf, size);

    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window *window =
        SDL_CreateWindow(CHIP8_WINDOW_NAME, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                         CHIP8_SCALED_SCREEN_WIDTH, CHIP8_SCALED_SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_TEXTUREACCESS_TARGET);

    while (1) {

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            int i;
            switch (e.type) {
            case SDL_QUIT:
                goto quit;
                break;
            case SDL_KEYUP:
                if (chip8_keyboard_map(e.key.keysym.sym) != -1)
                    chip8_keyboard_release(&chip, chip8_keyboard_map(e.key.keysym.sym));
                break;
            case SDL_KEYDOWN:
                if (chip8_keyboard_map(e.key.keysym.sym) != -1)
                    chip8_keyboard_hold(&chip, chip8_keyboard_map(e.key.keysym.sym));
                break;
            }
            if (e.key.keysym.sym == 'q')
                exit(0);

            if (e.key.keysym.sym == 'r') {
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);

                SDL_Quit();
                goto run;
            }
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
            msleep(10), --chip.dt;

        chip8_exec(&chip);
    }

quit:
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
