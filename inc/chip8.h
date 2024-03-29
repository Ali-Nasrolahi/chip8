#ifndef CHIP8_H
#define CHIP8_H

#include "config.h"

#define CHIP8_MEM_SIZE (4096)

const uint8_t chip8_charset[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70, 0xF0, 0x10, 0xF0, 0x80,
    0xF0, 0xF0, 0x10, 0xF0, 0x10, 0xF0, 0x90, 0x90, 0xF0, 0x10, 0x10, 0xF0, 0x80, 0xF0,
    0x10, 0xF0, 0xF0, 0x80, 0xF0, 0x90, 0xF0, 0xF0, 0x10, 0x20, 0x40, 0x40, 0xF0, 0x90,
    0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0, 0x10, 0xF0, 0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0,
    0x90, 0xE0, 0x90, 0xE0, 0xF0, 0x80, 0x80, 0x80, 0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0,
    0xF0, 0x80, 0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80

};

extern const uint8_t chip8_keyboard_map[];

struct chip8 {

    /* Registers */
    uint8_t V[16];
    uint8_t dt; /* Delay timer*/
    uint8_t st; /* Sound timer */
    uint8_t sp; /* Stack pointer */
    uint16_t I;
    uint16_t pc;

    /* Memory */
    uint8_t memory[CHIP8_MEM_SIZE];

    /* Stack */
    uint16_t stack[16];

    /* Keyboard */
    uint8_t keyboard[17];

    /* Display */
    uint8_t screen[CHIP8_SCREEN_HEIGHT][CHIP8_SCREEN_WIDTH];
};

void chip8_init(struct chip8 *c)
{
    memset(c, 0, sizeof(struct chip8));
    memcpy(&c->memory, chip8_charset, sizeof(chip8_charset));
}

void chip8_load(struct chip8 *c, const char *buf, size_t size)
{
    assert(size + 0x200 < CHIP8_MEM_SIZE);
    memcpy(&c->memory[0x200], buf, size);
    c->pc = 0x200;
}

void chip8_exec(struct chip8 *c, uint16_t opcode) {}

void chip8_mem_set(struct chip8 *c, uint16_t addr, uint8_t val)
{
    assert(addr >= 0 && addr < CHIP8_MEM_SIZE);
    c->memory[addr] = val;
}
uint8_t chip8_mem_get(struct chip8 *c, uint16_t addr)
{
    assert(addr >= 0 && addr < CHIP8_MEM_SIZE);
    return c->memory[addr];
}

uint16_t chip8_mem_getw(struct chip8 *c, uint16_t addr)
{
    return (c->memory[addr] << 8) | c->memory[addr];
}

void chip8_stack_push(struct chip8 *c8, uint16_t val)
{
    assert(c8->sp + 1 < sizeof(c8->stack));
    c8->stack[c8->sp] = val;
}

uint8_t chip8_stack_pop(struct chip8 *c8)
{
    assert(c8->sp < sizeof(c8->stack));
    return c8->stack[c8->sp--];
}

void chip8_keyboard_release(struct chip8 *c, char k)
{
    assert((k >= '0' && k <= '9') || (k >= 'a' && k <= 'f'));
    k -= (k > '9' ? ('a' - 10) : '0');
    c->keyboard[k] = 0;
}

void chip8_keyboard_hold(struct chip8 *c, char k)
{
    assert((k >= '0' && k <= '9') || (k >= 'a' && k <= 'f'));
    k -= (k > '9' ? ('a' - 10) : '0');
    c->keyboard[k] = 1;
}

uint8_t chip8_keyboard_is_held(struct chip8 *c, char k)
{
    assert((k >= '0' && k <= '9') || (k >= 'a' && k <= 'f'));
    /*
     * ('0' to '9') - '0' --> 0-9
     * ('a' to 'f') - 'a' + 10 --> 10-16
     */
    k -= (k > '9' ? ('a' - 10) : '0');
    return c->keyboard[k];
}

void chip8_screen_set_xy(struct chip8 *c, uint8_t x, uint8_t y)
{
    assert(x >= 0 && x < CHIP8_SCREEN_WIDTH && y >= 0 && y < CHIP8_SCREEN_HEIGHT);
    c->screen[y][x] = 1;
}

uint8_t chip8_screen_is_set(struct chip8 *c, uint8_t x, uint8_t y)
{
    assert(x >= 0 && x < CHIP8_SCREEN_WIDTH && y >= 0 && y < CHIP8_SCREEN_HEIGHT);
    return c->screen[y][x];
}

uint8_t chip8_screen_draw_sprite(struct chip8 *c, uint8_t x, uint8_t y, const char *spr, uint8_t n)
{
    uint8_t col = 0;
    for (uint8_t ly = 0; ly < n; ++ly)
        for (uint8_t lx = 0; lx < 8; ++lx)
            if (spr[ly] & (0x80 >> lx)) {
                col |= c->screen[(ly + y) % CHIP8_SCREEN_HEIGHT][(lx + x) % CHIP8_SCREEN_WIDTH];
                c->screen[(ly + y) % CHIP8_SCREEN_HEIGHT][(lx + x) % CHIP8_SCREEN_WIDTH] ^= 1;
            }

    return col;
}

#endif /* CHIP8_H */
