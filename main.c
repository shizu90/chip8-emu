#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "chip8.h"


int main(int argc, char **argv) {

    chip8 *ch8 = malloc(sizeof(*ch8));

    init(ch8, argv[1]);

    free(ch8);

    return 0;
}