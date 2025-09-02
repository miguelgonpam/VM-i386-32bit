#include <stdint.h>
#include "../lib/flags.h"

/**
 * EFLAGS register. Used from instr.c.
 */
uint32_t eflags = 0x202;

/**
 * Sets to 1 the specified flag.
 * 
 * @param f the flag's bit.
 */
void set_Flag(uint32_t f){
    eflags |= (f);
}

/**
 * Clears a flag (Set to 0).
 * 
 * @param f the flag's bit.
 */
void clear_Flag(uint32_t f){
    eflags &= ~(f);
}

/**
 * Complements a flag, If is set, it clears it and viceversa.
 * 
 * @param f the flag's bit.
 */
void complement_Flag(uint32_t f){
    eflags ^= (f);
}

/**
 * Tests if a flag is set or cleared.
 * 
 * @param f the flag's bit.
 * 
 * @return 1 if is set or 0 if not.
 */
int test_Flag(uint32_t f){
    return (eflags & f)?1:0;
}

/**
 * Tests if a value is 0.
 * 
 * @param v value to test
 * 
 * @return 1 if value is 0 or 0 if not.
 */
int zero(uint32_t v){
    return !v?1:0;
}

/**
 * Checks if the sign bit on a value is set, given the value's base.
 * 
 * @param v value to ckeck
 * @param base the value's base
 * 
 * @return 1 if MSB is 1 or 0 if not.
 */
int sign(uint32_t v, uint8_t base){
    return (v >> (base-1))?1:0;
}

/**
 * Ckecks if overflow occurs given the operands and result.
 * 
 * @param op1 first operand.
 * @param op2 second operand.
 * @param res result.
 * @param base of the operands and result.
 * 
 * @return 1 if overflow, 0 if not.
 */
int overflow(uint32_t op1, uint32_t op2, uint32_t res, uint8_t base){
    return (((op1 ^ res) & (op2 ^ res)) >> (base-1)) & 1;
}

/**
 * Check if lower 8bits of an operand has an even number of ones.
 * 
 * @param vv value to ckeck parity.
 * 
 * @return 1 if parity, 0 if not.
 */
int parity(uint32_t vv){
    uint8_t v = vv & 0xFF;
    int c=0;
    for (int i=0; i<8; i++){
        if((v >> i) & 0x1)
            c++;
    }
    return (!(c % 2))?1:0;
}

/**
 * Checks if adjust flag should be set, given the operands and result.
 * 
 * @param op1 first operand.
 * @param op2 second operand.
 * @param res result.
 * 
 * @return 1 if adjust, 0 if not.
 */
int adjust(uint32_t op1, uint32_t op2, uint32_t res){
    return ((op1 ^ op2 ^ res) & 0x10) != 0;
}

