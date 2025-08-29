#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <elf.h>
#include "../lib/loader.h"
#include "../lib/instr.h"

#define AUXV_VALUE 0
#define AUXV_ADDR  1

typedef uint32_t (*AUXV_getters)(uint64_t v);


extern uint8_t * mem;
extern uint32_t eip, esp;
uint32_t ph_addr, ph_size, ph_num, platform_addr, execfn, rr;

int count_size(char **p){
    int count = 0;
    while (p[count] != NULL) {
        count++;
    }
    return count;
}

/**
 *  Returns AUXV value number 3.
 */
uint32_t get_AT_PHDR(uint64_t v){
    return ph_addr;    
}

/**
 *  Returns AUXV value number 4.
 */
uint32_t get_AT_PHENT(uint64_t v){
    return ph_size;
}

/**
 *  Returns AUXV value number 5.
 */
uint32_t get_AT_PHNUM(uint64_t v){
    return ph_num;
}

/**
 *  Returns AUXV value number 7.
 */
uint32_t get_AT_BASE(uint64_t v){
    return 0x0;
}

/**
 *  Returns AUXV value number 9.
 */
uint32_t get_AT_ENTRY(uint64_t v){
    return eip;
}

/**
 *  Returns AUXV value number ...
 */
uint32_t get_AT_GENERIC(uint64_t v){
    return (uint32_t)v;
}

/**
 *  Returns AUXV value number 15.
 */
uint32_t get_AT_PLATFORM(uint64_t v){
    return platform_addr;
}

/**
 *  Returns AUXV value number 24.
 */
uint32_t get_AT_BASE_PLATFORM(uint64_t v){
    return platform_addr;
}

/**
 *  Returns AUXV value number 25.
 */
uint32_t get_AT_RANDOM(uint64_t v){
    return rr;
}

/**
 *  Returns AUXV value number 31.
 */
uint32_t get_AT_EXECFN(uint64_t v){
    return execfn;
}
/* SYS are ignored because of static compile */
/* Generics are 0, 1, 2, 6, 8, 10, 11, 12, 13, 14, 16, 17, 18, 19, 20, 21, 22, 23, 25, 26, 27, 28, 29, 30, 32, 33, 34, 35, 36, 37, 40, 41, 42, 43, 44, 45, 46, 47, 51*/


int read_auxv(){
    FILE *fd = fopen("/proc/self/auxv", "rb");
    if (fd < 0) {
        perror("open");
        return 1;
    }
    AUXV_getters getters[] = {get_AT_GENERIC, get_AT_GENERIC, get_AT_GENERIC, get_AT_PHDR, get_AT_PHENT, get_AT_PHNUM, get_AT_GENERIC, get_AT_BASE, get_AT_GENERIC, get_AT_ENTRY, 
        get_AT_GENERIC, get_AT_GENERIC, get_AT_GENERIC, get_AT_GENERIC, get_AT_GENERIC, get_AT_PLATFORM, get_AT_GENERIC, get_AT_GENERIC, get_AT_GENERIC, get_AT_GENERIC, get_AT_GENERIC,
        get_AT_GENERIC, get_AT_GENERIC, get_AT_GENERIC, get_AT_BASE_PLATFORM, get_AT_RANDOM, get_AT_GENERIC, get_AT_GENERIC, get_AT_GENERIC, get_AT_GENERIC, get_AT_GENERIC,
        get_AT_EXECFN, get_AT_GENERIC, get_AT_GENERIC, get_AT_GENERIC, get_AT_GENERIC, get_AT_GENERIC, get_AT_GENERIC, get_AT_GENERIC, get_AT_GENERIC, get_AT_GENERIC,
        get_AT_GENERIC,get_AT_GENERIC,get_AT_GENERIC,get_AT_GENERIC,get_AT_GENERIC,get_AT_GENERIC,get_AT_GENERIC,get_AT_GENERIC,get_AT_GENERIC,get_AT_GENERIC,get_AT_GENERIC,get_AT_GENERIC};
    Elf64_auxv_t entry;

    /* PUSH AT_NULL */
    esp -= 4;
    write32(esp, 0x00000000);
    esp -= 4;
    write32(esp, 0x00000000);
    while (fread(&entry, 1, sizeof(entry), fd) == sizeof(entry)) {
        /* If found AT_NULL, quit (No auxv values remaining)*/
        if (entry.a_type == AT_NULL){
            break;
        }
        /* Dont push SYS and VDSO pointers */
        if (entry.a_type == 32 || entry.a_type == 33)
            continue;
        /* Get value asociated to type in 32bit */
        uint32_t val = getters[entry.a_type](entry.a_un.a_val);
        /* Push value*/
        esp -= 4;
        write32(esp, val);
        /* Push type */
        esp -= 4;
        write32(esp, (uint32_t)entry.a_type);
        
    }
    fclose(fd);
    return 0;
}



void load_stack(int argc, char *argv[], char *envp[]){
    int n_argv = count_size(argv);
    int n_envp = count_size(envp);
    uint32_t argv_emu[n_argv];
    uint32_t envp_emu[n_envp];

    /* Push argv strings, first last one*/
    for (int i=n_envp-1; i>=0; i--){
        char * p = envp[i];
        int len = strlen(p)+1;

        esp -= len;
        memcpy(&mem[esp], p, len);
        envp_emu[i]=esp;
    }
    /* Push argv strings, first last one*/
    for (int i=n_argv-1; i>1; i--){ /* i>1 because program name is argv[0] and emulator mode is argv[1]*/
        char * p = argv[i];
        int len = strlen(p)+1;
        esp -= len;
        memcpy(&mem[esp], p, len);
        if (i==1){
            execfn = esp;
        }
        argv_emu[i]=esp;
    }
    /* Push strings needed by auxv */
    /* 15, 25*/
    char platform[] = "x86";
    uint8_t l = strlen(platform)+1;
    esp -= l;
    memcpy(&mem[esp], platform, l);
    platform_addr = esp;
    esp-= 4;
    write32(esp, random());
    esp-= 4;
    write32(esp, random());
    rr = esp;
    
    /* Stack alignment at 4 Bytes (Padding) */
    esp &= ~0x3;

    /* Push auxv. To be implemented */
    read_auxv();

    /* Push NULL word to end envp */
    esp -= 4;
    write32(esp, 0x00000000);
    for (int i=n_envp-1; i>=0; i--){
        esp-=4;
        write32(esp, envp_emu[i]);
    }
    /* Push NULL word to end argv */
    esp -= 4;
    write32(esp, 0x00000000);
    for (int i=n_argv-1; i>0; i--){
        esp-=4;
        write32(esp, argv_emu[i]);
    }
    /* Push argc */
    esp -= 4;
    write32(esp, argc-1); 

}

uint32_t read_elf_file(int argc, char *argv[], char *envp[], uint32_t **sheader, uint32_t *count) {

    /* mem should be already pointing to an allocated memory array of 4GB (calloc so every byte is 00 by default) */

    /* LOG File Initialization */
    FILE *log = fopen("log.txt", "w"); 
    if (log == NULL) {
        perror("Error al abrir el archivo de log");
        return 1;
    }

    /* Open ELF file to perform binary read */
    /* argc was already checked at proc.c so it must be at least 3 */
    FILE *elf_file = fopen(argv[2], "rb");
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

    

    /* Read all program headers */
    fseek(elf_file, ehdr.e_phoff, SEEK_SET);
    uint8_t found = 0;
    long phdrs_start = ehdr.e_phoff;
    long shdrs_start = ehdr.e_shoff;

    /* Values for pushing auxv */
    ph_num = ehdr.e_phnum;
    ph_size = ehdr.e_phentsize;

    /* Iterate trough Program Headers */
    for (int i = 0; i < ehdr.e_phnum; i++) {
        fseek(elf_file, phdrs_start + i * sizeof(Elf32_Phdr), SEEK_SET);
        Elf32_Phdr phdr;
        fread(&phdr, 1, sizeof(phdr), elf_file);
        
        if (phdr.p_type != PT_LOAD)
            continue;

        /*
        if (!found && phdr.p_flags & PF_X){ 
            // If program header Executable Flag is set to 1, and its the first to be this way, we store this vaddr, for disassembly. 
            //   This allows to have the first v_addr to disassembly. PF_X is defined in <elf.h>.
            
            found = 1;
            *ini=phdr.p_vaddr;
        }
        */
        fseek(elf_file, phdr.p_offset, SEEK_SET);
        if (!i){
            ph_addr = phdr.p_vaddr + ehdr.e_phoff;
        }
        fread(mem + phdr.p_vaddr, 1, phdr.p_filesz, elf_file);
        //*last = phdr.p_vaddr + phdr.p_filesz;
        /* No zero fill needed*/

        fprintf(log, "Segment loaded. vaddr=0x%08x, size=%u bytes\n",phdr.p_vaddr, phdr.p_memsz);
        
    }
    
    *sheader = calloc(ehdr.e_shnum*2, sizeof(uint32_t));
    *count = 0;
    /* Iterate Section Headers */
    
    for (int i = 0; i < ehdr.e_shnum; i++) {
        fseek(elf_file, shdrs_start + i * ehdr.e_shentsize, SEEK_SET);
        Elf32_Shdr shdr;
        fread(&shdr, 1, sizeof(shdr), elf_file);
        if (shdr.sh_type == SHT_PROGBITS && (shdr.sh_flags & SHF_EXECINSTR)) {
            (*sheader)[2* (*count)]   = shdr.sh_addr;
            (*sheader)[2*(*count)+1] = shdr.sh_size;
            //*count += shdr.sh_size;
            //*ini = shdr.sh_addr;
            //break; // nos quedamos solo con el primero
            *count += 1; 
        }
        
    }
    fclose(elf_file);
    fclose(log);

    /* EIP Initialization */
    eip = ehdr.e_entry;

    load_stack(argc, argv, envp);

    /* No need to free mem pointer, because it is used in the main program */

    return 0;
}