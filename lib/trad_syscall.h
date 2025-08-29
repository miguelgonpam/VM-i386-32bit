#ifndef TRAD_SYSCALL_H
#define TRAD_SYSCALL_H

#include <stdint.h>
#include "syscall.h"

typedef uint32_t(*Syscall32bits)(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6);


#endif