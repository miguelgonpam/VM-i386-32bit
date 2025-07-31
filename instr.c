#include <stdint.h>
#include <stdlib.h>
#include <capstone/capstone.h>
#include "instr.h"
#include "flags.h"


/******************************************************/
/******************** DECLARATIONS ********************/
/******************************************************/

uint8_t * mem;
uint32_t eax = 0, edx = 0, esp = 0, esi = 0, eip = 0, cs = 0, ds = 0, fs = 0, ecx = 0, ebx = 0, ebp = 0, edi = 0, ss = 0, es = 0, gs = 0; 
extern uint32_t eflags;

/* REGISTER            INVL,   AH,   AL,   AX,   BH,   BL,   BP,  BPL,   BX,   CH,   CL,   CS,   CX,   DH,   DI,  DIL,   DL,   DS,   DX,  EAX,  EBP,  EBX,  ECX,  EDI,  EDX,  EFLAGS,  EIP,  EIZ,   ES,  ESI,  ESP, FPSW,   FS,   GS,   IP,  RAX,  RBP,  RBX,  RCX,  RDI,  RDX,  RIP,  RIZ,  RSI,  RSP,   SI,  SIL,   SP,  SPL,   SS  */
/* INDEX                  0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16    17    18    19    20    21    22    23    24       25    26    27    28    29    30    31    32    33    34    35    36    37    38    39    40    41    42    43    44    45    46    47    48    49  */
void * regs[] =       {NULL, &eax, &eax, &eax, &ebx, &ebx, &ebp, NULL, &ebx, &ecx, &ecx,  &cs, &ecx, &edx, &edi, NULL, &edx,  &ds, &edx, &eax, &ebp, &ebx, &ecx, &edi, &edx, &eflags, &eip, NULL,  &es, &esi, &esp, NULL,  &fs,  &gs, &eip, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &esi, NULL, &esp, NULL,  &ss};
uint8_t regs_size[] = {   0, 0x08, 0x08, 0x10, 0x08, 0x08, 0x10,    0, 0x10, 0x08, 0x08, 0x10, 0x10, 0x08, 0x10,    0, 0x08, 0x10, 0x10, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,    0x20, 0x20,    0, 0x10, 0x20, 0x20,    0, 0x10, 0x10, 0x10,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0x10,    0, 0x10,    0, 0x10};



/******************************************************/
/*********************** SETUP ************************/
/******************************************************/

/**
 *  Initializes the memory and the reisters.
 */
int initialize(){
    /* Initialize registers. EFLAGS is already set in flags.c */
    esp = 0xFFF00000;
    //eip = 0x08490000;

    mem = (uint8_t *)calloc(sizeof(uint8_t), UINT32_MAX ); //4GB de memoria del i386 (32 bits)
    if (!mem)
        return 0;

    return 1;
}

/******************************************************/
/******************* INSTRUCTIONS *********************/
/******************************************************/

/**
 *  AAA. ASCII Adjust after Addition.
 *
 *  Opcode 0x37.
 *
 *  No Protected mode exceptions.
 *
 *  Affects flags AF and CF as showed. Rest are undefined.
 *
 *  @param insn instruction struct that stores all the information.
 */
int aaa_i(cs_insn *insn){
    uint8_t * al = (uint8_t *) &eax;
    uint8_t * ah = al+1;
    if ((*al & 0x0F > 9) || (test_Flag(AF))){
        *al = (*al + 6) & 0x0F;
        *ah += 1;
        set_Flag(AF);
        set_Flag(CF); 
    }else{
        clear_Flag(AF);
        clear_Flag(ZF);
    }
}

/**
 *  AAD. ASCII Adjust AX before Division.
 *
 *  Opcode 0xD5 0A.
 *
 *  No Protected mode exceptions.
 *
 *  Affects flags PF, ZF and SF. Rest are undefined.
 *
 *  @param insn instruction struct that stores all the information.
 */
int aad_i(cs_insn *insn){
    uint8_t * al = (uint8_t *) &eax;
    uint8_t * ah = al+1;
    *al = *ah * 10 + *al;
    *ah = 0;

    sign(*al)?set_Flag(SF):clear_Flag(SF); //set SF to MSB of AL
    zero(*al)?clear_Flag(ZF):set_Flag(ZF); //set ZF if AL is 0
    parity(*al)?set_Flag(PF):clear_Flag(PF); //set PF if there is an even number of 1's.
}

/**
 *  AAM. ASCII Adjust AX after Multiply.
 *
 *  Opcode 0xD4 0A.
 *
 *  No Protected mode exceptions.
 *
 *  Affects flags PF, ZF and SF. Rest are undefined.
 *
 *  @param insn instruction struct that stores all the information.
 */
int aam_i(cs_insn *insn){
    uint8_t * al = (uint8_t *) &eax;
    uint8_t * ah = al+1;

    *ah = *al / 10;
    *al = *al % 10;

    sign(*al)?set_Flag(SF):clear_Flag(SF); //set SF to MSB of AL
    zero(*al)?clear_Flag(ZF):set_Flag(ZF); //set ZF if AL is 0
    parity(*al)?set_Flag(PF):clear_Flag(PF); //set PF if there is an even number of 1's.
}

/**
 *  AAS. ASCII Adjust after Subtraction.
 *
 *  Opcode 0x3F.
 *
 *  No Protected mode exceptions.
 *
 *  Affects flags AF and CF as showed. Rest are undefined.
 *
 *  @param insn instruction struct that stores all the information.
 */
int aas_i(cs_insn *insn){
    uint8_t * al = (uint8_t *) &eax;
    uint8_t * ah = al+1;

    if ((*al & 0x0F) > 9 || test_Flag(AF)){
        *al -= 6;
        *al &= 0x0F;
        *ah -= 1;
        set_Flag(AF);
        set_Flag(CF);
    }else{
        clear_Flag(AF);
        clear_Flag(CF);
    }
}

/**
 *  ADC. Add with Carry.
 *
 *  Opcodes (0x10, 0x11, 0x12, 01x13, 0x14, 0x15, 0x80 /2, 0x81 /2, 0x83 /2).
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  Flags as described in Appendix C.
 *
 *  @param insn instruction struct that stores all the information.
 */
int adc_i(cs_insn *insn){
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint32_t val;
    if (op2.type == X86_OP_REG){
        val = reg_val(op2.reg);
    }else if(op2.type == X86_OP_IMM){
        val = op2.imm;
    }else if (op2.type == X86_OP_MEM){
        val = *((uint32_t *)(mem + eff_addr(op2.mem)));
    }

    if (op1.type == X86_OP_REG){
        void * p = regs[op1.reg];
        uint8_t base = regs_size[op1.reg];
        switch(base){
            case 0x8:
                *((uint8_t *) p)+= (val & 0xFF)+test_Flag(CF);
                break;
            case 0x10:
                *((uint16_t *) p) += (val & 0xFFFF)+test_Flag(CF);
                break;
            case 0x20:
                *((uint32_t *) p) += val+test_Flag(CF);
                break;
            default:
                *((uint32_t *) p) += val+test_Flag(CF);
                break;
        }
    }else if (op1.type == X86_OP_MEM){
        *((uint32_t *)(mem+eff_addr(op1.mem))) += (val + test_Flag(CF));
    }
    //op1 cant be X86_OP_IMM   
}

/**
 *  ADD. Add.
 *
 *  Opcodes (0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x80 /0, 0x81 /0, 0x83 /0).
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  Flags as described in Appendix C.
 *
 *  @param insn instruction struct that stores all the information.
 */
int add_i(cs_insn *insn){
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint32_t val;
    if (op2.type == X86_OP_REG){
        val = reg_val(op2.reg);
    }else if(op2.type == X86_OP_IMM){
        val = op2.imm;
    }else if (op2.type == X86_OP_MEM){
        val = *((uint32_t *)(mem + eff_addr(op2.mem)));
    }

    if (op1.type == X86_OP_REG){
        void * p = regs[op1.reg];
        uint8_t base = regs_size[op1.reg];
        switch(base){
            case 0x8:
                *((uint8_t *) p)+= (val & 0xFF);
                break;
            case 0x10:
                *((uint16_t *) p) += (val & 0xFFFF);
                break;
            case 0x20:
                *((uint32_t *) p) += val;
                break;
            default:
                *((uint32_t *) p) += val;
                break;
        }
    }else if (op1.type == X86_OP_MEM){
        *((uint32_t *)(mem+eff_addr(op1.mem))) += val;
    }
    //op1 cant be X86_OP_IMM   
}

/**
 *  AND. Logical AND.
 *
 *  Opcodes (0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x80 /4, 0x81 /4, 0x83 /4).
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  CF = 0 and OF = 0. Rest of Flags as described in Appendix C.
 *
 *  @param insn instruction struct that stores all the information.
 */
int and_i(cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint32_t val;
    if (op2.type == X86_OP_REG){
        val = reg_val(op2.reg);
    }else if(op2.type == X86_OP_IMM){
        val = op2.imm;
    }else if (op2.type == X86_OP_MEM){
        val = *((uint32_t *)(mem + eff_addr(op2.mem)));
    }

    if (op1.type == X86_OP_REG){
        void * p = regs[op1.reg];
        uint8_t base = regs_size[op1.reg];
        switch(base){
            case 0x8:
                *((uint8_t *) p) &= (val & 0xFF);
                break;
            case 0x10:
                *((uint16_t *) p) &= (val & 0xFFFF);
                break;
            case 0x20:
                *((uint32_t *) p) &= val;
                break;
            default:
                *((uint32_t *) p) &= val;
                break;
        }
    }else if (op1.type == X86_OP_MEM){
        *((uint32_t *)(mem+eff_addr(op1.mem))) &= val;
    }
    //op1 cant be X86_OP_IMM
    clear_Flag(CF);
    clear_Flag(OF);
}

/**
 *  ARPL. Adjust RPL Field of Selector.
 *
 *  Opcode 0x63 /r.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  ZF as showed.
 *
 *  @param insn instruction struct that stores all the information.
 */
int arpl_i(cs_insn *insn){
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];
    uint32_t val;
    if (op2.type == X86_OP_REG){
        val = reg_val(op2.reg);
    }else if(op2.type == X86_OP_IMM){
        val = op2.imm;
    }else if (op2.type == X86_OP_MEM){
        val = *((uint32_t *)(mem + eff_addr(op2.mem)));
    }

    uint8_t *p;
    if (op1.type == X86_OP_REG){
        p = (uint8_t *)regs[op1.reg];
    }else if (op1.type == X86_OP_MEM){
        p = (mem+eff_addr(op1.mem));
    }
    if (*p & 0x3 < val & 0x3){
            set_Flag(ZF);
            *p &= 0xFC;
            *p |= (val & 0x3);
        }else{
            clear_Flag(ZF);
        }
    //op1 cant be X86_OP_IMM
}

/**
 *  BOUND. Check Array Index Against Bounds.
 *
 *  Opcode 0x62 /r.
 *
 *  Interrupt 5 if index is out of bounds.
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int bound_i(cs_insn *insn){
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint32_t rsrc1, rsrc2;

    if (op1.type != X86_OP_REG || op2.type != X86_OP_MEM){return -1;}
    void * p = regs[op1.reg];
    uint8_t base = regs_size[op1.reg];
    if (base == 0x10){ /* 16-bit */
        uint16_t * r = (uint16_t *) p;
        rsrc1 = *((uint16_t *)(mem + eff_addr(op2.mem))); /* Offset 0 bytes with m32&32 */
        rsrc2 = *(((uint16_t *)(mem + eff_addr(op2.mem)))+1); /* Offset 2 bytes with m32&32*/
        if( *r < rsrc1 || *r > rsrc2){
            return -5; /* Interrupt 5 */
        }
    }else{             /* 32-bit */
        uint32_t * r = (uint32_t *) p;
        rsrc1 = *((uint32_t *)(mem + eff_addr(op2.mem))); /* Offset 0 bytes with m32&32 */
        rsrc2 = *(((uint32_t *)(mem + eff_addr(op2.mem)))+1); /* Offset 4 bytes with m32&32*/
        if( *r < rsrc1 || *r > rsrc2){
            return -5; /* Interrupt 5 */
        }
    }
}

/**
 *  BSF. Bit Scan Forward.
 *
 *  Opcode 0x0F BC.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  ZF as showed.
 *
 *  @param insn instruction struct that stores all the information.
 */
int bsf_i(cs_insn *insn){
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint32_t val;                           /* extract op2 value*/
    if (op2.type == X86_OP_REG){
        val = reg_val(op2.reg);
    }else if(op2.type == X86_OP_IMM){
        val = op2.imm;
    }else if (op2.type == X86_OP_MEM){
        val = *((uint32_t *)(mem + eff_addr(op2.mem)));
    }
    if(!val){set_Flag(ZF);return 0;}
    if (op1.type != X86_OP_REG){return -1;} /* op1 must be a register*/
    else{
        uint8_t base = regs_size[op1.reg];
        if (base == 0x10){ /* 16-bit */
            uint16_t * p = regs[op1.reg];
            int temp = 0;
            clear_Flag(ZF);
            while ((val >> temp) & 0x1 == 0) {
                temp++;
                *p=temp;
            }
        }else{             /* 32-bit */
            uint32_t * p = regs[op1.reg];
            int temp = 0;
            clear_Flag(ZF);
            while ((val >> temp) & 0x1 == 0) {
                temp++;
                *p=temp;
            }
        }
    }
}

/**
 *  BSR. Bit Scan Reverse.
 *
 *  Opcode 0x0F BD.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  ZF as showed.
 *
 *  @param insn instruction struct that stores all the information.
 */
int bsr_i(cs_insn *insn){
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint32_t val;                           /* extract op2 value*/
    if (op2.type == X86_OP_REG){
        val = reg_val(op2.reg);
    }else if(op2.type == X86_OP_IMM){
        val = op2.imm;
    }else if (op2.type == X86_OP_MEM){
        val = *((uint32_t *)(mem + eff_addr(op2.mem)));
    }
    if(!val){set_Flag(ZF);return 0;}
    if (op1.type != X86_OP_REG){return -1;} /* op1 must be a register*/
    else{
        uint8_t base = regs_size[op1.reg];
        if (base == 0x10){ /* 16-bit */
            uint16_t * p = regs[op1.reg];
            int temp = 15;
            clear_Flag(ZF);
            while ((val >> temp) & 0x1 == 0) {
                temp--;
                *p=temp;
            }
        }else{             /* 32-bit */
            uint32_t * p = regs[op1.reg];
            int temp = 31;
            clear_Flag(ZF);
            while ((val >> temp) & 0x1 == 0) {
                temp--;
                *p=temp;
            }
        }
    }
}

/**
 *  BT. Bit Test.
 *
 *  Opcode 0x0F A3, 0x0F BA /4.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  CF as showed.
 *
 *  @param insn instruction struct that stores all the information.
 */
int bt_i(cs_insn *insn){
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint32_t val1, val2;
    uint8_t b;
    if (op1.type == X86_OP_REG){ /* REG */
        b = regs_size[op1.reg];
        if (b == 0x1){
            val1 = (uint32_t)*((uint16_t *)regs[op1.reg]);
        }else{
            val1 = *((uint32_t *)regs[op1.reg]);
        }
        
    }else{ /* MEM */
        val1 = *((uint32_t *)(mem+eff_addr(op1.mem)));
        b = 0x20;
    }

    if (op2.type == X86_OP_REG){ /* REG */
        b = regs_size[op2.reg];
        if (b == 0x1){
            val2 = (uint32_t)*((uint16_t *)regs[op2.reg]);
        }else{
            val2 = *((uint32_t *)regs[op2.reg]);
        }
    }else{ /* IMM */
        val2 = (uint32_t) op2.imm;
        b = 0x8;
    }
    (val1 >> (val2 % b))?set_Flag(CF):clear_Flag(CF);
    return 0;
}

/**
 *  BTC. Bit Test and Complement.
 *
 *  Opcode 0x0F BB, 0x0F BA /7.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  CF as showed.
 *
 *  @param insn instruction struct that stores all the information.
 */
int btc_i(cs_insn *insn){
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint32_t val2;
    uint8_t b;
    

    if (op2.type == X86_OP_REG){ /* REG */
        b = regs_size[op2.reg];
        if (b == 0x1){
            val2 = (uint32_t)*((uint16_t *)regs[op2.reg]);
        }else{
            val2 = *((uint32_t *)regs[op2.reg]);
        }
    }else{ /* IMM */
        val2 = (uint32_t) op2.imm;
        b = 0x8;
    }

    if (op1.type == X86_OP_REG){ /* REG */
        b = regs_size[op1.reg];
        if (b == 0x1){
            uint16_t * val1 = ((uint16_t *)regs[op1.reg]);
            (*val1 >> (val2 % b))?set_Flag(CF):clear_Flag(CF);
            *val1 ^= pow_i(2, (val2 % b));
        }else{
            uint32_t * val1 = ((uint32_t *)regs[op1.reg]);
            (*val1 >> (val2 % b))?set_Flag(CF):clear_Flag(CF);
            *val1 ^= pow_i(2, (val2 % b));
        }
        
    }else{ /* MEM */
        uint32_t * val1 = ((uint32_t *)(mem+eff_addr(op1.mem)));
        b = 0x20;
        (*val1 >> (val2 % b))?set_Flag(CF):clear_Flag(CF);
        *val1 ^= pow_i(2, (val2 % b));
    }
    
    return 0;
}

/**
 *  BTR. Bit Test and Reset.
 *
 *  Opcode 0x0F B3, 0x0F BA /6.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  CF as showed.
 *
 *  @param insn instruction struct that stores all the information.
 */
int btr_i(cs_insn *insn){
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint32_t val2;
    uint8_t b;
    

    if (op2.type == X86_OP_REG){ /* REG */
        b = regs_size[op2.reg];
        if (b == 0x1){
            val2 = (uint32_t)*((uint16_t *)regs[op2.reg]);
        }else{
            val2 = *((uint32_t *)regs[op2.reg]);
        }
    }else{ /* IMM */
        val2 = (uint32_t) op2.imm;
        b = 0x8;
    }

    if (op1.type == X86_OP_REG){ /* REG */
        b = regs_size[op1.reg];
        if (b == 0x1){
            uint16_t * val1 = ((uint16_t *)regs[op1.reg]);
            (*val1 >> (val2 % b))?set_Flag(CF):clear_Flag(CF);
            *val1 &= (0xFFFF - pow_i(2, (val2 % b)));
        }else{
            uint32_t * val1 = ((uint32_t *)regs[op1.reg]);
            (*val1 >> (val2 % b))?set_Flag(CF):clear_Flag(CF);
            *val1 &= (0xFFFFFFFF - pow_i(2, (val2 % b)));
        }
        
    }else{ /* MEM */
        uint32_t * val1 = ((uint32_t *)(mem+eff_addr(op1.mem)));
        b = 0x20;
        (*val1 >> (val2 % b))?set_Flag(CF):clear_Flag(CF);
        *val1 &= (0xFFFFFFFF - pow_i(2, (val2 % b)));
    }
    
    return 0;
}

/**
 *  BTS. Bit Test and Set.
 *
 *  Opcode 0x0F AB, 0x0F BA /5.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  CF as showed.
 *
 *  @param insn instruction struct that stores all the information.
 */
int bts_i(cs_insn *insn){
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint32_t val2;
    uint8_t b;
    

    if (op2.type == X86_OP_REG){ /* REG */
        b = regs_size[op2.reg];
        if (b == 0x1){
            val2 = (uint32_t)*((uint16_t *)regs[op2.reg]);
        }else{
            val2 = *((uint32_t *)regs[op2.reg]);
        }
    }else{ /* IMM */
        val2 = (uint32_t) op2.imm;
        b = 0x8;
    }

    if (op1.type == X86_OP_REG){ /* REG */
        b = regs_size[op1.reg];
        if (b == 0x1){
            uint16_t * val1 = ((uint16_t *)regs[op1.reg]);
            (*val1 >> (val2 % b))?set_Flag(CF):clear_Flag(CF);
            *val1 |= pow_i(2, (val2 % b));
        }else{
            uint32_t * val1 = ((uint32_t *)regs[op1.reg]);
            (*val1 >> (val2 % b))?set_Flag(CF):clear_Flag(CF);
            *val1 |= pow_i(2, (val2 % b));
        }
        
    }else{ /* MEM */
        uint32_t * val1 = ((uint32_t *)(mem+eff_addr(op1.mem)));
        b = 0x20;
        (*val1 >> (val2 % b))?set_Flag(CF):clear_Flag(CF);
        *val1 |= pow_i(2, (val2 % b));
    }
    
    return 0;
}

int call_i(cs_insn *insn){
    return 0;
}

/**
 *  CBW. Convert Byte to Word.
 *
 *  Opcode 0x98.
 *
 *  No exceptions.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int cbw_i(cs_insn *insn){
    uint8_t * al = (uint8_t *)&eax;
    uint16_t * ax = (uint16_t *)&eax;

    (*al >> 7)?(*ax |=(0xFF00 | *al)):(*ax &= (0x00FF & *al));
    return 0;
}

/**
 *  CWDE. Convert Word to Doubleword.
 *
 *  Opcode 0x98.
 *
 *  No exceptions.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int cwde_i(cs_insn *insn){
    uint16_t * ax = (uint16_t *)&eax;

    (*ax >> 15)?(eax |=(0xFFFF0000 | *ax)):(eax &=(0x0000FFFF & *ax));
    return 0;
}

/**
 *  CLC. Clear Carry Flag.
 *
 *  Opcode 0xF8.
 *
 *  No exceptions.
 *
 *  CF = 0.
 *
 *  @param insn instruction struct that stores all the information.
 */
int clc_i(cs_insn *insn){
    clear_Flag(CF);
    return 0;
}

/**
 *  CLD. Clear Direction Flag.
 *
 *  Opcode 0xFC.
 *
 *  No exceptions.
 *
 *  DF = 0.
 *
 *  @param insn instruction struct that stores all the information.
 */
int cld_i(cs_insn *insn){
    clear_Flag(DF); /* After CLD is executed, string operations will increment the index registers (SI and/or DI) that they use. ?*/
    return 0;
}

/**
 *  CLI. Clear Interrupt Flag.
 *
 *  Opcode 0xFA.
 *
 *  Exceptions if no privilege level.
 *
 *  IF = 0.
 *
 *  @param insn instruction struct that stores all the information.
 */
int cli_i(cs_insn *insn){
    clear_Flag(IF);
    return 0;
}

/**
 *  CMC. Complement Carry Flag.
 *
 *  Opcode 0xF5.
 *
 *  No Exceptions.
 *
 *  CF = NOT CF.
 *
 *  @param insn instruction struct that stores all the information.
 */
int cmc_i(cs_insn *insn){
    complement_Flag(CF);
    return 0;
}

int cmp_i(cs_insn *insn){
    return 0;
}



int mov_i(cs_insn *insn){
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint32_t val;
    if (op2.type == X86_OP_REG){
        val = reg_val(op2.reg);
    }else if(op2.type == X86_OP_IMM){
        val = op2.imm;
    }else if (op2.type == X86_OP_MEM){
        val = *((uint32_t *)(mem + eff_addr(op2.mem)));
    }

    if (op1.type == X86_OP_REG){
        void * p = regs[op1.reg];
        uint8_t base = regs_size[op1.reg];
        switch(base){
            case 0x8:
                *((uint8_t *) p)= (val & 0xFF);
                break;
            case 0x10:
                *((uint16_t *) p) = (val & 0xFFFF);
                break;
            case 0x20:
                *((uint32_t *) p) = val;
                break;
            default:
                *((uint32_t *) p) = val;
                break;
        }
    }else if (op1.type == X86_OP_MEM){
        *((uint32_t *)(mem+eff_addr(op1.mem))) = val;
    }
    //op1 cant be X86_OP_IMM
    return 0;
}

int sub_i(cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint32_t val;
    if (op2.type == X86_OP_REG){
        val = reg_val(op2.reg);
    }else if(op2.type == X86_OP_IMM){
        val = op2.imm;
    }else if (op2.type == X86_OP_MEM){
        val = *((uint32_t *)(mem + eff_addr(op2.mem)));
    }

    if (op1.type == X86_OP_REG){
        void * p = regs[op1.reg];
        uint8_t base = regs_size[op1.reg];
        switch(base){
            case 0x8:
                *((uint8_t *) p)-= (val & 0xFF);
                break;
            case 0x10:
                *((uint16_t *) p) -= (val & 0xFFFF);
                break;
            case 0x20:
                *((uint32_t *) p) -= val;
                break;
            default:
                *((uint32_t *) p) -= val;
                break;
        }
    }else if (op1.type == X86_OP_MEM){
        *((uint32_t *)(mem+eff_addr(op1.mem))) -= val;
    }
    //op1 cant be X86_OP_IMM   
    return 0;
}

int or_i(cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint32_t val;
    if (op2.type == X86_OP_REG){
        val = reg_val(op2.reg);
    }else if(op2.type == X86_OP_IMM){
        val = op2.imm;
    }else if (op2.type == X86_OP_MEM){
        val = *((uint32_t *)(mem + eff_addr(op2.mem)));
    }

    if (op1.type == X86_OP_REG){
        void * p = regs[op1.reg];
        uint8_t base = regs_size[op1.reg];
        switch(base){
            case 0x8:
                *((uint8_t *) p)|= (val & 0xFF);
                break;
            case 0x10:
                *((uint16_t *) p) |= (val & 0xFFFF);
                break;
            case 0x20:
                *((uint32_t *) p) |= val;
                break;
            default:
                *((uint32_t *) p) |= val;
                break;
        }
    }else if (op1.type == X86_OP_MEM){
        *((uint32_t *)(mem+eff_addr(op1.mem))) |= val;
    }
    //op1 cant be X86_OP_IMM
    return 0;
}



uint32_t reg_val(int reg_id){
    if (reg_id < 0 || reg_id > 49){
        return -1;
    }
    void * p = regs[reg_id];
    uint8_t base = regs_size[reg_id];
    if (p != NULL && base != 0){
        switch(base){
            case 0x8:
                return (uint32_t)*((uint8_t *) p);
                break;
            case 0x10:
                return (uint32_t)*((uint16_t *) p);
                break;
            case 0x20:
                return (uint32_t)*((uint32_t *) p);
                break;
            default:
                return (uint32_t)*((uint32_t *) p);
                break;
        }
    }else{
        //return -1;
    }
}

uint32_t eff_addr(x86_op_mem m){
    uint32_t base = 0, index = 0, disp = 0, segment = 0;
    uint32_t scale = 1;
    if (m.base != X86_REG_INVALID)
        base = reg_val(m.base);
    if (m.index != X86_REG_INVALID)
        index = reg_val(m.index);
    if (m.scale != 0)
        scale = m.scale;
    disp = m.disp;
    if (m.segment != X86_REG_INVALID) //could be ignored (flat arch)
        segment = reg_val(m.segment);

    return segment + base + index*scale + disp;

}

uint32_t pow_i(uint32_t b, uint32_t exp){
    uint32_t result = 1;
    while (exp > 0) {
        result *= b;
        exp--;
    }
    return result;
}