#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <elf.h>
#include "loader.h"
#include "instr.h"

extern uint8_t * mem;
extern uint32_t eip;

int read_elf_file(int argc, char *argv[]) {

    /* mem should be already pointing to an allocated memory array of 4GB (calloc so every byte is 00 by default) */

    /* LOG File Initialization */
    FILE *log = fopen("log.txt", "w"); 
    if (log == NULL) {
        perror("Error al abrir el archivo de log");
        return 1;
    }

    /* Args comprobation */
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <archivo_elf>\n", argv[0]);
        fprintf(log, "Wrong number of arguments. \n");
        return 1;
    }

    /* Open ELF file to perform binary read*/
    FILE *elf_file = fopen(argv[1], "rb");
    if (!elf_file) {
        perror("Error al abrir ELF");
        fprintf(log, "Failed to open ELF file. \n");
        return 1;
    }

    /* Read ELF headers */
    Elf32_Ehdr ehdr;
    fread(&ehdr, 1, sizeof(ehdr), elf_file);
    fprintf(log, "Read ELF headers. \n");

    /* Validate ELF parameters */
    if (memcmp(ehdr.e_ident, ELFMAG, SELFMAG) != 0 ||
        ehdr.e_ident[EI_CLASS] != ELFCLASS32) {
        fprintf(stderr, "Archivo no es ELF32 válido.\n");
        fprintf(log, "Invalid ELF file type. \n");
        fclose(elf_file);
        return 1;
    }

    /* Validate mem pointer */
    if (!mem) {
        perror("Error al reservar 4 GB de memoria");
        fprintf(log, "Found an error while allocating memory. \n");
        fclose(elf_file);
        return 1;
    }

    

    /* Read all program headers*/
    fseek(elf_file, ehdr.e_phoff, SEEK_SET);
    
    long phdrs_start = ehdr.e_phoff;
    for (int i = 0; i < ehdr.e_phnum; i++) {
        fseek(elf_file, phdrs_start + i * sizeof(Elf32_Phdr), SEEK_SET);
        Elf32_Phdr phdr;
        fread(&phdr, 1, sizeof(phdr), elf_file);

        if (phdr.p_type != PT_LOAD)
            continue;

        fseek(elf_file, phdr.p_offset, SEEK_SET);
        fread(mem + phdr.p_vaddr, 1, phdr.p_filesz, elf_file);

        /* No zero fill needed*/

        fprintf(log, "Segment loaded. vaddr=0x%08x, size=%u bytes\n",phdr.p_vaddr, phdr.p_memsz);
        
    }

    fclose(elf_file);
    fclose(log);
    /* EIP Initialization */
    eip = ehdr.e_entry;

    /* No need to free mem pointer, because it is used in the main program */
    
    return 0;
}