#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "chip8.h"


int main(int argc, char *argv[]) {

    chip8 *ch8 = malloc(sizeof(*ch8));

    init(ch8, argv[1]);

    free(ch8);

    return 0;
}

/*unsigned char buffer[4000];
    unsigned short PC = 0;
    unsigned short opcode;

    FILE *ROM = fopen(argv[1], "rb");
    fread(buffer, 1, 4000, ROM);

    for(int i = 0; i < 4000; i++) {
        printf("0x%02X ", buffer[i]);
    }

    int opcd = 0;

    while(opcd == 0) {

        opcode = buffer[PC + 1] << 8 | buffer[PC];
 
        switch(opcode) {
            
            default:
                printf("opcode invalido: 0x%04X", opcode);
                opcd = 1;  
                break;       
        }
    }*/