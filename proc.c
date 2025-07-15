#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include "instructions.h"
#include "flags.h"
/**

#define SET_FLAG(f)   (eflags |= (f))
#define CLEAR_FLAG(f) (eflags &= ~(f))

#define CFV 0x1
#define PFV 0x4
#define AFV 0x10
#define ZFV 0x40
#define SFV 0x80
#define OFV 0x800

#define CF 0b0000000000000001
#define PF 0b0000000000000100
#define AF 0b0000000000010000
#define ZF 0b0000000001000000
#define SF 0b0000000010000000
#define OF 0b0000100000000000
 */

typedef int (*InstrFunc)(uint8_t *, uint32_t *);

//REGISTERS
extern uint32_t eflags; //= 0b00000000000000000000000000000010;
extern uint32_t eax, edx, esp, esi, eip, cs, ds, fs, ecx, ebx, ebp, edi, ss, es, gs;// = 0;
extern uint32_t * regs[8];// = {&eax, &ecx, &edx, &ebx, &esp, &ebp, &esi, &edi};
extern uint8_t * regs8[8];// = {(uint8_t *)&eax, (uint8_t *)&ecx, (uint8_t *)&edx, (uint8_t *)&ebx, ((uint8_t *)&eax)+1, ((uint8_t *)&ecx)+1, ((uint8_t *)&edx)+1, ((uint8_t *)&ebx)+1};


// AUXILIARY FUNCTIONS

/**
   @param c is the value to convert to binary.
   @param b is the array that will have the binary representation of c.

   Sets array b (expected to be lenght 8) to binary representation of c.

   MSB on position 7, LSB on position 0.
   Example: 15 (0x0f) equals to 0000 1111 -> {1,1,1,1,0,0,0,0}

*/

// AUXILIARY INSTRUCTION FUNCTIONS

/**
   @param stack is the reference to the stack array.

   Reverses the action od the ENTER instruction. Sets ESP to EBP and then pops to EBP.

*/
void leave(uint32_t* stack ){
   esp=ebp;
   ebp=stack[--esp];
}


/**
   @param stack is the reference to the stack array.
   @param d is the variable that will store the popped item.

   Pops an item from the top of the stack to the variable as second argument.
*/
void pop(uint32_t* stack, uint32_t * d){
   *d = stack[--esp];
}

/**
   @param stack is the reference to the stack array.

   Pops a direction from the stack and uses it as the next program counter (eip).
*/
void ret(uint32_t * stack){
   eip = stack[--esp];
}

/**
   @param i is the direction to jump to.

   Sets the next program counter (eip) to the given direction.
*/
void jump(uint32_t i){
   eip=i;
}

/**
   @param stack is the reference to the stack array.
   @param i is the direction OFV the subrutine to call.

   Calls a subrutine, storing current eip in the stack.
*/
void call(uint32_t* stack, uint32_t i){
   stack[esp++]=eip;
   eip=i;
}





void fillInstr(InstrFunc * i){
   i[0x00]=instr_add_rm8_r8;
   i[0x01]=instr_add_rm16_32_r16_32;
   i[0x02]=instr_add_r8_rm8;
   i[0x03]=instr_add_r16_32_rm16_32;
   i[0x04]=instr_add_imm8;
   i[0x05]=instr_add_imm16_32;

   i[0x14]=instr_adc_ib_al;
   i[0x15]=instr_adc_iw_id;

   i[0x20]=instr_and_rm8_r8;
   i[0x21]=instr_and_rm16_32_r16_32;
   i[0x22]=instr_and_r8_rm8;
   i[0x23]=instr_and_r16_32_rm16_32;
   i[0x24]=instr_and_imm8;
   i[0x25]=instr_and_imm16_32;

   i[0x27]=instr_daa;

   i[0x2F]=instr_das;

   i[0x37]=instr_aaa;
   i[0x38]=instr_cmp_rm8_r8;
   i[0x39]=instr_cmp_rm16_32_r16_32;
   i[0x3A]=instr_cmp_r8_rm8;
   i[0x3B]=instr_cmp_r16_32_rm16_32;
   i[0x3C]=instr_cmp_al_imm8;
   i[0x3D]=instr_cmp_eax_imm32;
   

   i[0x3F]=instr_aas;
   i[0x40]=instr_inc_eax;
   i[0x41]=instr_inc_ecx;
   i[0x42]=instr_inc_edx;
   i[0x43]=instr_inc_ebx;
   i[0x44]=instr_inc_esp;
   i[0x45]=instr_inc_ebp;
   i[0x46]=instr_inc_esi;
   i[0x47]=instr_inc_edi;
   i[0x48]=instr_dec_eax;
   i[0x49]=instr_dec_ecx;
   i[0x4A]=instr_dec_edx;
   i[0x4B]=instr_dec_ebx;
   i[0x4C]=instr_dec_esp;
   i[0x4D]=instr_dec_ebp;
   i[0x4E]=instr_dec_esi;
   i[0x4F]=instr_dec_edi;
   i[0x50]=instr_push_eax;
   i[0x51]=instr_push_ecx;
   i[0x52]=instr_push_edx;
   i[0x53]=instr_push_ebx;
   i[0x54]=instr_push_esp;
   i[0x55]=instr_push_ebp;
   i[0x56]=instr_push_esi;
   i[0x58]=instr_pop_eax;
   i[0x59]=instr_pop_ecx;
   i[0x5A]=instr_pop_edx;
   i[0x5B]=instr_pop_ebx;
   i[0x5C]=instr_pop_esp;
   i[0x5D]=instr_pop_ebp;
   i[0x5E]=instr_pop_esi;

   i[0x63]=instr_arpl;

   i[0x80]=opcode80;
   i[0x81]=opcode81;

   i[0x83]=opcode83;

   i[0x90]=instr_nop;
   i[0x91]=instr_xchg_ecx_eax;

   i[0x98]=instr_cwde;
   i[0x99]=instr_cdq;

   i[0xC8]=instr_enter;

   i[0xF5]=instr_cmc;
   i[0xF8]=instr_clc;
   i[0xFA]=instr_cli;
   i[0xFC]=instr_cld;

   i[0xFE]=instr_dec_rm8;
   i[0xFF]=instr_dec_rm16_32;

   //0x0F A3 - instr_bt_r
   //0x0F BA - instr_bt_imm8
   //0x0F BC - instr_bSFV
   //0x0F BD - instr_bsr
   
}

void printAll(){
   printf("eax    : 0x%08x\t\tecx    : 0x%08x\t\tedx    : 0x%08x\t\tebx    : 0x%08x\nesp    : 0x%08x\t\tebp    : 0x%08x\t\tesi    : 0x%08x\t\tedi    : 0x%08x\neip    : 0x%08x\t\teflags : 0x%08x\t\tcs     : 0x%08x\t\tds     : 0x%08x\nfs     : 0x%08x\t\tss     : 0x%08x\t\tes     : 0x%08x\t\tgs     : 0x%08x\n", eax, ecx, edx, ebx, esp, ebp, esi, edi, eip, eflags,cs, ds, fs, ss, es, gs);
}


int main(){
   //system("clear");execve?
   system("clear");

   //INSTRUCTIONS TABLE
   InstrFunc instr[256];
   fillInstr(instr);
   
   //MEMORY
   //uint8_t * memory = (uint8_t *)malloc(UINT32_MAX * sizeof(uint8_t)); //4GB de memoria del i386 (32 bits)
   //uint32_t * stack = (uint32_t) memory + 0xFFF00000;

   //STACK
   uint32_t * stack = (uint32_t *)malloc(135168 * sizeof(uint8_t)); //UINT32_MAX en vez de 135168? no, es tamaño de pila, no toda la memoria
   uint8_t * code = (uint8_t *)malloc(1024 * sizeof(uint8_t));
   printAll();
   instr_inc_eax(code, stack);
   printAll();

   /*
   eax = 0xFF;
   printf("eax : 0x%8x\n", eax);
   uint8_t code[6] ={ 0x81, 0xD0, 0xFF, 0x00, 0x00, 0x00};
   instr_add_r16_32_imm16_32(code, stack);
   printf("eax : 0x%8x\n", eax);


   ebx = 0;
   printf("eax:%8x ebx:%8x esp:%u\n", eax, ebx, esp);
   instr[0x50](&a, stack);
   printf("eax:%8x ebx:%8x esp:%u\n", eax, ebx, esp);
   instr[0x5B](&a, stack);
   printf("eax:%8x ebx:%8x esp:%u\n", eax, ebx, esp);
   */
   free(stack);

   //CODE
   //uint8_t * code = (uint8_t *)malloc(3000 * sizeof(uint8_t));
   //free(code);

   //LIBRARIES
   //uint8_t * lib1 = (uint8_t *)malloc(3000 * sizeof(uint8_t));
   //uint8_t * lib2 = (uint8_t *)malloc(3000 * sizeof(uint8_t));
   //free(lib1);
   //free(lib2);

   return 0;

}

/* NOTES

   Para hexadecimal: %x
   Para decimal sin signo: %u
   uint32_t a = UINT_MAX; // -> direcciones (0xffffffff)
   uint8_t b = UCHAR_MAX; // -> datos (0xff)
   printf("%8x %2x\n", a,b);

   printf("%p \n", (void *)rsp);

   uint8_t c = 31;
   uint8_t bin[8] = {0,0,0,0,0,0,0,0}; //bin[0] => LSB , bin[7] => MSB
   toBin(c, bin);
   printf("%1d%1d%1d%1d %1d%1d%1d%1d\n", bin[7], bin[6], bin[5], bin[4], bin[3], bin[2], bin[1], bin[0]);

   printf("%p %p \n",&stack[0], &stack[1]);
   esp = 0;
   printf("%u %u : %u\n", esp, eax, stack[0]);
   push(stack, 255);
   printf("%u %u : %u\n", esp, eax, stack[0]);
   pop(stack, &eax);
   printf("%u %u : %u\n", esp, eax, stack[0]);

   //uint8_t * p = (uint8_t *)&eax;
   //p+=3;
   // *p=0xFF;
**/
