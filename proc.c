#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <capstone/capstone.h>
#include <ncurses.h>
#include "instr.h"
#include "flags.h"
#include "loader.h"
#include "interface.h"

#define MIN(a,b) ((a > b)? (b) : (a))
#define MAX_STR 50

typedef int (*InstrFunc)(uint8_t *);

//REGISTERS
extern uint8_t * mem;
extern uint32_t eflags;
extern uint32_t eax, edx, esp, esi, eip, cs, ds, fs, ecx, ebx, ebp, edi, ss, es, gs;
extern uint32_t * regs[8];
extern uint8_t * regs8[8];

extern int rows, cols;

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

/**
   Dumps the memory from the direction offset. Dumps m words of memory (understanding a word as 4 bytes).

   @param offset first direction to dump.
   @param m number of contiguous words to dump.
*/
void dump_mem(uint32_t off, uint32_t m){
   uint8_t * p = mem;
   for (int i=0;i<m;i++){
      printf("0x%08x : %02x %02x %02x %02x\n", off+4*i, mem[off+4*i], mem[off+4*i+1], mem[off+4*i+2], mem[off+4*i+3]);
   }
}


int main(int argc, char *argv[], char *envp[]){
   /* INITIALIZATION */
   system("clear"); //execve?
   if(!initialize())
      return 1;

   uint32_t ini, r;

   if(read_elf_file(argc, argv, envp, &ini, &r)){
      perror("elf");
      exit(1);
   }

   init_interface();
   csh handle;
   cs_insn *insn, *ins;
   size_t count;


   if (cs_open(CS_ARCH_X86, CS_MODE_32, &handle) != CS_ERR_OK)
        return -1;
    
   cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);

   count = cs_disasm(handle, &mem[ini], r-ini, ini, 0, &insn); //poner 0 a 1 para solo decodificar una instr
   if (!count){
      printf("Failed to disassemble code\n");
      return 1;
   }
   char **lineas = malloc(rows * sizeof(char *));
   if (lineas == NULL){
      perror("malloc");
      exit(1);
   }

   for (int i = 0; i<rows; i++){
      lineas[i]=malloc(MAX_STR);
   }

   /* Char to get users char and scr to scroll instructions screen, focus to focus on code or*/
   int ch = 0, scr = 0, focus = 0;

   /* Initialize scr to EIP instr*/
   for(int i=0; i<count;i++){
      if (insn[i].address == eip){
         scr = i;
         break;
      }
   }
   
   
   while (ch != 'q'){
      int eip_ind = -1;
      for (size_t i = 0; i < MIN(rows, count -scr); i++) {
         uint32_t addr = insn[i+scr].address;
         if (addr == eip){
            eip_ind = i;
         }
         snprintf(lineas[i], MAX_STR, "<0x%08x>:%.6s %.30s",addr, insn[i+scr].mnemonic, insn[i+scr].op_str);
         //if (strcmp(insn[i].mnemonic, "push") == 0){
         //   push_i(&insn[i]);
         //}
         //eip += insn[i].size;
      }
      draw_regs();
      draw_stack();
      draw_code(lineas, rows, eip_ind);
      ch = getch();

      if ('\n' == ch || KEY_RIGHT == ch){
          /* Disassemble again the eip instruction because if its not on the screen,
             we cannot know its index within insn array.
             The 1 indicates to only disassemble 1 instruction.
          */
         if(!cs_disasm(handle, &mem[eip], r-eip, eip, 1, &ins)) // If number of disasm instructions is 0.
            return -1;
         if(dispatcher(ins[0].mnemonic, &ins[0]) == -1){
            exit(1);
         }
      }
      else if (ch == KEY_DOWN && scr < count - (rows - REGISTERS_HEIGHT - 2))
        scr++;
      else if (ch == KEY_UP && scr > 0)
        scr--;
      
   }


   /* Free memory */
   for (int i = 0; i<MIN(rows, count); i++){ free(lineas[i]); }
   free(lineas);
   cs_free(insn, count);
   
   
   exit_interface();

   free(mem);

   return 0;

}
