#include <stdio.h>
#include <limits.h>
#include <stdlib.h>


typedef void (*InstrFunc)(unsigned char *, unsigned int *);

//REGISTERS
unsigned int eax, edx, esp, esi, eip, cs, ds, fs, ecx, ebx, ebp, edi, eflags, ss, es, gs = 0;
unsigned int * regs[8] = {&eax, &ecx, &edx, &ebx, &esp, &ebp, &esi, &edi};
unsigned char * regs8[8] = {(unsigned char *)&eax, (unsigned char *)&ecx, (unsigned char *)&edx, (unsigned char *)&ebx, ((unsigned char *)&eax)+1, ((unsigned char *)&ecx)+1, ((unsigned char *)&edx)+1, ((unsigned char *)&ebx)+1};


// AUXILIARY FUNCTIONS

/**
   @param c is the value to convert to binary.
   @param b is the array that will have the binary representation of c.

   Sets array b (expected to be lenght 8) to binary representation of c.

   MSB on position 7, LSB on position 0.
   Example: 15 (0x0f) equals to 0000 1111 -> {1,1,1,1,0,0,0,0}

*/
void toBin(unsigned char c, unsigned char * b){
   unsigned char t = c;
   int i = 0;
   while (t > 0){
      b[i] = t % 2;
      t /= 2;
      i++;
   }
}

// AUXILIARY INSTRUCTION FUNCTIONS

/**
   @param stack is the reference to the stack array.

   Reverses the action od the ENTER instruction. Sets ESP to EBP and then pops to EBP.

*/
void leave(unsigned int* stack ){
   esp=ebp;
   ebp=stack[--esp];
}


/**
   @param stack is the reference to the stack array.
   @param d is the variable that will store the popped item.

   Pops an item from the top of the stack to the variable as second argument.
*/
void pop(unsigned int* stack, unsigned int * d){
   *d = stack[--esp];
}

/**
   @param stack is the reference to the stack array.

   Pops a direction from the stack and uses it as the next program counter (eip).
*/
void ret(unsigned int * stack){
   eip = stack[--esp];
}

/**
   @param i is the direction to jump to.

   Sets the next program counter (eip) to the given direction.
*/
void jump(unsigned int i){
   eip=i;
}

/**
   @param stack is the reference to the stack array.
   @param i is the direction of the subrutine to call.

   Calls a subrutine, storing current eip in the stack.
*/
void call(unsigned int* stack, unsigned int i){
   stack[esp++]=eip;
   eip=i;
}

//#############################################################
//##########         INSTRUCTION FUNCTIONS           ##########
//#############################################################

/**
 * OPCODE 0x00
 */
void instr_add_rm8_r8(unsigned char *code, unsigned int *stack){
   unsigned char modrm = *(code+1);
   unsigned char  * rm8 = (unsigned char *)regs8[(modrm & 0b00000111)];
   unsigned char  * r8 = (unsigned char *)regs8[(modrm & 0b00111000)];
   *rm8 += *r8;
   //flags
}

/**
 * OPCODE 0x01
 */
void instr_add_rm16_32_r16_32(unsigned char *code, unsigned int *stack){
   unsigned char modrm = *(code+1);
   unsigned int * rm32 = regs[(modrm & 0b00000111)];
   unsigned int * r32 = regs[(modrm & 0b00111000)];
   *rm32 += *r32;
}

/**
 * OPCODE 0x02
 */
void instr_add_r8_rm8(unsigned char *code, unsigned int *stack){
   unsigned char modrm = *(code+1);
   unsigned char  * rm8 = (unsigned char *)regs8[(modrm & 0b00000111)];
   unsigned char  * r8 = (unsigned char *)regs8[(modrm & 0b00111000)];
   *r8 += *rm8;
}

/**
 * OPCODE 0x03
 */
void instr_add_r16_32_rm16_32(unsigned char *code, unsigned int *stack){
   unsigned char modrm = *(code+1);
   unsigned int * rm32 = regs[(modrm & 0b00000111)];
   unsigned int * r32 = regs[(modrm & 0b00111000)];
   *r32+=*rm32;
}


/**
 * OPCODE 04   
 */
void instr_add_imm8(unsigned char *code, unsigned int *stack){
   unsigned char * al = (unsigned char *)&eax;
   *al += *(code+1);
}

/**
 * OPCODE 05
 */
void instr_add_imm16_32(unsigned char *code, unsigned int *stack){
   unsigned int v;
   unsigned char * vv = (unsigned char *)&v ;
   *vv = *(code+1);vv++;
   *vv = *(code+2);vv++;
   *vv = *(code+3);vv++;
   *vv = *(code+4);
   eax += v;
}

/**
 * OPCODE 0x0F A3
 */
void instr_bt_r(unsigned char *code, unsigned int *stack){
   unsigned char modrm = *(code+2);
   unsigned short int * f = (unsigned short int *)&eflags;
   unsigned int *rm32 = regs[(modrm & 0b00000111)];
   unsigned int *r32 = regs[(modrm & 0b00111000)];

   unsigned short int nbit = *r32;
   unsigned short int bin = 0x01;
   while(nbit){ //genera el binario en bin para enmascarar el bit a poner en el CF
      nbit--;
      bin+=bin;
   }
   unsigned short int v = *rm32 & bin;
   if(v)
      *f|= 0x1;
   else
      *f &= 0xFFFE;
}

/**
 * OPCODE 0x0F AB
 */
void instr_bts_r(unsigned char *code, unsigned int *stack){
   unsigned char modrm = *(code+2);
   unsigned short int * f = (unsigned short int *)&eflags;
   unsigned int *rm32 = regs[(modrm & 0b00000111)];
   unsigned int *r32 = regs[(modrm & 0b00111000)];

   unsigned short int nbit = *r32;
   unsigned short int bin = 0x01;
   while(nbit){ //genera el binario en bin para enmascarar el bit a poner en el CF
      nbit--;
      bin+=bin;
   }
   unsigned short int v = *rm32 & bin;
   if(v){
      *f|= 0x1;
      *rm32 |= 0x1;
   }else{
      *f &= 0xFFFE;
      *rm32 |= 0x1;
   }
}

/**
 * OPCODE 0x0F B3
 */
void instr_btr_r(unsigned char *code, unsigned int *stack){
   unsigned char modrm = *(code+2);
   unsigned short int * f = (unsigned short int *)&eflags;
   unsigned int *rm32 = regs[(modrm & 0b00000111)];
   unsigned int *r32 = regs[(modrm & 0b00111000)];

   unsigned short int nbit = *r32;
   unsigned short int bin = 0x01;
   while(nbit){ //genera el binario en bin para enmascarar el bit a poner en el CF
      nbit--;
      bin+=bin;
   }
   unsigned short int v = *rm32 & bin;
   if(v){
      *f|= 0x1;
      *rm32 &= 0xFFFFFFFE;
   }else{
      *f &= 0xFFFE;
      *rm32 &= 0xFFFFFFFE;
   }
}

/**
 * OPCODE 0x0F BA /4
 * 
 * Debe tener un valor de 100 (4) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
void instr_bt_imm8(unsigned char *code, unsigned int *stack){
   unsigned char modrm = *(code+2);
   unsigned short int * f = (unsigned short int *)&eflags;
   unsigned int *rm32 = regs[(modrm & 0b00000111)];
   unsigned char nbit = *(code+3);

   unsigned short int bin = 0x01;
   while(nbit){ //genera el binario en bin para enmascarar el bit a poner en el CF
      nbit--;
      bin+=bin;
   }
   unsigned short int v = *rm32 & bin;
   if(v){
      *f|= 0x1;
   }else{
      *f &= 0xFFFE;
   }
}

/**
 * OPCODE 0x0F BA /5
 * 
 * Debe tener un valor de 101 (5) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
void instr_bts_imm8(unsigned char *code, unsigned int *stack){
   unsigned char modrm = *(code+2);
   unsigned short int * f = (unsigned short int *)&eflags;
   unsigned int *rm32 = regs[(modrm & 0b00000111)];
   unsigned char nbit = *(code+3);

   unsigned short int bin = 0x01;
   while(nbit){ //genera el binario en bin para enmascarar el bit a poner en el CF
      nbit--;
      bin+=bin;
   }
   unsigned short int v = *rm32 & bin;
   if(v){
      *f|= 0x1;
      *rm32 |= 0x1;
   }else{
      *f &= 0xFFFE;
      *rm32 |= 0x1;
   }
}

/**
 * OPCODE 0x0F BA /6
 * 
 * Debe tener un valor de 110 (6) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
void instr_btr_imm8(unsigned char *code, unsigned int *stack){
   unsigned char modrm = *(code+2);
   unsigned short int * f = (unsigned short int *)&eflags;
   unsigned int *rm32 = regs[(modrm & 0b00000111)];
   unsigned char nbit = *(code+3);

   unsigned short int bin = 0x01;
   while(nbit){ //genera el binario en bin para enmascarar el bit a poner en el CF
      nbit--;
      bin+=bin;
   }
   unsigned short int v = *rm32 & bin;
   if(v){
      *f|= 0x1;
      *rm32 &= 0xFFFFFFFE;
   }else{
      *f &= 0xFFFE;
      *rm32 &= 0xFFFFFFFE;
   }

}

/**
 * OPCODE 0x0F BA /7
 * 
 * Debe tener un valor de 111 (7) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
void instr_btc_imm8(unsigned char *code, unsigned int *stack){
   unsigned char modrm = *(code+2);
   unsigned short int * f = (unsigned short int *)&eflags;
   unsigned int *rm32 = regs[(modrm & 0b00000111)];
   unsigned char nbit = *(code+3);

   unsigned short int bin = 0x01;
   while(nbit){ //genera el binario en bin para enmascarar el bit a poner en el CF
      nbit--;
      bin+=bin;
   }
   unsigned short int v = *rm32 & bin;
   if(v){
      *f|= 0x1;
      *rm32 ^= (1 << *(code+3));
   }else{
      *f &= 0xFFFE;
      *rm32 ^= (1 << *(code+3));
   }
}

/**
 * OPCODE 0x0F BB
 */
void instr_btc_r(unsigned char *code, unsigned int *stack){
   unsigned char modrm = *(code+2);
   unsigned short int * f = (unsigned short int *)&eflags;
   unsigned int *rm32 = regs[(modrm & 0b00000111)];
   unsigned int *r32 = regs[(modrm & 0b00111000)];

   unsigned short int nbit = *r32;
   unsigned short int bin = 0x01;
   while(nbit){ //genera el binario en bin para enmascarar el bit a poner en el CF
      nbit--;
      bin+=bin;
   }
   unsigned short int v = *rm32 & bin;
   if(v){
      *f|= 0x1;
      *rm32 ^= (1 << *r32);
   }else{
      *f &= 0xFFFE;
      *rm32 ^= (1 << *r32);
   }
}


/**
 * OPCODE 0x0F BC
 */
void instr_bsf(unsigned char *code, unsigned int *stack){
   unsigned char modrm = *(code+2);
   unsigned short int * f = (unsigned short int *)&eflags;
   unsigned int *rm32 = regs[(modrm & 0b00000111)];
   unsigned int *r32 = regs[(modrm & 0b00111000)];

   if ( ! *rm32){
      *f |= 0x0040;
   }else{
      short int temp = 0;
      *f &= 0xFFBF;
      short int cont = 0x1;
      while (!((cont & *rm32) | 0x0)){
         cont+=cont;
         temp++;
         *r32 = temp;
      }
   }

}

/**
 * OPCODE 0x0F BD
 */
void instr_bsr(unsigned char *code, unsigned int *stack){
   unsigned char modrm = *(code+2);
   unsigned short int * f = (unsigned short int *)&eflags;
   unsigned int *rm32 = regs[(modrm & 0b00000111)];
   unsigned int *r32 = regs[(modrm & 0b00111000)];

   if ( ! *rm32){
      *f |= 0x0040;
   }else{
      short int temp = 31;
      *f &= 0xFFBF;
      unsigned int cont = 0x80000000;
      while (!((cont & *rm32) | 0x0)){
         cont/=2;
         temp--;
         *r32 = temp;
      }
   }
}

/**
 * OPCODE 0x14
 */
void instr_adc_ib_al(unsigned char *code, unsigned int *stack){
   unsigned char * al = (unsigned char *)&eax;
   unsigned char b = *(code+1);
   unsigned short int * f = (unsigned short int *)&eflags;
   *al= *al+b+( *f & 0x1);
}

/**
 * OPCODE 0x15
 */
void instr_adc_iw_id(unsigned char *code, unsigned int *stack){
   unsigned int v;
   unsigned char * vv = (unsigned char *)&v;
   unsigned short int * f = (unsigned short int *)&eflags;

   *vv = *(code+1);vv++;
   *vv = *(code+2);vv++;
   *vv = *(code+3);vv++;
   *vv = *(code+4);

   eax+= (v+ (*f &0x1));
}

/**
 * OPCODE 0x20
 */
void instr_and_rm8_r8(unsigned char *code, unsigned int *stack){
   unsigned char modrm = *(code+1);
   unsigned char  * rm8 = (unsigned char *)regs8[(modrm & 0b00000111)];
   unsigned char  * r8 = (unsigned char *)regs8[(modrm & 0b00111000)];
   *rm8 &= *r8;
   unsigned short int * f= (unsigned short int *)&eflags;
   *f&=0xF7BF;
}

/**
 * OPCODE 0x21
 */
void instr_and_rm16_32_r16_32(unsigned char *code, unsigned int *stack){
   unsigned char modrm = *(code+1);
   unsigned int * rm32 = regs[(modrm & 0b00000111)];
   unsigned int * r32 = regs[(modrm & 0b00111000)];
   *rm32 &= *r32;
   unsigned short int * f = (unsigned short int *)&eflags;
   *f&=0xF7BF;
}

/**
 * OPCODE 0x22
 */
void instr_and_r8_rm8(unsigned char *code, unsigned int *stack){
   unsigned char modrm = *(code+1);
   unsigned char  * rm8 = (unsigned char *)regs8[(modrm & 0b00000111)];
   unsigned char  * r8 = (unsigned char *)regs8[(modrm & 0b00111000)];
   *r8 &= *rm8;
   unsigned short int * f = (unsigned short int *)&eflags;
   *f&=0xF7BF;
}

/**
 * OPCODE 0x23
 */
void instr_and_r16_32_rm16_32(unsigned char *code, unsigned int *stack){
   unsigned char modrm = *(code+1);
   unsigned int * rm32 = regs[(modrm & 0b00000111)];
   unsigned int * r32 = regs[(modrm & 0b00111000)];
   *r32 &= *rm32;
   unsigned short int * f = (unsigned short int *)&eflags;
   *f&=0xF7BF;
}

/**
 * OPCODE 0x24
 */
void instr_and_imm8(unsigned char *code, unsigned int *stack){
   unsigned char * al = (unsigned char *)&eax;
   unsigned char v = *(code+1);
   *al &= v;
   unsigned short int * f = (unsigned short int *)&eflags;
   *f&=0xF7BF;
}

/**
 * OPCODE 0x25
 */
void instr_and_imm16_32(unsigned char *code, unsigned int *stack){
   unsigned int v;
   unsigned char * vv = (unsigned char *)&v;

   *vv=*(code+1);vv++;
   *vv=*(code+2);vv++;
   *vv=*(code+3);vv++;
   *vv=*(code+4);

   eax &= v;
   unsigned short int * f = (unsigned short int *)&eflags;
   *f&=0xF7BF;
}



/**
 * OPCODE 0xD5 0A
 */
void instr_aad(unsigned char *code, unsigned int *stack){
   unsigned char * al = (unsigned char *)&eax;
   unsigned char * ah = al+1;
   *al = *ah * 10 + *al;
   *ah = 0;
   //flags? SF PF ZF
}

/**
 * OPCODE 0xD4 0A
 */
void instr_aam(unsigned char *code, unsigned int *stack){
   unsigned char * al = (unsigned char *)&eax;
   unsigned char * ah = al+1;
   *ah = *al / 10;
   *al %= 10;
   //flags?
}

/**
 * OPCODE 0x37
 */
void instr_aaa(unsigned char *code, unsigned int *stack){
   unsigned char * c = (unsigned char *)&eax;
   unsigned short int * f = (unsigned short int *)&eflags;
   if (*f & 0b00000100 || (*c & 0b00001111) > 9){
      *c += 6;
      *c&= 0b00001111;
      c+=3;
      *c+=1;
      *f |= 0x0011;
   }else{
      *f &= 0xFFEE;
   }
   //flags?
}

/**
 * OPCODE 0x3F
 */
void instr_aas(unsigned char *code, unsigned int *stack){
   unsigned char * al = (unsigned char *)&eax;
   unsigned short int * f = (unsigned short int *)&eflags;
   if(*f & 0b00000100 || (*al & 0b00001111) > 9){
      *al-=6;
      *al&=0x0F;
      al+=1; //converts from AL into AH
      *al-=1;
      *f|=0x0011;
   }else{
      *f&=0xFFEE;
   }
   //flags?
}


/**
 * OPCODE 0x40
 */
void instr_inc_eax(unsigned char *code, unsigned int *stack){
   eax++;
}

/**
 * OPCODE 0x41
 */
void instr_inc_ecx(unsigned char *code, unsigned int *stack){
   ecx++;
}

/**
 * OPCODE 0x42
 */
void instr_inc_edx(unsigned char *code, unsigned int *stack){
   edx++;
}

/**
 * OPCODE 0x43
 */
void instr_inc_ebx(unsigned char *code, unsigned int *stack){
   ebx++;
}

/**
 * OPCODE 0x44
 */
void instr_inc_esp(unsigned char *code, unsigned int *stack){
   esp++;
}

/**
 * OPCODE 0x45
 */
void instr_inc_ebp(unsigned char *code, unsigned int *stack){
   ebp++;
}

/**
 * OPCODE 0x46
 */
void instr_inc_esi(unsigned char *code, unsigned int *stack){
   esi++;
}

/**
 * OPCODE 0x47
 */
void instr_inc_edi(unsigned char *code, unsigned int *stack){
   edi++;
}

/**
 * OPCODE 0x48
 */
void instr_dec_eax(unsigned char *code, unsigned int *stack){
   eax--;
}

/**
 * OPCODE 0x49
 */
void instr_dec_ecx(unsigned char *code, unsigned int *stack){
   ecx--;
}

/**
 * OPCODE 0x4A
 */
void instr_dec_edx(unsigned char *code, unsigned int *stack){
   edx--;
}

/**
 * OPCODE 0x4B
 */
void instr_dec_ebx(unsigned char *code, unsigned int *stack){
   ebx--;
}

/**
 * OPCODE 0x4C
 */
void instr_dec_esp(unsigned char *code, unsigned int *stack){
   esp--;
}

/**
 * OPCODE 0x4D
 */
void instr_dec_ebp(unsigned char *code, unsigned int *stack){
   ebp--;
}

/**
 * OPCODE 0x4E
 */
void instr_dec_esi(unsigned char *code, unsigned int *stack){
   esi--;
}

/**
 * OPCODE 0x4F
 */
void instr_dec_edi(unsigned char *code, unsigned int *stack){
   edi--;
}

/**
 * OPCODE 0x50
 */
void instr_push_eax(unsigned char *code, unsigned int *stack) {
   stack[esp++]=eax;}

/**
 * OPCODE 0x51
 */
void instr_push_ecx(unsigned char *code, unsigned int *stack) {
   stack[esp++]=ecx;
}

/**
 * OPCODE 0x52
 */
void instr_push_edx(unsigned char *code, unsigned int *stack) {
   stack[esp++]=edx;
}

/**
 * OPCODE 0x53
 */
void instr_push_ebx(unsigned char *code, unsigned int *stack) {
   stack[esp++]=ebx;
}

/**
 * OPCODE 0x54
 */
void instr_push_esp(unsigned char *code, unsigned int *stack) {
   stack[esp]=esp;
   esp++;
}

/**
 * OPCODE 0x55
 */
void instr_push_ebp(unsigned char *code, unsigned int *stack) {
   stack[esp++]=ebp;
}

/**
 * OPCODE 0x56
 */
void instr_push_esi(unsigned char *code, unsigned int *stack) {
   stack[esp++]=esi;
}

/**
 * OPCODE 0x57
 */
void instr_push_edi(unsigned char *code, unsigned int *stack) {
   stack[esp++]=edi;
}

/**
 * OPCODE 0x58
 */
void instr_pop_eax(unsigned char *code, unsigned int *stack) {
   eax = stack[--esp];
}

/**
 * OPCODE 0x59
 */
void instr_pop_ecx(unsigned char *code, unsigned int *stack) {
   ecx = stack[--esp];
}

/**
 * OPCODE 0x5A
 */
void instr_pop_edx(unsigned char *code, unsigned int *stack) {
   edx = stack[--esp];
}

/**
 * OPCODE 0x5B
 */
void instr_pop_ebx(unsigned char *code, unsigned int *stack) {
   ebx = stack[--esp];
}

/**
 * OPCODE 0x5C
 */
void instr_pop_esp(unsigned char *code, unsigned int *stack) {
   --esp;
   esp = stack[esp];
}

/**
 * OPCODE 0x5D
 */
void instr_pop_ebp(unsigned char *code, unsigned int *stack) {
   ebp = stack[--esp];
}

/**
 * OPCODE 0x5E
 */
void instr_pop_esi(unsigned char *code, unsigned int *stack) {
   esi = stack[--esp];
}

/**
 * OPCODE 0x5F
 */
void instr_pop_edi(unsigned char *code, unsigned int *stack) {
   edi = stack[--esp];
}

/**
 * OPCODE 0x63
 */
void instr_arpl(unsigned char *code, unsigned int *stack){
   unsigned char modrm = *(code+1);
   unsigned short int * rm16 = (unsigned short int *)regs[(modrm & 0b00000111)];
   unsigned short int * r16 = (unsigned short int *)regs[(modrm & 0b00111000)];
   unsigned short int v1,v2;
   unsigned short int * f = (unsigned short int *)&eflags;
   v1 = *rm16 & 0x3;
   v2 = *r16 & 0x3;
   if (v1 < v2){
      *f|=0x0040;
      *rm16 &= 0b1111111111111100;
      *rm16 |= v2;
   }else{
      *f &= 0xFFBF;
   }
}

//------

/**
 * OPCODE 0x80
 * 
 * Includes all the 0x80 instructions, and calls the one needed based on the ModR/M byte
 */
void opcode80(unsigned char *code, unsigned int *stack){

}

/**
 * OPCODE 0x80 /0
 * 
 * Debe tener un valor de 000 (0) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
void instr_add_r8_imm8(unsigned char *code, unsigned int *stack){
   unsigned char *r8 = (unsigned char*)regs8[((*(code+1)) & 0x7)];
   unsigned char v = *(code+2);

   *r8 += v;
   //flags?
}

/**
 * OPCODE 0x80 /2
 * 
 * Debe tener un valor de 010 (2) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
void instr_adc_r8_imm8(unsigned char *code, unsigned int *stack){
   unsigned short int v = *(code+1) ; //byte ModR/M
   unsigned short int * f = (unsigned short int *)&eflags;
   v&=0b00000111;
   unsigned char * reg = (unsigned char *) regs8[v];
   *reg = *reg + *(code+2) + (*f & 0x1);
}

/**
 * OPCODE 0x80 /4
 * 
 * Debe tener un valor de 100 (4) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
void instr_and_r8_imm8(unsigned char *code, unsigned int *stack){
   unsigned char * r8 = (unsigned char*)regs8[((*(code+1)) & 0x7)];
   unsigned char v = *(code+2);

   *r8 &= v;
}


/**
 * OPCODE 0x81
 * 
 * Includes all the 0x81 instructions, and calls the one needed based on the ModR/M byte
 */
void opcode81(unsigned char *code, unsigned int *stack){

}

/**
 * OPCODE 0x81 /0
 * 
 * Debe tener un valor de 000 (0) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
void instr_add_r16_32_imm16_32(unsigned char *code, unsigned int *stack){
   unsigned int * r32 = regs[((*(code+1))&0x7)];
   unsigned int v;
   unsigned char * vv = (unsigned char *)&v;

   *vv=*(code+2);vv++;
   *vv=*(code+3);vv++;
   *vv=*(code+4);vv++;
   *vv=*(code+5);

   *r32 += v;
}

/**
 * OPCODE 0x81 /2
 * 
 * Debe tener un valor de 010 (2) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
void instr_adc_r16_32_imm16_32(unsigned char *code, unsigned int *stack){
   unsigned short int b = *(code+1) ; //byte ModR/M
   unsigned short int * f = (unsigned short int *)&eflags;
   b&=0b00000111;
   unsigned int * reg = regs[b];

   unsigned int v; //valor inmediato
   unsigned char * vv = (unsigned char *)&v; //puntero a byte que recorre el valor inmediato para rellenarlo desde el bytecode

   *vv = *(code+2);vv++;
   *vv = *(code+3);vv++;
   *vv = *(code+4);vv++;
   *vv = *(code+5);

   *reg = *reg + v + ( *f & 0x1);
}

/**
 * OPCODE 0x81 /4
 * 
 * Debe tener un valor de 100 (4) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
void instr_and_r16_32_imm16_32(unsigned char *code, unsigned int *stack){
   unsigned int * r32 = regs[((*code+1) & 0x7)];
   unsigned int v; //valor inmediato
   unsigned char * vv = (unsigned char *)&v; //puntero a byte que recorre el valor inmediato para rellenarlo desde el bytecode

   *vv = *(code+2);vv++;
   *vv = *(code+3);vv++;
   *vv = *(code+4);vv++;
   *vv = *(code+5);

   *r32 &= v;
}


/**
 * OPCODE 0x83
 * 
 * Includes all the 0x83 instructions, and calls the one needed based on the ModR/M byte
 */
void opcode83(unsigned char *code, unsigned int *stack){

}

/**
 * OPCODE 0x83 /0
 * 
 * Debe tener un valor de 000 (0) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
void instr_add_r16_32_imm8(unsigned char *code, unsigned int *stack){
   unsigned int * r32 = regs[((*(code+1))&0x7)];
   unsigned char v = *(code+2);

   *r32 += v;
}

/**
 * OPCODE 0x83 /2
 * 
 * Debe tener un valor de 010 (2) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
void instr_adc_r16_32_imm8(unsigned char *code, unsigned int *stack){
   unsigned short int v = *(code+1) ; //byte ModR/M
   unsigned short int * f = (unsigned short int *)&eflags;
   v&=0b00000111;
   unsigned int * reg = regs[v];
   *reg = *reg + *(code+2) + (*f & 0x1);
}

/**
 * OPCODE 0x83 /4
 */
void instr_and_r16_32_imm8(unsigned char *code, unsigned int *stack){
   unsigned int * r32 = regs[((*(code+1)) & 0x7)];
   unsigned char v = *(code+2);

   *r32 &= v;
}




//------

/**
 * OPCODE 0x90
 */
void instr_nop(unsigned char *code, unsigned int *stack){
   
}
/**
 * OPCODE 0x91
 */
void instr_xchg_ecx_eax(unsigned char *code, unsigned int *stack){
   unsigned int a;
   a = eax;
   eax = ecx;
   ecx = a;
}

/**
 * OPCODE 0x98
 */
void instr_cwde(unsigned char *code, unsigned int *stack){
   unsigned short int *ax = (unsigned short int *) &eax;
   unsigned char bit = (*ax >> 15) & 1;
   unsigned short int v = bit ? 0xFFFF : 0x0000;
   ax++;
   *ax = v; 
}

/**
 * OPCODE 0xF5
 */
void instr_cmc(unsigned char *code, unsigned int *stack){
   unsigned short int * f = (unsigned short int *)&eflags;
   *f ^= 0x1;
}

/**
 * OPCODE 0xF8
 */
void instr_clc(unsigned char *code, unsigned int *stack){
   unsigned short int * f = (unsigned short int *)&eflags;
   *f &=0xFFFE; 
}

/**
 * OPCODE 0xFA
 */
void instr_cli(unsigned char *code, unsigned int *stack){
   unsigned short int * f = (unsigned short int *)&eflags;
   *f&=0xFDFF;
}

/**
 * OPCODE 0xFC
 */
void instr_cld(unsigned char *code, unsigned int *stack){
   unsigned short int * f= (unsigned short int *)&eflags;
   *f&=0xFBFF;
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

   i[0x37]=instr_aaa;

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

   i[0xF5]=instr_cmc;
   i[0xF8]=instr_clc;
   i[0xFA]=instr_cli;
   i[0xFC]=instr_cld;

   //0x0F A3 - instr_bt_r
   //0x0F BA - instr_bt_imm8
   //0x0F BC - instr_bsf
   //0x0F BD - instr_bsr
   
}


int main(){

   //INSTRUCTIONS TABLE
   InstrFunc instr[256];
   fillInstr(instr);
   //STACK
   unsigned int * stack = (unsigned int *)malloc(135168 * sizeof(unsigned char));

   
   eax = 0xFF;
   printf("eax : 0x%8x\n", eax);
   unsigned char code[6] ={ 0x81, 0xD0, 0xFF, 0x00, 0x00, 0x00};
   instr_add_r16_32_imm16_32(code, stack);
   printf("eax : 0x%8x\n", eax);
   

   


   /*
   ebx = 0;
   printf("eax:%8x ebx:%8x esp:%u\n", eax, ebx, esp);
   instr[0x50](&a, stack);
   printf("eax:%8x ebx:%8x esp:%u\n", eax, ebx, esp);
   instr[0x5B](&a, stack);
   printf("eax:%8x ebx:%8x esp:%u\n", eax, ebx, esp);
   */
   free(stack);

   //CODE
   //unsigned char * code = (unsigned char *)malloc(3000 * sizeof(unsigned char));
   //free(code);

   //LIBRARIES
   //unsigned char * lib1 = (unsigned char *)malloc(3000 * sizeof(unsigned char));
   //unsigned char * lib2 = (unsigned char *)malloc(3000 * sizeof(unsigned char));
   //free(lib1);
   //free(lib2);

   return 0;

}

/* NOTES

   Para hexadecimal: %x
   Para decimal sin signo: %u
   unsigned int a = UINT_MAX; // -> direcciones (0xffffffff)
   unsigned char b = UCHAR_MAX; // -> datos (0xff)
   printf("%8x %2x\n", a,b);

   printf("%p \n", (void *)rsp);

   unsigned char c = 31;
   unsigned char bin[8] = {0,0,0,0,0,0,0,0}; //bin[0] => LSB , bin[7] => MSB
   toBin(c, bin);
   printf("%1d%1d%1d%1d %1d%1d%1d%1d\n", bin[7], bin[6], bin[5], bin[4], bin[3], bin[2], bin[1], bin[0]);

   printf("%p %p \n",&stack[0], &stack[1]);
   esp = 0;
   printf("%u %u : %u\n", esp, eax, stack[0]);
   push(stack, 255);
   printf("%u %u : %u\n", esp, eax, stack[0]);
   pop(stack, &eax);
   printf("%u %u : %u\n", esp, eax, stack[0]);

   //unsigned char * p = (unsigned char *)&eax;
   //p+=3;
   // *p=0xFF;
**/
