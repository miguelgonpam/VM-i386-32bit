#ifndef LOADER_H
#define LOADER_H

typedef uint8_t byte;

#define MEM_SIZE 0x100000000  // 4 GB

int read_elf_file(int argc, char *argv[]);

#endif