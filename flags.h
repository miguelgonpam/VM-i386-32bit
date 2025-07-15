#include <stdint.h>

#ifndef FLAGS_H
#define FLAGS_H

void set_Flag(uint32_t f);
void clear_Flag(uint32_t f);

#define CF 0b0000000000000001
#define PF 0b0000000000000100
#define AF 0b0000000000010000
#define ZF 0b0000000001000000
#define SF 0b0000000010000000
#define OF 0b0000100000000000

#endif
