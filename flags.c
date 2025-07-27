#include <stdint.h>
#include "flags.h"

uint32_t eflags = 0x2;

void set_Flag(uint32_t f){
    eflags |= (f);
}

void clear_Flag(uint32_t f){
    eflags &= ~(f);
}

int test_Flag(uint32_t f){
    return (eflags & f)?1:0;
}

int zero(uint8_t v){
    return v?1:0;
}

int sign(uint8_t v){
    (v >> 7)?1:0;
}

int parity(uint8_t v){
    int c=0;
    for (int i=0; i<8; i++){
        if((v >> i) & 0x1)
            c++;
    }
    return (c % 2)?1:0;
}

