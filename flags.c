#include <stdint.h>
#include "flags.h"

uint32_t eflags = 0x10;

void set_Flag(uint32_t f){
    eflags |= (f);
}

void clear_Flag(uint32_t f){
    eflags &= ~(f);
}

