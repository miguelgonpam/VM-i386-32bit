#include <stdio.h>
#include <stdint.h>
#include <string.h>

const char* get_section_type(uint32_t type);

void readAll(FILE *fp){
    int32_t c;
    while ( (c = getc(fp)) != EOF ) {
        printf("%02x ", c);
    }

    

}

void readELF(FILE *fp, char verbose){
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

    if (verbose){    
        //e_indent
        printf("Magic number : %02x\n", mn);
        printf("ELF          : %02x %02x %02x (%c%c%c)\n", elf[0], elf[1], elf[2],elf[0], elf[1], elf[2]);
        printf("ELF class    : %02u%s\n", elftype*32,"bit");
        printf("Endianness   : %s\n", endian == 0x1 ? "Little Endian" : "Big Endian");
        printf("ELF version  : %02x %s\n", elfv, elfv?"(Current)":"");
        printf("OS/ABI       : %02x %s\n", abi, !abi?"(System V)": "");
        printf("ABI version  : %02x\n", abiv);
        printf("Padding      : %02x %02x %02x %02x %02x %02x %02x\n\n", padding[0],padding[1],padding[2],padding[3],padding[4],padding[5],padding[6]);


        printf("File type                   : 0x%04x %s\n", e_type, e_type==0x1?"Relocatable":e_type==0x2?"Executable":e_type==0x3?"Shared (os)":"Core Dump" );
        printf("Machine                     : 0x%04x %s\n", e_machine, e_machine == 0x3?"(i386)":"");
        printf("Version                     : 0x%08x %s\n", e_type, e_type?"(Current)":"");
        printf("Program Entry               : 0x%08x\n", e_entry);
        printf("Program Header Table Offset : 0x%08x (%u)\n", e_phoff, e_phoff);
        printf("Section Header Table Offset : 0x%08x (%u)\n", e_shoff, e_shoff);
        printf("Flags                       : 0x%08x\n", e_flags);
        printf("ELF Header Size             : 0x%04x (%u)\n", e_ehsize, e_ehsize);
        printf("Program Header Entry Size   : 0x%04x (%u)\n", e_phentsize, e_phentsize);
        printf("Number of Program Entries   : 0x%04x (%u)\n", e_phnum, e_phnum);
        printf("Section Header Entry Size   : 0x%04x (%u)\n", e_shentsize, e_shentsize);
        printf("Number of Section Entries   : 0x%04x (%u)\n", e_shnum, e_shnum);
        printf("Index to Section Index      : 0x%04x\n", e_shstrndx);
    }


    printf("\n\n");

    /**fseek(FILE *f, offset, origin) 
    ORIGIN:
        SEEK_SET : inicio del archivo
        SEEK_CUR : posicion actual
        SEEK_END : final del archivo
    */

   fseek(fp, e_shoff, SEEK_SET);
   //int32_t sh_name, sh_type, sh_flags, sh_addr, sh_offset, sh_size, sh_link, sh_info, sh_addralign, sh_entsize;
   
   int32_t sh[e_shnum][10];
   char names[e_shnum][30];
   printf("SECTION HEADER TABLE\n[NUM] [NAME                        ] [TYPE          ] [FLAGS] [ADDR    ] [OFFSET  ] [SIZE    ] [LK] [INF] [AL] [ES]\n");
   
   for( int i=0; i<e_shnum; i++){
        for (int j=0;j<10;j++){
            temp = (unsigned char *)&sh[i][j];
            temp[0]=getc(fp);
            temp[1]=getc(fp);
            temp[2]=getc(fp);
            temp[3]=getc(fp);
        }
   }
    uint32_t shstrtab_offset = sh[e_shstrndx][4];
    //array de tipos
    for (int i=0; i<e_shnum;i++){
        fseek(fp, shstrtab_offset+sh[i][0], SEEK_SET);
        int8_t c;
        int cont = 0;
        while ((c = getc(fp)) != '\0'){
            names[i][cont++]=c;
        }
        names[i][cont++]='\0';
        if(verbose){
            printf("[%3u] %30s %16s  %5x  0x%08x 0x%08x 0x%08x %4u %5u %4u %4u\n", i,names[i], get_section_type(sh[i][1]),sh[i][2],sh[i][3], sh[i][4], sh[i][5], sh[i][6], sh[i][7], sh[i][8], sh[i][9]);
            //        i   name type  flag  addr off    size  lk  inf  al   es      /sh[i][0]
            }
    }
   
}

const char* get_section_type(uint32_t type) {
    switch(type) {
        case 0:  return "NULL";
        case 1:  return "PROGBITS";
        case 2:  return "SYMTAB";
        case 3:  return "STRTAB";
        case 4:  return "RELA";
        case 5:  return "HASH";
        case 6:  return "DYNAMIC";
        case 7:  return "NOTE";
        case 8:  return "NOBITS";
        case 9:  return "REL";
        case 10: return "SHLIB";
        case 11: return "DYNSYM";
        case 0x70000000: return "LOPROC";
        case 0x7FFFFFFF: return "HIPROC";
        case 0x80000000: return "LOUSER";
        case 0xFFFFFFFF: return "HIUSER";
        default: return "UNKNOWN";
    }
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
    readELF(fp, 1);

    fclose(fp);
    return 0;
}


