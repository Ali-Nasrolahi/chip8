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
    uint8_t v[16];
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

void chip8_screen_clear(struct chip8 *c) { memset(c->screen, 0, sizeof(c->screen)); }

void chip8_exec(struct chip8 *c, uint16_t opcode)
{
    uint16_t nnn = opcode & 0x0fff;
    uint16_t kk = opcode & 0x00ff;
    uint16_t x = opcode & 0x0f00;
    uint16_t y = opcode & 0x00f0;
    uint16_t o = opcode & 0xf000;

    switch (opcode) {
    case 0x00e0:
        chip8_screen_clear(c);
        break;
    case 0x00ee:
        c->sp = chip8_stack_pop(c);
        break;
    }

    switch (o) {
    case 1:
        c->pc = nnn;
        break;
    case 2:
        chip8_stack_push(c, c->pc);
        c->pc = nnn;
        break;
    case 3:
        if (c->v[x] == kk)
            c->pc += 2;
        break;
    case 4:
        if (c->v[x] != kk)
            c->pc += 2;
        break;
    case 5:
        if (c->v[x] == c->v[y])
            c->pc += 2;
        break;
    case 6:
        c->v[x] == kk;
        break;
    case 7:
        c->v[x] += kk;
        break;
    /* Skip 8 */
    case 9:
        if (c->v[x] != c->v[y])
            c->pc += 2;
        break;
    case 0xa:
        c->I = nnn;
        break;

    case 0xb:
        c->pc = nnn + c->v[0];
        break;
    case 0xc:
        c->v[x] = 0xdada & kk;
        break;
    case 0xd:
        /* TODO */
        break;
    }

    if (o == 0x8) {
        switch (opcode & 0xf) {
        case 0:
            c->v[x] = c->v[y];
            break;
        case 1:
            c->v[x] |= c->v[y];
            break;
        case 2:
            c->v[x] &= c->v[y];
            break;
        case 3:
            c->v[x] ^= c->v[y];
            break;
        case 4:
            c->v[x] ^= c->v[y];
            break;
        case 5:
            c->v[0xf] |= (c->v[x] + c->v[y] > 255 ? 1 : 0);
            c->v[x] = (c->v[x] + c->v[y]) & 0xff;
            break;
            /* TODO */
        case 6:
            break;
        case 7:
            break;
        case 0xe:
            break;
        }
    } else if (o == 0xf) {
        switch (kk) {
        case 0x07:
            c->v[x] = c->dt;
            break;
        case 0x0a:
            break;
        case 0x15:
            c->dt = c->v[x];
            break;
        case 0x18:
            c->st = c->v[x];
            break;
        case 0x1E:
            c->I += c->v[x];
            break;
        case 0x29:
            break;
        case 0x33:
            break;
        case 0x55:
            break;
        case 0x65:
            break;
        }

    } else if (o == 0xe && kk == 0x9e) {
        if (chip8_keyboard_is_held(c, c->v[x]))
            c->pc += 2;
    } else if (o == 0xe && kk == 0xa1) {
        if (!chip8_keyboard_is_held(c, c->v[x]))
            c->pc += 2;
    }
}
#endif /* CHIP8_H */
