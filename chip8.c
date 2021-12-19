#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "common.h"
#include "chip8.h"

#define SCREEN_W 640
#define SCREEN_H 320
#define SCREEN_BPP 32

int keypad[16] = {
    
        SDLK_0,
        SDLK_1,
        SDLK_2,
        SDLK_3,
        SDLK_4,
        SDLK_5,
        SDLK_6,
        SDLK_7,
        SDLK_8,
        SDLK_9,
        SDLK_a,
        SDLK_b,
        SDLK_c,
        SDLK_d,
        SDLK_e,
        SDLK_f

}; //keypad (0x00 -> 0xFF);

uc8 fontset[80] = {

        0xF0, 0x90, 0x90, 0x90, 0xF0,
        0x20, 0x60, 0x20, 0x20, 0x70,
        0xF0, 0x10, 0xF0, 0x80, 0xF0,
        0xF0, 0x10, 0xF0, 0x10, 0xF0,
        0x90, 0x90, 0xF0, 0x10, 0x10,
        0xF0, 0x80, 0xF0, 0x10, 0xF0,
        0xF0, 0x80, 0xF0, 0x90, 0xF0,
        0xF0, 0x10, 0x20, 0x40, 0x40,
        0xF0, 0x90, 0xF0, 0x90, 0xF0,
        0xF0, 0x90, 0xF0, 0x10, 0xF0,
        0xF0, 0x90, 0xF0, 0x90, 0x90,
        0xE0, 0x90, 0xE0, 0x90, 0xE0,
        0xF0, 0x80, 0x80, 0x80, 0xF0,
        0xE0, 0x90, 0x90, 0x90, 0xE0,
        0xF0, 0x80, 0xF0, 0x80, 0xF0,
        0xF0, 0x80, 0xF0, 0x80, 0x80 

};

void init(chip8 * chip8, char *romname) {
    
    chip8->PC = 0x200; //program counter starts at 0x200;
    chip8->opcode = 0; //clear opcodes;
    chip8->I = 0; //clear index register;
    chip8->SP = 0; //clear stack pointer;

    chip8->ROM = fopen(romname, "rb");
    fread(chip8->memory+0x200, 1, 4096-0x200, chip8->ROM);

    memset(chip8->gfx, 0, sizeof(chip8->gfx)); //reset display;
    memset(chip8->stack, 0, sizeof(chip8->stack)); //reset stack
    memset(chip8->v, 0, sizeof(chip8->v)); //reset registers

    for(int i = 0; i < 80; i++) {
        chip8->memory[i] = fontset[i];
    }

    malloc(sizeof(fontset));
    malloc(sizeof(keypad));

    chip8->delay_timer = 0;
    chip8->sound_timer = 0;

    SDL_Event event;
    SDL_Init(SDL_INIT_EVERYTHING);
    
    chip8->window = SDL_CreateWindow("chimp8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 64, 32, SDL_WINDOW_SHOWN);
    chip8->renderer = SDL_CreateRenderer(chip8->window, -1, SDL_RENDERER_ACCELERATED);
    chip8->texture = SDL_CreateTexture(chip8->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, 64, 32);

    
    for(;;) {

        if(SDL_PollEvent(&event)) {
            continue;
        }
        emuCycle(chip8);
        draw(chip8);
        keys(chip8, romname);
    }
  
}

void draw(chip8 * chip8) {

    int x, y;

    unsigned int screen[64 * 32 * 4];

    for(y = 0; y < 32; y++) {
        for(x = 0; x < 64; x++) {
            screen[x + (y * 64)] = chip8->gfx[x + (y * 64)] ? 0xFFFFFFFF : 0;
        }
    }

    SDL_UpdateTexture(chip8->texture, NULL, screen, 64 * sizeof(unsigned int));
    SDL_RenderCopy(chip8->renderer, chip8->texture, NULL, NULL);
    SDL_RenderPresent(chip8->renderer);

}

void keys(chip8 * chip8, char * game) {

    const uc8 * key = SDL_GetKeyboardState(NULL);

    if(key[SDLK_ESCAPE]) {
        exit(1);
    }
    if(key[SDLK_SPACE]) {
        init(chip8, game);
    }

}

void emuCycle(chip8 * chip8) {


    unsigned short vx = 0, vy = 0, x = 0, y = 0, i;
    unsigned short hg, pxl;
    const uc8 * key;

    
        chip8->opcode = chip8->memory[chip8->PC] << 8 | chip8->memory[chip8->PC + 1];
        printf("Opcode: 0x%04X PC: %04X I: %02X SP: %02X \n", chip8->opcode, chip8->PC, chip8->I, chip8->SP);
        switch(chip8->opcode & 0xF000) {
            case 0x0000: 
                switch(chip8->opcode & 0x000F) {
                    case 0x0000: //Clear display
                        memset(chip8->gfx, 0, sizeof(chip8->gfx));
                        chip8->PC += 2;
                        break;

                    case 0x000E: //Return from subroutine
                        chip8->PC = chip8->stack[(chip8->SP)];
                        --chip8->SP;
                        chip8->PC += 2; 
                        break;

                    default:
                        printf("Invalid opcode: 0x%X \n", chip8->opcode);
                        break;        
                }
                break;
            case 0x1000: //Jump to address 0x1NNN
                chip8->PC = chip8->opcode & 0x0FFF;
                break;

            case 0x2000: //Call subroutine at 0x2NNN
                chip8->SP++;   
                chip8->stack[(chip8->SP)] = chip8->PC;
                chip8->PC = chip8->opcode & 0x0FFF;
                break;

            case 0x3000: //Skip next instruction if Vx = kk
                if((chip8->v[(chip8->opcode & 0x0F00) >> 8]) == (chip8->opcode & 0x00FF)) {
                    chip8->PC += 4;
                }else{
                    chip8->PC += 2;
                }
                break;

            case 0x4000: //Skip next instruction if Vx != kk
                if((chip8->v[(chip8->opcode & 0x0F00) >> 8]) != (chip8->opcode & 0x00FF)) {
                    chip8->PC += 4;
                }else{
                    chip8->PC += 2;
                }
                break;
            
            case 0x5000: //Skip next instruction if Vx = Vy
                if((chip8->v[(chip8->opcode & 0x0F00) >> 8]) == (chip8->v[(chip8->opcode & 0x00F0) >> 4])) {
                    chip8->PC += 4;
                }else{
                    chip8->PC += 2;
                }
                break;

            case 0x6000: //Set Vx = kk
                chip8->v[(chip8->opcode & 0x0F00) >> 8] = chip8->opcode & 0x00FF;
                chip8->PC += 2;
                break;

            case 0x7000: //Set Vx = Vx + kk
                chip8->v[(chip8->opcode & 0x0F00) >> 8] += chip8->opcode & 0x00FF;
                chip8->PC += 2;
                break;

            case 0x8000: 
                switch(chip8->opcode & 0x000F) {
                    case 0x0000: //Set Vx = Vy
                        chip8->v[(chip8->opcode & 0x0F00) >> 8] = chip8->v[(chip8->opcode & 0x00F0) >> 4];
                        chip8->PC += 2;
                        break;
                    case 0x0001: //Set Vx = Vx |(or) Vy
                        chip8->v[(chip8->opcode & 0x0F00) >> 8] = chip8->v[(chip8->opcode & 0x0F00) >> 8] | chip8->v[(chip8->opcode & 0x00F0) >> 4];
                        chip8->PC += 2;
                        break;
                    case 0x0002: //Set Vx = Vx &(and) Vy
                        chip8->v[(chip8->opcode & 0x0F00) >> 8] = chip8->v[(chip8->opcode & 0x0F00) >> 8] & chip8->v[(chip8->opcode & 0x00F0) >> 4];
                        chip8->PC += 2;
                        break;
                    case 0x0003: //Set Vx = Vx ^(xor) Vy
                        chip8->v[(chip8->opcode & 0x0F00) >> 8] = chip8->v[(chip8->opcode & 0x0F00) >> 8] ^ chip8->v[(chip8->opcode & 0x00F0) >> 4];
                        chip8->PC += 2;
                        break;
                    case 0x0004: //Set Vx = Vx + Vy, set VF = carry
                        if((int)chip8->v[(chip8->opcode & 0x0F00) >> 8] + (int)chip8->v[(chip8->opcode & 0x00F0) >> 4] < 0xFF) {
                            chip8->v[0xF] = 0;
                        }else{
                            chip8->v[0xF] = 1;
                        }

                        chip8->v[(chip8->opcode & 0x0F00) >> 8] += chip8->v[(chip8->opcode & 0x00F0) >> 4];
                        chip8->PC += 2;
                        break;
                    
                    case 0x0005: //Set Vx = Vx - Vy, set VF = NOT borrow
                        if(((int) chip8->v[(chip8->opcode & 0x0F00) >> 8] - (int) chip8->v[(chip8->opcode & 0x00F0) >> 4]) >= 0) {
                            chip8->v[0xF] = 1;
                        }else{
                            chip8->v[0xF] &= 0;
                        }

                        chip8->v[(chip8->opcode & 0x0F00) >> 8] -= chip8->v[(chip8->opcode & 0x00F0) >> 4];
                        chip8->PC += 2;

                        break;

                    case 0x0006: //Set Vx = Vx SHR 1
                        chip8->v[0xF] = chip8->v[(chip8->opcode & 0x0F00) >> 8] & 0x7;
                        chip8->v[(chip8->opcode & 0x0F00) >> 8] = chip8->v[(chip8->opcode & 0x0F00)] >> 1;
                        chip8->PC += 2;
                        break;

                    case 0x0007: //Set Vx = Vy - Vx, set VF = NOT borrow
                        if(((int)chip8->v[(chip8->opcode & 0x0F00) >> 8] - (int)chip8->v[(chip8->opcode & 0x00F0) >> 4]) > 0) {
                            chip8->v[0xF] = 1;
                        }else{
                            chip8->v[0xF] &= 0;
                        }

                        chip8->v[(chip8->opcode & 0x0F00) >> 8] = chip8->v[(chip8->opcode & 0x00F0) >> 4] - chip8->v[(chip8->opcode & 0x0F00) >> 8];
                        chip8->PC += 2;

                        break;

                    case 0x000E: //Set Vx = Vx SHL 1
                        chip8->v[0xF] = chip8->v[(chip8->opcode & 0x0F00) >> 8] >> 7;
                        chip8->v[(chip8->opcode & 0x0F00) >> 8] = chip8->v[(chip8->opcode & 0x0F00) >> 8] << 1;
                        chip8->PC += 2;
                        
                        break;

                    default:
                        printf("Invalid opcode: %04X \n", chip8->opcode);
                }

                break;

            case 0x9000: //Skip next instruction if Vx != Vy
                if(chip8->v[(chip8->opcode & 0x0F00) >> 8] != chip8->v[(chip8->opcode & 0x00F0) >> 4]) {
                    chip8->PC += 4;
                }else{
                    chip8->PC += 2;
                }
                break;

            case 0xA000: //Set I = nnn
                chip8->I = chip8->opcode & 0x0FFF;
                chip8->PC += 2;
                break;

            case 0xB000: //Jump to location nnn + V0
                chip8->PC = (chip8->opcode & 0xFFF) + chip8->v[0x0];
                break;

            case 0xC000: //Set Vx = random byte AND kk
                chip8->v[(chip8->opcode & 0x0F00) >> 8] = rand() & (chip8->opcode & 0x00FF);
                chip8->PC += 2;
                break;

            case 0xD000: //Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision    
                vx = chip8->v[(chip8->opcode & 0x0F00) >> 8];
                vy = chip8->v[(chip8->opcode & 0x00F0) >> 4];

                hg = chip8->opcode & 0x000F;
                chip8->v[0xF] = 0;

                for(y = 0; y < hg; y++) {
                    pxl = chip8->memory[chip8->I + y];
                    for(x = 0; x < 8; x++) {
                        if(pxl & (0x80 >> x)) {
                            if(chip8->gfx[x+vx+((y+vy) * 64)]) {
                                chip8->v[0xF] = 1;

                            }
                            chip8->gfx[x+vx+((y+vy)*64)] ^= 1;
                        }
                    }
                }
                chip8->PC += 2;
                break;

            case 0xE000: 
                switch(chip8->opcode & 0x00FF) {
                    case 0x009E: //Skip next instruction if key with the value of Vx is pressed
                        key = SDL_GetKeyboardState(NULL);
                        if(key[keypad[chip8->v[(chip8->opcode & 0x0F00) >> 8]]]) {
                            chip8->PC += 4;
                        }else {
                            chip8->PC += 2;
                        }

                        break;
                    case 0x00A1: //Skip next instruction if key with the value of Vx is not pressed
                        key = SDL_GetKeyboardState(NULL);
                        if(!key[keypad[chip8->v[(chip8->opcode & 0x0F00) >> 8]]]) {
                            chip8->PC += 4;
                        }else{
                            chip8->PC += 2;
                        }
                        break;

                    default:
                        printf("Invalid opcode: %04X", chip8->opcode);
                }
                break;

            case 0x0F000: 
                switch(chip8->opcode & 0x00FF) {
                    case 0x0007: //Set Vx = delay timer value
                        chip8->v[chip8->opcode & 0x0F00] = chip8->delay_timer;
                        chip8->PC += 2;
                        break;

                    case 0x000A: //Wait for a key press, store the value of the key in Vx
                        key = SDL_GetKeyboardState(NULL);
                        for(i = 0; i < 0x10; i++) {
                            if(key[keypad[i]]) {
                                chip8->v[(chip8->opcode & 0x0F00) >> 8] = 1;
                                chip8->PC += 2;
                            }
                        }
                        break;

                    case 0x0015: //Set delay timer = Vx
                        chip8->delay_timer = chip8->v[(chip8->opcode & 0x0F00) >> 8];
                        chip8->PC += 2;
                        break;
                    
                    case 0x0018: //Set sound timer = Vx
                        chip8->sound_timer = chip8->v[(chip8->opcode & 0x0F00) >> 8];
                        chip8->PC += 2;
                        break;

                    case 0x001E: //Set I = I + Vx
                        chip8->I += chip8->v[(chip8->opcode & 0x0F00) >> 8];
                        chip8->PC += 2;
                        break;

                    case 0x0029: //Set I = location of sprite for digit Vx
                        chip8->I = chip8->v[(chip8->opcode & 0x0F00) >> 8] * 5;
                        chip8->PC += 2;
                        break;

                    case 0x0033: //Store BCD representation of Vx in memory locations I, I+1, and I+2
                        chip8->memory[chip8->I] = chip8->v[(chip8->opcode & 0x0F00) >> 8] / 100;
                        chip8->memory[chip8->I + 1] = (chip8->v[(chip8->opcode & 0x0F00) >> 8] / 10) % 10;
                        chip8->memory[chip8->I + 2] = chip8->v[(chip8->opcode & 0x0F00) >> 8] % 10;
                        chip8->PC += 2;
                        break;

                    case 0x0055: //Store registers V0 through Vx in memory starting at location I
                        for(i = 0; i <= ((chip8->opcode & 0x0F00) >> 8); i++) {
                            chip8->memory[chip8->I + i] = chip8->v[i];
                        }
                        chip8->PC += 2;
                        break;

                    case 0x0065: //Read registers V0 through Vx from memory starting at location I
                        for(i = 0; i <=((chip8->opcode & 0x0F00) >> 8); i++) {
                            chip8->v[i] = chip8->memory[chip8->I + i];
                        }
                        chip8->PC += 2;
                        break;
                    
                    default:
                        printf("Invalid opcode: %04X", chip8->opcode); 
                }
                
                
        }

        if(chip8->delay_timer > 0) {
            --chip8->delay_timer;
        }
        if(chip8->sound_timer == 1) {
            printf("BEEP! \n");
        }else{
            --chip8->sound_timer;
        }
    
}
