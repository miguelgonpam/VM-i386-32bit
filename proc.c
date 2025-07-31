#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include <capstone/capstone.h>
#include "instr.h"
#include "flags.h"
#include "loader.h"

typedef int (*InstrFunc)(uint8_t *);

//REGISTERS
extern uint8_t * mem;
extern uint32_t eflags;
extern uint32_t eax, edx, esp, esi, eip, cs, ds, fs, ecx, ebx, ebp, edi, ss, es, gs;
extern uint32_t * regs[8];
extern uint8_t * regs8[8];

// AUXILIARY INSTRUCTION FUNCTIONS

void printAll(){
   printf("\neax    : 0x%08x\t\tecx    : 0x%08x\t\tedx    : 0x%08x\t\tebx    : 0x%08x\nesp    : 0x%08x\t\tebp    : 0x%08x\t\tesi    : 0x%08x\t\tedi    : 0x%08x\neip    : 0x%08x\t\teflags : 0x%08x\t\tcs     : 0x%08x\t\tds     : 0x%08x\nfs     : 0x%08x\t\tss     : 0x%08x\t\tes     : 0x%08x\t\tgs     : 0x%08x\n", eax, ecx, edx, ebx, esp, ebp, esi, edi, eip, eflags,cs, ds, fs, ss, es, gs);
}

void printStack(){
   //printf();
}

void printPointers(){
   printf("\neax    : 0x%p\t\tecx    : 0x%p\t\tedx    : 0x%p\t\tebx    : 0x%p\nesp    : 0x%p\t\tebp    : 0x%p\t\tesi    : 0x%p\t\tedi    : 0x%p\neip    : 0x%p\t\teflags : 0x%p\t\tcs     : 0x%p\t\tds     : 0x%p\nfs     : 0x%p\t\tss     : 0x%p\t\tes     : 0x%p\t\tgs     : 0x%p\n", &eax, &ecx, &edx, &ebx, &esp, &ebp, &esi, &edi, &eip, &eflags,&cs, &ds, &fs, &ss, &es, &gs);
}

/**
   Makes one step into the execution. If count is 0 executes 1 instruction. If count is 1 executes all
   instructions within the code array.

   @param hand csh handler for capstone decoder.
   @param code array containing the i386 bytecode.
   @param code_size size of the array containing the i386 bytecode.
   @param address start address (should be eip).
   @param count number of instructions to decode. 0 is 1 instruction, 1 is all within the array.
   @param in pointer to instruction struct. Stores all the decoded data.
*/
void step(csh hand, const uint8_t *code, size_t code_size, uint64_t address, size_t count, cs_insn **in){

   count = cs_disasm(hand, code, sizeof(code), 0x8048000, count, in); //poner 0 a 1 para solo decodificar una instr
                                                                        //poner direccion a eip
}


int main(int argc, char *argv[]){
   /* INITIALIZATION */
   system("clear"); //execve?
   if(!initialize())
      return -1;


   int r = read_elf_file(argc, argv);
   printf(" EIP : 0x%08x", eip);
   /* */

   /* DECODE VARIABLES */
   /*
   csh handle;
   cs_insn *insn;
   size_t count;
   if (cs_open(CS_ARCH_X86, CS_MODE_32, &handle) != CS_ERR_OK)
        return -1;
    
   cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
   */


   /* One instruction forward */
   /*
   while(false){
      step(handle, mem+eip, sizeof(mem)-eip, eip, 0, &insn);
      eip+=insn[0].size;
      //exec instr

      cs_free(insn, count);
   }

   */

   free(mem);

   return 0;

}
