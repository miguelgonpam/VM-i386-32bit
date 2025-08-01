#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include "interface.h"
#include "flags.h"
#include "instr.h"

extern uint32_t eflags;
extern uint8_t * mem;
extern uint32_t eax, edx, esp, esi, eip, cs, ds, fs, ecx, ebx, ebp, edi, ss, es, gs;

int rows, cols;//, scroll;
WINDOW * win_pila, * win_registros, * win_codigo;

void init_interface(){
    initscr();              // Inicia ncurses
    noecho();               // No mostrar entrada
    cbreak();               // Modo sin buffering
    keypad(stdscr, TRUE);   // Habilita teclas especiales

    
    getmaxyx(stdscr, rows, cols);

    // Crear ventanas
    win_registros = newwin(REGISTERS_HEIGHT, cols, 0, 0);
    win_pila = newwin(rows - REGISTERS_HEIGHT, STACK_WIDTH, REGISTERS_HEIGHT, cols - STACK_WIDTH);
    win_codigo = newwin(rows - REGISTERS_HEIGHT, cols - STACK_WIDTH, REGISTERS_HEIGHT, 0);
}

void draw_regs(){
    werase(win_registros);
    char * buffer = malloc(sizeof(char) * REGS_BUFF_S);

    uint8_t c = test_Flag(CF),p = test_Flag(PF),z = test_Flag(ZF),s = test_Flag(SF),o = test_Flag(OF),a = test_Flag(AF), i = test_Flag(IF);
    snprintf(buffer, REGS_BUFF_S, "EAX : 0x%08x %010u\t\tECX : 0x%08x %010u\t\tEDX : 0x%08x %010u\n EBX : 0x%08x %010u\t\tESI : 0x%08x %010u\t\tEDI : 0x%08x %010u\n ESP : 0x%08x %010u\t\tEBP : 0x%08x %010u\t\tEIP : 0x%08x %010u\n DS  : 0x%08x %010u\t\tFS  : 0x%08x %010u\t\tEFLAGS : 0x%08x\n SS  : 0x%08x %010u\t\tES  : 0x%08x %010u\t\t[ %2s %2s %2s %2s %2s %2s %2s]\n CS  : 0x%08x %010u\t\tGS  : 0x%08x %010u\nEFLAGS : 0x%08x\t", 
        eax,eax,ecx,ecx,edx,edx,ebx,ebx,esi,esi,edi,edi,esp,esp,ebp,ebp,eip,eip,ds,ds,fs,fs,eflags,ss,ss,es,es,c?"CF":"",p?"PF":"",z?"ZF":"",s?"SF":"",o?"OF":"",a?"AF":"",i?"IF":"",cs,cs,gs,gs);
    mvwprintw(win_registros, 1, 1, buffer);
    box(win_registros, 0, 0);
    wrefresh(win_registros);
    free(buffer);
}

void draw_stack(){
    werase(win_pila);
    mvwprintw(win_pila, 1, 5, "STACK: ");

    
    int i=0;
    int lim = cols - REGISTERS_HEIGHT - 3 ;
    int j = (STACK_BOTTOM - esp)/4;    

    do{
        mvwprintw(win_pila, 2 + i, 2, "0x%08x : 0x%08x", esp + 4*i,  *((uint32_t *)(mem+(esp+4*i))));    
        i++;
    }while(i <= lim && i <= j);
    /*
    for (int i = 0; i < 5; i++) {
        mvwprintw(win_pila, i + 2, 2, "0x%X", 0xFF - i);
    }
    */
    box(win_pila, 0, 0);
    wrefresh(win_pila);
}

void draw_code(const char ** lineas, int count){
    werase(win_codigo);
    box(win_codigo, 0, 0);

    for (int i = 0; i < (rows - REGISTERS_HEIGHT - 2); i++) {
        if (i < count) {
            mvwprintw(win_codigo, i + 1, 1, "%s", lineas[i]);
        }
    }
    wrefresh(win_codigo);

    // Control de scroll
    //if (ch == KEY_DOWN && scroll < num_lineas - 1){
    //    scroll++;
    //}else if (ch == KEY_UP && scroll > 0){
    //    scroll--;
    //}
}

void exit_interface(){
    delwin(win_registros);
    delwin(win_pila);
    delwin(win_codigo);
    endwin();
}


int main2() {
    
    init_interface();

    // Contenido simulado
    const char *lineas[] = {
        "Línea 1: MOV A, B",
        "Línea 2: ADD A, 5",
        "Línea 3: JMP 0x10",
        "Línea 4: SUB B, A",
        "Línea 5: PUSH A",
        "Línea 6: POP B",
        "Línea 7: NOP",
        "Línea 8: INC C",
        "Línea 9: DEC D",
        "Línea 10: RET"
    };
    int num_lineas = sizeof(lineas) / sizeof(lineas[0]);
    //int scroll = 0;

    // Dibujar estáticos
    draw_regs();

    draw_stack();

    // Scroll loop
    int ch;
    while ((ch = getch()) != 'q') {
        // --- Dibujar registros ---
        draw_regs();

        // --- Dibujar pila ---
        draw_stack();
        // --- Dibujar código con scroll ---
        draw_code(lineas, num_lineas);
        
    }
    // Limpieza
    delwin(win_registros);
    delwin(win_pila);
    delwin(win_codigo);
    endwin();
    return 0;
}