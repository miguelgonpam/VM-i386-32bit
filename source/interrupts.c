#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "../lib/interrupts.h"
#include "../lib/trad_syscall.h"
#include "../lib/syscall.h"

Interrupt idt[] = {
    unimplemented, unimplemented, unimplemented, unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,
    unimplemented, unimplemented, unimplemented, unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,
    unimplemented, unimplemented, unimplemented, unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,
    unimplemented, unimplemented, unimplemented, unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,
    unimplemented, unimplemented, unimplemented, unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,
    unimplemented, unimplemented, unimplemented, unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,
    unimplemented, unimplemented, unimplemented, unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,
    unimplemented, unimplemented, unimplemented, unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,
    int80        , unimplemented, unimplemented, unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,
    unimplemented, unimplemented, unimplemented, unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,
    unimplemented, unimplemented, unimplemented, unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,
    unimplemented, unimplemented, unimplemented, unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,
    unimplemented, unimplemented, unimplemented, unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,
    unimplemented, unimplemented, unimplemented, unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,
    unimplemented, unimplemented, unimplemented, unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,
    unimplemented, unimplemented, unimplemented, unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,
    unimplemented, unimplemented, unimplemented, unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented,unimplemented
};
extern Syscall32bits syscalls[]; 

/**
 * Gets the interrupt corresponding to v value and calls it with all 7 arguments (eax - ebp).
 * 
 * @param v number of interrupt.
 * @param eax eax register value.
 * @param ebx ebx register value.
 * @param ecx ecx register value.
 * @param edx edx register value.
 * @param esi esi register value.
 * @param edi edi register value.
 * @param ebp ebp register value.
 * 
 * @return value returned by the interrput.
 */
uint32_t int_dispatcher(uint8_t v, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx, uint32_t *esi, uint32_t *edi, uint32_t *ebp){
    return  idt[v](eax, ebx, ecx, edx, esi, edi, ebp);
}

/**
 * Default function for deprecated or useless interrupts. Like 3 or 4.
 * 
 * @param eax eax register value.
 * @param ebx ebx register value.
 * @param ecx ecx register value.
 * @param edx edx register value.
 * @param esi esi register value.
 * @param edi edi register value.
 * @param ebp ebp register value.
 * 
 * @return 0;
 */
uint32_t unimplemented(uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx, uint32_t *esi, uint32_t *edi, uint32_t *ebp){
    return 0;
}

/**
 * Calls syscall matching number with eax and its arguments (ebx - ebp).
 * 
 * @param eax eax register value.
 * @param ebx ebx register value.
 * @param ecx ecx register value.
 * @param edx edx register value.
 * @param esi esi register value.
 * @param edi edi register value.
 * @param ebp ebp register value.
 * 
 * @return value returned by the syscall.
 */
uint32_t int80(uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx, uint32_t *esi, uint32_t *edi, uint32_t *ebp){
    *eax = syscalls[*eax](eax, ebx, ecx, edx, esi, edi, ebp);
    return *eax;
}
