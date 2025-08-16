#include <stdint.h>

#ifndef INTERRUPTS_H
#define INTERRUPTS_H

typedef uint32_t (*Interrupt)(uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx, uint32_t *esi, uint32_t *edi, uint32_t *ebp);

void init_idt();
uint32_t int_dispatcher(uint8_t v, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx, uint32_t *esi, uint32_t *edi, uint32_t *ebp);
uint32_t int80(uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx, uint32_t *esi, uint32_t *edi, uint32_t *ebp);
uint32_t unimplemented(uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx, uint32_t *esi, uint32_t *edi, uint32_t *ebp);

#endif
