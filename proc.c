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
#define MAX_BRK 25

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

uint8_t contains(uint32_t arr[], size_t size, uint32_t val){
   for (int i=0; i<size; i++){
      if (arr[i] == val)
         return 1;
   }
   return 0;
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

   init_interface();

   char **lineas = malloc(rows * sizeof(char *));
   if (lineas == NULL){
      perror("malloc");
      exit(1);
   }

   for (int i = 0; i<rows; i++){
      lineas[i]=malloc(MAX_STR);
   }

   /* Char to get users char and scr_c to scroll instructions screen, scr_s to scroll stack screen, focus to focus on code or*/
   int ch = 0, old_ch = 0, scr_c = 0, scr_s = 0, focus = 0;

   /* Initialize scr_c to EIP instr*/
   for(int i=0; i<count;i++){
      if (insn[i].address == eip){
         scr_c = i;
         break;
      }
   }
   
   uint32_t brkpts[MAX_BRK];
   uint8_t brk_ctr = 0;
   
   while ('q' != ch){
      int eip_ind = -1;
      for (size_t i = 0; i < MIN(rows, count -scr_c); i++) {
         uint32_t addr = insn[i+scr_c].address;
         if (addr == eip){
            eip_ind = i;
         }
         snprintf(lineas[i], MAX_STR, "<0x%08x>:%.6s %.30s",addr, insn[i+scr_c].mnemonic, insn[i+scr_c].op_str);
         //if (strcmp(insn[i].mnemonic, "push") == 0){
         //   push_i(&insn[i]);
         //}
         //eip += insn[i].size;
      }
      draw_regs();
      draw_stack(scr_s);
      draw_code(lineas, rows, eip_ind);
      draw_cmd();
      ch = getch();

      if('\n' == ch){
         /* Repeat last choice if ENTER is pressed */
         ch = old_ch;
      }
      if ('s' == ch){
          /* Disassemble again the eip instruction because if its not on the screen,
             we cannot know its index within insn array.
             The 1 indicates to only disassemble 1 instruction.
          */
         if(!cs_disasm(handle, &mem[eip], r-eip, eip, 1, &ins)) // If number of disasm instructions is 0.
            return -1;

         /* Check interrupts ???*/
         dispatcher(ins[0].mnemonic, &ins[0]);


         /* Find EIP and set it at the top of the screen */
         for(int i=0; i<count;i++){
            if (insn[i].address == eip){
               /* If it is the first instruction, show it,
                  If it is the second or greater, let the previous one show */
               scr_c = i?i-1:i;
               break;
            }
         }
         /* Set top of stack at the top of stack window */
         scr_s=0;
      }else if('c' == ch){   
         /* While doesnt it a breakpoint, continue executing */
         while (!contains(brkpts, brk_ctr, eip)){
            /* Disassemble again the eip instruction because if its not on the screen,
               we cannot know its index within insn array.
               The 1 indicates to only disassemble 1 instruction.
            */
            if(!cs_disasm(handle, &mem[eip], r-eip, eip, 1, &ins)) // If number of disasm instructions is 0.
               return -1;

            /* Check interrupts ???*/
            dispatcher(ins[0].mnemonic, &ins[0]);
         }
         /* Find EIP and set it at the top of the screen */
         for(int i=0; i<count;i++){
            if (insn[i].address == eip){
               /* If it is the first instruction, show it,
                  If it is the second or greater, let the previous one show */
               scr_c = i?i-1:i;
               break;
            }
         }
         /* Set top of stack at the top of stack window */
         scr_s=0;

      }else if (KEY_DOWN == ch){
         /* Focus is set on Code and scr_c doesnt overflow */
         if (focus == 0 && scr_c < count - (rows - H_REGS - 2)){
            /* Scroll down */
            scr_c++;
         }
         /* Focus is set on Stack, overflow checked in draw_regs() */
         if (focus == 1 ){
            /* Scroll down */
            scr_s++;
         }
      }else if (KEY_UP == ch){
         /* Focus is set on Code and scr_c doesnt underflow */
         if (focus == 0 && scr_c > 0){
            /* Scroll up */
            scr_c--;
         }
         /* Focus is set on Stack and scr_s doesnt underflow */
         if (focus == 1 && scr_s > 0){
            /* Scroll up */
            scr_s--;
         }
      }else if(KEY_LEFT == ch){
         /* Switch scroll focus to Code */
         focus = 0;
      }else if(KEY_RIGHT == ch){
         /* Switch scroll focus to Stack */
         focus = 1;
      }else if('b' == ch){
         char str[MAX_STR];
         int res = 0, c = 0;
         uint32_t dir;

         /* While string received not matches the format 0x00000000 */
         while(!res){
            cmd_get_str(str, MAX_STR,c);
            res = sscanf(str, "0x%08x", &dir);
            c = 1;
         }
         /* Store breakpoint */
         brkpts[brk_ctr]=dir;
         /* Increment breakpoint counter */
         brk_ctr++;
         /* If counter overflows MAX, override the existing ones*/
         brk_ctr %= MAX_BRK;
         
         //printf("0x%08x %10u", dir, dir);

      }
      old_ch = ch;
      
   }


   /* Free memory */
   for (int i = 0; i<MIN(rows, count); i++){ free(lineas[i]); }
   free(lineas);
   cs_free(insn, count);
   
   
   exit_interface();

   free(mem);

   return 0;

}
