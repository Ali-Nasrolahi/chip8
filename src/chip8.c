#include "chip8.h"

static int keymap[0x10] = {SDLK_0, SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7,
                           SDLK_8, SDLK_9, SDLK_a, SDLK_b, SDLK_c, SDLK_d, SDLK_e, SDLK_f};

const uint8_t chip8_charset[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70, 0xF0, 0x10, 0xF0, 0x80,
    0xF0, 0xF0, 0x10, 0xF0, 0x10, 0xF0, 0x90, 0x90, 0xF0, 0x10, 0x10, 0xF0, 0x80, 0xF0,
    0x10, 0xF0, 0xF0, 0x80, 0xF0, 0x90, 0xF0, 0xF0, 0x10, 0x20, 0x40, 0x40, 0xF0, 0x90,
    0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0, 0x10, 0xF0, 0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0,
    0x90, 0xE0, 0x90, 0xE0, 0xF0, 0x80, 0x80, 0x80, 0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0,
    0xF0, 0x80, 0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80

};

/* Private routines */
/* Memory */
static void chip8_mem_set(struct chip8 *c, uint16_t addr, uint8_t val);
static uint8_t chip8_mem_get(struct chip8 *c, uint16_t addr);
static uint16_t chip8_mem_getw(struct chip8 *c, uint16_t addr);

/* Stack */
static void chip8_stack_push(struct chip8 *c, uint16_t val);
static uint16_t chip8_stack_pop(struct chip8 *c);

/* Peripherals */
static uint8_t chip8_keyboard_is_held(struct chip8 *c, uint8_t k);
static uint8_t chip8_keyboard_wait_event(struct chip8 *c);

static void chip8_screen_set_xy(struct chip8 *c, uint8_t x, uint8_t y);
static uint8_t chip8_screen_draw_sprite(struct chip8 *c, uint8_t x, uint8_t y, const uint8_t *spr,
                                        uint8_t n);

/* End of Private routines */

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
    return (c->memory[addr] << 8) | c->memory[addr + 1];
}

void chip8_stack_push(struct chip8 *c, uint16_t val)
{
    assert(c->sp < 0x10);
    c->stack[++c->sp] = val;
}

uint16_t chip8_stack_pop(struct chip8 *c)
{
    assert(c->sp < 0x10 && c->sp);
    return c->stack[c->sp--];
}

void chip8_keyboard_release(struct chip8 *c, char k) { c->keyboard[k] = 0; }
void chip8_keyboard_hold(struct chip8 *c, char k) { c->keyboard[k] = 1; }
uint8_t chip8_keyboard_is_held(struct chip8 *c, uint8_t k) { return c->keyboard[k]; }

int chip8_keyboard_map(char k)
{
    for (int i = 0; i < 0x10; i++) {
        if (keymap[i] == k)
            return i;
    }
    return -1;
}

uint8_t chip8_keyboard_wait_event(struct chip8 *c)
{
    SDL_Event e;
    while (SDL_WaitEvent(&e)) {
        if (e.type != SDL_KEYDOWN)
            continue;
        return chip8_keyboard_map(e.key.keysym.sym);
    }
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

uint8_t chip8_screen_draw_sprite(struct chip8 *c, uint8_t x, uint8_t y, const uint8_t *spr,
                                 uint8_t n)
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

void chip8_exec(struct chip8 *c)
{
    uint16_t opcode = chip8_mem_getw(c, c->pc);
    uint16_t nnn = opcode & 0x0fff;
    uint16_t kk = opcode & 0x00ff;
    uint16_t x = (opcode & 0x0f00) >> 8;
    uint16_t y = (opcode & 0x00f0) >> 4;
    uint16_t o = (opcode & 0xf000) >> 12;

    /* DEBUG printf("PC: %x opcode: %x\n", c->pc, opcode); */
    c->pc += 2;

    switch (opcode) {
    case 0x00e0:
        chip8_screen_clear(c);
        break;
    case 0x00ee:
        c->pc = chip8_stack_pop(c);
        assert(c->pc >= 0x200);
        break;
    }

    switch (o) {
    case 1:
        c->pc = nnn;
        assert(c->pc >= 0x200);
        break;
    case 2:
        chip8_stack_push(c, c->pc);
        c->pc = nnn;
        assert(c->pc >= 0x200);
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
        c->v[x] = kk;
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
        assert(c->pc >= 0x200);
        break;
    case 0xc:
        srand(time(NULL));
        c->v[x] = (uint8_t)(rand() & kk);
        break;
    case 0xd:
        c->v[0xf] = chip8_screen_draw_sprite(c, c->v[x], c->v[y], &c->memory[c->I], opcode & 0xf);
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
            c->v[0xf] |= (c->v[x] + c->v[y]) > 255;
            c->v[x] = (c->v[x] + c->v[y]) & 0xff;
            break;
        case 5:
            c->v[0xf] = c->v[x] > c->v[y];
            c->v[x] -= c->v[y];
            break;
        case 6:
            c->v[0xf] = c->v[x] & 0b1;
            c->v[x] >>= 1;
            break;
        case 7:
            c->v[0xf] = c->v[x] < c->v[y];
            c->v[x] = c->v[y] - c->v[x];
            break;
        case 0xe:
            c->v[0xf] = c->v[x] & (uint8_t)(1 << 7);
            c->v[x] <<= 1;
            break;
        }
    } else if (o == 0xf) {
        switch (kk) {
        case 0x07:
            c->v[x] = c->dt;
            break;
        case 0x0a:
            c->v[x] = chip8_keyboard_wait_event(c);
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
            c->I = c->v[x] * 5;
            break;
        case 0x33:
            chip8_mem_set(c, c->I, c->v[x] / 100);
            chip8_mem_set(c, c->I + 1, (c->v[x] % 100) / 10);
            chip8_mem_set(c, c->I + 2, c->v[x] % 10);
            break;
        case 0x55:
            for (int i = 0; i <= x; ++i)
                chip8_mem_set(c, c->I + i, c->v[i]);
            break;
        case 0x65:
            for (int i = 0; i <= x; ++i)
                c->v[i] = chip8_mem_get(c, c->I + i);
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