#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>


typedef void (*InstrFunc)(uint8_t *, uint32_t *);

//REGISTERS
uint32_t eax, edx, esp, esi, eip, cs, ds, fs, ecx, ebx, ebp, edi, eflags, ss, es, gs = 0;
uint32_t * regs[8] = {&eax, &ecx, &edx, &ebx, &esp, &ebp, &esi, &edi};
uint8_t * regs8[8] = {(uint8_t *)&eax, (uint8_t *)&ecx, (uint8_t *)&edx, (uint8_t *)&ebx, ((uint8_t *)&eax)+1, ((uint8_t *)&ecx)+1, ((uint8_t *)&edx)+1, ((uint8_t *)&ebx)+1};


// AUXILIARY FUNCTIONS

/**
   @param c is the value to convert to binary.
   @param b is the array that will have the binary representation of c.

   Sets array b (expected to be lenght 8) to binary representation of c.

   MSB on position 7, LSB on position 0.
   Example: 15 (0x0f) equals to 0000 1111 -> {1,1,1,1,0,0,0,0}

*/
void toBin(uint8_t c, uint8_t * b){
   uint8_t t = c;
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
   @param i is the direction of the subrutine to call.

   Calls a subrutine, storing current eip in the stack.
*/
void call(uint32_t* stack, uint32_t i){
   stack[esp++]=eip;
   eip=i;
}

//#############################################################
//##########         INSTRUCTION FUNCTIONS           ##########
//#############################################################

/**
 * OPCODE 0x00
 */
void instr_add_rm8_r8(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+1);
   uint8_t  * rm8 = (uint8_t *)regs8[(modrm & 0b00000111)];
   uint8_t  * r8 = (uint8_t *)regs8[(modrm & 0b00111000)];
   *rm8 += *r8;
   //flags
}

/**
 * OPCODE 0x01
 */
void instr_add_rm16_32_r16_32(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+1);
   uint32_t * rm32 = regs[(modrm & 0b00000111)];
   uint32_t * r32 = regs[(modrm & 0b00111000)];
   *rm32 += *r32;
}

/**
 * OPCODE 0x02
 */
void instr_add_r8_rm8(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+1);
   uint8_t  * rm8 = (uint8_t *)regs8[(modrm & 0b00000111)];
   uint8_t  * r8 = (uint8_t *)regs8[(modrm & 0b00111000)];
   *r8 += *rm8;
}

/**
 * OPCODE 0x03
 */
void instr_add_r16_32_rm16_32(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+1);
   uint32_t * rm32 = regs[(modrm & 0b00000111)];
   uint32_t * r32 = regs[(modrm & 0b00111000)];
   *r32+=*rm32;
}


/**
 * OPCODE 04   
 */
void instr_add_imm8(uint8_t *code, uint32_t *stack){
   uint8_t * al = (uint8_t *)&eax;
   *al += *(code+1);
}

/**
 * OPCODE 05
 */
void instr_add_imm16_32(uint8_t *code, uint32_t *stack){
   uint32_t v;
   uint8_t * vv = (uint8_t *)&v ;
   *vv = *(code+1);vv++;
   *vv = *(code+2);vv++;
   *vv = *(code+3);vv++;
   *vv = *(code+4);
   eax += v;
}

/**
 * OPCODE 0x0F A3
 */
void instr_bt_r(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+2);
   uint16_t * f = (uint16_t *)&eflags;
   uint32_t *rm32 = regs[(modrm & 0b00000111)];
   uint32_t *r32 = regs[(modrm & 0b00111000)];

   uint16_t nbit = *r32;
   uint16_t bin = 0x01;
   while(nbit){ //genera el binario en bin para enmascarar el bit a poner en el CF
      nbit--;
      bin+=bin;
   }
   uint16_t v = *rm32 & bin;
   if(v)
      *f|= 0x1;
   else
      *f &= 0xFFFE;
}

/**
 * OPCODE 0x0F AB
 */
void instr_bts_r(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+2);
   uint16_t * f = (uint16_t *)&eflags;
   uint32_t *rm32 = regs[(modrm & 0b00000111)];
   uint32_t *r32 = regs[(modrm & 0b00111000)];

   uint16_t nbit = *r32;
   uint16_t bin = 0x01;
   while(nbit){ //genera el binario en bin para enmascarar el bit a poner en el CF
      nbit--;
      bin+=bin;
   }
   uint16_t v = *rm32 & bin;
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
void instr_btr_r(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+2);
   uint16_t * f = (uint16_t *)&eflags;
   uint32_t *rm32 = regs[(modrm & 0b00000111)];
   uint32_t *r32 = regs[(modrm & 0b00111000)];

   uint16_t nbit = *r32;
   uint16_t bin = 0x01;
   while(nbit){ //genera el binario en bin para enmascarar el bit a poner en el CF
      nbit--;
      bin+=bin;
   }
   uint16_t v = *rm32 & bin;
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
void instr_bt_imm8(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+2);
   uint16_t * f = (uint16_t *)&eflags;
   uint32_t *rm32 = regs[(modrm & 0b00000111)];
   uint8_t nbit = *(code+3);

   uint16_t bin = 0x01;
   while(nbit){ //genera el binario en bin para enmascarar el bit a poner en el CF
      nbit--;
      bin+=bin;
   }
   uint16_t v = *rm32 & bin;
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
void instr_bts_imm8(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+2);
   uint16_t * f = (uint16_t *)&eflags;
   uint32_t *rm32 = regs[(modrm & 0b00000111)];
   uint8_t nbit = *(code+3);

   uint16_t bin = 0x01;
   while(nbit){ //genera el binario en bin para enmascarar el bit a poner en el CF
      nbit--;
      bin+=bin;
   }
   uint16_t v = *rm32 & bin;
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
void instr_btr_imm8(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+2);
   uint16_t * f = (uint16_t *)&eflags;
   uint32_t *rm32 = regs[(modrm & 0b00000111)];
   uint8_t nbit = *(code+3);

   uint16_t bin = 0x01;
   while(nbit){ //genera el binario en bin para enmascarar el bit a poner en el CF
      nbit--;
      bin+=bin;
   }
   uint16_t v = *rm32 & bin;
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
void instr_btc_imm8(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+2);
   uint16_t * f = (uint16_t *)&eflags;
   uint32_t *rm32 = regs[(modrm & 0b00000111)];
   uint8_t nbit = *(code+3);

   uint16_t bin = 0x01;
   while(nbit){ //genera el binario en bin para enmascarar el bit a poner en el CF
      nbit--;
      bin+=bin;
   }
   uint16_t v = *rm32 & bin;
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
void instr_btc_r(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+2);
   uint16_t * f = (uint16_t *)&eflags;
   uint32_t *rm32 = regs[(modrm & 0b00000111)];
   uint32_t *r32 = regs[(modrm & 0b00111000)];

   uint16_t nbit = *r32;
   uint16_t bin = 0x01;
   while(nbit){ //genera el binario en bin para enmascarar el bit a poner en el CF
      nbit--;
      bin+=bin;
   }
   uint16_t v = *rm32 & bin;
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
void instr_bsf(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+2);
   uint16_t * f = (uint16_t *)&eflags;
   uint32_t *rm32 = regs[(modrm & 0b00000111)];
   uint32_t *r32 = regs[(modrm & 0b00111000)];

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
void instr_bsr(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+2);
   uint16_t * f = (uint16_t *)&eflags;
   uint32_t *rm32 = regs[(modrm & 0b00000111)];
   uint32_t *r32 = regs[(modrm & 0b00111000)];

   if ( ! *rm32){
      *f |= 0x0040;
   }else{
      short int temp = 31;
      *f &= 0xFFBF;
      uint32_t cont = 0x80000000;
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
void instr_adc_ib_al(uint8_t *code, uint32_t *stack){
   uint8_t * al = (uint8_t *)&eax;
   uint8_t b = *(code+1);
   uint16_t * f = (uint16_t *)&eflags;
   *al= *al+b+( *f & 0x1);
}

/**
 * OPCODE 0x15
 */
void instr_adc_iw_id(uint8_t *code, uint32_t *stack){
   uint32_t v;
   uint8_t * vv = (uint8_t *)&v;
   uint16_t * f = (uint16_t *)&eflags;

   *vv = *(code+1);vv++;
   *vv = *(code+2);vv++;
   *vv = *(code+3);vv++;
   *vv = *(code+4);

   eax+= (v+ (*f &0x1));
}

/**
 * OPCODE 0x20
 */
void instr_and_rm8_r8(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+1);
   uint8_t  * rm8 = (uint8_t *)regs8[(modrm & 0b00000111)];
   uint8_t  * r8 = (uint8_t *)regs8[(modrm & 0b00111000)];
   *rm8 &= *r8;
   uint16_t * f= (uint16_t *)&eflags;
   *f&=0xF7BF;
}

/**
 * OPCODE 0x21
 */
void instr_and_rm16_32_r16_32(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+1);
   uint32_t * rm32 = regs[(modrm & 0b00000111)];
   uint32_t * r32 = regs[(modrm & 0b00111000)];
   *rm32 &= *r32;
   uint16_t * f = (uint16_t *)&eflags;
   *f&=0xF7BF;
}

/**
 * OPCODE 0x22
 */
void instr_and_r8_rm8(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+1);
   uint8_t  * rm8 = (uint8_t *)regs8[(modrm & 0b00000111)];
   uint8_t  * r8 = (uint8_t *)regs8[(modrm & 0b00111000)];
   *r8 &= *rm8;
   uint16_t * f = (uint16_t *)&eflags;
   *f&=0xF7BF;
}

/**
 * OPCODE 0x23
 */
void instr_and_r16_32_rm16_32(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+1);
   uint32_t * rm32 = regs[(modrm & 0b00000111)];
   uint32_t * r32 = regs[(modrm & 0b00111000)];
   *r32 &= *rm32;
   uint16_t * f = (uint16_t *)&eflags;
   *f&=0xF7BF;
}

/**
 * OPCODE 0x24
 */
void instr_and_imm8(uint8_t *code, uint32_t *stack){
   uint8_t * al = (uint8_t *)&eax;
   uint8_t v = *(code+1);
   *al &= v;
   uint16_t * f = (uint16_t *)&eflags;
   *f&=0xF7BF;
}

/**
 * OPCODE 0x25
 */
void instr_and_imm16_32(uint8_t *code, uint32_t *stack){
   uint32_t v;
   uint8_t * vv = (uint8_t *)&v;

   *vv=*(code+1);vv++;
   *vv=*(code+2);vv++;
   *vv=*(code+3);vv++;
   *vv=*(code+4);

   eax &= v;
   uint16_t * f = (uint16_t *)&eflags;
   *f&=0xF7BF;
}



/**
 * OPCODE 0xD5 0A
 */
void instr_aad(uint8_t *code, uint32_t *stack){
   uint8_t * al = (uint8_t *)&eax;
   uint8_t * ah = al+1;
   *al = *ah * 10 + *al;
   *ah = 0;
   //flags? SF PF ZF
}

/**
 * OPCODE 0xD4 0A
 */
void instr_aam(uint8_t *code, uint32_t *stack){
   uint8_t * al = (uint8_t *)&eax;
   uint8_t * ah = al+1;
   *ah = *al / 10;
   *al %= 10;
   //flags?
}

/**
 * OPCODE 0x37
 */
void instr_aaa(uint8_t *code, uint32_t *stack){
   uint8_t * c = (uint8_t *)&eax;
   uint16_t * f = (uint16_t *)&eflags;
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
 * OPCODE 0x38
 */
void instr_cmp_rm8_r8(uint8_t *code, uint32_t *stack){
   //rm8 -r8
}

/**
 * OPCODE 0x39
 */
void instr_cmp_rm16_32_r16_32(uint8_t *code, uint32_t *stack){
   //rm32-r32
}

/**
 * OPCODE 0x3A
 */
void instr_cmp_r8_rm8(uint8_t *code, uint32_t *stack){
   //r8 -rm8
}

/**
 * OPCODE 0x3B
 */
void instr_cmp_r16_32_rm16_32(uint8_t *code, uint32_t *stack){
   //r32-rm32
}

/**
 * OPCODE 0x3C
 */
void instr_cmp_al_imm8(uint8_t *code, uint32_t *stack){

}

/**
 * OPCODE 0x3D
 */
void instr_cmp_eax_imm32(uint8_t *code, uint32_t *stack){

}

/**
 * OPCODE 0x3F
 */
void instr_aas(uint8_t *code, uint32_t *stack){
   uint8_t * al = (uint8_t *)&eax;
   uint16_t * f = (uint16_t *)&eflags;
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
void instr_inc_eax(uint8_t *code, uint32_t *stack){
   eax++;
}

/**
 * OPCODE 0x41
 */
void instr_inc_ecx(uint8_t *code, uint32_t *stack){
   ecx++;
}

/**
 * OPCODE 0x42
 */
void instr_inc_edx(uint8_t *code, uint32_t *stack){
   edx++;
}

/**
 * OPCODE 0x43
 */
void instr_inc_ebx(uint8_t *code, uint32_t *stack){
   ebx++;
}

/**
 * OPCODE 0x44
 */
void instr_inc_esp(uint8_t *code, uint32_t *stack){
   esp++;
}

/**
 * OPCODE 0x45
 */
void instr_inc_ebp(uint8_t *code, uint32_t *stack){
   ebp++;
}

/**
 * OPCODE 0x46
 */
void instr_inc_esi(uint8_t *code, uint32_t *stack){
   esi++;
}

/**
 * OPCODE 0x47
 */
void instr_inc_edi(uint8_t *code, uint32_t *stack){
   edi++;
}

/**
 * OPCODE 0x48
 */
void instr_dec_eax(uint8_t *code, uint32_t *stack){
   eax--;
}

/**
 * OPCODE 0x49
 */
void instr_dec_ecx(uint8_t *code, uint32_t *stack){
   ecx--;
}

/**
 * OPCODE 0x4A
 */
void instr_dec_edx(uint8_t *code, uint32_t *stack){
   edx--;
}

/**
 * OPCODE 0x4B
 */
void instr_dec_ebx(uint8_t *code, uint32_t *stack){
   ebx--;
}

/**
 * OPCODE 0x4C
 */
void instr_dec_esp(uint8_t *code, uint32_t *stack){
   esp--;
}

/**
 * OPCODE 0x4D
 */
void instr_dec_ebp(uint8_t *code, uint32_t *stack){
   ebp--;
}

/**
 * OPCODE 0x4E
 */
void instr_dec_esi(uint8_t *code, uint32_t *stack){
   esi--;
}

/**
 * OPCODE 0x4F
 */
void instr_dec_edi(uint8_t *code, uint32_t *stack){
   edi--;
}

/**
 * OPCODE 0x50
 */
void instr_push_eax(uint8_t *code, uint32_t *stack) {
   stack[esp++]=eax;}

/**
 * OPCODE 0x51
 */
void instr_push_ecx(uint8_t *code, uint32_t *stack) {
   stack[esp++]=ecx;
}

/**
 * OPCODE 0x52
 */
void instr_push_edx(uint8_t *code, uint32_t *stack) {
   stack[esp++]=edx;
}

/**
 * OPCODE 0x53
 */
void instr_push_ebx(uint8_t *code, uint32_t *stack) {
   stack[esp++]=ebx;
}

/**
 * OPCODE 0x54
 */
void instr_push_esp(uint8_t *code, uint32_t *stack) {
   stack[esp]=esp;
   esp++;
}

/**
 * OPCODE 0x55
 */
void instr_push_ebp(uint8_t *code, uint32_t *stack) {
   stack[esp++]=ebp;
}

/**
 * OPCODE 0x56
 */
void instr_push_esi(uint8_t *code, uint32_t *stack) {
   stack[esp++]=esi;
}

/**
 * OPCODE 0x57
 */
void instr_push_edi(uint8_t *code, uint32_t *stack) {
   stack[esp++]=edi;
}

/**
 * OPCODE 0x58
 */
void instr_pop_eax(uint8_t *code, uint32_t *stack) {
   eax = stack[--esp];
}

/**
 * OPCODE 0x59
 */
void instr_pop_ecx(uint8_t *code, uint32_t *stack) {
   ecx = stack[--esp];
}

/**
 * OPCODE 0x5A
 */
void instr_pop_edx(uint8_t *code, uint32_t *stack) {
   edx = stack[--esp];
}

/**
 * OPCODE 0x5B
 */
void instr_pop_ebx(uint8_t *code, uint32_t *stack) {
   ebx = stack[--esp];
}

/**
 * OPCODE 0x5C
 */
void instr_pop_esp(uint8_t *code, uint32_t *stack) {
   --esp;
   esp = stack[esp];
}

/**
 * OPCODE 0x5D
 */
void instr_pop_ebp(uint8_t *code, uint32_t *stack) {
   ebp = stack[--esp];
}

/**
 * OPCODE 0x5E
 */
void instr_pop_esi(uint8_t *code, uint32_t *stack) {
   esi = stack[--esp];
}

/**
 * OPCODE 0x5F
 */
void instr_pop_edi(uint8_t *code, uint32_t *stack) {
   edi = stack[--esp];
}

/**
 * OPCODE 0x63
 */
void instr_arpl(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+1);
   uint16_t * rm16 = (uint16_t *)regs[(modrm & 0b00000111)];
   uint16_t * r16 = (uint16_t *)regs[(modrm & 0b00111000)];
   uint16_t v1,v2;
   uint16_t * f = (uint16_t *)&eflags;
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
void opcode80(uint8_t *code, uint32_t *stack){

}

/**
 * OPCODE 0x80 /0
 * 
 * Debe tener un valor de 000 (0) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
void instr_add_r8_imm8(uint8_t *code, uint32_t *stack){
   uint8_t *r8 = (uint8_t*)regs8[((*(code+1)) & 0x7)];
   uint8_t v = *(code+2);

   *r8 += v;
   //flags?
}

/**
 * OPCODE 0x80 /2
 * 
 * Debe tener un valor de 010 (2) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
void instr_adc_r8_imm8(uint8_t *code, uint32_t *stack){
   uint16_t v = *(code+1) ; //byte ModR/M
   uint16_t * f = (uint16_t *)&eflags;
   v&=0b00000111;
   uint8_t * reg = (uint8_t *) regs8[v];
   *reg = *reg + *(code+2) + (*f & 0x1);
}

/**
 * OPCODE 0x80 /4
 * 
 * Debe tener un valor de 100 (4) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
void instr_and_r8_imm8(uint8_t *code, uint32_t *stack){
   uint8_t * r8 = (uint8_t*)regs8[((*(code+1)) & 0x7)];
   uint8_t v = *(code+2);

   *r8 &= v;
}

/**
 * OPCODE 0x80 /7
 * 
 * Debe tener un valor de 111 (7) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
void instr_cmp_r8_imm8(uint8_t *code, uint32_t *stack){
   
}

/**
 * OPCODE 0x81
 * 
 * Includes all the 0x81 instructions, and calls the one needed based on the ModR/M byte
 */
void opcode81(uint8_t *code, uint32_t *stack){

}

/**
 * OPCODE 0x81 /0
 * 
 * Debe tener un valor de 000 (0) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
void instr_add_r16_32_imm16_32(uint8_t *code, uint32_t *stack){
   uint32_t * r32 = regs[((*(code+1))&0x7)];
   uint32_t v;
   uint8_t * vv = (uint8_t *)&v;

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
void instr_adc_r16_32_imm16_32(uint8_t *code, uint32_t *stack){
   uint16_t b = *(code+1) ; //byte ModR/M
   uint16_t * f = (uint16_t *)&eflags;
   b&=0b00000111;
   uint32_t * reg = regs[b];

   uint32_t v; //valor inmediato
   uint8_t * vv = (uint8_t *)&v; //puntero a byte que recorre el valor inmediato para rellenarlo desde el bytecode

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
void instr_and_r16_32_imm16_32(uint8_t *code, uint32_t *stack){
   uint32_t * r32 = regs[((*code+1) & 0x7)];
   uint32_t v; //valor inmediato
   uint8_t * vv = (uint8_t *)&v; //puntero a byte que recorre el valor inmediato para rellenarlo desde el bytecode

   *vv = *(code+2);vv++;
   *vv = *(code+3);vv++;
   *vv = *(code+4);vv++;
   *vv = *(code+5);

   *r32 &= v;
}

/**
 * OPCODE 0x81 /7
 * 
 * Debe tener un valor de 111 (7) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
void instr_cmp_r16_32_imm16_32(uint8_t *code, uint32_t *stack){
   
}

/**
 * OPCODE 0x83
 * 
 * Includes all the 0x83 instructions, and calls the one needed based on the ModR/M byte
 */
void opcode83(uint8_t *code, uint32_t *stack){

}

/**
 * OPCODE 0x83 /0
 * 
 * Debe tener un valor de 000 (0) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
void instr_add_r16_32_imm8(uint8_t *code, uint32_t *stack){
   uint32_t * r32 = regs[((*(code+1))&0x7)];
   uint8_t v = *(code+2);

   *r32 += v;
}

/**
 * OPCODE 0x83 /2
 * 
 * Debe tener un valor de 010 (2) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
void instr_adc_r16_32_imm8(uint8_t *code, uint32_t *stack){
   uint16_t v = *(code+1) ; //byte ModR/M
   uint16_t * f = (uint16_t *)&eflags;
   v&=0b00000111;
   uint32_t * reg = regs[v];
   *reg = *reg + *(code+2) + (*f & 0x1);
}

/**
 * OPCODE 0x83 /4
 */
void instr_and_r16_32_imm8(uint8_t *code, uint32_t *stack){
   uint32_t * r32 = regs[((*(code+1)) & 0x7)];
   uint8_t v = *(code+2);

   *r32 &= v;
}

/**
 * OPCODE 0x83 /7
 * 
 * Debe tener un valor de 111 (7) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
void instr_cmp_r16_32_imm8(uint8_t *code, uint32_t *stack){

}



//------

/**
 * OPCODE 0x90
 */
void instr_nop(uint8_t *code, uint32_t *stack){
   
}
/**
 * OPCODE 0x91
 */
void instr_xchg_ecx_eax(uint8_t *code, uint32_t *stack){
   uint32_t a;
   a = eax;
   eax = ecx;
   ecx = a;
}

/**
 * OPCODE 0x98
 */
void instr_cwde(uint8_t *code, uint32_t *stack){
   uint16_t *ax = (uint16_t *) &eax;
   uint8_t bit = (*ax >> 15) & 1;
   uint16_t v = bit ? 0xFFFF : 0x0000;
   ax++;
   *ax = v; 
}

/**
 * OPCODE 0x99 //cwd same but with 16b operand
 */
 void instr_cdq(uint8_t *code, uint32_t *stack){
   
   uint8_t bit = (eax >> 31) & 1;
   uint32_t v = bit ? 0xFFFFFFFF : 0x00000000;
   edx = v; 
}

/**
 * OPCODE 0xF5
 */
void instr_cmc(uint8_t *code, uint32_t *stack){
   uint16_t * f = (uint16_t *)&eflags;
   *f ^= 0x1;
}

/**
 * OPCODE 0xF8
 */
void instr_clc(uint8_t *code, uint32_t *stack){
   uint16_t * f = (uint16_t *)&eflags;
   *f &=0xFFFE; 
}

/**
 * OPCODE 0xFA
 */
void instr_cli(uint8_t *code, uint32_t *stack){
   uint16_t * f = (uint16_t *)&eflags;
   *f&=0xFDFF;
}

/**
 * OPCODE 0xFC
 */
void instr_cld(uint8_t *code, uint32_t *stack){
   uint16_t * f= (uint16_t *)&eflags;
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
   uint32_t * stack = (uint32_t *)malloc(135168 * sizeof(uint8_t));

   
   eax = 0xFF;
   printf("eax : 0x%8x\n", eax);
   uint8_t code[6] ={ 0x81, 0xD0, 0xFF, 0x00, 0x00, 0x00};
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
