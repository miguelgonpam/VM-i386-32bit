#include <stdint.h>
#include <stdlib.h>
#include "../lib/typesi386.h"
#include <signal.h>
#include <string.h>

void convert_mask32_to_mask64(const sigset_t32 src, sigset_t *dst) {
    // Primero, borramos el destino
    //memset(dst, 0, sizeof(sigset_t));

    // Copiamos las señales válidas del i386
    // i386 rt_sigaction usa 128 bits = 16 señales por uint64_t * 2 = 128 bits
    for (int i = 0; i < 2; i++) {
        ((uint64_t*)dst)[i] = src[i];
    }
}

void convert_flags32_to_flags64(const uint32_t f32, uint32_t * f64){
    *f64 = 0x0;
    if (f32 & SA_RESTORER){
        *f64 |= SA_SIGINFO; 
    }
        
}