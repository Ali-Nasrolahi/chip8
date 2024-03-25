#ifndef CHIP8_H
#define CHIP8_H

#include "config.h"

#define CHIP8_MEM_SIZE (4096)

struct chip8_mem {
    uint8_t memory[CHIP8_MEM_SIZE];
};

struct chip8_stack {
    uint16_t stack[16];
};

struct chip8 {

    /* Registers */
    uint8_t V[16];
    uint8_t dt; /* Delay timer*/
    uint8_t st; /* Sound timer */
    uint8_t sp; /* Stack pointer */
    uint16_t I;
    uint16_t pc;

    /* Memory */
    struct chip8_mem memory;

    /* Stack */
    struct chip8_stack stack;
};

void chip8_mem_set(struct chip8_mem *m, uint16_t addr, uint8_t val)
{
    assert(addr >= 0 && addr < CHIP8_MEM_SIZE);
    m->memory[addr] = val;
}
uint8_t chip8_mem_get(struct chip8_mem *m, uint16_t addr)
{
    assert(addr >= 0 && addr < CHIP8_MEM_SIZE);
    return m->memory[addr];
}

void chip8_stack_push(struct chip8 *c8, uint16_t val)
{
    assert(c8->sp + 1 < sizeof(c8->stack.stack));
    c8->stack.stack[c8->sp] = val;
}

uint8_t chip8_stack_pop(struct chip8 *c8)
{
    assert(c8->sp < sizeof(c8->stack.stack));
    return c8->stack.stack[c8->sp--];
}

#endif /* CHIP8_H */
