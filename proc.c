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
#define MAX_STR 75
#define MAX_BRK 25

/* REGISTERS */
extern uint8_t * mem;
extern uint32_t eflags;
extern uint32_t eax, edx, esp, esi, eip, cs, ds, fs, ecx, ebx, ebp, edi, ss, es, gs;
extern uint32_t * regs[8];
extern uint8_t * regs8[8];

/* Screen display size */
extern int rows, cols;

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

/**
 *  Returns 1 if a value is contained in a certain array or 0 if not.
 *
 *  @param arr is the array to check in.
 *  @param size is arr's size
 *  @param val is the value to look for.
 *
 *  @return 1 if val is found or 0 if not.
 */
uint8_t contains(uint32_t arr[], size_t size, uint32_t val){
   for (int i=0; i<size; i++){
      if (arr[i] == val)
         return 1;
   }
   return 0;
}


/** 
 * Main function. Executes the i386 emulator.
 *
 *  @param argc number of arguments (argv size).
 *  @param argv array containing pointer to args.
 *  @param envp array containing pointer to environment variables.
 *
 *  @return 0 if the execution is sucessful or any other value if not.
 *
 */
int main(int argc, char *argv[], char *envp[]){
   /* INITIALIZATION */
   system("clear"); //execve?

   /* Initializes some registers and allocates memory for mem variable */
   if(!initialize())
      return 1;

   /* ini is the first executable instruction's address, r is the last */
   uint32_t ini, r;

   /* Reads the elf, loads it into the memory and pushes argc, argv and envp into the stack */
   if(read_elf_file(argc, argv, envp, &ini, &r)){
      perror("elf");
      exit(1);
   }

   /* Handler for disassembling */
   csh handle;
   /* insn is the pointer to instructions array, ins is the step by step instruction to execute */
   cs_insn *insn, *ins;
   /* Number of instructions disassembled */
   size_t count;

   /* Sets arch */
   if (cs_open(CS_ARCH_X86, CS_MODE_32, &handle) != CS_ERR_OK)
        return -1;
   
   /* Activate detail feature. Useful for obtaining operands and other info */
   cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);

   /* Disassemble all executable data loaded into mem. From ini to r. Stores it into insn array. */
   /* If 5th argument is 0, disassembles all, if its 1 disassembles only one instruction */
   count = cs_disasm(handle, &mem[ini], r-ini, ini, 0, &insn);
   if (!count){
      printf("Failed to disassemble code\n");
      return 1;
   }

   /* Initializes ncurses interaface */
   init_interface();

   /* Allocates memory for pointer array. Its used to store the code lines to print. i.e "<0x08049752>:pop esi" */
   char **lineas = malloc(2*rows * sizeof(char *));
   if (lineas == NULL){
      perror("malloc");
      exit(1);
   }

   /* Allocates memory for each pointer so it can store a string. */
   for (int i = 0; i<rows; i++){
      /* Only for address */
      lineas[i*2]=malloc(ADDR_TXT_S);
      lineas[i*2+1]=malloc(MAX_STR);

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
   
   /* Initialize breakpoints array */
   uint32_t brkpts[MAX_BRK];
   /* Initialize breakpoints counter */
   uint8_t brk_ctr = 0;
   
   /* While q is not pressed, what would quit the program */
   while ('q' != ch){
      /* EIP index, if its negative, EIP is not visible on the current screen, due to scroll */
      int eip_ind = -1;

      for (size_t i = 0; i < MIN(rows, count -scr_c); i++) {
         uint32_t addr = insn[i+scr_c].address;
         if (addr == eip){
            eip_ind = i;
         }
         snprintf(lineas[i*2], ADDR_TXT_S-1, "<0x%08x>:", addr);
         snprintf(lineas[i*2+1], MAX_STR, "%.10s %.50s", insn[i+scr_c].mnemonic, insn[i+scr_c].op_str);
         //if (strcmp(insn[i].mnemonic, "push") == 0){
         //   push_i(&insn[i]);
         //}
         //eip += insn[i].size;
      }
      /* Draw registers, code, stack and cmd box */
      draw_regs();
      draw_stack(scr_s);
      draw_code(lineas, rows, eip_ind);
      draw_cmd();

      /* Gets user's option */
      ch = getch();

      /* If user's choice is ENTER key */
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
            cmd_get_str(str, "Breakpoint on direction : (0x00000000 format)",MAX_STR,c);
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

      }else if('f' == ch){
         /* Sets one instruction or stack addr (depending on focus variable) at the top of its window (either code or stack) */
         char str[MAX_STR];
         int res = 0, c = 0;
         uint32_t dir;
         char txt[MAX_STR];

         /* If focus is et on code, ask for code address, if focus is 1, ask for stack address */
         focus?snprintf(txt, MAX_STR, "Stack address to lookup : (0x00000000 format)"):snprintf(txt, MAX_STR,"Code address to lookup : (0x00000000 format)");

         /* While string received not matches the format 0x00000000 */
         while(!res){
            cmd_get_str(str, txt, MAX_STR, c);
            res = sscanf(str, "0x%08x", &dir);
            c = 1;
         }

         if (focus){
            int i = 0;
            /* Find user's address and set it at the top of Stack window*/
            for (uint32_t v = esp; v < STACK_BOTTOM; v+=4){
               if (v == dir){
                  scr_s = i;
                  break;
               }
               i++;
            }
         }else{
            for (int i=0;i<count;i++){
               if (dir == insn[i].address){
                  scr_c = i;
                  break;
               }
            }
         }

         
      }
      /* Store old user's choice in case ENTER is pressed */
      old_ch = ch;
      
   }


   /* Free memory */
   for (int i = 0; i<rows*2; i++){
      free(lineas[i]); 
   }
   free(lineas);
   cs_free(insn, count);
   
   /* Exit interface */
   exit_interface();

   /* Free all memory (4GB on the heap) */
   free(mem);

   return 0;

}
