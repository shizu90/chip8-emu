#include "common.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>


typedef struct {

    FILE * ROM;

    uc8 memory[4096]; //0x00 -> 0x1FF reserved for CHIP-8 system; 0x200 -> 0xFFF reserved for program data (roms);
    uc8 v[0x10]; //16 8-bit registers, V0 -> VF;
    us16 I; //Index register, store memory addresses;
    us16 PC; //Program counter;
    uc8 delay_timer;
    uc8 sound_timer;
    us16 stack[16];
    us16 SP; //Stack Pointer;

    
    us16 opcode; //Opcodes; 
    uc8 gfx[64 * 32]; //Graphics (2048);

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;

}chip8;


void init(chip8 *, char *);
void emuCycle(chip8 *);
void draw(chip8 *);
void keys(chip8 *, char *);






