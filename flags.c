#include <stdint.h>
#include "flags.h"

uint32_t eflags = 0x202;

void set_Flag(uint32_t f){
    eflags |= (f);
}

void clear_Flag(uint32_t f){
    eflags &= ~(f);
}

void complement_Flag(uint32_t f){
    eflags ^= (f);
}

int test_Flag(uint32_t f){
    return (eflags & f)?1:0;
}

int zero(uint8_t v){
    return v?1:0;
}

int sign(uint32_t v, uint8_t base){
    (v >> (base-1))?1:0;
}

int overflow(uint32_t op1, uint32_t op2, uint32_t res, uint8_t base){
    return (((op1 ^ res) & (op2 ^ res)) >> (base-1)) & 1;
}

int parity(uint32_t vv){
    uint8_t v = vv & 0xFF;
    int c=0;
    for (int i=0; i<8; i++){
        if((v >> i) & 0x1)
            c++;
    }
    return (c % 2)?1:0;
}

int adjust(uint32_t op1, uint32_t op2, uint32_t res){
    return ((op1 ^ op2 ^ res) & 0x10) != 0;
}

