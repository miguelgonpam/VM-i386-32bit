# VM-i386-32bit
Virtual machine that executes the i386-32bit Instruction set. \
The goal is to create a 64-bit program that is able to execute a 32-bit program, ELF32 format. \
This project is designed to either execute in a x86_64 machine or a x86_64 docker. Ubuntu24.04 is recommended. \
The syntax should be the following: \
```
./vm-i386-32bit 32b-prog
``` 

## Packages
In order to the program to work, we need to install the following packages: \
```
apt install libcapstone-dev
```

If we also want to compile our own 32-bit executables, we will need an x86_64 version compatible with 32 bits and install the following.
```
apt install gcc-multilib
```
And then compile the program with the `-m32` flag. \
For example : \
```
gcc -m32 -o 32b-prog 32b-prog.c
```

## Files
`proc.c` is the main file, which will contain the `int main()` function and will use every other file. 

`flags.h` is the header file implemented in `flags.c`. It defines flags values (bit position). \
`flags.c` is the implementation file of `flags.h`. It implements the functions needed to set, clear, complement and test flags. It also contains the `EFLAGS` register 

`instr.h` is the header file that defines all the instruction functions (add, sub, ret...). \
`instr.c` is the implementation of the `instr.h` file. It also contains the 32 bit registers (eax, eip, esp, ecx, edx, ebx, ebp, esi, edi, conceived as a uint32_t) and the memory, conceived as a uint8_t pointer, with a real memory allocation of 2^32 bytes (4GB). 

`syscall.h` is the header file that defines all the syscalls. \
`syscall.c` is the implementataion file for `syscall.h`. Implements the argument transalte from 32 to 64 bit and does the 64bit syscall.

`loader.h` is the header file that defines the functions needed to load the executable. \
`loader.c` is the implementation file that actually loads the executable bytecode into the memory. 
