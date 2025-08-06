#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include "interface.h"
#include "flags.h"
#include "instr.h"

extern uint32_t eflags;
extern uint8_t * mem;
extern uint32_t eax, edx, esp, esi, eip, cs, ds, fs, ecx, ebx, ebp, edi, ss, es, gs;

int rows, cols;
WINDOW * win_stack, * win_regs, * win_code;

void init_interface(){
    initscr();              // Inicia ncurses
    noecho();               // No mostrar entrada
    cbreak();               // Modo sin buffering
    keypad(stdscr, TRUE);   // Habilita teclas especiales
    

    
    getmaxyx(stdscr, rows, cols);

    // Crear ventanas
    win_regs = newwin(REGISTERS_HEIGHT, cols, 0, 0);
    win_stack = newwin(rows - REGISTERS_HEIGHT, STACK_WIDTH, REGISTERS_HEIGHT, cols - STACK_WIDTH);
    win_code = newwin(rows - REGISTERS_HEIGHT, cols - STACK_WIDTH, REGISTERS_HEIGHT, 0);
}

void draw_regs(){
    werase(win_regs);
    char * buffer = malloc(sizeof(char) * REGS_BUFF_S);
    uint8_t c = test_Flag(CF),p = test_Flag(PF),z = test_Flag(ZF),s = test_Flag(SF),o = test_Flag(OF),a = test_Flag(AF), i = test_Flag(IF);
    snprintf(buffer, REGS_BUFF_S, "EAX : 0x%08x %010u\t\tECX : 0x%08x %010u\t\tEDX : 0x%08x %010u\n EBX : 0x%08x %010u\t\tESI : 0x%08x %010u\t\tEDI : 0x%08x %010u\n ESP : 0x%08x %010u\t\tEBP : 0x%08x %010u\t\tEIP : 0x%08x %010u\n DS  : 0x%08x %010u\t\tFS  : 0x%08x %010u\t\tEFLAGS : 0x%08x\n SS  : 0x%08x %010u\t\tES  : 0x%08x %010u\t\t[ %2s %2s %2s %2s %2s %2s %2s]\n CS  : 0x%08x %010u\t\tGS  : 0x%08x %010u\nEFLAGS : 0x%08x\t", 
        eax,eax,ecx,ecx,edx,edx,ebx,ebx,esi,esi,edi,edi,esp,esp,ebp,ebp,eip,eip,ds,ds,fs,fs,eflags,ss,ss,es,es,c?"CF":"",p?"PF":"",z?"ZF":"",s?"SF":"",o?"OF":"",a?"AF":"",i?"IF":"",cs,cs,gs,gs);
    mvwprintw(win_regs, 1, 1, buffer);
    box(win_regs, 0, 0);
    wrefresh(win_regs);
    free(buffer);
}

void draw_stack(){
    werase(win_stack);
    mvwprintw(win_stack, 1, 5, "STACK: ");
    
    int i=0;
    int lim = rows - REGISTERS_HEIGHT - 3 ;
    /* Substraction with unsigned ints so we must cast to int */
    int j = ((int)(STACK_BOTTOM - esp))/4;    
    do{
        if (j > 0){
            mvwprintw(win_stack, 2 + i, 2, "0x%08x : 0x%08x", esp + 4*i,  *((uint32_t *)(mem+(esp+4*i))));    
        }
        i++;
    }while(i <= lim && i < j);

    box(win_stack, 0, 0);
    wrefresh(win_stack);
}

void draw_code(char ** lineas, int count, int eip_ind){
    werase(win_code);
    box(win_code, 0, 0);

    for (int i = 0; i < (rows - REGISTERS_HEIGHT - 2); i++) {
        if (i < count) {
            if ( eip_ind >= 0 && i == eip_ind){
                wattron(win_code, A_REVERSE);
                mvwprintw(win_code, i + 1, 1, "%s", lineas[i]);
                wattroff(win_code, A_REVERSE);
            }else{
                mvwprintw(win_code, i + 1, 1, "%s", lineas[i]);
            }
            
        }
    }
    wrefresh(win_code);
}

void exit_interface(){
    delwin(win_regs);
    delwin(win_stack);
    delwin(win_code);
    endwin();
}

