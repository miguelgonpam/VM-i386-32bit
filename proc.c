#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <capstone/capstone.h>
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
 *  Bild_main function. Executes the i386 emulator without graphic interface.
 *
 *  @param argc number of arguments (argv size).
 *  @param argv array containing pointer to args.
 *  @param envp array containing pointer to environment variables.
 *
 *  @return 0 if the execution is sucessful or any other value if not.
 *
 */
int blind_main(int argc, char *argv[], char *envp[]){
   /* Args comprobation */
   if (argc < 2) {
      fprintf(stdout, "Uso: %s <archivo_elf>\n", argv[0]);
      return 1;
   }

   /* Initializes some registers and allocates memory for mem variable */
   if(!initialize())
      return 1;

   /* ini is the first executable instruction's address, r is the last */
   uint32_t *ini, r;

   uint32_t res;

   /* Reads the elf, loads it into the memory and pushes argc, argv and envp into the stack */
   if(read_elf_file(argc, argv, envp, &ini, &r)){
      perror("elf");
      goto exit;
   }

   /* Handler for disassembling */
   csh handle;
   /* insn is the pointer to instructions array, ins is the step by step instruction to execute */
   cs_insn *insn, *ins;
   /* Number of instructions disassembled */
   size_t count;

   /* Sets arch */
   if (cs_open(CS_ARCH_X86, CS_MODE_32, &handle) != CS_ERR_OK){
      perror("setarch");
      return 1;
   }
   
   /* Activate detail feature. Useful for obtaining operands and other info */
   cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);

   /* Disassemble all executable data loaded into mem. From ini to r. Stores it into insn array. */
   /* If 5th argument is 0, disassembles all, if its 1 disassembles only one instruction */
   //count = cs_disasm(handle, &mem[ini], r-ini, ini, 0, &insn);
   //if (!count){
   //   printf("Failed to disassemble code\n");
   //   return 1;
   //}

   /* Main loop */
   while (true){

      /* Disassemble the current instruction. Bad alignment could have happened. */
      if(!cs_disasm(handle, &mem[eip], r-eip, eip, 1, &ins)) // If number of disasm instructions is 0.
         goto exit;

      /* Check interrupts ???*/
      res = dispatcher(ins[0].mnemonic, &ins[0]);

      if (res == 0xdeadbeef){
         goto exit;
      }

      /* Free ins */
      cs_free(ins, 1);
      ins = NULL;
   }
   exit:
   free(mem);
   return 0;
}

/** 
 *  Interface_main function. Executes the i386 emulator with graphic interface.
 *
 *  @param argc number of arguments (argv size).
 *  @param argv array containing pointer to args.
 *  @param envp array containing pointer to environment variables.
 *
 *  @return 0 if the execution is sucessful or any other value if not.
 *
 */
int interface_main(int argc, char *argv[], char *envp[]){

   /* Args comprobation */
   if (argc < 2) {
      fprintf(stdout, "Uso: %s <archivo_elf>\n", argv[0]);
      return 1;
   }

   /* Clear screen and move pointer to (0,0) */
   printf("\033[2J\033[H\033[3J"); 
   
   /* Initializes some registers and allocates memory for mem variable */
   if(!initialize())
      return 1;

   /* ini is the first executable instruction's address, r is the last */
   uint32_t *sheader, cc = 0;


   /* Reads the elf, loads it into the memory and pushes argc, argv and envp into the stack */
   if(read_elf_file(argc, argv, envp, &sheader, &cc)){
      perror("elf");
      goto exit;
   }
   
   /* Handler for disassembling */
   csh handle;
   /* insn is the pointer to instructions array, ins is the step by step instruction to execute */
   cs_insn *insn, *ins;
   /* Number of instructions disassembled */
   size_t count = 0;

   /* Sets arch */
   if (cs_open(CS_ARCH_X86, CS_MODE_32, &handle) != CS_ERR_OK){
      perror("setarch");
      return 1;

   }
   
   /* Activate detail feature. Useful for obtaining operands and other info */
   cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);

   /* Disassemble all executable data loaded into mem. From ini to r. Stores it into insn array. */
   /* If 5th argument is 0, disassembles all, if its 1 disassembles only one instruction */
   
   /* Array of pointer to sections instructions */
   cs_insn **insns = calloc(cc, sizeof(cs_insn *));
   /* Array containing accumulated number of instructions. Actual section + all instructions before */
   int * counts = calloc(sizeof(int), cc);

   

   for (int i=0; i < cc; i++){
      uint32_t addr = sheader[2*i];
      uint32_t siz = sheader[2*i+1];
      uint32_t val = 0;
      if (addr && siz)
          val = cs_disasm(handle, &mem[addr], siz, addr, 0, &insns[i]);
      count += ((val > 0)? val : 0);
      counts[i] = count;  /* Obtain last instr number that belongs to this sh */
      
   }

   

   if (!count){
      printf("Failed to disassemble code\n");
      return 1;
   }
   

   /* Initializes ncurses interaface */
   init_interface();

   

   /* getchar() does not need ENTER anymore */
   init_raw_mode();
   
   /* Allocates memory for pointer array. Its used to store the code lines to print. i.e "<0x08049752>:pop esi" */
   char **lineas = malloc(2*rows * sizeof(char *));
   if (lineas == NULL){
      perror("malloc");
      return 1;
   }
   
   /* Allocates memory for each pointer so it can store a string. */
   for (int i = 0; i<rows; i++){
      /* Only for address */
      lineas[i*2]=malloc(ADDR_TXT_S);
      lineas[i*2+1]=malloc(MAX_STR);

   }
   
   /* Char to get users char and scr_c to scroll instructions screen, scr_s to scroll stack screen, focus to focus on code or stack */
   int ch = 0, old_ch = 0, scr_c = 0, scr_s = 0, focus = 0;
   uint8_t found = 0, cont = 0;
   
   /* Initialize scr_c to EIP instr*/
   for(int i=0; i<cc;i++){
      uint32_t end = sheader[2*i+1];
      uint32_t ini = (!i)?0:counts[i-1];
      
      for (int j=0; j<end;j++){
         if (eip == insns[i][j].address){
            found = 1;
            scr_c = j+ini;
            scr_c = scr_c?scr_c-1:scr_c;
            break;
         }
      }
      if(found){
         break;
      }
   }
   
   /* Initialize breakpoints array */
   uint32_t brkpts[MAX_BRK];
   /* Initialize breakpoints counter */
   uint8_t brk_ctr = 0;
   
   /* Result of a instruction's execution */
   uint32_t res = 0;

   /* While q is not pressed, what would quit the program */
   while ('q' != ch){
      /* EIP index, if its negative, EIP is not visible on the current screen, due to scroll */
      int eip_ind = -1;
      int exited = -1;
      for (size_t i = 0; i < MIN(rows, count -scr_c+1); i++) {
         uint32_t index = i+scr_c;
         uint32_t sh = 0;
         for (int j=0; j<cc;j++){ /* Iterate through section headers */
            if(index < counts[j]){ /* If sh last instruction number is greater than index */
               sh = j; /* Instruction's sh is previous one */
               break;
            }
         }

         /* Get previous section header last instruction index */
         uint32_t prev = (!sh)?0:counts[sh-1];

         uint32_t addr = insns[sh][index - prev].address;
         if (addr == eip){
            eip_ind = i;
         }
         snprintf(lineas[i*2], ADDR_TXT_S-1, "0x%08x", addr);
         snprintf(lineas[i*2+1], MAX_STR, "%.10s %.50s", insns[sh][index - prev].mnemonic, insns[sh][index - prev].op_str);
         exited = i;
      }

      if (exited != rows-1){ /* No more instructions to show */
         while(exited != rows){

            snprintf(lineas[exited*2], 3, " ");
            snprintf(lineas[exited*2+1], 3, " ");
            exited++;
         }
      }
      /* Draw registers, code and stack */
      draw_screen(scr_s, scr_c, lineas, rows, eip_ind);
      
      no_print:
      

      /* Move pointer to last line and Gets user's option */
      move(rows);
      ch = getch();
      
      //ch = getchar(); /* Does not allow arrows */
      
      /* Clean stdin */
      cleanv(rows-2, rows-2);

      /* Set stdin cursor at first line of stdin */
      cleanv(rows, rows);

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

         if(!cs_disasm(handle, &mem[eip], 16, eip, 1, &ins)) // If number of disasm instructions is 0.
            goto exit1;
         

         /* If returned from syscall, set the terminal on raw mode */
         if (ins[0].bytes[0] == 0xCD){ /* INT imm8*/
            disable_raw_mode();
         }

         
         cleann(rows, rows);
         movev(rows);

         /* Check interrupts ???*/
         res = dispatcher(ins[0].mnemonic, &ins[0]);

         /* If returned from syscall, set the terminal on raw mode */
         if (ins[0].bytes[0] == 0xCD){ /* INT imm8*/
            enable_raw_mode();
         }

         /* Free ins */
         cs_free(ins, 1);
         ins = NULL;

         /* Check if ins is syscall(1) (exit), to free memory before exiting */
         if(0xdeadbeef == res)
            goto exit1;

         /* Find EIP and set it at the top of the screen */
         found = 0;
         for(int i=0; i<cc;i++){
            uint32_t end = sheader[2*i+1];
            uint32_t ini = (!i)?0:counts[i-1];
            
            for (int j=0; j<end;j++){
               if (eip == insns[i][j].address){
                  found = 1;
                  scr_c = j+ini;
                  scr_c = scr_c?scr_c-1:scr_c;
                  break;
               }
            }
            if(found){
               break;
            }
         }
         /* Set top of stack at the top of stack window */
         scr_s=0;
      }else if('c' == ch){   
         /* While doesnt hit a breakpoint, continue executing */
         while (!contains(brkpts, brk_ctr, eip)){
            /* Disassemble again the eip instruction because if its not on the screen,
               we cannot know its index within insn array.
               The 1 indicates to only disassemble 1 instruction.
            */
            if(!cs_disasm(handle, &mem[eip], 16, eip, 1, &ins)) // If number of disasm instructions is 0.
               goto exit1;
            

            /* If returned from syscall, set the terminal on raw mode */
            if (ins[0].bytes[0] == 0xCD){ /* INT imm8*/
               disable_raw_mode();
            }

            
            

            /* Check interrupts ???*/
            res = dispatcher(ins[0].mnemonic, &ins[0]);

            cleanv(rows, rows);
            movev(rows);
            /* If returned from syscall, set the terminal on raw mode */
            if (ins[0].bytes[0] == 0xCD){ /* INT imm8*/
               enable_raw_mode();
            }
            
            /* Free ins */
            cs_free(ins, 1);
            ins = NULL;

            /* Check if ins is syscall(1) (exit), to free memory before exiting */
            if(0xdeadbeef == res)
               goto exit1;
         }
         /* Find EIP and set it at the top of the screen */
         found = 0;
         for(int i=0; i<cc;i++){
            uint32_t end = sheader[2*i+1];
            uint32_t ini = (!i)?0:counts[i-1];
            
            for (int j=0; j<end;j++){
               if (eip == insns[i][j].address){
                  found = 1;
                  scr_c = j+ini;
                  scr_c = scr_c?scr_c-1:scr_c;
                  break;
               }
            }
            if(found){
               break;
            }
         }
         /* Set top of stack at the top of stack window */
         scr_s=0;

      }else if('n' == ch){   
         if(!cs_disasm(handle, &mem[eip], 16, eip, 1, &ins)) // If number of disasm instructions is 0.
               goto exit1;
         /* Get next instruction. Designed to avoid stepping into CALL instruction. */
         uint32_t stop = eip + ins[0].size;
         /* While doesnt hit a breakpoint, continue executing */
         while (eip != stop){
            /* Disassemble again the eip instruction because if its not on the screen,
               we cannot know its index within insn array.
               The 1 indicates to only disassemble 1 instruction.
            */
            if(!cs_disasm(handle, &mem[eip], 16, eip, 1, &ins)) // If number of disasm instructions is 0.
               goto exit1;
            

            /* If syscall, set the terminal on normal mode */
            if (ins[0].bytes[0] == 0xCD){ /* INT imm8*/
               disable_raw_mode();
            }

            
            cleanv(rows, rows);
            movev(rows);

            /* Check interrupts ???*/
            res = dispatcher(ins[0].mnemonic, &ins[0]);

            /* If returned from syscall, set the terminal on raw mode */
            if (ins[0].bytes[0] == 0xCD){ /* INT imm8*/
               enable_raw_mode();
            }

            /* Free ins */
            cs_free(ins, 1);
            ins = NULL;

            /* Check if ins is syscall(1) (exit), to free memory before exiting */
            if(0xdeadbeef == res)
               goto exit1;

            
         }
         /* Find EIP and set it at the top of the screen */
         found = 0;
         for(int i=0; i<cc;i++){
            uint32_t end = sheader[2*i+1];
            uint32_t ini = (!i)?0:counts[i-1];
            
            for (int j=0; j<end;j++){
               if (eip == insns[i][j].address){
                  found = 1;
                  scr_c = j+ini;
                  scr_c = scr_c?scr_c-1:scr_c;
                  break;
               }
            }
            if(found){
               break;
            }
         }
         /* Set top of stack at the top of stack window */
         scr_s=0;

      }else if (KEY_DOWN == ch || '2' == ch){
         cleann(rows, rows);
         movev(rows);
         /* Focus is set on Code and scr_c doesnt overflow */
         if (focus == 0){
            if( scr_c+1 < count){
               /* Scroll down */
               scr_c++;
            }else{
               goto no_print;
            }
         }
         /* Focus is set on Stack, overflow checked */
         else{
            if( scr_s+1 < (STACK_BOTTOM - (int)esp)/4 ){
               /* Scroll down */
               scr_s++;
            }else{
               goto no_print;
            }
         }
      }else if (KEY_UP == ch || '8' == ch){
         cleann(rows, rows);
         movev(rows);
         /* Focus is set on Code and scr_c doesnt underflow */
         if (focus == 0){
            if (scr_c > 0){
               /* Scroll up */
               scr_c--;
            }else{
               goto no_print;
            }
         }
         /* Focus is set on Stack and scr_s doesnt underflow */
         else{
            if(scr_s > 0){
               /* Scroll up */
               scr_s--;
            }else{
               goto no_print;
            }
         }
      }else if(KEY_LEFT == ch || '4' == ch){
         cleann(rows, rows);
         movev(rows);
         /* Switch scroll focus to Code */
         focus = 0;
         goto no_print;
      }else if(KEY_RIGHT == ch || '6' == ch){
         cleann(rows, rows);
         movev(rows);
         /* Switch scroll focus to Stack */
         focus = 1;
         goto no_print;
      }else if('b' == ch){
         char str[MAX_STR];
         int res = 0, c = 0;
         uint32_t dir;

         /* While string received not matches the format 0x00000000 */
         while(!res){
            /* Print asking string and get user's response */
            get_str("Breakpoint on direction : (0x00000000 format)", str, MAX_STR-1, c);
            /* Format check */
            res = sscanf(str, "0x%08x", &dir);
            /* Flag for adding "Wrong format" */
            c = 1;
         }
         
         /* Store breakpoint */
         brkpts[brk_ctr]=dir;
         
         /* Clean stdin zone */
         cleanv(rows-2, rows);
         movev(rows-2);

         /* Prints breakpont indexes from 1 to MAX_BRK*/
         printf("Created breakpoint %u at 0x%08x", brk_ctr+1, dir);

         /* Increment breakpoint counter */
         brk_ctr++;
         /* If counter overflows MAX, override the existing ones*/
         brk_ctr %= MAX_BRK;

      }else if('f' == ch){
         /* Sets one instruction or stack addr (depending on focus variable) at the top of its window (either code or stack) */
         char str[MAX_STR];
         int res = 0, c = 0;
         uint32_t dir;
         char txt[MAX_STR];

         /* If focus is et on code, ask for code address, if focus is 1, ask for stack address */
         focus?snprintf(txt, MAX_STR, "Stack address to lookup : (0x00000000 format)"):snprintf(txt, MAX_STR,"Code address to lookup : (0x00000000 format)");

         /* While string received does not match the format 0x00000000 */
         while(!res){
            /* Print asking string and get user's response */
            get_str(txt, str, MAX_STR-1, c);
            /* Format check */
            res = sscanf(str, "0x%08x", &dir);
            /* Flag for showing "Wrong format" */
            c = 1;
         }
         
         if (focus){
            int i = 0;
            /* Find user's address and set it at the top of Stack window*/
            if (dir < STACK_BOTTOM && dir > STACK_TOP){
               scr_s = (int)((dir-esp)/4);
            }
            /*
            for (uint32_t v = esp; v < STACK_BOTTOM; v+=4){
               if (v == dir){
                  scr_s = i;
                  break;
               }
               i++;
            }
            */
         }else{
            /* Find DIR and set it at the top of the screen */
            found = 0;
            for(int i=0; i<cc;i++){
               uint32_t end = (!i)?counts[i]:counts[i]-counts[i-1];
               uint32_t ini = (!i)?0:counts[i-1];
               
               for (int j=0; j<end;j++){
                  if (dir == insns[i][j].address){
                     found = 1;
                     scr_c = j+ini;
                     break;
                  }
               }
               if(found){
                  break;
               }
            }
         }
         cleanv(rows-2,rows);
         
      }else if('x' == ch){
         char str[MAX_STR];
         int res = 0, c = 0;
         uint32_t dir;

         /* While string received not matches the format 0x00000000 */
         while(!res){
            /* Print asking string and get user's response */
            get_str("Address to dump content : (0x00000000 format)", str, MAX_STR-1, c);
            /* Format check */
            res = sscanf(str, "0x%08x", &dir);
            /* Flag for showing "Wrong format" */
            c = 1;
         }
         char txt[25];
         snprintf(txt, 24, "0x%08x : 0x%08x", dir, *((uint32_t *)(mem +dir)));
         cleanv(rows-2, rows);
         movev(rows-2);
         printf(" %s",txt);
      }else if('t' == ch){
         char str[MAX_STR];
         int res = 0, c = 0;
         uint32_t dir;

         /* While string received not matches the format 0x00000000 */
         while(!res){
            /* Print asking string and get user's response */
            get_str("Address to dump string : (0x00000000 format)", str, MAX_STR-1, c);
            /* Format check */
            res = sscanf(str, "0x%08x", &dir);
            /* Flag for showing "Wrong format" */
            c = 1;
         }
         char txt[25];
         cleanv(rows-2, rows);
         movev(rows-2);
         printf(" 0x%08x : %s",dir, mem+dir);
      }else if('d' == ch){
         char str[MAX_STR];
         int res = 0, c = 0;
         uint32_t dir;

         /* While string received not matches the format 0x00000000 */
         while(!res){
            /* Print asking string and get user's response */
            get_str("Breakpoint number to remove : (0 format)", str, MAX_STR-1, c);
            /* Format check */
            res = sscanf(str, "%d", &dir);
            /* Flag for showing "Wrong format" */
            c = 1;
         }

         /* Transform breakpoint number to breakpoint index */
         dir--;
         /* Check overflow to avoid segmentation fault */
         dir %= MAX_BRK;
         /* Delete breakpoint */
         brkpts[dir]=0x00000000;

         /* Clear stdin and move pointer */
         cleanv(rows-2, rows);
         movev(rows-2);
      }
      /* Store old user's choice in case ENTER is pressed */
      old_ch = ch;
      
   }

   /* Tag to clean everything before exiting if something goes wrong */
   exit1:

   /* Free memory */
   for (int i = 0; i<rows*2; i++){
      free(lineas[i]); 
   }

   free(lineas);

   //cs_free(insn, count);
   /* Exit interface */
   exit_interface();

   exit:
   /* Free all memory (4GB on the heap) */
   free(mem);
   /* Set terminal to default */
   disable_raw_mode();

   /* Clear screen and move pointer to (0,0) */
   //printf("\033[2J\033[H\033[3J"); 
   return 0;

}

int main(int argc, char *argv[], char *envp[]){
   return interface_main(argc, argv, envp);
   //return blind_main(argc, argv, envp);
}
