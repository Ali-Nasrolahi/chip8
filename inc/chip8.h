/**
 * @file chip8.h
 * @author Ali Nasrolahi (a.nasrolahi01@gmail.com)
 * @date 2024-03-25
 */
#ifndef CHIP8_H
#define CHIP8_H

#include "config.h"

struct chip8 {

    /* Registers */
    uint8_t v[16];
    uint8_t dt; /* Delay timer*/
    uint8_t st; /* Sound timer */
    uint8_t sp; /* Stack pointer */
    uint16_t I;
    uint16_t pc;

    /* Hardware components */
    uint8_t memory[CHIP8_MEM_SIZE];
    uint16_t stack[0x10];

    /* Peripherals */
    uint8_t keyboard[0x10];
    uint8_t screen[CHIP8_SCREEN_HEIGHT][CHIP8_SCREEN_WIDTH];
};

void chip8_init(struct chip8 *c);
void chip8_load(struct chip8 *c, const char *buf, size_t size);
void chip8_exec(struct chip8 *c);

/* Peripherals routines */
void chip8_keyboard_hold(struct chip8 *c, char k);
void chip8_keyboard_release(struct chip8 *c, char k);
int chip8_keyboard_map(char k);

void chip8_screen_clear(struct chip8 *c);
uint8_t chip8_screen_is_set(struct chip8 *c, uint8_t x, uint8_t y);

#endif /* CHIP8_H */
