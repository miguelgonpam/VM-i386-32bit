# VM-i386-32bit
Virtual machine that executes the i386-32bit Instruction set. \
The goal is to create a 64-bit program that is able to execute a i386 32-bit program, ELF32 format. \
This project is designed to either execute in a x86_64 machine or a x86_64 docker. Ubuntu24.04 is recommended. \
The syntax should be the following: \
```
./vm-i386-32bit 32b-prog
``` 

## Packages
In order to the program to work, we need to install the following packages: 
```
apt install libcapstone-dev libncurses-dev
```
`Capstone`library allows to disassemble i386 bytecode. \
`Ncurses` library implements a graphic interface we are going to use for the host program.

## Compiling your own 32bit i386 programs
In order to compile i386 legacy programs (without i686 instructions), we cant use gcc-multilib and the -m32 flag, so we are going to use a cross-compiler.
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
And we will have the cross compiler on the Output folder. We can either add the `$(OUTPUT)/bin/` folder to the path or create a symbolic link to the i386-linux-musl-gcc object.
```
ln -s /usr/local/gcc-i386/bin/i386-linux-musl-gcc gcc-i386
```
Result will be a `gcc-i386` symbolic link in our current folder that we can run with `./gcc-i386`. \
This process also brings cross binutils as `objdump` or `readelf`.


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

`interface.h` is the header file that defines the functions needed to create the program's interface. \
`interface.c` is the implementation file that implements the interface using the `ncurses` library. 
