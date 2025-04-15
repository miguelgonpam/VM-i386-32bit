#include <stdio.h>
#include <stdint.h>


void readAll(FILE *fp){
    int32_t c;
    while ( (c = getc(fp)) != EOF ) {
        printf("%02x ", c);
    }

    

}

void readELF(FILE *fp){
    int32_t mn, elf[3], elftype, endian, elfv, abi, abiv, padding[7];
    mn = getc(fp);
    elf[0]=getc(fp);
    elf[1]=getc(fp);
    elf[2]=getc(fp);
    elftype=getc(fp);
    endian=getc(fp);
    elfv=getc(fp);
    abi=getc(fp);
    abiv=getc(fp);
    padding[0]=getc(fp);
    padding[1]=getc(fp);
    padding[2]=getc(fp);
    padding[3]=getc(fp);
    padding[4]=getc(fp);
    padding[5]=getc(fp);
    padding[6]=getc(fp);

    //e_indent
    printf("Magic number : %02x\n", mn);
    printf("ELF          : %02x %02x %02x (%c%c%c)\n", elf[0], elf[1], elf[2],elf[0], elf[1], elf[2]);
    printf("ELF class    : %02u%s\n", elftype*32,"bit");
    printf("Endianness   : %s\n", endian == 0x1 ? "Little Endian" : "Big Endian");
    printf("ELF version  : %02x %s\n", elfv, elfv?"(Current)":"");
    printf("OS/ABI       : %02x %s\n", abi, !abi?"(System V)": "");
    printf("ABI version  : %02x\n", abiv);
    printf("Padding      : %02x %02x %02x %02x %02x %02x %02x\n\n", padding[0],padding[1],padding[2],padding[3],padding[4],padding[5],padding[6]);

    //
    int32_t  e_version, e_entry, e_phoff, e_shoff, e_flags = 0x00000000;
    int16_t e_type, e_machine, e_ehsize, e_phentsize, e_phnum, e_shentsize, e_shnum, e_shstrndx = 0x0000;
    unsigned char * temp = (unsigned char *)&e_type;
    temp[0]=getc(fp);
    temp[1]=getc(fp);
    temp = (unsigned char *)&e_machine;
    temp[0]=getc(fp);
    temp[1]=getc(fp);
    temp = (unsigned char *)&e_version;
    temp[0]=getc(fp);
    temp[1]=getc(fp);
    temp[2]=getc(fp);
    temp[3]=getc(fp);
    temp = (unsigned char *)&e_entry;
    temp[0]=getc(fp);
    temp[1]=getc(fp);
    temp[2]=getc(fp);
    temp[3]=getc(fp);
    temp = (unsigned char*)&e_phoff;
    temp[0]=getc(fp);
    temp[1]=getc(fp);
    temp[2]=getc(fp);
    temp[3]=getc(fp);
    temp = (unsigned char*)&e_shoff;
    temp[0]=getc(fp);
    temp[1]=getc(fp);
    temp[2]=getc(fp);
    temp[3]=getc(fp);
    temp = (unsigned char*)&e_flags;
    temp[0]=getc(fp);
    temp[1]=getc(fp);
    temp[2]=getc(fp);
    temp[3]=getc(fp);
    temp = (unsigned char*)&e_ehsize;
    temp[0]=getc(fp);
    temp[1]=getc(fp);
    temp = (unsigned char*)&e_phentsize;
    temp[0]=getc(fp);
    temp[1]=getc(fp);
    temp = (unsigned char*)&e_phnum;
    temp[0]=getc(fp);
    temp[1]=getc(fp);
    temp = (unsigned char*)&e_shentsize;
    temp[0]=getc(fp);
    temp[1]=getc(fp);
    temp = (unsigned char*)&e_shnum;
    temp[0]=getc(fp);
    temp[1]=getc(fp);
    temp = (unsigned char*)&e_shstrndx;
    temp[0]=getc(fp);
    temp[1]=getc(fp);

    printf("File type                   : 0x%04x %s\n", e_type, e_type==0x1?"Relocatable":e_type==0x2?"Executable":e_type==0x3?"Shared (os)":"Core Dump" );
    printf("Machine                     : 0x%04x %s\n", e_machine, e_machine == 0x3?"(i386)":"");
    printf("Version                     : 0x%08x %s\n", e_type, e_type?"(Current)":"");
    printf("Program Entry               : 0x%08x\n", e_entry);
    printf("Program Header Table Offset : 0x%08x\n", e_phoff);
    printf("Section Header Table Offset : 0x%08x\n", e_shoff);
    printf("Flags                       : 0x%08x\n", e_flags);
    printf("ELF Header Size             : 0x%04x (%u)\n", e_ehsize, e_ehsize);
    printf("Program Header Entry Size   : 0x%04x (%u)\n", e_phentsize, e_phentsize);
    printf("Number of Program Entries   : 0x%04x (%u)\n", e_phnum, e_phnum);
    printf("Section Header Entry Size   : 0x%04x (%u)\n", e_shentsize, e_shentsize);
    printf("Number of Section Entries   : 0x%04x (%u)\n", e_shnum, e_shnum);
    printf("Index to Section Index      : 0x%04x\n", e_shstrndx);
    


    /**fseek(FILE *f, offset, origin) 
    ORIGIN:
        SEEK_SET : inicio del archivo
        SEEK_CUR : posicion actual
        SEEK_END : final del archivo
    */
}



int main() {
    FILE *fp;
    
    char file[32] = "tarea";

    fp = fopen(file, "rb");
    if (fp == NULL) {
        fprintf(stderr, "cannot open input file\n");
        return 1;
    }
    //readAll(fp);
    readELF(fp);

    fclose(fp);
    return 0;
}


