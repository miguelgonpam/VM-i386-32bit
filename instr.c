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
uint8_t regs_size[] = {NULL, 0x08, 0x08, 0x10, 0x08, 0x08, 0x10, NULL, 0x10, 0x08, 0x08, 0x10, 0x10, 0x08, 0x10, NULL, 0x08, 0x10, 0x10, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,    0x20, 0x20, NULL, 0x10, 0x20, 0x20, NULL, 0x10, 0x10, 0x10, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0x10, NULL, 0x10, NULL, 0x10};



/******************************************************/
/*********************** SETUP ************************/
/******************************************************/

/**
 *  Initializes the memory and the reisters.
 */
int initialize(){
    /* Initialize registers. EFLAGS is already set in flags.c */
    esp = 0xFFF00000;
    eip = 0x08490000;

    mem = (uint8_t *)malloc(UINT32_MAX * sizeof(uint8_t)); //4GB de memoria del i386 (32 bits)
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
int aaa(cs_insn *insn){
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
int aad(cs_insn *insn){
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
int aam(cs_insn *insn){
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
int aas(cs_insn *insn){
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
int adc(cs_insn *insn){
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
int add(cs_insn *insn){
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
int and(cs_insn *insn){
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
int arpl(cs_insn *insn){
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
int bound(cs_insn *insn){
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint32_t rsrc1, rsrc2;

    if (op1.type != X86_OP_REG || op2.type != X86_OP_MEM){return -1;}
    void * p = regs[op1.reg];
    uint8_t base = regs_size[op1.reg];
    if (base == 0x10){ /* 16-bit */
        //in protected mode shouldn't be 16-bit address
        return -1;
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
int bsf(cs_insn *insn){
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
int bsr(cs_insn *insn){
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

int mov(cs_insn *insn){
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
}

int sub(cs_insn *insn){
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
}

int or(cs_insn *insn){
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
    
}



int reg_val(int reg_id){
    if (reg_id < 0 || reg_id > 49){
        return -1;
    }
    void * p = regs[reg_id];
    uint8_t base = regs_size[reg_id];
    if (p != NULL && base != NULL){
        switch(base){
            case 0x8:
                return *((uint8_t *) p);
                break;
            case 0x10:
                return *((uint16_t *) p);
                break;
            case 0x20:
                return *((uint32_t *) p);
                break;
            default:
                return *((uint32_t *) p);
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