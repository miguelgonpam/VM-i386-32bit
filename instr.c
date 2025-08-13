#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <capstone/capstone.h>
#include "instr.h"
#include "flags.h"


/******************************************************/
/******************** DECLARATIONS ********************/
/******************************************************/
typedef int(*Instruction)(cs_insn *insn);
uint8_t * mem;
uint32_t eax = 0, edx = 0, esp = 0, esi = 0, eip = 0, cs = 0, ds = 0, fs = 0, ecx = 0, ebx = 0, ebp = 0, edi = 0, ss = 0, es = 0, gs = 0, gdtr = 0; 
extern uint32_t eflags;

/* REGISTER            INVL,   AH,   AL,   AX,   BH,   BL,   BP,  BPL,   BX,   CH,   CL,   CS,   CX,   DH,   DI,  DIL,   DL,   DS,   DX,  EAX,  EBP,  EBX,  ECX,  EDI,  EDX,  EFLAGS,  EIP,  EIZ,   ES,  ESI,  ESP, FPSW,   FS,   GS,   IP,  RAX,  RBP,  RBX,  RCX,  RDI,  RDX,  RIP,  RIZ,  RSI,  RSP,   SI,  SIL,   SP,  SPL,   SS  */
/* INDEX                  0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16    17    18    19    20    21    22    23    24       25    26    27    28    29    30    31    32    33    34    35    36    37    38    39    40    41    42    43    44    45    46    47    48    49  */
void * regs[] =       {NULL, &eax, &eax, &eax, &ebx, &ebx, &ebp, NULL, &ebx, &ecx, &ecx,  &cs, &ecx, &edx, &edi, NULL, &edx,  &ds, &edx, &eax, &ebp, &ebx, &ecx, &edi, &edx, &eflags, &eip, NULL,  &es, &esi, &esp, NULL,  &fs,  &gs, &eip, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &esi, NULL, &esp, NULL,  &ss};
uint8_t regs_size[] = {   0, 0x08, 0x08, 0x10, 0x08, 0x08, 0x10,    0, 0x10, 0x08, 0x08, 0x10, 0x10, 0x08, 0x10,    0, 0x08, 0x10, 0x10, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,    0x20, 0x20,    0, 0x10, 0x20, 0x20,    0, 0x10, 0x10, 0x10,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0x10,    0, 0x10,    0, 0x10};

const char *inss[] = {
    "aaa","aad","aam","aas","adc","add","and","bt","bts","call","cbw","clc",
    "cld","cli","cmc","cmp","cmps","cwd","cwde", "cpuid","daa","das","dec","div",
    "hlt","idiv","imul","in","inc","int","into","iret","ja","jae","jb","jbe",
    "jc","jcxz","jecxz_i","je","jg","jge","jl","jle","jmp","jna","jnae","jnb","jnbe",
    "jnc","jne","jng","jnge","jnl","jnle","jno","jnp","jns","jo","jp","jpe",
    "jpo","js", "jz","lahf","lar","lcall","lds","lea","leave","les","lfs",
    "lgdt","lgs","lidt","lldt","lmsw","lods","loop","loope","loopne","loopnz",
    "loopz","lsl","ltr","mov","movs","movsx","movzx","mul","neg","nop","not",
    "or","out","outs","pop","popa","popf","push","pusha","pushf","ret","rcl","rcr",
    "rol","ror","sahf","sal","sar","sbb","scas","seta","setae","setb","setbe",
    "setc","sete","setg","setge","setl","setle","setna","setnae","setnb",
    "setnbe","setnc","setne","setng","setnge","setnl","setnle","setno","setnp",
    "setns","seto","setp","setpe","setpo","sets","shl","shr","sal", "sar", "stc","std","sti",
    "stos","sub","test","wait","xchg","xlat","xor","rep ins", "rep movs", "rep outs", "rep stosb",
    "rep stosw","rep stosd", "cmovne"
    };

Instruction instructions[] = {aaa_i, aad_i, aam_i, aas_i, adc_i, add_i, and_i, 
    bt_i, bts_i, call_i, cbw_i, clc_i, cld_i, cli_i, cmc_i, cmp_i, cmps_i, cwd_i, 
    cwde_i, cpuid_i, daa_i, das_i, dec_i, div_i, hlt_i, idiv_i, imul_i, in_i, inc_i, int_i, 
    into_i, iret_i, ja_i, jae_i, jb_i, jbe_i, jc_i, jcxz_i, jecxz_i, je_i, jg_i, jge_i, jl_i, 
    jle_i, jmp_i, jna_i, jnae_i, jnb_i, jnbe_i, jnc_i, jne_i, jng_i, jnge_i, jnl_i, 
    jnle_i, jno_i, jnp_i, jns_i, jo_i, jp_i, jpe_i, jpo_i, js_i, jz_i, lahf_i, 
    lar_i, lcall_i, lds_i, lea_i, leave_i, les_i, lfs_i, lgdt_i, lgs_i, lidt_i, 
    lldt_i, lmsw_i, lods_i, loop_i, loope_i, loopne_i, loopnz_i, loopz_i, lsl_i, 
    ltr_i, mov_i, movs_i, movsx_i, movzx_i, mul_i, neg_i, nop_i, not_i, or_i, out_i,
    outs_i, pop_i, popa_i, popf_i, push_i, pusha_i, pushf_i, ret_i, rcl_i, rcr_i, rol_i, 
    ror_i, sahf_i, sal_i, sar_i, sbb_i, scas_i, seta_i, setae_i, setb_i, setbe_i, 
    setc_i, sete_i, setg_i, setge_i, setl_i, setle_i, setna_i, setnae_i, setnb_i, 
    setnbe_i, setnc_i, setne_i, setng_i, setnge_i, setnl_i, setnle_i, setno_i, 
    setnp_i, setns_i, seto_i, setp_i, setpe_i, setpo_i, sets_i, shl_i, shr_i, sal_i, sar_i, stc_i, 
    std_i, sti_i, stos_i, sub_i, test_i, wait_i, xchg_i, xlat_i, xor_i, rep_ins_i,
    rep_movs_i, rep_outs_i, rep_stos_i, rep_stos_i, rep_stos_i, cmovne_i};









/******************************************************/
/*********************** SETUP ************************/
/******************************************************/

/**
 *  Initializes the memory and the reisters.
 */
int initialize(){
    /* Initialize registers. EFLAGS is already set in flags.c */
    esp = STACK_BOTTOM;
    
    /* Initialize segments */
    cs = 0x23;
    ds = 0x2b;
    ss = 0x2b;
    es = 0x2b;
    fs = 0x00;
    gs = 0x00;

    mem = (uint8_t *)calloc(sizeof(uint8_t), UINT32_MAX ); //4GB de memoria del i386 (32 bits) inicializada a 00 cada byte
    if (!mem)
        return 0;

    return 1;

    /* Initialize GDT at GDT_ADDR (defined in instr.h) */
    GDT_Descriptor * gdt = (GDT_Descriptor *)(mem + GDT_ADDR);
    init_gdt(gdt);
    gdtr = GDT_ADDR;
}



/******************************************************/
/*******************  DISPATCHER  *********************/
/******************************************************/



int dispatcher(char * mnemonic, cs_insn * insn){
    const size_t count = sizeof(instructions)/sizeof(*instructions);
    for (int i = 0; i< count ; i++){
        if(strcmp(inss[i], mnemonic) == 0){
            return instructions[i](insn);
        }
    }

    return -1;
}


/******************************************************/
/********** Global Descriptors Table (GDT) ************/
/******************************************************/

/**
 * Initializes the Global Descriptor Table. Segment selectors with TI=0 access it.
 *
 * @param table the struct GDT_Descriptor array to initialize. Its virtual address is stored in gdtr.
 */
void init_gdt(GDT_Descriptor *table){
    for (uint8_t i=0; i<GDT_ENTRIES; i++){
        table[i].limit_low = 0xFFFF;
        table[i].base_low = 0x0000;
        table[i].base_mid = 0x00;
        table[i].base_high = 0x00;
        switch(i){
            case 0:
                /* Null Descriptor, should have limit at 0x0000000*/
                table[i].access = 0x00;
                table[i].granularity = 0x00;
                table[i].limit_low = 0x0000;
                break;
            case 1:
                table[i].access = 0x9A;
                table[i].granularity = 0xCF;
                break;
            case 2:
                table[i].access = 0x92;
                table[i].granularity = 0xCF;
                break;
            case 3:
                table[i].access = 0x89;
                table[i].granularity = 0x0;
                break;
            case 4:
                table[i].access = 0xFA;
                table[i].granularity = 0xCF;
                break;
            case 5:   
                table[i].access = 0xF2;
                table[i].granularity = 0xCF;
                break;
            default:
                table[i].access = 0x89;
                table[i].granularity = 0x0;
                break;
        }

    }
}

/**
 *  Obtains a segment descriptor's base.
 *
 *  @param selector the selector to find out its base.
 *
 *  @return the segments base.
 */
uint32_t get_gdt_base(uint16_t selector){
    /* Obtain descriptor number (bits 15-3) */
    uint16_t n = (selector >> 3) & 0x1FFF;
    /* Avoid overflow */
    n %= GDT_ENTRIES;
    /* Obtain descriptor */
    GDT_Descriptor *descriptor = (GDT_Descriptor *)(mem + gdtr + n*sizeof(GDT_Descriptor));
    /* Build base from 3 parts of the base */
    uint32_t base = descriptor->base_low | (descriptor->base_mid << 16) | (descriptor->base_high << 24);
    return base;
}

/**
 *  Obtains a segment descriptor's limit.
 *
 *  @param selector the selector to find out its limit.
 *
 *  @return the segments limit.
 */
uint32_t get_gdt_limit(uint16_t selector){
    /* Obtain descriptor number (bits 15-3) */
    uint16_t n = (selector >> 3) & 0x1FFF;
    /* Avoid overflow */
    n %= GDT_ENTRIES;
    /* Obtain descriptor */
    GDT_Descriptor *descriptor = (GDT_Descriptor *)(mem + gdtr + n*sizeof(GDT_Descriptor));
    
    return descriptor->limit_low;
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
    eip += insn->size;
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
    eip += insn->size;
    uint8_t * al = (uint8_t *) &eax;
    uint8_t * ah = al+1;
    *al = *ah * 10 + *al;
    *ah = 0;

    sign(*al, 0x8)?set_Flag(SF):clear_Flag(SF); //set SF to MSB of AL
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
    eip += insn->size;
    uint8_t * al = (uint8_t *) &eax;
    uint8_t * ah = al+1;

    *ah = *al / 10;
    *al = *al % 10;

    sign(*al, 0x8)?set_Flag(SF):clear_Flag(SF); //set SF to MSB of AL
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
    eip += insn->size;
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
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint8_t s1 = op1.size, s2 = op2.size;

    uint32_t val1, val2, res;
    if (op2.type == X86_OP_REG){
        val2 = reg_val(op2.reg);
    }else if(op2.type == X86_OP_IMM){
        val2 = op2.imm;
    }else if (op2.type == X86_OP_MEM){
        void * t = mem + eff_addr(op2.mem);
        if(s2 == 4){
            val2 = *((uint32_t *)t);
        }else if (s2 == 2){
            val2 = *((uint16_t *)t);
        }else{
            val2 = *((uint8_t *)t);
        }
        
    }

    if (op1.type == X86_OP_REG){
        void * p = regs[op1.reg];
        uint8_t base = regs_size[op1.reg];
        switch(base){
            case 0x8:
                val1 = *((uint8_t *) p);
                *((uint8_t *) p)+= (val2 & 0xFF)+test_Flag(CF);
                res = *((uint8_t *) p);
                break;
            case 0x10:
                val1 = *((uint16_t *) p);
                *((uint16_t *) p) += (val2 & 0xFFFF)+test_Flag(CF);
                res = *((uint16_t *) p);
                break;
            case 0x20:
                val1 = *((uint32_t *) p);
                *((uint32_t *) p) += val2+test_Flag(CF);
                res = *((uint32_t *) p);
                break;
            default:
                val1 = *((uint32_t *) p);
                *((uint32_t *) p) += val2+test_Flag(CF);
                res = *((uint32_t *) p);
                break;
        }
    }else if (op1.type == X86_OP_MEM){
        void * t = mem+eff_addr(op1.mem);
        if(s1 == 4){
            uint32_t * p = (uint32_t *)t;
            val1 = *p;
            *p += (val2 + test_Flag(CF));
            res = *p;
        }else if(s1 == 2){
            uint16_t * p = (uint16_t *)t;
            val1 = *p;
            *p += (val2 + test_Flag(CF));
            res = *p;
        }else{
            uint8_t * p = (uint8_t *)t;
            val1 = *p;
            *p += (val2 + test_Flag(CF));
            res = *p;
        }
        
    }

    (val1 > res)?set_Flag(CF):clear_Flag(CF);
    overflow(val1, val2, res, s1*8)?set_Flag(OF):clear_Flag(OF);
    sign(res, s1*8)?set_Flag(SF):clear_Flag(SF);
    (!res)?set_Flag(ZF):clear_Flag(ZF);
    adjust(val1, val2, res)?set_Flag(AF):clear_Flag(AF);
    parity(res)?set_Flag(PF):clear_Flag(PF);  
    
    return 0;
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
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint8_t s1 = op1.size, s2 = op2.size;

    uint32_t val1, val2, res;
    if (op2.type == X86_OP_REG){
        val2 = reg_val(op2.reg);
    }else if(op2.type == X86_OP_IMM){
        val2 = op2.imm;
    }else if (op2.type == X86_OP_MEM){
        void * t = mem + eff_addr(op2.mem);
        if(s2 == 4){
            val2 = *((uint32_t *)t);
        }else if (s2 == 2){
            val2 = *((uint16_t *)t);
        }else{
            val2 = *((uint8_t *)t);
        }
    }

    if (op1.type == X86_OP_REG){
        void * p = regs[op1.reg];
        uint8_t base = regs_size[op1.reg];
        switch(base){
            case 0x8:
                val1 = *((uint8_t *) p);
                *((uint8_t *) p)+= (val2 & 0xFF);
                res = *((uint8_t *) p);

                break;
            case 0x10:
                val1 = *((uint16_t *) p);
                *((uint16_t *) p) += (val2 & 0xFFFF);
                res = *((uint16_t *) p);
                break;
            case 0x20:
                val1 = *((uint32_t *) p);
                *((uint32_t *) p) += val2;
                res = *((uint32_t *) p);
                break;
            default:
                val1 = *((uint32_t *) p);
                *((uint32_t *) p) += val2;
                res = *((uint32_t *) p);
                break;
        }
    }else if (op1.type == X86_OP_MEM){
        void * t = mem+eff_addr(op1.mem);
        if(s1 == 4){
            uint32_t * p = (uint32_t *)t;
            val1 = *p;
            *p += (val2);
            res = *p;
        }else if(s1 == 2){
            uint16_t * p = (uint16_t *)t;
            val1 = *p;
            *p += (val2);
            res = *p;
        }else{
            uint8_t * p = (uint8_t *)t;
            val1 = *p;
            *p += (val2);
            res = *p;
        }
    }

    (val1 > res)?set_Flag(CF):clear_Flag(CF);
    overflow(val1, val2, res, s1*8)?set_Flag(OF):clear_Flag(OF);
    sign(res, s1*8)?set_Flag(SF):clear_Flag(SF);
    (!res)?set_Flag(ZF):clear_Flag(ZF);
    adjust(val1, val2, res)?set_Flag(AF):clear_Flag(AF);
    parity(res)?set_Flag(PF):clear_Flag(PF);  

    return 0;
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

    uint8_t s1 = op1.size, s2 = op2.size;

    uint32_t val;
    if (op2.type == X86_OP_REG){
        val = reg_val(op2.reg);
    }else if(op2.type == X86_OP_IMM){
        val = op2.imm;
    }else if (op2.type == X86_OP_MEM){
        void * t = mem + eff_addr(op2.mem);
        if(s2 == 4){
            val = *((uint32_t *)t);
        }else if (s2 == 2){
            val = *((uint16_t *)t);
        }else{
            val = *((uint8_t *)t);
        }
    }

    uint32_t res;
    uint8_t base;
    if (op1.type == X86_OP_REG){
        void * p = regs[op1.reg];
        base = regs_size[op1.reg];
        switch(base){
            case 0x8:
                *((uint8_t *) p) &= (val & 0xFF);
                res = (uint32_t)*((uint8_t *) p);
                break;
            case 0x10:
                *((uint16_t *) p) &= (val & 0xFFFF);
                res = (uint32_t)*((uint16_t *) p);
                break;
            case 0x20:
                *((uint32_t *) p) &= val;
                res = *((uint32_t *) p);
                break;
            default:
                *((uint32_t *) p) &= val;
                res = *((uint32_t *) p);
                break;
        }
    }else if (op1.type == X86_OP_MEM){

        void * t = mem+eff_addr(op1.mem);
        if(s1 == 4){
            uint32_t * p = (uint32_t *)t;
            *p &= (val);
            res = *p;
        }else if(s1 == 2){
            uint16_t * p = (uint16_t *)t;
            *p &= (val);
            res = *p;
        }else{
            uint8_t * p = (uint8_t *)t;
            *p &= (val);
            res = *p;
        }
        base = s1*8;
    }
    //op1 cant be X86_OP_IMM
    sign(res, base)?set_Flag(SF):clear_Flag(SF);
    zero(res)?set_Flag(ZF):clear_Flag(ZF);
    parity(res)?set_Flag(PF):clear_Flag(PF);
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
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint8_t s1 = op1.size, s2 = op2.size;

    uint32_t val;
    if (op2.type == X86_OP_REG){
        val = reg_val(op2.reg);
    }else if(op2.type == X86_OP_IMM){
        val = op2.imm;
    }else if (op2.type == X86_OP_MEM){
        void * t = mem + eff_addr(op2.mem);
        if(s2 == 4){
            val = *((uint32_t *)t);
        }else if (s2 == 2){
            val = *((uint16_t *)t);
        }else{
            val = *((uint8_t *)t);
        }
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
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint32_t rsrc1, rsrc2;

    if (op1.type != X86_OP_REG || op2.type != X86_OP_MEM){return -1;}
    void * p = regs[op1.reg];
    uint8_t base = regs_size[op1.reg];
    if (base == 0x10){ /* 16-bit */
        uint16_t * r = (uint16_t *) p;
        rsrc1 = *((uint16_t *)(mem + eff_addr(op2.mem))); /* Offset 0 bytes with m16&16 */
        rsrc2 = *(((uint16_t *)(mem + eff_addr(op2.mem)))+1); /* Offset 2 bytes with m16&16*/
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
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint8_t s1 = op1.size, s2 = op2.size;

    uint32_t val;                           /* extract op2 value*/
    if (op2.type == X86_OP_REG){
        val = reg_val(op2.reg);
    }else if(op2.type == X86_OP_IMM){
        val = op2.imm;
    }else if (op2.type == X86_OP_MEM){
        void * t = mem + eff_addr(op2.mem);
        if(s2 == 4){
            val = *((uint32_t *)t);
        }else if (s2 == 2){
            val = *((uint16_t *)t);
        }else{
            val = *((uint8_t *)t);
        }
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
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint8_t s1 = op1.size, s2 = op2.size;

    uint32_t val;                           /* extract op2 value*/
    if (op2.type == X86_OP_REG){
        val = reg_val(op2.reg);
    }else if(op2.type == X86_OP_IMM){
        val = op2.imm;
    }else if (op2.type == X86_OP_MEM){
        void * t = mem + eff_addr(op2.mem);
        if(s2 == 4){
            val = *((uint32_t *)t);
        }else if (s2 == 2){
            val = *((uint16_t *)t);
        }else{
            val = *((uint8_t *)t);
        }
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
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint8_t s1 = op1.size, s2 = op2.size;

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
        void * t = mem + eff_addr(op1.mem);
        if(s1 == 4){
            val1 = *((uint32_t *)t);
        }else if (s2 == 2){
            val1 = *((uint16_t *)t);
        }else{
            val1 = *((uint8_t *)t);
        }
        b = s1*8;
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
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint8_t s1 = op1.size, s2 = op2.size;

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
        uint32_t * val1;
        void * t = mem + eff_addr(op1.mem);
        if(s1 == 4){
            *val1 = *((uint32_t *)t);
        }else if (s1 == 2){
            *val1 = *((uint16_t *)t);
        }else{
            *val1 = *((uint8_t *)t);
        }
        b = s1*8;
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
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint8_t s1 = op1.size, s2 = op2.size;

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
        uint32_t * val1;
        void * t = mem + eff_addr(op1.mem);
        if(s1 == 4){
            *val1 = *((uint32_t *)t);
        }else if (s1 == 2){
            *val1 = *((uint16_t *)t);
        }else{
            *val1 = *((uint8_t *)t);
        }
        b = s1 * 4;
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
    eip += insn->size;
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

/**
 *  RET. Return from procedure.
 *
 *  Opcode 0xC2, 0xC3, 0xCA, 0xCB.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int ret_i(cs_insn *insn){
    /* Pop return address from Stack */
    eip = read32(esp);
    esp+=4;
    cs_x86 x86 = insn->detail->x86;

    if (x86.op_count == 1){
        esp += x86.operands[0].imm;
    }

    /* Adjust privilege based on Opcode*/

    return 0;
}

/**
 *  CALL. Call procedure.
 *
 *  Opcode 0xE8, 0x9A, 0xFF /2, 0xFF /3.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  Flags may change if a task switch occurs.
 *
 *  @param insn instruction struct that stores all the information.
 */
int call_i(cs_insn *insn){
    /* Next EIP, the one to push */
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;
    if(op1.type == X86_OP_REG){
        val = reg_val(op1.reg);
    }else if(op1.type == X86_OP_IMM){
        val = op1.imm;
    }else{
        val = *((uint32_t *)(mem+eff_addr(op1.mem)));
    }
    /* Push old EIP*/
    esp -=4;
    write32(esp, eip);
    /* New EIP is the operand of the instruction */
    eip = val;

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
    eip += insn->size;
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
    eip += insn->size;
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
    eip += insn->size;
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
    eip += insn->size;
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
    eip += insn->size;
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
    eip += insn->size;
    complement_Flag(CF);
    return 0;
}

/**
 *  CMP. Compare Two Operands.
 *
 *  Opcodes 0x3C, 0x3D, 0x80 /7, 0x81 /7, 0x83 /7, 0x38, 0x39, 0x3A, 0x3B.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  OF, SF, ZF, AF, PF, and CF as described in Appendix C
 *
 *  @param insn instruction struct that stores all the information.
 */
int cmp_i(cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];
    /* Get operands sizes */
    uint8_t s1 = op1.size, s2 = op2.size;
    
    /* Operand 1 and 2 value */
    uint32_t val1, val2;
    /* Result */
    uint32_t res;
    if (op2.type == X86_OP_REG){
        val2 = reg_val(op2.reg);
    }else if(op2.type == X86_OP_IMM){
        /* If imm operand is 1 byte, we need to sign-extend it */
        if (s2 == 1){
            if (s1 == 2){
                val2 = sign_extend8_16(op2.imm);
            }else if(s1 == 4){
                val2 = sign_extend8_32(op2.imm);
            }else{
                val2 = op2.imm;
            }
        }else{
            val2 = op2.imm;
        }
    }else if (op2.type == X86_OP_MEM){
        val2 = *((uint32_t *)(mem + eff_addr(op2.mem)));
    }

    if (op1.type == X86_OP_REG){
        void * p = regs[op1.reg];
        uint8_t base = regs_size[op1.reg];
        switch(base){
            case 0x8:
                val1= *((uint8_t *) p);
                res = val1- (val2 & 0xFF);
                break;
            case 0x10:
                val1 = *((uint16_t *) p);
                res = val1 - (val2 & 0xFFFF);
                break;
            case 0x20:
                val1 = *((uint32_t *) p);    
                res = val1 - val2;
                break;
            default:
                val1 = *((uint32_t *) p);    
                res = val1 - val2;
                break;
        }
    }else if (op1.type == X86_OP_MEM){
        val1 = *((uint32_t *)(mem+eff_addr(op1.mem)));
        res = val1 - val2;
    }

    (val1 < val2)?set_Flag(CF):clear_Flag(CF);
    overflow(val1, val2, res, 32)?set_Flag(OF):clear_Flag(OF);
    sign(res, 0x20)?set_Flag(SF):clear_Flag(SF);
    (!res)?set_Flag(ZF):clear_Flag(ZF);
    adjust(val1, val2, res)?set_Flag(AF):clear_Flag(AF);
    parity(res)?set_Flag(PF):clear_Flag(PF);
       
    return 0;
}

/**
 *  DEC. Decrement by 1.
 *
 *  Opcodes 0xFE /1, 0xFF /1, 0x48-0x4F.
 *
 *  Segment Exceptions.
 *
 *  OF, SF, AF, ZF and PF as described in Appendix C.
 *
 *  @param insn instruction struct that stores all the information.
 */
int dec_i(cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t res = 0x0, oper1 = 0x0;
    if (op1.type == X86_OP_REG){
        void *p = regs[op1.reg];
        uint8_t base = regs_size[op1.reg];
        switch(base){
            case 0x08:
                oper1=*((uint8_t*)p);
                *((uint8_t*)p)-=1;
                res = *((uint8_t*)p);
                break;
            case 0x10:
                oper1=*((uint16_t*)p);
                *((uint16_t*)p)-=1;
                res = *((uint16_t*)p);
                break;
            case 0x20:
                oper1=*((uint32_t*)p);
                *((uint32_t*)p)-=1;
                res = *((uint32_t*)p);
                break;
        }
    }else if (op1.type == X86_OP_MEM){
        uint32_t* p = ((uint32_t *)(mem + eff_addr(op1.mem)));
        oper1 = *p;
        *p-=1;
        res = *p;
    }

    /* Flags as described on Appendix C */
    /* CF formula for substracting */
    (oper1 < 1)?set_Flag(CF):clear_Flag(CF);
    overflow(oper1, 1, res, 32)?set_Flag(OF):clear_Flag(OF);
    sign(res, 0x20)?set_Flag(SF):clear_Flag(SF);
    (!res)?set_Flag(ZF):clear_Flag(ZF);
    adjust(oper1, 1, res)?set_Flag(AF):clear_Flag(AF);
    parity(res)?set_Flag(PF):clear_Flag(PF);

    return 0;
}


/**
 *  MOV. Move data.
 *
 *  Opcodes 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0xA0, 0xA1, 0xA2, 0xA3, 0xB0, 0xB8, 0xC6, 0xC7.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int mov_i(cs_insn *insn){
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

/**
 *  MOVSX. Move with Sign Extend.
 *
 *  Opcodes 0x0F BE, 0x0F BF.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int movsx_i(cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    /* Obtain operands sizes */
    uint8_t s1 = op1.size, s2 = op2.size;

    uint32_t val;
    /* Obtain SRC value */
    if (op2.type == X86_OP_REG){
        val = reg_val(op2.reg);
    }else if(op2.type == X86_OP_IMM){
        val = op2.imm;
    }else if (op2.type == X86_OP_MEM){
        val = *((uint32_t *)(mem + eff_addr(op2.mem)));
    }

    if (op1.type != X86_OP_REG){return -1;} /* DEST can only be a register */
    /* Obtain pointer to DEST register */
    void * p = regs[op1.reg];
    if (s1 == 4){
        /* 32bit DEST register */
        if (s2 == 2){
            /* 16bit SRC value */
            *((uint32_t *)p) = sign_extend16_32((uint16_t) val);
        }else if (s2 == 1){
            /* 8bit SRC value */
            *((uint32_t *)p) = sign_extend8_32((uint8_t) val);
        }
    }else{
        /* 16bit DEST register */
        /* Store in DEST sign-extended SRC */
        *((uint16_t *)p) = sign_extend8_16((uint16_t) val);
    }

    return 0;
}

/**
 *  MOVZX. Move with Zero Extend.
 *
 *  Opcodes 0x0F B6, 0x0F B7.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int movzx_i(cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    /* Obtain operands sizes */
    uint8_t s1 = op1.size, s2 = op2.size;

    uint32_t val;
    /* Obtain SRC value */
    if (op2.type == X86_OP_REG){
        val = reg_val(op2.reg);
    }else if(op2.type == X86_OP_IMM){
        val = op2.imm;
    }else if (op2.type == X86_OP_MEM){
        val = *((uint32_t *)(mem + eff_addr(op2.mem)));
    }

    if (op1.type != X86_OP_REG){return -1;} /* DEST can only be a register */
    /* Obtain pointer to DEST register */
    void * p = regs[op1.reg];
    if (s1 == 4){
        /* 32bit DEST register */
        if (s2 == 2){
            /* 16bit SRC value */
            *((uint32_t *)p) = zero_extend16_32((uint16_t) val);
        }else{
            /* 8bit SRC value */
            *((uint32_t *)p) = zero_extend8_32((uint8_t) val);
        }
    }else{
        /* 16bit DEST register */
        /* Store in DEST sign-extended SRC */
        *((uint16_t *)p) = zero_extend8_16((uint16_t) val);
    }

    return 0;
}

/**
 *  PUSH. Push operand onto the stack.
 *
 *  Opcodes 0x50+/r, 0x6A, 0x68, 0x0E, 0x16, 0x1E, 0x06, 0x0F A0, 0x0F A8. 
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int push_i(cs_insn*insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];

    uint32_t val;
    uint8_t s = op1.size;

    if (op1.type == X86_OP_REG){
        val = reg_val(op1.reg);
    }else if(op1.type == X86_OP_IMM){
        val = op1.imm;
    }else if (op1.type == X86_OP_MEM){
        val = *((uint32_t *)(mem + eff_addr(op1.mem)));
    }

    
    if (s == 2){
        esp-=2;
        write16(esp, (uint16_t)val);
    }else{ /* Operands can be only 16b (2B) or 32b (4B) */
        esp-=4;
        write32(esp, val);
    }

    return 0;

}

/**
 *  POP. Pop a word from the stack.
 *
 *  Opcodes 0x8F /0, 0x58 + r, 0x1F, 0x07, 0x17, 0x0F A1, 0x0F A9. 
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int pop_i(cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint8_t s = op1.size;

    if (op1.type == X86_OP_REG){
        void * p = regs[op1.reg];
        if (s == 2){
            *((uint16_t *)p)=read16(esp);
            esp+=2;
        }else{
            *((uint32_t *)p)=read32(esp);
            esp+=4;
        }
    }else if(op1.type == X86_OP_MEM){
        uint32_t addr = eff_addr(op1.mem);
        write32(addr, read32(esp));
        esp+=4;
    }

}


/**
 *  SUB. Integer substraction.
 *
 *  Opcodes 0x2C, 0x2D, 0x80 /5, 0x81 /5, 0x83 /5, 0x28, 0x29, 0x2A, 0x2B. 
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  OF, SF, ZF, AF, PF and CF as described on Appendix C.
 *
 *  @param insn instruction struct that stores all the information.
 */
int sub_i(cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];
    /* Get operands sizes */
    uint8_t s1 = op1.size, s2 = op2.size;
    
    /* Operand 1 and 2 value */
    uint32_t val1, val2, res;
    if (op2.type == X86_OP_REG){
        val2 = reg_val(op2.reg);
    }else if(op2.type == X86_OP_IMM){
        /* If imm operand is 1 byte, we need to sign-extend it */
        if (s2 == 1){
            if (s1 == 2){
                val2 = sign_extend8_16(op2.imm);
            }else if(s1 == 4){
                val2 = sign_extend8_32(op2.imm);
            }else{
                val2 = op2.imm;
            }
        }else{
            val2 = op2.imm;
        }
    }else if (op2.type == X86_OP_MEM){
        val2 = *((uint32_t *)(mem + eff_addr(op2.mem)));
    }

    if (op1.type == X86_OP_REG){
        void * p = regs[op1.reg];
        uint8_t base = regs_size[op1.reg];
        switch(base){
            case 0x8:
                val1 = *((uint8_t *) p);
                *((uint8_t *) p) -= (val2 & 0xFF);
                res = *((uint8_t *) p);
                break;
            case 0x10:
                val1 = *((uint16_t *) p);
                *((uint16_t *) p) -= (val2 & 0xFFFF);
                res = *((uint16_t *) p);
                break;
            case 0x20:
                val1 = *((uint32_t *) p);
                *((uint32_t *) p) -= val2;
                res = *((uint32_t *) p);
                break;
            default:
                val1 = *((uint32_t *) p);
                *((uint32_t *) p) -= val2;
                res = *((uint32_t *) p);
                break;
        }
    }else if (op1.type == X86_OP_MEM){
        uint32_t *p = ((uint32_t *)(mem+eff_addr(op1.mem)));
        val1 = *p;
        *p -= val2;
        res = *p;
    }
    
    (val1 < val2)?set_Flag(CF):clear_Flag(CF);
    overflow(val1, val2, res, 32)?set_Flag(OF):clear_Flag(OF);
    sign(res, 0x20)?set_Flag(SF):clear_Flag(SF);
    (!res)?set_Flag(ZF):clear_Flag(ZF);
    adjust(val1, val2, res)?set_Flag(AF):clear_Flag(AF);
    parity(res)?set_Flag(PF):clear_Flag(PF);


    return 0;
}

/**
 *  OR. Logical Inclusive OR.
 *
 *  Opcodes 0x0C, 0x0D, 0x80 /1, 0x81 /1, 0x83 /1, 0x08, 0x09, 0x0A, 0x0B. 
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  OF = 0, CF = 0.  SF, ZF, AF, and PF as described on Appendix C.
 *
 *  @param insn instruction struct that stores all the information.
 */
int or_i(cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint8_t s1 = op1.size, s2 = op2.size;

    uint32_t val;
    if (op2.type == X86_OP_REG){
        val = reg_val(op2.reg);
    }else if(op2.type == X86_OP_IMM){
        val = op2.imm;
    }else if (op2.type == X86_OP_MEM){
        void * t = mem + eff_addr(op2.mem);
        if(s2 == 4){
            val = *((uint32_t *)t);
        }else if (s2 == 2){
            val = *((uint16_t *)t);
        }else{
            val = *((uint8_t *)t);
        }
    }
    uint32_t res;
    uint8_t base;
    if (op1.type == X86_OP_REG){
        void * p = regs[op1.reg];
        base = regs_size[op1.reg];
        switch(base){
            case 0x8:
                *((uint8_t *) p)|= (val & 0xFF);
                res = (uint32_t)*((uint8_t *) p);
                break;
            case 0x10:
                *((uint16_t *) p) |= (val & 0xFFFF);
                res = (uint32_t)*((uint16_t *) p);
                break;
            case 0x20:
                *((uint32_t *) p) |= val;
                res = *((uint32_t *) p);
                break;
            default:
                *((uint32_t *) p) |= val;
                res = *((uint32_t *) p);
                break;
        }
    }else if (op1.type == X86_OP_MEM){
        void * t = mem + eff_addr(op2.mem);
        if(s2 == 4){
           *((uint32_t *)t) |= val;
           res = *((uint32_t *)t);
        }else if (s2 == 2){
            *((uint16_t *)t) |= val;
            res = *((uint16_t *)t);
        }else{
            *((uint8_t *)t) |= val;
            res = *((uint8_t *)t);
        }
        base = s1 * 4;
    }
    //op1 cant be X86_OP_IMM
    sign(res, base)?set_Flag(SF):clear_Flag(SF);
    zero(res)?set_Flag(ZF):clear_Flag(ZF);
    parity(res)?set_Flag(PF):clear_Flag(PF);
    clear_Flag(CF);
    clear_Flag(OF);
    return 0;
}

/**
 *  XOR. Logical Exclusive XOR.
 *
 *  Opcodes 0x34, 0x34, 0x80 /6, 0x81 /6, 0x83 /6, 0x30, 0x31, 0x32, 0x33. 
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  OF = 0, CF = 0.  SF, ZF, AF, and PF as described on Appendix C.
 *
 *  @param insn instruction struct that stores all the information.
 */
int xor_i(cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint8_t s1 = op1.size, s2 = op2.size;

    uint32_t val;
    if (op2.type == X86_OP_REG){
        val = reg_val(op2.reg);
    }else if(op2.type == X86_OP_IMM){
        val = op2.imm;
    }else if (op2.type == X86_OP_MEM){
        void * t = mem + eff_addr(op2.mem);
        if(s2 == 4){
            val = *((uint32_t *)t);
        }else if (s2 == 2){
            val = *((uint16_t *)t);
        }else{
            val = *((uint8_t *)t);
        }
    }
    uint32_t res;
    uint8_t base;
    if (op1.type == X86_OP_REG){
        void * p = regs[op1.reg];
        base = regs_size[op1.reg];
        switch(base){
            case 0x8:
                *((uint8_t *) p)^= (val & 0xFF);
                res = (uint32_t)*((uint8_t *) p);
                break;
            case 0x10:
                *((uint16_t *) p) ^= (val & 0xFFFF);
                res = (uint32_t)*((uint16_t *) p);
                break;
            case 0x20:
                *((uint32_t *) p) ^= val;
                res = res = *((uint32_t *) p);
                break;
            default:
                *((uint32_t *) p) ^= val;
                res = *((uint32_t *) p);
                break;
        }
    }else if (op1.type == X86_OP_MEM){
        void * t = mem + eff_addr(op2.mem);
        if(s2 == 4){
           *((uint32_t *)t) ^= val;
           res = *((uint32_t *)t);
        }else if (s2 == 2){
            *((uint16_t *)t) ^= val;
            res = *((uint16_t *)t);
        }else{
            *((uint8_t *)t) ^= val;
            res = *((uint8_t *)t);
        }
        base = s1 * 4;
    }
    //op1 cant be X86_OP_IMM
    sign(res, base)?set_Flag(SF):clear_Flag(SF);
    zero(res)?set_Flag(ZF):clear_Flag(ZF);
    parity(res)?set_Flag(PF):clear_Flag(PF);
    clear_Flag(CF);
    clear_Flag(OF);


    return 0;
}

int cmps_i (cs_insn *insn){
    eip += insn->size;
}

int cwd_i (cs_insn *insn){
    eip += insn->size;
} 
int daa_i (cs_insn *insn){
    eip += insn->size;
} 
int das_i (cs_insn *insn){
    eip += insn->size;
} 
int div_i (cs_insn *insn){
    eip += insn->size;
} 
int hlt_i (cs_insn *insn){
    eip += insn->size;
} 
int idiv_i (cs_insn *insn){
    eip += insn->size;
} 
int imul_i (cs_insn *insn){
    eip += insn->size;
} 
int in_i (cs_insn *insn){
    eip += insn->size;
} 
int inc_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];

    uint8_t s1 = op1.size;

    uint32_t res = 0x0, oper1 = 0x0;
    if (op1.type == X86_OP_REG){
        void *p = regs[op1.reg];
        uint8_t base = regs_size[op1.reg];
        switch(base){
            case 0x08:
                oper1=*((uint8_t*)p);
                *((uint8_t*)p)+=1;
                res = *((uint8_t*)p);
                break;
            case 0x10:
                oper1=*((uint16_t*)p);
                *((uint16_t*)p)+=1;
                res = *((uint16_t*)p);
                break;
            case 0x20:
                oper1=*((uint32_t*)p);
                *((uint32_t*)p)+=1;
                res = *((uint32_t*)p);
                break;
        }
    }else if (op1.type == X86_OP_MEM){
        void * t = (mem + eff_addr(op1.mem));
        if (s1 == 4){
            uint32_t * p = (uint32_t *)t;
            oper1 = *p;
            *p+=1;
            res = *p;
        }else if (s1 == 2){
            uint16_t * p = (uint16_t *)t;
            oper1 = *p;
            *p+=1;
            res = *p;
        }else{
            uint8_t * p = (uint8_t *)t;
            oper1 = *p;
            *p+=1;
            res = *p;
        }
    }

    /* Flags as described on Appendix C */
    /* CF formula for adding */
    (oper1 > res)?set_Flag(CF):clear_Flag(CF);
    overflow(oper1, 1, res, s1*8)?set_Flag(OF):clear_Flag(OF);
    sign(res, s1*8)?set_Flag(SF):clear_Flag(SF);
    (!res)?set_Flag(ZF):clear_Flag(ZF);
    adjust(oper1, 1, res)?set_Flag(AF):clear_Flag(AF);
    parity(res)?set_Flag(PF):clear_Flag(PF);

    return 0;
} 
int int_i (cs_insn *insn){
    eip += insn->size;
} 
int into_i (cs_insn *insn){
    eip += insn->size;
} 
int iret_i (cs_insn *insn){
    eip += insn->size;
}

/**
 *  JA. Jump  if above (CF = 0 and ZF = 0).
 *
 *  Opcode 0x77.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int ja_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (!test_Flag(CF) && !test_Flag(ZF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;

} 

/**
 *  JAE. Jump  if above or equal(CF = 0).
 *
 *  Opcode 0x73.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jae_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (!test_Flag(CF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
}

/**
 *  JB. Jump  if below (CF = 1).
 *
 *  Opcode 0x72.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jb_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (test_Flag(CF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
} 

/**
 *  JBE. Jump  if below or equal (CF = 1 or ZF = 1).
 *
 *  Opcode 0x76.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jbe_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (test_Flag(CF) && test_Flag(ZF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
}

/**
 *  JC. Jump  if carry (CF = 1).
 *
 *  Opcode 0x72.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jc_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (test_Flag(CF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
} 

/**
 *  JCXZ. Jump  if CX register is 0.
 *
 *  Opcode 0xE3.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jcxz_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (!(ecx & 0xFFFF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
} 

/**
 *  JECXZ. Jump  if ECX register is 0.
 *
 *  Opcode 0xE3.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jecxz_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (!ecx){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
} 

/**
 *  JE. Jump  if equal (and ZF = 1).
 *
 *  Opcode 0x74.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int je_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (test_Flag(ZF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
} 

/**
 *  JZ. Jump  if equal (ZF = 1).
 *
 *  Opcode 0x74.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jz_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (test_Flag(ZF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
} 

/**
 *  JG. Jump  if greater (ZF = 0 and SF = 0F).
 *
 *  Opcode 0x7F.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jg_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (!test_Flag(ZF) && test_Flag(SF) == test_Flag(OF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
} 

/**
 *  JGE. Jump  if greater or equal (SF = OF).
 *
 *  Opcode 0x7D.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jge_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (test_Flag(SF) == test_Flag(OF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
} 

/**
 *  JL. Jump  if less (SF != OF).
 *
 *  Opcode 0x7C.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jl_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (test_Flag(SF) != test_Flag(OF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
} 

/**
 *  JLE. Jump  if less or equal (SF != OF or ZF = 1).
 *
 *  Opcode 0x7E.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jle_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (test_Flag(ZF) && test_Flag(SF) != test_Flag(OF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
}

/**
 *  JMP. Jump .
 *
 *  Opcodes 0xEB, 0xE9, 0xFF /4, 0xEA, 0xFF /5, .
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  Flags may change if a task switch occurs.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jmp_i (cs_insn *insn){
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    
    if(op1.type == X86_OP_REG){
        val = reg_val(op1.reg);
    }else if(op1.type == X86_OP_IMM){
        val = op1.imm;
    }else{
        val = *((uint32_t *)(mem+eff_addr(op1.mem)));
    }
    eip = val;
    
    return 0;
} 

/**
 *  JNA. Jump  if not above (CF = 1 or ZF = 1).
 *
 *  Opcode 0x76.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jna_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (test_Flag(CF) || test_Flag(ZF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
} 

/**
 *  JA. Jump  if not above or equal (CF = 1).
 *
 *  Opcode 0x72.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jnae_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (test_Flag(CF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
}

/**
 *  JNB. Jump  if not below (CF = 0).
 *
 *  Opcode 0x73.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jnb_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (!test_Flag(CF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
} 

/**
 *  JA. Jump  if not below or equal (CF = 0 and ZF = 0).
 *
 *  Opcode 0x77.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jnbe_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (!test_Flag(CF) && !test_Flag(ZF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
}

/**
 *  JNC. Jump if not carry (CF = 0).
 *
 *  Opcode 0x73.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jnc_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (!test_Flag(CF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
} 

/**
 *  JNE. Jump  if above (CF = 0 and ZF = 0).
 *
 *  Opcode 0x77.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jne_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (!test_Flag(ZF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
} 

/**
 *  JNG. Jump if not greater (ZF = 1 or SF != OF).
 *
 *  Opcode 0x7E.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jng_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (test_Flag(ZF) || test_Flag(SF) != test_Flag(OF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
} 

/**
 *  JNGE. Jump if not greater or equal (SF != OF).
 *
 *  Opcode 0x7C.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jnge_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (test_Flag(SF) != test_Flag(OF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
}


/**
 *  JNL. Jump if not less (SF = OF).
 *
 *  Opcode 0x7D.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jnl_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (test_Flag(SF) == test_Flag(OF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
} 

/**
 *  JNLE. Jump if not less or equal (SF = OF and ZF = 0).
 *
 *  Opcode 0x7F.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jnle_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (!test_Flag(ZF) && test_Flag(SF) == test_Flag(OF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
} 

/**
 *  JNO. Jump if not overflow (OF = 0).
 *
 *  Opcode 0x71.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jno_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (!test_Flag(OF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
} 

/**
 *  JNL. Jump if not parity (PF = 0).
 *
 *  Opcode 0x7B.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jnp_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (!test_Flag(PF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
} 

/**
 *  JNL. Jump if not sign (SF = 0).
 *
 *  Opcode 0x79.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jns_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (!test_Flag(SF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
} 

/**
 *  JNZ. Jump if not zero (ZF = 0).
 *
 *  Opcode 0x75.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jnz_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (!test_Flag(ZF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
} 

/**
 *  JO. Jump if overflow (OF = 1).
 *
 *  Opcode 0x70.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jo_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (test_Flag(OF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
}

/**
 *  JP. Jump if parity (PF = 1).
 *
 *  Opcode 0x7A.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jp_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (test_Flag(PF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
} 

/**
 *  JPE. Jump if parity even (PF = 1).
 *
 *  Opcode 0x7A.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jpe_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (test_Flag(PF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
} 

/**
 *  JPO. Jump if parity odd (PF = 0).
 *
 *  Opcode 0x7B.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int jpo_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (!test_Flag(PF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
} 

/**
 *  JS. Jump if sign (SF = 1).
 *
 *  Opcode 0x78.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int js_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint32_t val;

    if (test_Flag(SF)){
        if(op1.type == X86_OP_REG){
            val = reg_val(op1.reg);
        }else if(op1.type == X86_OP_IMM){
            val = op1.imm;
        }else{
            val = *((uint32_t *)(mem+eff_addr(op1.mem)));
        }
        eip = val;
    }
    return 0;
} 

int lahf_i (cs_insn *insn){
    eip += insn->size;
} 
int lar_i (cs_insn *insn){
    eip += insn->size;
} 
int lcall_i (cs_insn *insn){
    eip += insn->size;
} 
int lds_i (cs_insn *insn){
    eip += insn->size;
} 

/**
 *  LEA. Load Effective Address.
 *
 *  Opcode 0x8D.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int lea_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];
    uint8_t s = x86.prefix[0] == 0x66 ? 2 : 4;

    if(op1.type != X86_OP_REG || op2.type != X86_OP_MEM)
        return -1;

    uint32_t addr = eff_addr(op2.mem);
    if (2 == s){
        /* Operand size override -> prefix = 0x66 */
        uint16_t * p = (uint16_t *)regs[op1.reg];
        *p = (uint16_t)(addr & 0xFFFF);
        
    }else{
        /* Usual operand size */
        uint32_t * p = (uint32_t *)regs[op1.reg];
        *p = addr;
    }
    return 0;


} 
int leave_i (cs_insn *insn){
    eip += insn->size;
} 
int les_i (cs_insn *insn){
    eip += insn->size;
} 
int lfs_i (cs_insn *insn){
    eip += insn->size;
} 
int lgdt_i (cs_insn *insn){
    eip += insn->size;
} 
int lgs_i (cs_insn *insn){
    eip += insn->size;
} 
int lidt_i (cs_insn *insn){
    eip += insn->size;
} 
int lldt_i (cs_insn *insn){
    eip += insn->size;
} 
int lmsw_i (cs_insn *insn){
    eip += insn->size;
} 
int lods_i (cs_insn *insn){
    eip += insn->size;
} 
int loop_i (cs_insn *insn){
    eip += insn->size;
} 
int loope_i (cs_insn *insn){
    eip += insn->size;
} 
int loopne_i (cs_insn *insn){
    eip += insn->size;
} 
int loopnz_i (cs_insn *insn){
    eip += insn->size;
} 
int loopz_i (cs_insn *insn){
    eip += insn->size;
} 
int lsl_i (cs_insn *insn){
    eip += insn->size;
} 
int ltr_i (cs_insn *insn){
    eip += insn->size;
} 
int movs_i (cs_insn *insn){
    eip += insn->size;
} 

int mul_i (cs_insn *insn){
    eip += insn->size;
} 
int neg_i (cs_insn *insn){
    eip += insn->size;
} 
int nop_i (cs_insn *insn){
    eip += insn->size;
} 
int not_i (cs_insn *insn){
    eip += insn->size;
} 
int out_i (cs_insn *insn){
    eip += insn->size;
} 
int outs_i (cs_insn *insn){
    eip += insn->size;
} 
int popa_i (cs_insn *insn){
    eip += insn->size;
} 
int popf_i (cs_insn *insn){
    eip += insn->size;
} 
int pusha_i (cs_insn *insn){
    eip += insn->size;
} 
int pushf_i (cs_insn *insn){
    eip += insn->size;
} 
int rcl_i (cs_insn *insn){
    eip += insn->size;
} 
int rcr_i (cs_insn *insn){
    eip += insn->size;
} 
int rol_i (cs_insn *insn){
    eip += insn->size;
} 
int ror_i (cs_insn *insn){
    eip += insn->size;
} 
int sahf_i (cs_insn *insn){
    eip += insn->size;
} 
int sbb_i (cs_insn *insn){
    eip += insn->size;
} 
int scas_i (cs_insn *insn){
    eip += insn->size;
} 
int seta_i (cs_insn *insn){
    eip += insn->size;
} 
int setae_i (cs_insn *insn){
    eip += insn->size;
} 
int setb_i (cs_insn *insn){
    eip += insn->size;
} 
int setbe_i (cs_insn *insn){
    eip += insn->size;
} 
int setc_i (cs_insn *insn){
    eip += insn->size;
} 
int sete_i (cs_insn *insn){
    eip += insn->size;
} 
int setg_i (cs_insn *insn){
    eip += insn->size;
} 
int setge_i (cs_insn *insn){
    eip += insn->size;
} 
int setl_i (cs_insn *insn){
    eip += insn->size;
} 
int setle_i (cs_insn *insn){
    eip += insn->size;
} 
int setna_i (cs_insn *insn){
    eip += insn->size;
} 
int setnae_i (cs_insn *insn){
    eip += insn->size;
} 
int setnb_i (cs_insn *insn){
    eip += insn->size;
} 
int setnbe_i (cs_insn *insn){
    eip += insn->size;
} 
int setnc_i (cs_insn *insn){
    eip += insn->size;
} 
int setne_i (cs_insn *insn){
    eip += insn->size;
} 
int setng_i (cs_insn *insn){
    eip += insn->size;
} 
int setnge_i (cs_insn *insn){
    eip += insn->size;
} 
int setnl_i (cs_insn *insn){
    eip += insn->size;
} 
int setnle_i (cs_insn *insn){
    eip += insn->size;
} 
int setno_i (cs_insn *insn){
    eip += insn->size;
} 
int setnp_i (cs_insn *insn){
    eip += insn->size;
} 
int setns_i (cs_insn *insn){
    eip += insn->size;
} 
int seto_i (cs_insn *insn){
    eip += insn->size;
} 
int setp_i (cs_insn *insn){
    eip += insn->size;
} 
int setpe_i (cs_insn *insn){
    eip += insn->size;
} 
int setpo_i (cs_insn *insn){
    eip += insn->size;
} 
int sets_i (cs_insn *insn){
    eip += insn->size;
} 

int sal_i(cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint8_t s1 = op1.size, s2 = op2.size;
    uint8_t count;
    if (op2.type == X86_OP_IMM){
        count = op2.imm;
    }else{
        count = *((uint8_t *)regs[op2.reg]);
    }
    count %= s1*8;

    if (!count){return 0;}

    uint32_t res;

    if(op1.type != X86_OP_REG){return -1;}
    else{
        if (s1 == 1){
            uint8_t * p = (uint8_t *)regs[op1.reg];
            (*p >> ((s1*8) - count)) & 1 ? set_Flag(CF) : clear_Flag(CF);
            *p <<= count;
            res = *p;
        }else if(s1 == 2){
            uint16_t * p = (uint16_t *)regs[op1.reg];
            (*p >> ((s1*8) - count)) & 1 ? set_Flag(CF) : clear_Flag(CF);
            *p <<= count;
            res = *p;
        }else{
            uint32_t * p = (uint32_t *)regs[op1.reg];
            (*p >> ((s1*8) - count)) & 1 ? set_Flag(CF) : clear_Flag(CF);
            *p <<= count;
            res = *p;
        }
    }

    if (count == 1) {
        // MSB antes del desplazamiento
        uint32_t msb_before = (res >> ((s1*8) - 1)) & 1;
        // OF = MSB_antes XOR CF_despues
        if (msb_before ^ test_Flag(CF))
            set_Flag(OF);
        else
            clear_Flag(OF);
    } 
    zero(res)?set_Flag(ZF):clear_Flag(ZF);
    parity(res)?set_Flag(PF):clear_Flag(PF);
    sign(res, s1*8)?set_Flag(SF):clear_Flag(SF);

    return 0;
}

int shl_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint8_t s1 = op1.size, s2 = op2.size;
    uint8_t count;
    if (op2.type == X86_OP_IMM){
        count = op2.imm;
    }else{
        count = *((uint8_t *)regs[op2.reg]);
    }
    count %= s1*8;

    if (!count){return 0;}

    uint32_t res;

    if(op1.type != X86_OP_REG){return -1;}
    else{
        if (s1 == 1){
            uint8_t * p = (uint8_t *)regs[op1.reg];
            (*p >> ((s1*8) - count)) & 1 ? set_Flag(CF) : clear_Flag(CF);
            *p <<= count;
            res = *p;
        }else if(s1 == 2){
            uint16_t * p = (uint16_t *)regs[op1.reg];
            (*p >> ((s1*8) - count)) & 1 ? set_Flag(CF) : clear_Flag(CF);
            *p <<= count;
            res = *p;
        }else{
            uint32_t * p = (uint32_t *)regs[op1.reg];
            (*p >> ((s1*8) - count)) & 1 ? set_Flag(CF) : clear_Flag(CF);
            *p <<= count;
            res = *p;
        }
    }

    if (count == 1) {
        // MSB antes del desplazamiento
        uint32_t msb_before = (res >> ((s1*8) - 1)) & 1;
        // OF = MSB_antes XOR CF_despues
        if (msb_before ^ test_Flag(CF))
            set_Flag(OF);
        else
            clear_Flag(OF);
    } 
    zero(res)?set_Flag(ZF):clear_Flag(ZF);
    parity(res)?set_Flag(PF):clear_Flag(PF);
    sign(res, s1*8)?set_Flag(SF):clear_Flag(SF);

    return 0;
} 

int sar_i(cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint8_t s1 = op1.size, s2 = op2.size;
    uint8_t count;
    if (op2.type == X86_OP_IMM){
        count = op2.imm;
    }else{
        count = *((uint8_t *)regs[op2.reg]);
    }
    count %= s1*8;

    if (!count){return 0;}

    uint32_t res;

    if(op1.type != X86_OP_REG){return -1;}
    else{
        if (s1 == 1){
            int8_t * p = (int8_t *)regs[op1.reg];
            (*p >> (count-1))&0x1?set_Flag(CF):clear_Flag(CF);
            *p >>= count;
            res = (uint8_t)*p;
        }else if(s1 == 2){
            int16_t * p = (int16_t *)regs[op1.reg];
            (*p >> (count-1))&0x1?set_Flag(CF):clear_Flag(CF);
            *p >>= count;
            res = (uint16_t)*p;
        }else{
            int32_t * p = (int32_t *)regs[op1.reg];
            (*p >> (count-1))&0x1?set_Flag(CF):clear_Flag(CF);
            *p >>= count;
            res = (uint32_t)*p;
        }
    }

    if (count == 1) {
        clear_Flag(OF);
    }
    zero(res)?set_Flag(ZF):clear_Flag(ZF);
    parity(res)?set_Flag(PF):clear_Flag(PF);
    sign(res, s1*8)?set_Flag(SF):clear_Flag(SF);

    return 0;
}

int shr_i(cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];

    uint8_t s1 = op1.size, s2 = op2.size;
    uint8_t count;
    if (op2.type == X86_OP_IMM){
        count = op2.imm;
    }else{
        count = *((uint8_t *)regs[op2.reg]);
    }
    count %= s1*8;

    if (!count){return 0;}

    uint32_t res;

    if(op1.type != X86_OP_REG){return -1;}
    else{
        if (s1 == 1){
            uint8_t * p = (uint8_t *)regs[op1.reg];
            (*p >> (count-1))&0x1?set_Flag(CF):clear_Flag(CF);
            *p >>= count;
            res = *p;
        }else if(s1 == 2){
            uint16_t * p = (uint16_t *)regs[op1.reg];
            (*p >> (count-1))&0x1?set_Flag(CF):clear_Flag(CF);
            *p >>= count;
            res = *p;
        }else{
            uint32_t * p = (uint32_t *)regs[op1.reg];
            (*p >> (count-1))&0x1?set_Flag(CF):clear_Flag(CF);
            *p >>= count;
            res = *p;
        }
    }

    if (count == 1) {
        // bit más alto antes del desplazamiento
        uint32_t msb = 1u << (s1*8 - 1);
        if (res & msb)
            set_Flag(OF);
        else
            clear_Flag(OF);
    } else {
        // OF indefinido, aquí puedes dejarlo sin tocar o limpiar
        clear_Flag(OF);
    }
    zero(res)?set_Flag(ZF):clear_Flag(ZF);
    parity(res)?set_Flag(PF):clear_Flag(PF);
    sign(res, s1*8)?set_Flag(SF):clear_Flag(SF);

    return 0;
}

int shrd_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    cs_x86_op op2 = x86.operands[1];
    cs_x86_op op3 = x86.operands[2];

    uint8_t s1 = op1.size, s2 = op2.size;

    uint8_t count;
    if(op3.type == X86_OP_IMM){
        count = op3.imm;
    }else{
        /* Is CL */
        uint8_t *p = regs[op3.reg];
        count = *p;
    }

    uint32_t * r32, *rm32;
    uint16_t * r16, *rm16;

    if(op2.type != X86_OP_REG){return 1;} /* Op2 must be a register */
    else{
        if (s2 == 2){
            r16 = regs[op2.reg];
        }else{
            r32 = regs[op2.reg];
        }
    }
    if (op1.type == X86_OP_REG){
        if(s2 == 2){
            rm16 = regs[op1.reg];
        }else{
            rm32 = regs[op1.reg];
        }
    }else{
        /* MEM */
        void * t = (mem+eff_addr(op1.mem));
        if (s2 == 2){
            rm16 = (uint16_t *)t;
        }else{
            rm32 = (uint32_t *)t;
        }
    }

    uint32_t shiftAmt = count % 32;
    if (!shiftAmt){return 0;}
    uint32_t inBits = (s2 == 4)? *r32 : *r16;
    if (shiftAmt >= s2 *8){return 0;}
    else{
        if (s2 == 2){
            (*rm16 >> (shiftAmt-1))&0x1? set_Flag(CF):clear_Flag(CF);
            *rm16 >>= shiftAmt;
            uint16_t temp = *r16 << (s2 * 8 - shiftAmt);
            *rm16 |= temp;
        }else{
            (*rm32 >> (shiftAmt-1))&0x1? set_Flag(CF):clear_Flag(CF);
            *rm32 >>= shiftAmt;
            uint32_t temp = *r32 << (s2 * 8 - shiftAmt);
            *rm32 |= temp;
        }
    }
    uint32_t res;
    res = (s2 == 4)? *rm32 : *r16;
    
    sign(res, s2*8)?set_Flag(SF):clear_Flag(SF);
    zero(res)?set_Flag(ZF):clear_Flag(ZF);
    parity(res)?set_Flag(PF):clear_Flag(PF);

    return 0;
} 


int stc_i (cs_insn *insn){
    eip += insn->size;
} 
int std_i (cs_insn *insn){
    eip += insn->size;
} 
int sti_i (cs_insn *insn){
    eip += insn->size;
} 

/**
 *  STOS. Store String Data.
 *
 *  Opcode 0xAA, 0xAB.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int stos_i (cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    uint8_t s = op1.size;
    /* Address size of 32b, so we use ES:EDI*/
    uint32_t segbase = get_gdt_base(es);
    uint32_t destreg;

    if (s == 1){
        /* STOSB, using AL and moving 1 byte each iteration */
        uint8_t * al = (uint8_t *)&eax;
        /* DestReg = ES:EDI*/
        destreg = segbase + edi;
        /* ES:DestReg := AL */
        *((uint8_t *)(mem+destreg))=*al;
        /* If DF = 0, destreg+=1, else destreg-=1 */
        !test_Flag(DF)?edi++:edi--;
            
    }else if(s == 2){
        /* STOSB, using AX and moving 1 word (2Byte) each iteration */
        uint16_t * ax = (uint16_t *)&eax;
        /* DestReg = ES:EDI*/
        destreg = segbase + edi;
        /* ES:DestReg := AX */
        *((uint16_t *)(mem+destreg))=*ax;
        /* If DF = 0, destreg+=2, else destreg-=2 */
        edi += !test_Flag(DF)?2:-2;
        
    }else{
        /* STOSD, using EAX and moving 1 doubleword (4Byte) each iteration */
        /* DestReg = ES:EDI*/
        destreg = segbase + edi;
        /* ES:DestReg := EAX */
        *((uint32_t *)(mem+destreg))=eax;
        /* If DF = 0, destreg+=4, else destreg-=4 */
        edi+=!test_Flag(DF)?4:-4;
    }
} 

/**
 *  TEST. Logical Compare.
 *
 *  Opcode 0xA8, 0xA9, 0xF6 /0, 0xF7 /0, 0x84, 0x85.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  OF = 0, CF = 0, SF, ZF and PF as described in Appendix C.
 *
 *  @param insn instruction struct that stores all the information.
 */
int test_i (cs_insn *insn){
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

    uint32_t res;
    uint8_t base;
    if (op1.type == X86_OP_REG){
        void * p = regs[op1.reg];
        base = regs_size[op1.reg];
        switch(base){
            case 0x8:
                res = (uint32_t)*((uint8_t *) p) & (val & 0xFF);
                break;
            case 0x10:
                res = (uint32_t)*((uint16_t *) p) & (val & 0xFFFF);
                break;
            case 0x20:
                res = *((uint32_t *) p) & val;
                break;
            default:
                res = *((uint32_t *) p) & val;
                break;
        }
    }else if (op1.type == X86_OP_MEM){
        uint32_t addr = eff_addr(op1.mem);
        res = *((uint32_t *)(mem+addr)) & val;
        base = 0x20;
    }
    //op1 cant be X86_OP_IMM
    sign(res, base)?set_Flag(SF):clear_Flag(SF);
    zero(res)?set_Flag(ZF):clear_Flag(ZF);
    parity(res)?set_Flag(PF):clear_Flag(PF);
    clear_Flag(CF);
    clear_Flag(OF);
} 


int wait_i (cs_insn *insn){
    eip += insn->size;
} 
int xchg_i (cs_insn *insn){
    eip += insn->size;
} 
int xlat_i (cs_insn *insn){
    eip += insn->size;
}

/**
 *  REP. Repeat Following String Operation (STOS).
 *
 *  Opcode 0xF3 AA, 0xF3 AB.
 *
 *  Segment and Page Exceptions in Protected Mode.
 *
 *  No flags affected.
 *
 *  @param insn instruction struct that stores all the information.
 */
int rep_stos_i(cs_insn *insn){
    eip += insn->size;
    cs_x86 x86 = insn->detail->x86;
    cs_x86_op op1 = x86.operands[0];
    /* Operand size in bytes */
    uint8_t s = op1.size;
    /* Address size of 32b, use ES:EDI*/
    uint32_t segbase = get_gdt_base(es);
    uint32_t destreg;

    /* Assuming ECX holds a positive value*/
    if (s == 1){
        /* STOSB, using AL and moving 1 byte each iteration */
        uint8_t * al = (uint8_t *)&eax;
        while(ecx != 0){
            /* DestReg = ES:EDI*/
            destreg = segbase + edi;
            /* ES:DestReg := AL */
            *((uint8_t *)(mem+destreg))=*al;
            /* If DF = 0, destreg+=1, else destreg-=1 */
            !test_Flag(DF)?edi++:edi--;
            ecx--;
        }
    }else if(s == 2){
        /* STOSB, using AX and moving 1 word (2Byte) each iteration */
        uint16_t * ax = (uint16_t *)&eax;
        while(ecx != 0){
            /* DestReg = ES:EDI*/
            destreg = segbase + edi;
            /* ES:DestReg := AX */
            *((uint16_t *)(mem+destreg))=*ax;
            /* If DF = 0, destreg+=2, else destreg-=2 */
            edi += !test_Flag(DF)?2:-2;
            ecx--;
        }
    }else{
        /* STOSD, using EAX and moving 1 doubleword (4Byte) each iteration */
        while(ecx != 0){
            /* DestReg = ES:EDI*/
            destreg = segbase + edi;
            /* ES:DestReg := EAX */
            *((uint32_t *)(mem+destreg))=eax;
            /* If DF = 0, destreg+=4, else destreg-=4 */
            edi+=!test_Flag(DF)?4:-4;
            ecx--;
        }
    }
    return 0;

}

int rep_ins_i(cs_insn *insn){
    //const char *inss[] = { }; 
    //insn.mnemonic;
}

int rep_movs_i(cs_insn *insn){
    //const char *inss[] = { }; 
    //insn.mnemonic;
}

int rep_outs_i(cs_insn *insn){
    //const char *inss[] = { }; 
    //insn.mnemonic;
}

int cpuid_i(cs_insn *insn){
    eip += insn->size;

    /* Max func supported*/
    eax = 0x00000000;
    /* Genu*/
    ebx = 0x756e6547;
    /* ineI*/
    edx = 0x49656e69;
    /* ntel */
    ecx = 0x6c65746e;

    return 0;
}

int cmovne_i(cs_insn *insn){
    if (!test_Flag(ZF)){
        return mov_i(insn);
    }else{
        eip += insn->size;
    }
    return 0;
}








/**
 *  Obtains an effective address using a x86_op_mem argument.
 * 
 * @param m struct containing all addressing variables.
 * 
 * @return effective address.
 */
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
    if (m.segment != X86_REG_INVALID){ //could be ignored (flat arch)
        segment = reg_val(m.segment);
        segment = get_gdt_base((uint16_t)segment);
    }
    
    return (uint32_t)((int32_t)segment + (int32_t)base + (int32_t)index*(int32_t)scale + (int32_t)disp);

}

/**
 * Obtains the result of powing a base to an exponent.
 * 
 * @param b base
 * @param exp exponent
 * 
 * @return result of the operation
 */
uint32_t pow_i(uint32_t b, uint32_t exp){
    uint32_t result = 1;
    while (exp > 0) {
        result *= b;
        exp--;
    }
    return result;
}

/**
 * Writes a word (16 bits) into the address given.
 * 
 * @param addr addres to write on
 * @param value to write
 */
void write16(uint32_t addr, uint16_t value) {
    mem[addr]     = value & 0xFF;
    mem[addr + 1] = (value >> 8) & 0xFF;
}

/**
 * Writes a doubleword (32 bits) into the address given.
 * 
 * @param addr addres to write on
 * @param value to write
 */
void write32(uint32_t addr, uint32_t value) {
    mem[addr]     = value & 0xFF;
    mem[addr + 1] = (value >> 8) & 0xFF;
    mem[addr + 2] = (value >> 16) & 0xFF;
    mem[addr + 3] = (value >> 24) & 0xFF;
}

/**
 * Returns a word (16 bits) from the address given.
 * 
 * @param addr addres to read from
 */
uint16_t read16(uint32_t addr) {
    return mem[addr] | (mem[addr + 1] << 8);
}

/**
 * Returns a doubleword (32 bits) from the address given.
 * 
 * @param addr addres to read from
 */
uint32_t read32(uint32_t addr) {
    return mem[addr] |
           (mem[addr + 1] << 8) |
           (mem[addr + 2] << 16) |
           (mem[addr + 3] << 24);
}

/**
 * Obtains a register value using a reg_id. Reg_id most likely comes from a operand where type is x86_op_reg.
 */
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

/**
 * Sign-extends a 8bit value into 32 bits.
 * 
 * @param v to sign-extend
 * 
 * @return sign-extended value
 */
uint32_t sign_extend8_32(uint8_t v){
    return (v & 0x80)?(v | 0xFFFFFF00):v;
}

/**
 * Sign-extends a 16bit value into 32 bits.
 * 
 * @param v to sign-extend
 * 
 * @return sign-extended value
 */
uint32_t sign_extend16_32(uint16_t v){
    return (v & 0x8000)?(v | 0xFFFF0000):v;
}

/**
 * Sign-extends a 8bit value into 16 bits.
 * 
 * @param v to sign-extend
 * 
 * @return sign-extended value
 */
uint16_t sign_extend8_16(uint8_t v){
    return (v & 0x80)?(v | 0xFF00):v;
}

/**
 * Zero-extends a 8bit value into 32 bits.
 * 
 * @param v to zero-extend
 * 
 * @return zero-extended value
 */
uint32_t zero_extend8_32(uint8_t v){
    return (v & 0x000000FF);
}


/**
 * Zero-extends a 16bit value into 32 bits.
 * 
 * @param v to zero-extend
 * 
 * @return zero-extended value
 */
uint32_t zero_extend16_32(uint16_t v){
    return (v & 0x0000FFFF);
}

/**
 * Zero-extends a 8bit value into 16 bits.
 * 
 * @param v to zero-extend
 * 
 * @return zero-extended value
 */
uint16_t zero_extend8_16(uint8_t v){
    return (v & 0x00FF);
}