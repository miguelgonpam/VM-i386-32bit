#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include "interface.h"
#include "flags.h"
#include "instr.h"

extern uint32_t eflags;
extern uint8_t * mem;
extern uint32_t eax, edx, esp, esi, eip, cs, ds, fs, ecx, ebx, ebp, edi, ss, es, gs;
uint32_t old_eax, old_edx, old_esp, old_esi, old_ecx, old_ebx, old_ebp, old_edi, old_eflags;

int rows, cols;
WINDOW * win_stack, * win_regs, * win_code, * win_cmd;

void init_interface(){
    initscr();              // Inicia ncurses
    echo();               // No mostrar entrada
    cbreak();               // Modo sin buffering
    keypad(stdscr, TRUE);   // Habilita teclas especiales
    
    old_eax = eax;
    old_edx = edx;
    old_esp = esp;
    old_esi = esi;
    old_ecx = ecx;
    old_ebx = ebx;
    old_ebp = ebp;
    old_edi = edi;
    old_eflags = eflags;
    
    getmaxyx(stdscr, rows, cols);

    // Crear ventanas
    win_regs = newwin(H_REGS, cols, 0, 0);
    win_stack = newwin(rows - H_REGS, W_STACK, H_REGS, cols - W_STACK);
    win_code = newwin(rows - H_REGS-5, cols - W_STACK, H_REGS, 0);
    win_cmd = newwin(H_CMD, cols-W_STACK, rows-H_CMD, 0);
    scrollok(win_cmd, TRUE); 
    scrollok(win_code, TRUE);
    scrollok(win_stack, TRUE);
    refresh();
}

void cmd_get_str(char * str, char * txt,size_t size, uint8_t c){
    werase(win_cmd);
    
    if (c){
        mvwprintw(win_cmd, 1, 1, "Wrong format, %s", txt);
    }else{
        mvwprintw(win_cmd, 1, 1, "%s", txt);
    }
    
    mvwprintw(win_cmd, 2, 1, ">>> ");
    wclrtoeol(win_cmd); // limpia la línea
    wrefresh(win_cmd);

    wgetnstr(win_cmd, str, size); // espera input del usuario

    box(win_cmd, 0, 0);
    wrefresh(win_cmd);
}

void draw_cmd(){
    werase(win_cmd);
    
    box(win_cmd, 0, 0);
    wrefresh(win_cmd);
}

void draw_regs(){
    werase(win_regs);
    char * buffer = malloc(sizeof(char) * REGS_BUFF_S);
    uint8_t c = test_Flag(CF),p = test_Flag(PF),z = test_Flag(ZF),s = test_Flag(SF),o = test_Flag(OF),a = test_Flag(AF), i = test_Flag(IF);

    uint8_t space = (cols-27*3-4)/2;
    space = space > 20?20:space;
    //mvwprintw(win_regs, 1, 2, "EAX : 0x%08x %010u%*sECX : 0x%08x %010u%*sESI : 0x%08x %010u\n", eax,eax, space,"", ecx,ecx,space,"",esi,esi);    
    //mvwprintw(win_regs, 2, 2, "EDX : 0x%08x %010u%*sEBX : 0x%08x %010u%*sEDI : 0x%08x %010u\n", edx,edx, space,"", ebx,ebx,space,"",edi,edi);
    //mvwprintw(win_regs, 3, 2, "ESP : 0x%08x %010u%*sEIP : 0x%08x %010u%*sEBP : 0x%08x %010u\n", esp,esp, space,"",eip,eip,space,"",ebp,ebp);
    //mvwprintw(win_regs, 4, 2, "DS  : 0x%08x %010u%*sFS  : 0x%08x %010u%*sEFLAGS : 0x%08x\n", ds,ds, space,"",fs,fs,space,"", eflags);
    //mvwprintw(win_regs, 5, 2, "SS  : 0x%08x %010u%*sES  : 0x%08x %010u%*s[ %2s %2s %2s %2s %2s %2s %2s]\n", ss,ss, space,"", es,es, space,"", c?"CF":"",p?"PF":"",z?"ZF":"",s?"SF":"",o?"OF":"",a?"AF":"",i?"IF":"");
    //mvwprintw(win_regs, 6, 2, "CS  : 0x%08x %010u%*sGS  : 0x%08x %010u\n", cs,cs, space,"",gs,gs);

    if(old_eax != eax){
        wattron(win_regs, A_REVERSE);
        old_eax = eax;
    }else{
        wattroff(win_regs, A_REVERSE);
    }
    mvwprintw(win_regs, 1, 2, "EAX : 0x%08x %010u",eax,eax);

    if(old_ecx != ecx){
        wattron(win_regs, A_REVERSE);
        old_ecx = ecx;
    }else{
        wattroff(win_regs, A_REVERSE);
    }
    mvwprintw(win_regs, 1, 2+27*1+space, "ECX : 0x%08x %010u",ecx,ecx);

    if(old_esi != esi){
        wattron(win_regs, A_REVERSE);
        old_esi = esi;
    }else{
        wattroff(win_regs, A_REVERSE);
    }
    mvwprintw(win_regs, 1, 2+27*2+space*2, "ESI : 0x%08x %010u",esi,esi);
    

    if(old_edx != edx){
        wattron(win_regs, A_REVERSE);
        old_edx = edx;
    }else{
        wattroff(win_regs, A_REVERSE);
    }
    mvwprintw(win_regs, 2, 2, "EDX : 0x%08x %010u",edx,edx);

    if(old_ebx != ebx){
        wattron(win_regs, A_REVERSE);
        old_ebx = ebx;
    }else{
        wattroff(win_regs, A_REVERSE);
    }
    mvwprintw(win_regs, 2, 2+27*1+space, "EBX : 0x%08x %010u",ebx,ebx);

    if(old_edi != edi){
        wattron(win_regs, A_REVERSE);
        old_edi = edi;
    }else{
        wattroff(win_regs, A_REVERSE);
    }
    mvwprintw(win_regs, 2, 2+27*2+space*2, "EDI : 0x%08x %010u",edi,edi);

    if(old_esp != esp){
        wattron(win_regs, A_REVERSE);
        old_esp = esp;
    }else{
        wattroff(win_regs, A_REVERSE);
    }
    mvwprintw(win_regs, 3, 2, "ESP : 0x%08x %010u",esp,esp);

    wattroff(win_regs, A_REVERSE); /* EIP changes every instruction so no highlight */
    mvwprintw(win_regs, 3, 2+27*1+space, "EIP : 0x%08x %010u",eip,eip);

    if(old_ebp != ebp){
        wattron(win_regs, A_REVERSE);
        old_ebp = ebp;
    }else{
        wattroff(win_regs, A_REVERSE);
    }
    mvwprintw(win_regs, 3, 2+27*2+space*2, "EBP : 0x%08x %010u",ebp,ebp);

    wattroff(win_regs, A_REVERSE); /* Dont highlight segments */
    mvwprintw(win_regs, 4, 2, "DS  : 0x%08x %010u%*sFS  : 0x%08x %010u%*sEFLAGS : 0x%08x\n", ds,ds, space,"",fs,fs,space,"", eflags);
    mvwprintw(win_regs, 5, 2, "SS  : 0x%08x %010u%*sES  : 0x%08x %010u", ss,ss, space,"", es,es);

    if(old_eflags != eflags){
        wattron(win_regs, A_REVERSE);
        old_eflags = eflags;
    }else{
        wattroff(win_regs, A_REVERSE);
    }
    mvwprintw(win_regs, 5, 2+27*2+space*2, "[ %2s %2s %2s %2s %2s %2s %2s]\n", c?"CF":"",p?"PF":"",z?"ZF":"",s?"SF":"",o?"OF":"",a?"AF":"",i?"IF":"");    

    wattroff(win_regs, A_REVERSE);
    mvwprintw(win_regs, 6, 2, "CS  : 0x%08x %010u%*sGS  : 0x%08x %010u\n", cs,cs, space,"",gs,gs);


    /*
    snprintf(buffer, REGS_BUFF_S, "EAX : 0x%08x %010u\t\tECX : 0x%08x %010u\t\tEDX : 0x%08x %010u\n EBX : 0x%08x %010u\t\tESI : 0x%08x %010u\t\tEDI : 0x%08x %010u\n ESP : 0x%08x %010u\t\tEBP : 0x%08x %010u\t\tEIP : 0x%08x %010u\n DS  : 0x%08x %010u\t\tFS  : 0x%08x %010u\t\tEFLAGS : 0x%08x\n SS  : 0x%08x %010u\t\tES  : 0x%08x %010u\t\t[ %2s %2s %2s %2s %2s %2s %2s]\n CS  : 0x%08x %010u\t\tGS  : 0x%08x %010u\nEFLAGS : 0x%08x\t", 
        eax,eax,ecx,ecx,edx,edx,ebx,ebx,esi,esi,edi,edi,esp,esp,ebp,ebp,eip,eip,ds,ds,fs,fs,eflags,ss,ss,es,es,c?"CF":"",p?"PF":"",z?"ZF":"",s?"SF":"",o?"OF":"",a?"AF":"",i?"IF":"",cs,cs,gs,gs);
    mvwprintw(win_regs, 1, 1, buffer);
    */
    box(win_regs, 0, 0);
    wrefresh(win_regs);
    free(buffer);
}

void draw_stack(int scr_s){
    werase(win_stack);
    mvwprintw(win_stack, 1, 5, "STACK: ");
    
    int i=scr_s;
    int k = 0;
    int lim = rows - H_REGS - 3 ;
    /* Substraction with unsigned ints so we must cast to int */
    int j = ((int)(STACK_BOTTOM - esp))/4;    
    do{
        if (j > 0 && esp + 4*i < STACK_BOTTOM){
            mvwprintw(win_stack, 2 + k, 2, "0x%08x : 0x%08x", esp + 4*i,  *((uint32_t *)(mem+(esp+4*i))));    
        }
        if (esp + 4*i < STACK_BOTTOM){
            i++;
        }
        k++;
    }while(k <= lim && i < j);

    box(win_stack, 0, 0);
    wrefresh(win_stack);
}

void draw_code(char ** lineas, int count, int eip_ind){
    werase(win_code);
    box(win_code, 0, 0);

    for (int i = 0; i < (rows - H_REGS - H_CMD-2); i++) {
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
    delwin(win_cmd);
    endwin();
}


