#include <stdint.h>
#include <stdlib.h>
#include "instructions.h"
#include "flags.h"

uint8_t * mem;
uint32_t eax, edx, esp, esi, eip, cs, ds, fs, ecx, ebx, ebp, edi, ss, es, gs = 0; 
extern uint32_t eflags;
uint32_t * regs[8] = {&eax, &ecx, &edx, &ebx, &esp, &ebp, &esi, &edi};
uint8_t * regs8[8] = {(uint8_t *)&eax, (uint8_t *)&ecx, (uint8_t *)&edx, (uint8_t *)&ebx, ((uint8_t *)&eax)+1, ((uint8_t *)&ecx)+1, ((uint8_t *)&edx)+1, ((uint8_t *)&ebx)+1};

void initialize(){
   mem = (uint8_t *)malloc(UINT32_MAX * sizeof(uint8_t)); //4GB de memoria del i386 (32 bits)
}

//#############################################################
//##########         INSTRUCTION FUNCTIONS           ##########
//#############################################################

/**
 * OPCODE 0x00
 */
int add_rm8_r8(uint8_t *mem){
   uint8_t modrm = *(mem+eip+1);
   uint8_t  * rm8 = (uint8_t *)regs8[(modrm & 0b00000111)];
   uint8_t  * r8 = (uint8_t *)regs8[(modrm & 0b00111000)];
   *rm8 += *r8;
   //flags
   return 0;
}

/**
 * OPCODE 0x01
 */
int add_rm16_32_r16_32(uint8_t *mem){
   uint8_t modrm = *(mem+eip+1);
   uint32_t * rm32 = regs[(modrm & 0b00000111)];
   uint32_t * r32 = regs[(modrm & 0b00111000)];
   *rm32 += *r32;
   //flags
   return 0;
}

/**
 * OPCODE 0x02
 */
int add_r8_rm8(uint8_t *mem){
   uint8_t modrm = *(mem+eip+1);
   uint8_t  * rm8 = (uint8_t *)regs8[(modrm & 0b00000111)];
   uint8_t  * r8 = (uint8_t *)regs8[(modrm & 0b00111000)];
   *r8 += *rm8;
   //flags
   return 0;
}

/**
 * OPCODE 0x03
 */
int add_r16_32_rm16_32(uint8_t *mem){
   uint8_t modrm = *(mem+eip+1);
   uint32_t * rm32 = regs[(modrm & 0b00000111)];
   uint32_t * r32 = regs[(modrm & 0b00111000)];
   *r32+=*rm32;
   //flags
   return 0;
}


/**
 * OPCODE 0x04   
 */
int add_imm8(uint8_t *mem){
   uint8_t * al = (uint8_t *)&eax;
   *al += *(mem+eip+1);
   //flags
   return 0;
}

/**
 * OPCODE 0x05
 */
int add_imm16_32(uint8_t *mem){
   uint32_t v;
   uint8_t * vv = (uint8_t *)&v ;
   *vv = *(mem+eip+1);vv++;
   *vv = *(mem+eip+2);vv++;
   *vv = *(mem+eip+3);vv++;
   *vv = *(mem+eip+4);
   eax += v;
   //flags
   return 0;
}

/**
 * OPCODE 0x06
 */
int push_es(uint8_t *mem){
    *((uint16_t *)(mem+esp))=es;
    esp-=2;
}

/**
 * OPCODE 0x07
 */
int pop_es(uint8_t *mem){
    es = *((uint16_t *)(mem+esp));
    esp+=2;
}

/**
 * OPCODE 0x08
 */
int or_rm8_r8(uint8_t *mem){
   uint8_t op1, op2;
   parse_ops_1B(&op1, &op2);
   
   clear_Flag(OF);
   clear_Flag(ZF);
}

/**
 * OPCODE 0x09
 */
int or_rm32_r32 (uint8_t *mem){

}

/**
 * OPCODE 0x0A
 */
int or_r8_rm8(uint8_t *mem){

}

/**
 * OPCODE 0x0B
 */
int or_r32_rm32(){

}

int opcode0F(uint8_t *mem){

}

/**
 * OPCODE 0x0F A3
 */
int bt_r(uint8_t *mem){
   uint8_t modrm = *(mem+eip+2);
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
int bts_r(uint8_t *mem){
   uint8_t modrm = *(mem+eip+2);
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
int btr_r(uint8_t *mem){
   uint8_t modrm = *(mem+eip+2);
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
int bt_imm8(uint8_t *mem){
   uint8_t modrm = *(mem+eip+2);
   uint16_t * f = (uint16_t *)&eflags;
   uint32_t *rm32 = regs[(modrm & 0b00000111)];
   uint8_t nbit = *(mem+eip+3);

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
int bts_imm8(uint8_t *mem){
   uint8_t modrm = *(mem+eip+2);
   uint16_t * f = (uint16_t *)&eflags;
   uint32_t *rm32 = regs[(modrm & 0b00000111)];
   uint8_t nbit = *(mem+eip+3);

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
int btr_imm8(uint8_t *mem){
   uint8_t modrm = *(mem+eip+2);
   uint16_t * f = (uint16_t *)&eflags;
   uint32_t *rm32 = regs[(modrm & 0b00000111)];
   uint8_t nbit = *(mem+eip+3);

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
int btc_imm8(uint8_t *mem){
   uint8_t modrm = *(mem+eip+2);
   uint16_t * f = (uint16_t *)&eflags;
   uint32_t *rm32 = regs[(modrm & 0b00000111)];
   uint8_t nbit = *(mem+eip+3);

   uint16_t bin = 0x01;
   while(nbit){ //genera el binario en bin para enmascarar el bit a poner en el CF
      nbit--;
      bin+=bin;
   }
   uint16_t v = *rm32 & bin;
   if(v){
      *f|= 0x1;
      *rm32 ^= (1 << *(mem+eip+3));
   }else{
      *f &= 0xFFFE;
      *rm32 ^= (1 << *(mem+eip+3));
   }
}

/**
 * OPCODE 0x0F BB
 */
int btc_r(uint8_t *mem){
   uint8_t modrm = *(mem+eip+2);
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
int bsf(uint8_t *mem){
   uint8_t modrm = *(mem+eip+2);
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
int bsr(uint8_t *mem){
   uint8_t modrm = *(mem+eip+2);
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
int adc_ib_al(uint8_t *mem){
   uint8_t * al = (uint8_t *)&eax;
   uint8_t b = *(mem+eip+1);
   uint16_t * f = (uint16_t *)&eflags;
   *al= *al+b+( *f & 0x1);
}

/**
 * OPCODE 0x15
 */
int adc_iw_id(uint8_t *mem){
   uint32_t v;
   uint8_t * vv = (uint8_t *)&v;
   uint16_t * f = (uint16_t *)&eflags;

   *vv = *(mem+eip+1);vv++;
   *vv = *(mem+eip+2);vv++;
   *vv = *(mem+eip+3);vv++;
   *vv = *(mem+eip+4);

   eax+= (v+ (*f &0x1));
}

/**
 * OPCODE 0x20
 */
int and_rm8_r8(uint8_t *mem){
   uint8_t modrm = *(mem+eip+1);
   uint8_t  * rm8 = (uint8_t *)regs8[(modrm & 0b00000111)];
   uint8_t  * r8 = (uint8_t *)regs8[(modrm & 0b00111000)];
   *rm8 &= *r8;
   uint16_t * f= (uint16_t *)&eflags;
   *f&=0xF7BF;
}

/**
 * OPCODE 0x21
 */
int and_rm16_32_r16_32(uint8_t *mem){
   uint8_t modrm = *(mem+eip+1);
   uint32_t * rm32 = regs[(modrm & 0b00000111)];
   uint32_t * r32 = regs[(modrm & 0b00111000)];
   *rm32 &= *r32;
   uint16_t * f = (uint16_t *)&eflags;
   *f&=0xF7BF;
}

/**
 * OPCODE 0x22
 */
int and_r8_rm8(uint8_t *mem){
   uint8_t modrm = *(mem+eip+1);
   uint8_t  * rm8 = (uint8_t *)regs8[(modrm & 0b00000111)];
   uint8_t  * r8 = (uint8_t *)regs8[(modrm & 0b00111000)];
   *r8 &= *rm8;
   uint16_t * f = (uint16_t *)&eflags;
   *f&=0xF7BF;
}

/**
 * OPCODE 0x23
 */
int and_r16_32_rm16_32(uint8_t *mem){
   uint8_t modrm = *(mem+eip+1);
   uint32_t * rm32 = regs[(modrm & 0b00000111)];
   uint32_t * r32 = regs[(modrm & 0b00111000)];
   *r32 &= *rm32;
   uint16_t * f = (uint16_t *)&eflags;
   *f&=0xF7BF;
}

/**
 * OPCODE 0x24
 */
int and_imm8(uint8_t *mem){
   uint8_t * al = (uint8_t *)&eax;
   uint8_t v = *(mem+eip+1);
   *al &= v;
   uint16_t * f = (uint16_t *)&eflags;
   *f&=0xF7BF;
}

/**
 * OPCODE 0x25
 */
int and_imm16_32(uint8_t *mem){
   uint32_t v;
   uint8_t * vv = (uint8_t *)&v;

   *vv=*(mem+eip+1);vv++;
   *vv=*(mem+eip+2);vv++;
   *vv=*(mem+eip+3);vv++;
   *vv=*(mem+eip+4);

   eax &= v;
   uint16_t * f = (uint16_t *)&eflags;
   *f&=0xF7BF;
}

/**
 * OPCODE 0x27
 */
int daa(uint8_t *mem){
   uint8_t *al = (uint8_t *)&eax;
   if((*al & 0xF) > 9 || (eflags >> 4) & 1){
      *al+=6;
      set_Flag(AF);
   }else{
      clear_Flag(AF);
   }
   if(*al > 0x9F || eflags & 0x1){
      *al += 0x60;
      set_Flag(CF);
   }else{
      clear_Flag(CF);
   }
   (*al >> 7) & 1? (set_Flag(SF)):(clear_Flag(SF));
   (!*al)?(set_Flag(ZF)):(clear_Flag(ZF));
   parity(*al)?(set_Flag(PF)):(clear_Flag(PF));
}

/**
 * OPCODE 0x2F
 */
int das(uint8_t *mem){
   uint8_t *al = (uint8_t *)&eax;
   if((*al & 0xF) > 9 || (eflags >> 4) & 1){
      *al-=6;
      set_Flag(AF);
   }else{
      clear_Flag(AF);
   }
   if(*al > 0x9F || eflags & 0x1){
      *al -= 0x60;
      set_Flag(CF);
   }else{
      clear_Flag(CF);
   }
   (*al >> 7) & 1? (set_Flag(SF)):(clear_Flag(SF));
   (!*al)?(set_Flag(ZF)):(clear_Flag(ZF));
   parity(*al)?(set_Flag(PF)):(clear_Flag(PF));
}

/**
 * OPCODE 0xD5 0A
 */
int aad(uint8_t *mem){
   uint8_t * al = (uint8_t *)&eax;
   uint8_t * ah = al+1;
   *al = *ah * 10 + *al;
   *ah = 0;
   //flags? SF PF ZF
}

/**
 * OPCODE 0xD4 0A
 */
int aam(uint8_t *mem){
   uint8_t * al = (uint8_t *)&eax;
   uint8_t * ah = al+1;
   *ah = *al / 10;
   *al %= 10;
   //flags?
}

/**
 * OPCODE 0x37
 */
int aaa(uint8_t *mem){
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
int cmp_rm8_r8(uint8_t *mem){
   //rm8 -r8 
   uint8_t modrm = *(mem+eip+1);
   uint8_t * rm8 = regs8[modrm  & 0x7];
   uint8_t * r8 = regs8[modrm  & 0x38];
   uint8_t res = *rm8 - *r8;
   (*r8 > *rm8)? (set_Flag(CF)): (clear_Flag(CF));//activar CF
   (parity(res))? (set_Flag(PF)): (clear_Flag(PF));//activar PF
   ((*rm8 ^ *r8 ^ res) & 0x10)? (set_Flag(AF)): (clear_Flag(AF));//activar AF
   (!res)?(set_Flag(ZF)): (clear_Flag(ZF));//activar ZF
   (res >> 7) ? (set_Flag(SF)): (clear_Flag(SF)); //activar SF
   (((*rm8 ^ *r8) & 0x80) && ((*rm8 ^ res) & 0x80))? (set_Flag(OF)): (clear_Flag(OF)) ;//activa OF

   /**
    * 
    * 
    * (*r8 > *rm8)? (eflags|=0x1): (eflags&=0xFFFFFFFE);//activar CF
   (parity(res))? (eflags|=0x4): (eflags&=0xFFFFFFFB);//activar PF
   ((*rm8 ^ *r8 ^ res) & 0x10)? (eflags|=0x10): (eflags&=0xFFFFFFEF);//activar AF
   (!res)? (eflags|=0x40): (eflags&=0xFFFFFFBF);//activar ZF
   (res >> 7) ? (eflags|=0x80) : (eflags&=0xFFFFFF7F); //activar SF
   (((*rm8 ^ *r8) & 0x80) && ((*rm8 ^ res) & 0x80))? (eflags|=0x800): (eflags&=0xFFFFF7FF) ;//activa OF
    */
}

/**
 * OPCODE 0x39
 */
int cmp_rm16_32_r16_32(uint8_t *mem){
   //rm32-r32
   uint8_t modrm = *(mem+eip+1);
   uint32_t *rm32 = regs[modrm & 0x7];
   uint32_t *r32 = regs[modrm & 0x38];
   uint32_t res = *rm32 - *r32;
   (*r32 > *rm32)? (set_Flag(CF)): (clear_Flag(CF));//activar CF
   uint8_t * res8 = (uint8_t *)&res;
   (parity(*res8))? (set_Flag(PF)): (clear_Flag(PF));//activar PF
   ((*rm32 ^ *r32 ^ res) & 0x10)? (set_Flag(AF)): (clear_Flag(AF));//activar AF
   (!res)?(set_Flag(ZF)): (clear_Flag(ZF));//activar ZF
   (res >> 31) ? (set_Flag(SF)): (clear_Flag(SF)); //activar SF
   ((((*rm32 ^ *r32) & 0x80) && ((*rm32 ^ res) & 0x80)))? (set_Flag(OF)): (clear_Flag(OF)) ;//activa OF
}

/**
 * OPCODE 0x3A
 */
int cmp_r8_rm8(uint8_t *mem){
   //r8 -rm8
   uint8_t modrm = *(mem+eip+1);
   uint8_t * rm8 = regs8[modrm  & 0x7];
   uint8_t * r8 = regs8[modrm  & 0x38];
   uint8_t res = *r8 - *rm8;
   (*rm8 > *r8)? (set_Flag(CF)): (clear_Flag(CF));//activar CF
   (parity(res))? (set_Flag(PF)): (clear_Flag(PF));//activar PF
   ((*r8 ^ *rm8 ^ res) & 0x10)? (set_Flag(AF)): (clear_Flag(AF));//activar AF
   (!res)?(set_Flag(ZF)): (clear_Flag(ZF));//activar ZF
   (res >> 7) ? (set_Flag(SF)): (clear_Flag(SF)); //activar SF
   (((*r8 ^ *rm8) & 0x80) && ((*r8 ^ res) & 0x80))? (set_Flag(OF)): (clear_Flag(OF)) ;//activa OF
}

/**
 * OPCODE 0x3B
 */
int cmp_r16_32_rm16_32(uint8_t *mem){
   //r32-rm32
   uint8_t modrm = *(mem+eip+1);
   uint32_t *rm32 = regs[modrm & 0x7];
   uint32_t *r32 = regs[modrm & 0x38];
   uint32_t res = *r32 - *rm32;
   (*rm32 > *r32)? (set_Flag(CF)): (clear_Flag(CF));//activar CF
   uint8_t * res8 = (uint8_t *)&res;
   (parity(*res8))? (set_Flag(PF)): (clear_Flag(PF));//activar PF
   ((*r32 ^ *rm32 ^ res) & 0x10)? (set_Flag(AF)): (clear_Flag(AF));//activar AF
   (!res)?(set_Flag(ZF)): (clear_Flag(ZF));//activar ZF
   (res >> 31) ? (set_Flag(SF)): (clear_Flag(SF)); //activar SF
   ((((*r32 ^ *rm32) & 0x80) && ((*r32 ^ res) & 0x80)))? (set_Flag(OF)): (clear_Flag(OF)) ;//activa OF
}

/**
 * OPCODE 0x3C
 */
int cmp_al_imm8(uint8_t *mem){
   //al -imm8
   uint8_t v = *(mem+eip+1);
   uint8_t *al = (uint8_t *)&eax;
   
   uint8_t res = *al - v;
   (v > *al)? (set_Flag(CF)): (clear_Flag(CF));//activar CF
   (parity(res))? (set_Flag(PF)): (clear_Flag(PF));//activar PF
   ((*al ^ v ^ res) & 0x10)? (set_Flag(AF)): (clear_Flag(AF));//activar AF
   (!res)?(set_Flag(ZF)): (clear_Flag(ZF));//activar ZF
   (res >> 7) ? (set_Flag(SF)): (clear_Flag(SF)); //activar SF
   (((*al ^ v) & 0x80) && ((*al ^ res) & 0x80))? (set_Flag(OF)): (clear_Flag(OF)) ;//activa OF
}

/**
 * OPCODE 0x3D
 */
int cmp_eax_imm32(uint8_t *mem){
   uint32_t v;
   uint8_t *p = (uint8_t *)&v;
   p[0]=*(mem+eip+1);
   p[1]=*(mem+eip+2);
   p[2]=*(mem+eip+3);
   p[3]=*(mem+eip+4);
   
   uint32_t res = eax - v;
   (v > eax)? (set_Flag(CF)): (clear_Flag(CF));//activar CF
   uint8_t * res8 = (uint8_t *)&res;
   (parity(*res8))? (set_Flag(PF)): (clear_Flag(PF));//activar PF
   ((eax ^ v ^ res) & 0x10)? (set_Flag(AF)): (clear_Flag(AF));//activar AF
   (!res)?(set_Flag(ZF)): (clear_Flag(ZF));//activar ZF
   (res >> 31) ? (set_Flag(SF)): (clear_Flag(SF)); //activar SF
   ((((eax ^ v) & 0x80) && ((eax ^ res) & 0x80)))? (set_Flag(OF)): (clear_Flag(OF)) ;//activa OF
}

/**
 * OPCODE 0x3F
 */
int aas(uint8_t *mem){
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
int inc_eax(uint8_t *mem){
   eax++;
}

/**
 * OPCODE 0x41
 */
int inc_ecx(uint8_t *mem){
   ecx++;
}

/**
 * OPCODE 0x42
 */
int inc_edx(uint8_t *mem){
   edx++;
}

/**
 * OPCODE 0x43
 */
int inc_ebx(uint8_t *mem){
   ebx++;
}

/**
 * OPCODE 0x44
 */
int inc_esp(uint8_t *mem){
   esp++;
}

/**
 * OPCODE 0x45
 */
int inc_ebp(uint8_t *mem){
   ebp++;
}

/**
 * OPCODE 0x46
 */
int inc_esi(uint8_t *mem){
   esi++;
}

/**
 * OPCODE 0x47
 */
int inc_edi(uint8_t *mem){
   edi++;
}

/**
 * OPCODE 0x48
 */
int dec_eax(uint8_t *mem){
   eax--;
}

/**
 * OPCODE 0x49
 */
int dec_ecx(uint8_t *mem){
   ecx--;
}

/**
 * OPCODE 0x4A
 */
int dec_edx(uint8_t *mem){
   edx--;
}

/**
 * OPCODE 0x4B
 */
int dec_ebx(uint8_t *mem){
   ebx--;
}

/**
 * OPCODE 0x4C
 */
int dec_esp(uint8_t *mem){
   esp--;
}

/**
 * OPCODE 0x4D
 */
int dec_ebp(uint8_t *mem){
   ebp--;
}

/**
 * OPCODE 0x4E
 */
int dec_esi(uint8_t *mem){
   esi--;
}

/**
 * OPCODE 0x4F
 */
int dec_edi(uint8_t *mem){
   edi--;
}

/**
 * OPCODE 0x50
 */
int push_eax(uint8_t *mem) {
   *((uint32_t *)(mem+esp))=eax;
   esp-=4;
}

/**
 * OPCODE 0x51
 */
int push_ecx(uint8_t *mem) {
   *((uint32_t *)(mem+esp))=ecx;
   esp-=4;
}

/**
 * OPCODE 0x52
 */
int push_edx(uint8_t *mem) {
   *((uint32_t *)(mem+esp))=edx;
   esp-=4;
}

/**
 * OPCODE 0x53
 */
int push_ebx(uint8_t *mem) {
   *((uint32_t *)(mem+esp))=ebx;
   esp-=4;
}

/**
 * OPCODE 0x54
 */
int push_esp(uint8_t *mem) {
   *((uint32_t *)(mem+esp))=esp;
   esp-=4;
}

/**
 * OPCODE 0x55
 */
int push_ebp(uint8_t *mem) {
   *((uint32_t *)(mem+esp))=ebp;
   esp-=4;
}

/**
 * OPCODE 0x56
 */
int push_esi(uint8_t *mem) {
   *((uint32_t *)(mem+esp))=esi;
   esp-=4;
}

/**
 * OPCODE 0x57
 */
int push_edi(uint8_t *mem) {
   *((uint32_t *)(mem+esp))=edi;
   esp-=4;
}

/**
 * OPCODE 0x58
 */
int pop_eax(uint8_t *mem) {
   eax = *((uint32_t *)(mem+esp));
   esp +=4;
}

/**
 * OPCODE 0x59
 */
int pop_ecx(uint8_t *mem) {
   ecx = *((uint32_t *)(mem+esp));
   esp+=4;
}

/**
 * OPCODE 0x5A
 */
int pop_edx(uint8_t *mem) {
   edx = *((uint32_t *)(mem+esp));
   esp+=4;

}

/**
 * OPCODE 0x5B
 */
int pop_ebx(uint8_t *mem) {
   ebx = *((uint32_t *)(mem+esp));
   esp+=4;
}

/**
 * OPCODE 0x5C
 */
int pop_esp(uint8_t *mem) {
   es = *((uint32_t *)(mem+esp));
}

/**
 * OPCODE 0x5D
 */
int pop_ebp(uint8_t *mem) {
   ebp = *((uint32_t *)(mem+esp));
   esp+=4;
}

/**
 * OPCODE 0x5E
 */
int pop_esi(uint8_t *mem) {
   esi = *((uint32_t *)(mem+esp));
   esp+=4;
}

/**
 * OPCODE 0x5F
 */
int pop_edi(uint8_t *mem) {
   edi = *((uint32_t *)(mem+esp));
   esp+=4;
}

/**
 * OPCODE 0x63
 */
int arpl(uint8_t *mem){
   uint8_t modrm = *(mem+eip+1);
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

/**
 * OPCODE 0x68
 */
int push_imm32(uint8_t *mem){
   uint32_t v;
   uint8_t * p = (uint8_t *)&v;
   for (int i=0;i<4;i++){
      *p=*(mem+ (++eip));
      p++;
   }
   *((uint32_t *) (mem+esp))= v;
   esp-=4;
}

//------

/**
 * OPCODE 0x80
 * 
 * Includes all the 0x80 instructions, and calls the one needed based on the ModR/M byte
 */
int opcode80(uint8_t *mem){

}

/**
 * OPCODE 0x80 /0
 * 
 * Debe tener un valor de 000 (0) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int add_r8_imm8(uint8_t *mem){
   uint8_t *r8 = (uint8_t*)regs8[((*(mem+eip+1)) & 0x7)];
   uint8_t v = *(mem+eip+2);

   *r8 += v;
   //flags?
}

/**
 * OPCODE 0x80 /2
 * 
 * Debe tener un valor de 010 (2) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int adc_r8_imm8(uint8_t *mem){
   uint16_t v = *(mem+eip+1) ; //byte ModR/M
   uint16_t * f = (uint16_t *)&eflags;
   v&=0b00000111;
   uint8_t * reg = (uint8_t *) regs8[v];
   *reg = *reg + *(mem+eip+2) + (*f & 0x1);
}

/**
 * OPCODE 0x80 /4
 * 
 * Debe tener un valor de 100 (4) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int and_r8_imm8(uint8_t *mem){
   uint8_t * r8 = (uint8_t*)regs8[((*(mem+eip+1)) & 0x7)];
   uint8_t v = *(mem+eip+2);

   *r8 &= v;
}

/**
 * OPCODE 0x80 /7
 * 
 * Debe tener un valor de 111 (7) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int cmp_r8_imm8(uint8_t *mem){
   uint8_t modrm = *(mem+eip+1);
   uint8_t v = *(mem+eip+2);
   uint8_t *rm8 = (uint8_t *)regs8[modrm & 0x7];
   
   uint8_t res = *rm8 - v;
   (v > *rm8)? (set_Flag(CF)): (clear_Flag(CF));//activar CF
   (parity(res))? (set_Flag(PF)): (clear_Flag(PF));//activar PF
   ((*rm8 ^ v ^ res) & 0x10)? (set_Flag(AF)): (clear_Flag(AF));//activar AF
   (!res)?(set_Flag(ZF)): (clear_Flag(ZF));//activar ZF
   (res >> 7) ? (set_Flag(SF)): (clear_Flag(SF)); //activar SF
   (((*rm8 ^ v) & 0x80) && ((*rm8 ^ res) & 0x80))? (set_Flag(OF)): (clear_Flag(OF)) ;//activa OF
}

/**
 * OPCODE 0x81
 * 
 * Includes all the 0x81 instructions, and calls the one needed based on the ModR/M byte
 */
int opcode81(uint8_t *mem){

}

/**
 * OPCODE 0x81 /0
 * 
 * Debe tener un valor de 000 (0) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int add_r16_32_imm16_32(uint8_t *mem){
   uint32_t * r32 = regs[((*(mem+eip+1))&0x7)];
   uint32_t v;
   uint8_t * vv = (uint8_t *)&v;

   *vv=*(mem+eip+2);vv++;
   *vv=*(mem+eip+3);vv++;
   *vv=*(mem+eip+4);vv++;
   *vv=*(mem+eip+5);

   *r32 += v;
}

/**
 * OPCODE 0x81 /2
 * 
 * Debe tener un valor de 010 (2) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int adc_r16_32_imm16_32(uint8_t *mem){
   uint16_t b = *(mem+eip+1) ; //byte ModR/M
   uint16_t * f = (uint16_t *)&eflags;
   b&=0b00000111;
   uint32_t * reg = regs[b];

   uint32_t v; //valor inmediato
   uint8_t * vv = (uint8_t *)&v; //puntero a byte que recorre el valor inmediato para rellenarlo desde el bytecode

   *vv = *(mem+eip+2);vv++;
   *vv = *(mem+eip+3);vv++;
   *vv = *(mem+eip+4);vv++;
   *vv = *(mem+eip+5);

   *reg = *reg + v + ( *f & 0x1);
}

/**
 * OPCODE 0x81 /4
 * 
 * Debe tener un valor de 100 (4) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int and_r16_32_imm16_32(uint8_t *mem){
   uint32_t * r32 = regs[((*mem+eip+1) & 0x7)];
   uint32_t v; //valor inmediato
   uint8_t * vv = (uint8_t *)&v; //puntero a byte que recorre el valor inmediato para rellenarlo desde el bytecode

   *vv = *(mem+eip+2);vv++;
   *vv = *(mem+eip+3);vv++;
   *vv = *(mem+eip+4);vv++;
   *vv = *(mem+eip+5);

   *r32 &= v;
}

/**
 * OPCODE 0x81 /7
 * 
 * Debe tener un valor de 111 (7) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int cmp_r16_32_imm16_32(uint8_t *mem){
   uint32_t v;
   uint8_t modrm = *(mem+eip+1);
   uint32_t * rm32 = regs[modrm & 0x7];
   uint8_t *p = (uint8_t *)&v;
   p[0]=*(mem+eip+2);
   p[1]=*(mem+eip+3);
   p[2]=*(mem+eip+4);
   p[3]=*(mem+eip+5);
   
   uint32_t res = *rm32 - v;
   (v > *rm32)? (set_Flag(CF)): (clear_Flag(CF));//activar CF
   uint8_t * res8 = (uint8_t *)&res;
   (parity(*res8))? (set_Flag(PF)): (clear_Flag(PF));//activar PF
   ((*rm32 ^ v ^ res) & 0x10)? (set_Flag(AF)): (clear_Flag(AF));//activar AF
   (!res)?(set_Flag(ZF)): (clear_Flag(ZF));//activar ZF
   (res >> 31) ? (set_Flag(SF)): (clear_Flag(SF)); //activar SF
   ((((*rm32 ^ v) & 0x80) && ((*rm32 ^ res) & 0x80)))? (set_Flag(OF)): (clear_Flag(OF)) ;//activa OF
}

/**
 * OPCODE 0x83
 * 
 * Includes all the 0x83 instructions, and calls the one needed based on the ModR/M byte
 */
int opcode83(uint8_t *mem){

}

/**
 * OPCODE 0x83 /0
 * 
 * Debe tener un valor de 000 (0) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int add_r16_32_imm8(uint8_t *mem){
   uint32_t * r32 = regs[((*(mem+eip+1))&0x7)];
   uint8_t v = *(mem+eip+2);

   *r32 += v;
}

/**
 * OPCODE 0x83 /2
 * 
 * Debe tener un valor de 010 (2) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int adc_r16_32_imm8(uint8_t *mem){
   uint16_t v = *(mem+eip+1) ; //byte ModR/M
   uint16_t * f = (uint16_t *)&eflags;
   v&=0b00000111;
   uint32_t * reg = regs[v];
   *reg = *reg + *(mem+eip+2) + (*f & 0x1);
}

/**
 * OPCODE 0x83 /4
 */
int and_r16_32_imm8(uint8_t *mem){
   uint32_t * r32 = regs[((*(mem+eip+1)) & 0x7)];
   uint8_t v = *(mem+eip+2);

   *r32 &= v;
}

/**
 * OPCODE 0x83 /7
 * 
 * Debe tener un valor de 111 (7) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int cmp_r16_32_imm8(uint8_t *mem){
   uint8_t modrm = *(mem+eip+1);
   uint32_t * rm32 = regs[modrm & 0x7];
   uint8_t v = *(mem+eip+2);
   
   uint32_t res = *rm32 - v;
   (v > *rm32)? (set_Flag(CF)): (clear_Flag(CF));//activar CF
   uint8_t * res8 = (uint8_t *)&res;
   (parity(*res8))? (set_Flag(PF)): (clear_Flag(PF));//activar PF
   ((*rm32 ^ v ^ res) & 0x10)? (set_Flag(AF)): (clear_Flag(AF));//activar AF
   (!res)?(set_Flag(ZF)): (clear_Flag(ZF));//activar ZF
   (res >> 31) ? (set_Flag(SF)): (clear_Flag(SF)); //activar SF
   ((((*rm32 ^ v) & 0x80) && ((*rm32 ^ res) & 0x80)))? (set_Flag(OF)): (clear_Flag(OF)) ;//activa OF
}



//------

/**
 * OPCODE 0x90
 */
int nop(uint8_t *mem){
   
}
/**
 * OPCODE 0x91
 */
int xchg_ecx_eax(uint8_t *mem){
   uint32_t a;
   a = eax;
   eax = ecx;
   ecx = a;
}

/**
 * OPCODE 0x98
 */
int cwde(uint8_t *mem){
   uint16_t *ax = (uint16_t *) &eax;
   uint8_t bit = (*ax >> 15) & 1;
   uint16_t v = bit ? 0xFFFF : 0x0000;
   ax++;
   *ax = v; 
}

/**
 * OPCODE 0x99 //cwd same but with 16b operand
 */
int cdq(uint8_t *mem){
   
   uint8_t bit = (eax >> 31) & 1;
   uint32_t v = bit ? 0xFFFFFFFF : 0x00000000;
   edx = v; 
}

/**
 * OPCODE 0xC8
 */
int enter(uint8_t *mem){
   uint16_t operand;
   uint8_t level;
   uint8_t * p = (uint8_t *)&operand;
   p[0]=*(mem+eip+1);
   p[1]=*(mem+eip+2);
   level=*(mem+eip+3);

   level = level % 32;
   push_ebp(mem); //push ebp
   uint32_t fp = esp;
   if (level){
      for(int i=1; i< level; i++){
         ebp-=1;
         push_ebp(mem); //push ebp
      }
      *((uint32_t *)(mem+esp))= fp;
      esp-=4;
   }
   ebp = fp;
   esp -= operand;
}


/**
 * OPCODE 0xF5
 */
int cmc(uint8_t *mem){
   uint16_t * f = (uint16_t *)&eflags;
   *f ^= 0x1;
}

/**
 * OPCODE 0xF6
 */
int opcodeF6(uint8_t *mem){
   
}

/**
 * OPCODE 0xF6
 * 
 * Debe tener un valor de 110 (6) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int div_al_rm8(uint8_t *mem){
   //INTERRUPT 0?
   uint8_t modrm = *(mem+eip+1);
   uint8_t *rm8 = regs8[modrm & 0x7]; //divisor
   uint16_t *ax = (uint16_t *)&eax; //dividendo -> 
   uint16_t vold = *ax;
   //ah el resto
   uint8_t r = (uint8_t)(*ax / *rm8);
   uint8_t *al = (uint8_t *)ax;
   *al  = r;
   *(al+1)=vold % *rm8 ;//remainder to ah
   //flags undefined
} 

/**
 * OPCODE 0xF6
 * 
 * Debe tener un valor de 111 (7) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int idiv_al_rm8(uint8_t *mem){
   //INTERRUPT 0?
   uint8_t modrm = *(mem+eip+1);
   uint8_t *rm8 = regs8[modrm & 0x7]; //divisor
   uint16_t *ax = (uint16_t *)&eax; //dividendo -> 
   uint16_t vold = *ax;
   //ah el resto
   uint8_t r = (uint8_t)(*ax / *rm8);
   uint8_t *al = (uint8_t *)ax;
   *al  = r;
   *(al+1)=vold % *rm8 ;//remainder to ah
   //flags undefined
} 

/**
 * OPCODE 0xF7
 */
int opcodeF7(uint8_t *mem){
   
}

/**
 * OPCODE 0xF7
 * 
 * Debe tener un valor de 110 (6) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int div_eax_rm16_32(uint8_t *mem){
   //INTERRUPT 0?
   uint8_t modrm = *(mem+eip+1);
   uint32_t *rm32 = regs[modrm & 0x7];
   uint32_t vold = eax;
   eax /= *rm32;
   edx = vold % *rm32;
   //flags undefined
}
/**
 * OPCODE 0xF7
 * 
 * Debe tener un valor de 110 (7) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int idiv_eax_rm16_32(uint8_t *mem){
   //INTERRUPT 0?
   uint8_t modrm = *(mem+eip+1);
   uint32_t *rm32 = regs[modrm & 0x7];
   uint32_t vold = eax;
   eax /= *rm32;
   edx = vold % *rm32;
   //flags undefined
}

/**
 * OPCODE 0xF8
 */
int clc(uint8_t *mem){
   uint16_t * f = (uint16_t *)&eflags;
   *f &=0xFFFE; 
}

/**
 * OPCODE 0xFA
 */
int cli(uint8_t *mem){
   uint16_t * f = (uint16_t *)&eflags;
   *f&=0xFDFF;
}

/**
 * OPCODE 0xFC
 */
int cld(uint8_t *mem){
   uint16_t * f= (uint16_t *)&eflags;
   *f&=0xFBFF;
}

/**
 * OPCODE0xFE
 * 
 * Debe tener un valor de 001 (1) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int dec_rm8(uint8_t *mem){
   uint8_t modrm = *(mem+eip+1);
   uint8_t *rm8 = regs8[modrm & 0x7];
   uint8_t oldv = *rm8;
   *rm8-=1;


   (parity(*rm8))? (set_Flag(PF)): (clear_Flag(PF));//activar PF
   ((oldv ^ 0x1 ^ *rm8) & 0x10)? (set_Flag(AF)): (clear_Flag(AF));//activar AF
   (!*rm8)?(set_Flag(ZF)): (clear_Flag(ZF));//activar ZF
   (*rm8 >> 7) ? (set_Flag(SF)): (clear_Flag(SF)); //activar SF
   (((oldv ^ 0x1) & 0x80) && ((oldv ^ *rm8) & 0x80))? (set_Flag(OF)): (clear_Flag(OF)) ;//activa OF

}

/**
 * OPCODE 0xFF
 * 
 * Debe tener un valor de 001 (1) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int dec_rm16_32(uint8_t *mem){
   uint8_t modrm = *(mem+eip+1);
   uint32_t *rm32 = regs[modrm & 0x7];

   uint32_t oldv = *rm32;
   *rm32-=1;

   uint8_t * res8 = (uint8_t *)rm32;
   (parity(*res8))? (set_Flag(PF)): (clear_Flag(PF));//activar PF
   ((oldv ^ 0x1 ^ *rm32) & 0x10)? (set_Flag(AF)): (clear_Flag(AF));//activar AF
   (!*rm32)?(set_Flag(ZF)): (clear_Flag(ZF));//activar ZF
   (*rm32 >> 31) ? (set_Flag(SF)): (clear_Flag(SF)); //activar SF
   ((((oldv ^ 0x1) & 0x80) && ((oldv ^ *rm32) & 0x80)))? (set_Flag(OF)): (clear_Flag(OF)) ;//activa OF
}

void toBin(uint8_t c, uint8_t * b){
   uint8_t t = c;
   int i = 0;
   while (t > 0){
      b[i] = t % 2;
      t /= 2;
      i++;
   }
}

uint8_t parity(uint8_t num){
   int cont=0;
   for (int i=0; i<8;i++){
      cont += (num >> i) & 1;
   }
   return !(cont%2);
}

/**
 * Parses immediate 1-byte operand using EIP register.
 */
uint8_t parse_1B_imm_op(){
   return *(mem+(eip++));
}

/**
 * Parses immediate 4-byte operand using EIP register.
 */
uint32_t parse_4B_imm_op(){
   uint32_t v;
   uint8_t * p = (uint8_t *)&v;
   for (int i=0;i<4;i++){
      *p = *(mem+(eip++));
      p++;
   }
   return v;
}

void parse_ops_1B(uint8_t *op1, uint8_t *op2){
   uint8_t modrm = *(mem+(eip++));
   uint8_t dmode = (modrm >> 6);

   if (dmode == 0x3){      //0b11
      uint8_t r1 = modrm & 0b00000111;
      uint8_t r2 = (modrm & 0b00111000) >> 3;
      op1 = regs8[r1];
      op2 = regs8[r2];
   }else if (dmode == 0x2){//0b10
      printf("\n0x2");
   }else if (dmode == 0x1){//0b01
      printf("\n0x1");
   }else{                  //0b00
      uint8_t r1 = modrm & 0b00000111;
      uint8_t r2 = (modrm & 0b00111000) >> 3;
      op1 = *(mem + *(regs8[r1]));
      op2 = regs8[(modrm & 0b00111000)];
      printf("\n0x0");
   }

}