typedef struct {
    uint8_t e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf32_Ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} Elf32_Phdr;

int load_elf(Emulator *emu, const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) return -1;
    
    Elf32_Ehdr ehdr;
    fread(&ehdr, sizeof(ehdr), 1, fp);
    
    // Verify ELF header
    if (memcmp(ehdr.e_ident, "\x7F""ELF", 4) != 0 || 
        ehdr.e_machine != 3 /* EM_386 */) {
        fclose(fp);
        return -1;
    }
    
    // Load program segments
    fseek(fp, ehdr.e_phoff, SEEK_SET);
    for (int i = 0; i < ehdr.e_phnum; i++) {
        Elf32_Phdr phdr;
        fread(&phdr, sizeof(phdr), 1, fp);
        
        if (phdr.p_type == 1 /* PT_LOAD */) {
            fseek(fp, phdr.p_offset, SEEK_SET);
            uint8_t *segment_data = malloc(phdr.p_filesz);
            fread(segment_data, phdr.p_filesz, 1, fp);
            
            // Copy to emulated memory
            memcpy(emu->memory + phdr.p_vaddr, segment_data, phdr.p_filesz);
            free(segment_data);
            
            // Zero out BSS section if needed
            if (phdr.p_memsz > phdr.p_filesz) {
                memset(emu->memory + phdr.p_vaddr + phdr.p_filesz, 
                      0, phdr.p_memsz - phdr.p_filesz);
            }
        }
    }
    
    fclose(fp);
    
    // Set entry point
    emu->registers.eip = ehdr.e_entry;
    
    // Initialize stack pointer (typical Linux i386 initial stack)
    emu->registers.esp = 0xBFFFFFF0;
    
    return 0;
}