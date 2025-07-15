#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include "instructions.h"
#include "flags.h"

typedef int (*InstrFunc)(uint8_t *);

//REGISTERS
extern uint8_t * mem;
extern uint32_t eflags;
extern uint32_t eax, edx, esp, esi, eip, cs, ds, fs, ecx, ebx, ebp, edi, ss, es, gs;
extern uint32_t * regs[8];
extern uint8_t * regs8[8];


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
   i[0x00]=add_rm8_r8;
   i[0x01]=add_rm16_32_r16_32;
   i[0x02]=add_r8_rm8;
   i[0x03]=add_r16_32_rm16_32;
   i[0x04]=add_imm8;
   i[0x05]=add_imm16_32;

   i[0x0F]=opcode0F;

   i[0x14]=adc_ib_al;
   i[0x15]=adc_iw_id;

   i[0x20]=and_rm8_r8;
   i[0x21]=and_rm16_32_r16_32;
   i[0x22]=and_r8_rm8;
   i[0x23]=and_r16_32_rm16_32;
   i[0x24]=and_imm8;
   i[0x25]=and_imm16_32;

   i[0x27]=daa;

   i[0x2F]=das;

   i[0x37]=aaa;
   i[0x38]=cmp_rm8_r8;
   i[0x39]=cmp_rm16_32_r16_32;
   i[0x3A]=cmp_r8_rm8;
   i[0x3B]=cmp_r16_32_rm16_32;
   i[0x3C]=cmp_al_imm8;
   i[0x3D]=cmp_eax_imm32;
   

   i[0x3F]=aas;
   i[0x40]=inc_eax;
   i[0x41]=inc_ecx;
   i[0x42]=inc_edx;
   i[0x43]=inc_ebx;
   i[0x44]=inc_esp;
   i[0x45]=inc_ebp;
   i[0x46]=inc_esi;
   i[0x47]=inc_edi;
   i[0x48]=dec_eax;
   i[0x49]=dec_ecx;
   i[0x4A]=dec_edx;
   i[0x4B]=dec_ebx;
   i[0x4C]=dec_esp;
   i[0x4D]=dec_ebp;
   i[0x4E]=dec_esi;
   i[0x4F]=dec_edi;
   i[0x50]=push_eax;
   i[0x51]=push_ecx;
   i[0x52]=push_edx;
   i[0x53]=push_ebx;
   i[0x54]=push_esp;
   i[0x55]=push_ebp;
   i[0x56]=push_esi;
   i[0x58]=pop_eax;
   i[0x59]=pop_ecx;
   i[0x5A]=pop_edx;
   i[0x5B]=pop_ebx;
   i[0x5C]=pop_esp;
   i[0x5D]=pop_ebp;
   i[0x5E]=pop_esi;

   i[0x63]=arpl;

   i[0x80]=opcode80;
   i[0x81]=opcode81;

   i[0x83]=opcode83;

   i[0x90]=nop;
   i[0x91]=xchg_ecx_eax;

   i[0x98]=cwde;
   i[0x99]=cdq;

   i[0xC8]=enter;

   i[0xF5]=cmc;
   i[0xF8]=clc;
   i[0xFA]=cli;
   i[0xFC]=cld;

   i[0xFE]=dec_rm8;
   i[0xFF]=dec_rm16_32;

   //0x0F A3 - bt_r
   //0x0F BA - bt_imm8
   //0x0F BC - bSFV
   //0x0F BD - bsr
   
}

void printAll(){
   printf("\neax    : 0x%08x\t\tecx    : 0x%08x\t\tedx    : 0x%08x\t\tebx    : 0x%08x\nesp    : 0x%08x\t\tebp    : 0x%08x\t\tesi    : 0x%08x\t\tedi    : 0x%08x\neip    : 0x%08x\t\teflags : 0x%08x\t\tcs     : 0x%08x\t\tds     : 0x%08x\nfs     : 0x%08x\t\tss     : 0x%08x\t\tes     : 0x%08x\t\tgs     : 0x%08x\n", eax, ecx, edx, ebx, esp, ebp, esi, edi, eip, eflags,cs, ds, fs, ss, es, gs);
}

void printStack(){
   //printf();
}

void printPointers(){
   printf("\neax    : 0x%p\t\tecx    : 0x%p\t\tedx    : 0x%p\t\tebx    : 0x%p\nesp    : 0x%p\t\tebp    : 0x%p\t\tesi    : 0x%p\t\tedi    : 0x%p\neip    : 0x%p\t\teflags : 0x%p\t\tcs     : 0x%p\t\tds     : 0x%p\nfs     : 0x%p\t\tss     : 0x%p\t\tes     : 0x%p\t\tgs     : 0x%p\n", &eax, &ecx, &edx, &ebx, &esp, &ebp, &esi, &edi, &eip, &eflags,&cs, &ds, &fs, &ss, &es, &gs);
}



int main(){
   //system("clear");execve?
   system("clear");
   initialize();
   //INSTRUCTIONS TABLE
   InstrFunc instr[256];
   fillInstr(instr);
   
   //MEMORY
   
   esp = 0xFFF00000;
   eip = 0x08490000;
   printPointers();

   *((mem)+(eip))=0xC2;
   //printf("\nmem   %p 0x%08x",mem+(eip), parse_4B_imm_op(mem));
   uint8_t * op1, op2;
   parse_ops_1B(op1, op2);
   *((mem)+(eip))=0x80;
   parse_ops_1B(op1, op2);
   *((mem)+(eip))=0x40;
   parse_ops_1B(op1, op2);
   *((mem)+(eip))=0x00;
   parse_ops_1B(op1, op2);


   
   printAll();
   eax=0xffddccaa;
   printAll();

   free(mem);

   return 0;

}

/* NOTES

   Para hexadecimal: %x
   Para decimal sin signo: %u
   Para puntero : %p
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
