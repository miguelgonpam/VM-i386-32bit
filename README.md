# Sim-i386-32bit
Simulator that executes the i386-32bit Instruction set. \
The goal is to create a 64-bit program that is able to execute a i386 32-bit program, ELF32 format. \
Until syscall support is given for any arch, the project is designed to either execute in a x86_64 machine or a x86_64 docker. Ubuntu24.04 is recommended. \
The syntax should be the following: 
```
./proc <mode> 32b-prog
```

Mode can either be `-i`, `-n` or `-h`: \
`-ie` runs the simulator using the graphic interface with an elf file. \
`-ne` runs the simulator with an elf file as if it was run from the terminal. \
`-ir` runs the simulator using the graphic interface with a raw file. \
`-nr` runs the simulator with a raw file as if it was run from the terminal. \
`-h` prints help regarding the usage of the program. If this option is specified, the 32bit program name is not necessary.

But first, program needs to be compiled. It can be compiled either using the `compile.sh` script or with the following gcc line. Make sure to first install all the packages needed (see next section).
```
gcc -o proc source/*.c -lcapstone -lc
```

## Packages
In order to make the program work, we need to install the following packages: 
```
sudo apt install libcapstone-dev
```
`Capstone` library allows to disassemble i386 bytecode. 

## Compiling your own 32bit i386 programs
In order to compile i386 legacy programs (without i686 instructions), we cant use gcc-multilib and the -m32 flag, because it would include i686 instructions, so we are going to use a cross-compiler.
We can obtain a cross-compiler using the [Musl-cross-make project](https://github.com/richfelker/musl-cross-make). Musl is a light implementation for the standard libc.
First we need to create a `config.mak` file and write the following lines. Output folder could be any folder.
```
TARGET = i386-linux-musl
OUTPUT = /usr/local/gcc-i386
COMMON_CONFIG += --disable-nls
```
Then we run the following comands.
```
make
sudo make install
```
And we will have the cross compiler on the Output folder. We can either add the `$(OUTPUT)/bin/` folder to the PATH or create a symbolic link to the i386-linux-musl-gcc object.
```
ln -s /usr/local/gcc-i386/bin/i386-linux-musl-gcc gcc-i386
```
Result will be a `gcc-i386` symbolic link in our current folder that we can run with `./gcc-i386`. \
Compiling the `musl-cross-make` project also brings cross binutils as `objdump` or `readelf`. Useful when looking for vulnerabilities within the code or understanding the program's flow. \
It is very important that, when compiling, we use the `-static` and the `-no-pie` flags:
- `-static` because the linker has not been implemented, so the simulator expects the libraries to be in the exectuable file.
- `-no-pie` so the code addresses are always the same, allowing replication. Useful when finding and using exploits.

For example:
```
./gcc-i386 -o program32 program32.c -static -no-pie
```


## Files

### Lib directory
Lib directory contains all the project's header files (.h).

-`flags.h` is the header file implemented in `flags.c`. It defines flags values (bit position). \
-`instr.h` is the header file that defines all the instruction functions (add, sub, ret...). \
-`syscall.h` is the header file that defines all the syscalls. \
-`trad_syscall.h`defines the `Syscall32bits` type, used by `trad_syscall.c` to translate i386 syscall numbers and args to x86_64 syscalls. \
-`typesi386.h` defines types in both 32 and 64 bits so the syscall translation is easier. \
-`loader.h` is the header file that defines the functions needed to load the executable. \
-`interface.h` is the header file that defines the functions needed to create the program's interface. \
-`interrupts.h` defines the header for the interrupts implementation file. 

### Source directory
Source directory contains all the project's implementation files (.c).

-`proc.c` is the main file, which will contain the `int main()` function and will use every other file. \
-`flags.c` is the implementation file of `flags.h`. It implements the functions needed to set, clear, complement and test flags. It also contains the `EFLAGS` register. \
-`instr.c` is the implementation of the `instr.h` file. It also contains the 32 bit registers (eax, eip, esp, ecx, edx, ebx, ebp, esi, edi, conceived as a uint32_t) and the memory, conceived as a uint8_t pointer, with a real memory allocation of 2^32 bytes (4GB). \
-`syscall.c` is the implementataion file for `syscall.h`. Implements the argument transalte from 32 to 64 bit and does the 64bit syscall. \
-`trad_syscall.c` translates i386 syscalls into x86_64 syscalls. \
-`loader.c` is the implementation file that actually loads the executable bytecode into the memory. \
-`interface.c` is the implementation file that implements the interface. \
-`interrupts.c` implements both the dispatcher and interrupts handling functions. \
-`typesi386.c` implements some auxiliary functions to translate types from 32bit to 64bit.

## Interface use
```
┌────────────────────────────────────────────────────────────────────────────────────┐ 
│ EAX : 0x00000000 0000000000  ECX : 0x00000000 0000000000  ESI : 0x00000000 00000000│ 
│ EDX : 0x00000000 0000000000  EBX : 0x00000000 0000000000  EDI : 0x00000000 00000000│ 
│ ESP : 0xffeff198 4293915032  EIP : 0x08048142 0134512962  EBP : 0x00000000 00000000│ 
│ DS  : 0x0000002b 0000000043  ES  : 0x0000002b 0000000043  EFLAGS : 0x00000202      │ 
│ SS  : 0x0000002b 0000000043  FS  : 0x00000000 0000000000  [                   IF]  │ 
│ CS  : 0x00000023 0000000035  GS  : 0x00000000 0000000000                           │ 
└────────────────────────────────────────────────────────────────────────────────────┘ 
┌─────────────────────────────────────────────────────────┐┌─────────────────────────┐ 
│ <0x08048140> : add byte ptr [eax], al                   ││ 0xffeff198 : 0x00000001 │ 
│ <0x08048142> : xor ebp, ebp                             ││ 0xffeff19c : 0xffeff2d2 │ 
│ <0x08048144> : mov eax, esp                             ││ 0xffeff1a0 : 0x00000000 │ 
│ <0x08048146> : and esp, 0xfffffff0                      ││ 0xffeff1a4 : 0xffeff2d4 │ 
│ <0x08048149> : push eax                                 ││ 0xffeff1a8 : 0xffeff2e4 │ 
│ <0x0804814a> : push eax                                 ││ 0xffeff1ac : 0xffeff2fc │ 
│ <0x0804814b> : call 0x8048150                           ││ 0xffeff1b0 : 0xffeff319 │ 
│ <0x08048150> : add dword ptr [esp], 0xf7fb7eb0          ││ 0xffeff1b4 : 0xffeff32e │ 
│ <0x08048157> : push eax                                 ││ 0xffeff1b8 : 0xffeff34d │ 
│ <0x08048158> : call 0x804815d                           ││ 0xffeff1bc : 0xffeff35c │ 
│ <0x0804815d> : sub esp, 0x14                            ││ 0xffeff1c0 : 0xffeff36e │ 
│ <0x08048160> : mov eax, dword ptr [esp + 0x18]          ││ 0xffeff1c4 : 0xffeff37b │ 
│ <0x08048164> : push 0                                   ││ 0xffeff1c8 : 0xffeff39d │ 
│ <0x08048166> : push 0x8048efa                           ││ 0xffeff1cc : 0xffeffab6 │ 
│ <0x0804816b> : lea edx, [eax + 4]                       ││ 0xffeff1d0 : 0xffeffad0 │ 
│ <0x0804816e> : push 0x804811c                           ││ 0xffeff1d4 : 0xffeffaf2 │ 
│ <0x08048173> : push edx                                 ││ 0xffeff1d8 : 0xffeffb06 │ 
└─────────────────────────────────────────────────────────┘└─────────────────────────┘ 
```

`s` executes a single instruction. Steps one instruction forward. 

`b` allows to set a breakpoint in a given address. 

`d` allows to delete a breakpoint with a given breakpoint number. 

`c` continues the execution until it finds a breakpoint. Steps as many instructions as needed until lands into a breakpoint. 

`n` is similar to `c`, continues the execution until lands in immediate next instruction. Designed to avoid going into CALL instruction or JMP instruction (if program flow ever lands in the next instruction). 

`KEY_LEFT`, `KEY_RIGHT`, `4` and `6` switch **focus** between code (`KEY_LEFT` or `4`) and stack (`KEY_RIGHT` or `6`). Focus allows to perform some actions in current window. 

`KEY_UP`, `8`, `KEY_DOWN` and `2` move window lines up (`KEY_UP` or `8`) or down (`KEY_DOWN` or `2`). The main idea is to be able o see the whole stack or move through code lines. Only affects *focused* window.

`f` finds an address and sets it at the top of the current *focused* window, either code or stack (if the address is found). 

`x` shows the hex content of a given memory address. 

`t` shows the string content of a given memory address. 



