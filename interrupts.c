#include <stdint.h>
#include "interrupts.h"
#include "trad_syscall.h"
#include "syscall.h"

Interrupt idt[0xFF];
extern Syscall32bits syscalls[452]; 

/**
 * Initializes Interrupt Descriptor Table. Contains all Interrupt functions.
 * 
 * @param void none
 * 
 * @return void
 */
void init_idt(){
    init_int_trad();
    idt[0x80]=int80;
    idt[0x3]=unimplemented; /* Used by debuggers to set breakpoints - Not useful for current program */
    idt[0x4]=unimplemented; /* Used to avoid arithmetic overflow - Deprecated - Compilers do not use it anymore */
}

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
    return idt[v](eax, ebx, ecx, edx, esi, edi, ebp);
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
    return syscalls[*eax](eax, ebx, ecx, edx, esi, edi, ebp);
}


