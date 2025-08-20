#ifndef LOADER_H
#define LOADER_H

typedef uint8_t byte;

#define MEM_SIZE 0x100000000  // 4 GB

#define AT_NULL         0               /* End of vector */
#define AT_IGNORE       1               /* Entry should be ignored */
#define AT_EXECFD       2               /* File descriptor of program */
#define AT_PHDR         3               /* Program headers for program */
#define AT_PHENT        4               /* Size of program header entry */
#define AT_PHNUM        5               /* Number of program headers */
#define AT_PAGESZ       6               /* System page size */
#define AT_BASE         7               /* Base address of interpreter */
#define AT_FLAGS        8               /* Flags */
#define AT_ENTRY        9               /* Entry point of program */
#define AT_NOTELF       10              /* Program is not ELF */
#define AT_UID          11              /* Real uid */
#define AT_EUID         12              /* Effective uid */
#define AT_GID          13              /* Real gid */
#define AT_EGID         14              /* Effective gid */
#define AT_CLKTCK       17              /* Frequency of times() */
#define AT_PLATFORM     15              /* String identifying platform.  */
#define AT_HWCAP        16              /* Machine-dependent hints about */
#define AT_FPUCW        18              /* Used FPU control word.  */
#define AT_DCACHEBSIZE  19              /* Data cache block size.  */
#define AT_ICACHEBSIZE  20              /* Instruction cache block size.  */
#define AT_UCACHEBSIZE  21              /* Unified cache block size.  */
#define AT_IGNOREPPC    22              /* Entry should be ignored.  */
#define AT_SECURE       23              /* Boolean, was exec setuid-like?  */
#define AT_BASE_PLATFORM 24             /* String identifying real platforms.*/
#define AT_RANDOM       25              /* Address of 16 random bytes.  */
#define AT_HWCAP2       26              /* More machine-dependent hints about */
#define AT_RSEQ_FEATURE_SIZE    27      /* rseq supported feature size.  */
#define AT_RSEQ_ALIGN   28              /* rseq allocation alignment.  */
#define AT_HWCAP3       29              /* extension of AT_HWCAP.  */
#define AT_HWCAP4       30              /* extension of AT_HWCAP.  */
#define AT_EXECFN       31              /* Filename of executable.  */
#define AT_SYSINFO      32
#define AT_SYSINFO_EHDR 33
#define AT_L1I_CACHESHAPE       34
#define AT_L1D_CACHESHAPE       35
#define AT_L2_CACHESHAPE        36
#define AT_L3_CACHESHAPE        37
#define AT_L1I_CACHESIZE        40
#define AT_L1I_CACHEGEOMETRY    41
#define AT_L1D_CACHESIZE        42
#define AT_L1D_CACHEGEOMETRY    43
#define AT_L2_CACHESIZE         44
#define AT_L2_CACHEGEOMETRY     45
#define AT_L3_CACHESIZE         46
#define AT_L3_CACHEGEOMETRY     47
#define AT_MINSIGSTKSZ          51 /* Stack needed for signal delivery  */


uint32_t read_elf_file(int argc, char *argv[], char *envp[], uint32_t **sheader, uint32_t *count);

#endif