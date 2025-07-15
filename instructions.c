#include <stdint.h>
#include "instructions.h"
#include "flags.h"


uint32_t eax, edx, esp, esi, eip, cs, ds, fs, ecx, ebx, ebp, edi, ss, es, gs = 0; 
extern uint32_t eflags;
uint32_t * regs[8] = {&eax, &ecx, &edx, &ebx, &esp, &ebp, &esi, &edi};
uint8_t * regs8[8] = {(uint8_t *)&eax, (uint8_t *)&ecx, (uint8_t *)&edx, (uint8_t *)&ebx, ((uint8_t *)&eax)+1, ((uint8_t *)&ecx)+1, ((uint8_t *)&edx)+1, ((uint8_t *)&ebx)+1};


//#############################################################
//##########         INSTRUCTION FUNCTIONS           ##########
//#############################################################

/**
 * OPCODE 0x00
 */
int instr_add_rm8_r8(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+1);
   uint8_t  * rm8 = (uint8_t *)regs8[(modrm & 0b00000111)];
   uint8_t  * r8 = (uint8_t *)regs8[(modrm & 0b00111000)];
   *rm8 += *r8;
   //flags
   return 0;
}

/**
 * OPCODE 0x01
 */
int instr_add_rm16_32_r16_32(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+1);
   uint32_t * rm32 = regs[(modrm & 0b00000111)];
   uint32_t * r32 = regs[(modrm & 0b00111000)];
   *rm32 += *r32;
   //flags
   return 0;
}

/**
 * OPCODE 0x02
 */
int instr_add_r8_rm8(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+1);
   uint8_t  * rm8 = (uint8_t *)regs8[(modrm & 0b00000111)];
   uint8_t  * r8 = (uint8_t *)regs8[(modrm & 0b00111000)];
   *r8 += *rm8;
   //flags
   return 0;
}

/**
 * OPCODE 0x03
 */
int instr_add_r16_32_rm16_32(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+1);
   uint32_t * rm32 = regs[(modrm & 0b00000111)];
   uint32_t * r32 = regs[(modrm & 0b00111000)];
   *r32+=*rm32;
   //flags
   return 0;
}


/**
 * OPCODE 04   
 */
int instr_add_imm8(uint8_t *code, uint32_t *stack){
   uint8_t * al = (uint8_t *)&eax;
   *al += *(code+1);
   //flags
   return 0;
}

/**
 * OPCODE 05
 */
int instr_add_imm16_32(uint8_t *code, uint32_t *stack){
   uint32_t v;
   uint8_t * vv = (uint8_t *)&v ;
   *vv = *(code+1);vv++;
   *vv = *(code+2);vv++;
   *vv = *(code+3);vv++;
   *vv = *(code+4);
   eax += v;
   //flags
   return 0;
}

/**
 * OPCODE 0x0F A3
 */
int instr_bt_r(uint8_t *code, uint32_t *stack){
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
int instr_bts_r(uint8_t *code, uint32_t *stack){
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
int instr_btr_r(uint8_t *code, uint32_t *stack){
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
int instr_bt_imm8(uint8_t *code, uint32_t *stack){
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
int instr_bts_imm8(uint8_t *code, uint32_t *stack){
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
int instr_btr_imm8(uint8_t *code, uint32_t *stack){
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
int instr_btc_imm8(uint8_t *code, uint32_t *stack){
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
int instr_btc_r(uint8_t *code, uint32_t *stack){
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
int instr_bsf(uint8_t *code, uint32_t *stack){
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
int instr_bsr(uint8_t *code, uint32_t *stack){
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
int instr_adc_ib_al(uint8_t *code, uint32_t *stack){
   uint8_t * al = (uint8_t *)&eax;
   uint8_t b = *(code+1);
   uint16_t * f = (uint16_t *)&eflags;
   *al= *al+b+( *f & 0x1);
}

/**
 * OPCODE 0x15
 */
int instr_adc_iw_id(uint8_t *code, uint32_t *stack){
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
int instr_and_rm8_r8(uint8_t *code, uint32_t *stack){
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
int instr_and_rm16_32_r16_32(uint8_t *code, uint32_t *stack){
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
int instr_and_r8_rm8(uint8_t *code, uint32_t *stack){
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
int instr_and_r16_32_rm16_32(uint8_t *code, uint32_t *stack){
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
int instr_and_imm8(uint8_t *code, uint32_t *stack){
   uint8_t * al = (uint8_t *)&eax;
   uint8_t v = *(code+1);
   *al &= v;
   uint16_t * f = (uint16_t *)&eflags;
   *f&=0xF7BF;
}

/**
 * OPCODE 0x25
 */
int instr_and_imm16_32(uint8_t *code, uint32_t *stack){
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
 * OPCODE 0x27
 */
int instr_daa(uint8_t *code, uint32_t *stack){
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
int instr_das(uint8_t *code, uint32_t *stack){
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
int instr_aad(uint8_t *code, uint32_t *stack){
   uint8_t * al = (uint8_t *)&eax;
   uint8_t * ah = al+1;
   *al = *ah * 10 + *al;
   *ah = 0;
   //flags? SF PF ZF
}

/**
 * OPCODE 0xD4 0A
 */
int instr_aam(uint8_t *code, uint32_t *stack){
   uint8_t * al = (uint8_t *)&eax;
   uint8_t * ah = al+1;
   *ah = *al / 10;
   *al %= 10;
   //flags?
}

/**
 * OPCODE 0x37
 */
int instr_aaa(uint8_t *code, uint32_t *stack){
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
int instr_cmp_rm8_r8(uint8_t *code, uint32_t *stack){
   //rm8 -r8 
   uint8_t modrm = *(code+1);
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
int instr_cmp_rm16_32_r16_32(uint8_t *code, uint32_t *stack){
   //rm32-r32
   uint8_t modrm = *(code+1);
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
int instr_cmp_r8_rm8(uint8_t *code, uint32_t *stack){
   //r8 -rm8
   uint8_t modrm = *(code+1);
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
int instr_cmp_r16_32_rm16_32(uint8_t *code, uint32_t *stack){
   //r32-rm32
   uint8_t modrm = *(code+1);
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
int instr_cmp_al_imm8(uint8_t *code, uint32_t *stack){
   //al -imm8
   uint8_t v = *(code+1);
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
int instr_cmp_eax_imm32(uint8_t *code, uint32_t *stack){
   uint32_t v;
   uint8_t *p = (uint8_t *)&v;
   p[0]=*(code+1);
   p[1]=*(code+2);
   p[2]=*(code+3);
   p[3]=*(code+4);
   
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
int instr_aas(uint8_t *code, uint32_t *stack){
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
int instr_inc_eax(uint8_t *code, uint32_t *stack){
   eax++;
}

/**
 * OPCODE 0x41
 */
int instr_inc_ecx(uint8_t *code, uint32_t *stack){
   ecx++;
}

/**
 * OPCODE 0x42
 */
int instr_inc_edx(uint8_t *code, uint32_t *stack){
   edx++;
}

/**
 * OPCODE 0x43
 */
int instr_inc_ebx(uint8_t *code, uint32_t *stack){
   ebx++;
}

/**
 * OPCODE 0x44
 */
int instr_inc_esp(uint8_t *code, uint32_t *stack){
   esp++;
}

/**
 * OPCODE 0x45
 */
int instr_inc_ebp(uint8_t *code, uint32_t *stack){
   ebp++;
}

/**
 * OPCODE 0x46
 */
int instr_inc_esi(uint8_t *code, uint32_t *stack){
   esi++;
}

/**
 * OPCODE 0x47
 */
int instr_inc_edi(uint8_t *code, uint32_t *stack){
   edi++;
}

/**
 * OPCODE 0x48
 */
int instr_dec_eax(uint8_t *code, uint32_t *stack){
   eax--;
}

/**
 * OPCODE 0x49
 */
int instr_dec_ecx(uint8_t *code, uint32_t *stack){
   ecx--;
}

/**
 * OPCODE 0x4A
 */
int instr_dec_edx(uint8_t *code, uint32_t *stack){
   edx--;
}

/**
 * OPCODE 0x4B
 */
int instr_dec_ebx(uint8_t *code, uint32_t *stack){
   ebx--;
}

/**
 * OPCODE 0x4C
 */
int instr_dec_esp(uint8_t *code, uint32_t *stack){
   esp--;
}

/**
 * OPCODE 0x4D
 */
int instr_dec_ebp(uint8_t *code, uint32_t *stack){
   ebp--;
}

/**
 * OPCODE 0x4E
 */
int instr_dec_esi(uint8_t *code, uint32_t *stack){
   esi--;
}

/**
 * OPCODE 0x4F
 */
int instr_dec_edi(uint8_t *code, uint32_t *stack){
   edi--;
}

/**
 * OPCODE 0x50
 */
int instr_push_eax(uint8_t *code, uint32_t *stack) {
   stack[esp++]=eax;
}

/**
 * OPCODE 0x51
 */
int instr_push_ecx(uint8_t *code, uint32_t *stack) {
   stack[esp++]=ecx;
}

/**
 * OPCODE 0x52
 */
int instr_push_edx(uint8_t *code, uint32_t *stack) {
   stack[esp++]=edx;
}

/**
 * OPCODE 0x53
 */
int instr_push_ebx(uint8_t *code, uint32_t *stack) {
   stack[esp++]=ebx;
}

/**
 * OPCODE 0x54
 */
int instr_push_esp(uint8_t *code, uint32_t *stack) {
   stack[esp]=esp;
   esp++;
}

/**
 * OPCODE 0x55
 */
int instr_push_ebp(uint8_t *code, uint32_t *stack) {
   stack[esp++]=ebp;
}

/**
 * OPCODE 0x56
 */
int instr_push_esi(uint8_t *code, uint32_t *stack) {
   stack[esp++]=esi;
}

/**
 * OPCODE 0x57
 */
int instr_push_edi(uint8_t *code, uint32_t *stack) {
   stack[esp++]=edi;
}

/**
 * OPCODE 0x58
 */
int instr_pop_eax(uint8_t *code, uint32_t *stack) {
   eax = stack[--esp];
}

/**
 * OPCODE 0x59
 */
int instr_pop_ecx(uint8_t *code, uint32_t *stack) {
   ecx = stack[--esp];
}

/**
 * OPCODE 0x5A
 */
int instr_pop_edx(uint8_t *code, uint32_t *stack) {
   edx = stack[--esp];
}

/**
 * OPCODE 0x5B
 */
int instr_pop_ebx(uint8_t *code, uint32_t *stack) {
   ebx = stack[--esp];
}

/**
 * OPCODE 0x5C
 */
int instr_pop_esp(uint8_t *code, uint32_t *stack) {
   --esp;
   esp = stack[esp];
}

/**
 * OPCODE 0x5D
 */
int instr_pop_ebp(uint8_t *code, uint32_t *stack) {
   ebp = stack[--esp];
}

/**
 * OPCODE 0x5E
 */
int instr_pop_esi(uint8_t *code, uint32_t *stack) {
   esi = stack[--esp];
}

/**
 * OPCODE 0x5F
 */
int instr_pop_edi(uint8_t *code, uint32_t *stack) {
   edi = stack[--esp];
}

/**
 * OPCODE 0x63
 */
int instr_arpl(uint8_t *code, uint32_t *stack){
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
int opcode80(uint8_t *code, uint32_t *stack){

}

/**
 * OPCODE 0x80 /0
 * 
 * Debe tener un valor de 000 (0) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int instr_add_r8_imm8(uint8_t *code, uint32_t *stack){
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
int instr_adc_r8_imm8(uint8_t *code, uint32_t *stack){
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
int instr_and_r8_imm8(uint8_t *code, uint32_t *stack){
   uint8_t * r8 = (uint8_t*)regs8[((*(code+1)) & 0x7)];
   uint8_t v = *(code+2);

   *r8 &= v;
}

/**
 * OPCODE 0x80 /7
 * 
 * Debe tener un valor de 111 (7) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int instr_cmp_r8_imm8(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+1);
   uint8_t v = *(code+2);
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
int opcode81(uint8_t *code, uint32_t *stack){

}

/**
 * OPCODE 0x81 /0
 * 
 * Debe tener un valor de 000 (0) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int instr_add_r16_32_imm16_32(uint8_t *code, uint32_t *stack){
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
int instr_adc_r16_32_imm16_32(uint8_t *code, uint32_t *stack){
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
int instr_and_r16_32_imm16_32(uint8_t *code, uint32_t *stack){
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
int instr_cmp_r16_32_imm16_32(uint8_t *code, uint32_t *stack){
   uint32_t v;
   uint8_t modrm = *(code+1);
   uint32_t * rm32 = regs[modrm & 0x7];
   uint8_t *p = (uint8_t *)&v;
   p[0]=*(code+2);
   p[1]=*(code+3);
   p[2]=*(code+4);
   p[3]=*(code+5);
   
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
int opcode83(uint8_t *code, uint32_t *stack){

}

/**
 * OPCODE 0x83 /0
 * 
 * Debe tener un valor de 000 (0) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int instr_add_r16_32_imm8(uint8_t *code, uint32_t *stack){
   uint32_t * r32 = regs[((*(code+1))&0x7)];
   uint8_t v = *(code+2);

   *r32 += v;
}

/**
 * OPCODE 0x83 /2
 * 
 * Debe tener un valor de 010 (2) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int instr_adc_r16_32_imm8(uint8_t *code, uint32_t *stack){
   uint16_t v = *(code+1) ; //byte ModR/M
   uint16_t * f = (uint16_t *)&eflags;
   v&=0b00000111;
   uint32_t * reg = regs[v];
   *reg = *reg + *(code+2) + (*f & 0x1);
}

/**
 * OPCODE 0x83 /4
 */
int instr_and_r16_32_imm8(uint8_t *code, uint32_t *stack){
   uint32_t * r32 = regs[((*(code+1)) & 0x7)];
   uint8_t v = *(code+2);

   *r32 &= v;
}

/**
 * OPCODE 0x83 /7
 * 
 * Debe tener un valor de 111 (7) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int instr_cmp_r16_32_imm8(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+1);
   uint32_t * rm32 = regs[modrm & 0x7];
   uint8_t v = *(code+2);
   
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
int instr_nop(uint8_t *code, uint32_t *stack){
   
}
/**
 * OPCODE 0x91
 */
int instr_xchg_ecx_eax(uint8_t *code, uint32_t *stack){
   uint32_t a;
   a = eax;
   eax = ecx;
   ecx = a;
}

/**
 * OPCODE 0x98
 */
int instr_cwde(uint8_t *code, uint32_t *stack){
   uint16_t *ax = (uint16_t *) &eax;
   uint8_t bit = (*ax >> 15) & 1;
   uint16_t v = bit ? 0xFFFF : 0x0000;
   ax++;
   *ax = v; 
}

/**
 * OPCODE 0x99 //cwd same but with 16b operand
 */
int instr_cdq(uint8_t *code, uint32_t *stack){
   
   uint8_t bit = (eax >> 31) & 1;
   uint32_t v = bit ? 0xFFFFFFFF : 0x00000000;
   edx = v; 
}

/**
 * OPCODE 0xC8
 */
int instr_enter(uint8_t *code, uint32_t *stack){
   uint16_t operand;
   uint8_t level;
   uint8_t * p = (uint8_t *)&operand;
   p[0]=*(code+1);
   p[1]=*(code+2);
   level=*(code+3);

   level = level % 32;
   stack[esp++]=ebp; //push ebp
   uint32_t fp = esp;
   if (level){
      for(int i=1; i< level; i++){
         ebp-=1;
         stack[esp++]=ebp; //push ebp
      }
      stack[esp++]=fp;
   }
   ebp = fp;
   esp -= operand;
}


/**
 * OPCODE 0xF5
 */
int instr_cmc(uint8_t *code, uint32_t *stack){
   uint16_t * f = (uint16_t *)&eflags;
   *f ^= 0x1;
}

/**
 * OPCODE 0xF6
 */
int opcodeF6(uint8_t *code, uint32_t *stack){
   
}

/**
 * OPCODE 0xF6
 * 
 * Debe tener un valor de 110 (6) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int instr_div_al_rm8(uint8_t *code, uint32_t *stack){
   //INTERRUPT 0?
   uint8_t modrm = *(code+1);
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
int instr_idiv_al_rm8(uint8_t *code, uint32_t *stack){
   //INTERRUPT 0?
   uint8_t modrm = *(code+1);
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
int opcodeF7(uint8_t *code, uint32_t *stack){
   
}

/**
 * OPCODE 0xF7
 * 
 * Debe tener un valor de 110 (6) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int instr_div_eax_rm16_32(uint8_t *code, uint32_t *stack){
   //INTERRUPT 0?
   uint8_t modrm = *(code+1);
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
int instr_idiv_eax_rm16_32(uint8_t *code, uint32_t *stack){
   //INTERRUPT 0?
   uint8_t modrm = *(code+1);
   uint32_t *rm32 = regs[modrm & 0x7];
   uint32_t vold = eax;
   eax /= *rm32;
   edx = vold % *rm32;
   //flags undefined
}

/**
 * OPCODE 0xF8
 */
int instr_clc(uint8_t *code, uint32_t *stack){
   uint16_t * f = (uint16_t *)&eflags;
   *f &=0xFFFE; 
}

/**
 * OPCODE 0xFA
 */
int instr_cli(uint8_t *code, uint32_t *stack){
   uint16_t * f = (uint16_t *)&eflags;
   *f&=0xFDFF;
}

/**
 * OPCODE 0xFC
 */
int instr_cld(uint8_t *code, uint32_t *stack){
   uint16_t * f= (uint16_t *)&eflags;
   *f&=0xFBFF;
}

/**
 * OPCODE0xFE
 * 
 * Debe tener un valor de 001 (1) en el campo reg -> mod-reg-r/m (del byte ModR/M)
 */
int instr_dec_rm8(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+1);
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
int instr_dec_rm16_32(uint8_t *code, uint32_t *stack){
   uint8_t modrm = *(code+1);
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