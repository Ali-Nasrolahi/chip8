/**
 * @file config.h
 * @author Ali Nasrolahi (a.nasrolahi01@gmail.com)
 * @date 2024-03-25
 */
#include <assert.h>
#include <stdint.h>

#ifndef CONFIG_H
#define CONFIG_H

#define CHIP8_MEM_SIZE (4096)

#define CHIP8_WINDOW_NAME "CHIP-8 Emulator"
#define CHIP8_WINDOW_SCALE_RATIO (10)

#define CHIP8_SCREEN_WIDTH (64)
#define CHIP8_SCREEN_HEIGHT (32)

#define CHIP8_SCALED_SCREEN_WIDTH (CHIP8_WINDOW_SCALE_RATIO * CHIP8_SCREEN_WIDTH)
#define CHIP8_SCALED_SCREEN_HEIGHT (CHIP8_WINDOW_SCALE_RATIO * CHIP8_SCREEN_HEIGHT)

#endif /* CONFIG_H */
