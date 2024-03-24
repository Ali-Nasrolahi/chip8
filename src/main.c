/**
 * @file main.cpp
 * @author Ali Nasrolahi (a.nasrolahi01@gmail.com)
 * @date 2024-03-24
 */

#include "chip8.h"

#include <SDL2/SDL.h>
#include <sys/param.h> /* MIN(...) */

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 320

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window *window =
        SDL_CreateWindow(CHIP8_WINDOW_NAME, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                         CHIP8_SCALED_SCREEN_WIDTH, CHIP8_SCALED_SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_TEXTUREACCESS_TARGET);

    SDL_Rect squareRect;

    squareRect.w = MIN(SCREEN_WIDTH, SCREEN_HEIGHT) / 2;
    squareRect.h = MIN(SCREEN_WIDTH, SCREEN_HEIGHT) / 2;

    squareRect.x = SCREEN_WIDTH / 2 - squareRect.w / 2;
    squareRect.y = SCREEN_HEIGHT / 2 - squareRect.h / 2;

    while (1) {

        SDL_Event e;
        SDL_WaitEvent(&e);

        if (e.type == SDL_QUIT)
            break;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
        SDL_RenderFillRect(renderer, &squareRect);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
