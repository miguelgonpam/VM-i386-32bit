    #include <termios.h>
#include <unistd.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "interface.h"
#include "flags.h"
#include "instr.h"


extern uint32_t eflags;
extern uint8_t * mem;
extern uint32_t eax, edx, esp, esi, eip, cs, ds, fs, ecx, ebx, ebp, edi, ss, es, gs;
uint32_t old_eax, old_edx, old_esp, old_esi, old_ecx, old_ebx, old_ebp, old_edi, old_eflags;

int rows, cols, spaces, w_code, w_stack;
char *lines, *code, *stack;

struct termios oldt, newt;

void init_raw_mode(){
    tcgetattr(STDIN_FILENO, &oldt);       // Guarda la config actual
    newt = oldt;
    newt.c_lflag &= ~(ICANON);     // Sin modo canónico y sin eco (~(ICANON | ECHO))
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void enable_raw_mode() {
    newt = oldt;
    newt.c_lflag &= ~(ICANON);     // Sin modo canónico y sin eco (~(ICANON | ECHO))
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restaura config
    //clean();
}

void get_lines(size_t size, char * str){
    const char *hline = "─";

    for (int i=0; i<size; i++){
        strcat(str, hline);
    }
}

int init_interface(){
    struct winsize w;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        perror("ioctl");
        return 1;
    }
    /* Get terminal size */
    rows = w.ws_row;
    cols = w.ws_col;

    /* Get spaces between registers */
    spaces = ((cols - 4)-(27*3))/2;
    if(spaces)spaces--;
    if (spaces > 15)spaces=10;

    /* Alloc memory for drawing the boxes */
    lines = calloc(3, cols-1);
    code = calloc(3,(cols/3)*2-1);
    stack = calloc(3, (cols/3));
    get_lines(cols-2, lines);

    /* Obtain code and stack boxes width */
    //w_code = (cols/3)*2;
    //w_stack = cols - w_code;

    w_code = cols-W_STACK;
    w_stack = W_STACK;

    /* Fill code and stack strings with box lines */
    get_lines(w_code-2, code);
    get_lines(w_stack-2, stack);

    /* Store old regs values to compare and hihlight if change */
    old_eax = eax;
    old_ebx = ebx;
    old_ecx = ecx;
    old_edx = edx;
    old_ebp = ebp;
    old_esp = esp;
    old_esi = esi;
    old_edi = edi;
    old_eflags = eflags;

}

int getch() {
    char c;
    move(rows);  // mover cursor antes
    if (read(STDIN_FILENO, &c, 1) != 1) return -1;

    if (c == '\x1B') { // ESC
        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1B';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1B';

        if (seq[0] == '[') {
            switch(seq[1]) {
                case 'A': return KEY_UP;
                case 'B': return KEY_DOWN;
                case 'C': return KEY_RIGHT;
                case 'D': return KEY_LEFT;
            }
        }
        return '\x1B';
    } else {
        return c;
    }
}

void draw_screen(int scr_s, int scr_c, char ** lineas, int count, int eip_ind){
    /* Clear screen */
    cleanv(1, rows-3);

    /* Test Flags to show on Registers Window */
    uint8_t c = test_Flag(CF),p = test_Flag(PF),z = test_Flag(ZF),s = test_Flag(SF),o = test_Flag(OF),a = test_Flag(AF), i = test_Flag(IF);

    /* Set cursor */
    move(1);

    /* Registers */
    printf("┌%*s┐\n", cols-2, lines);
    printf("│ %sEAX : 0x%08x %010u%s %*s%sECX : 0x%08x %010u%s %*s%sESI : 0x%08x %010u%s\n", eax!=old_eax?"\033[7m":"",eax, eax,"\033[0m" ,spaces, " ", ecx!=old_ecx?"\033[7m":"",ecx, ecx,"\033[0m" ,spaces, " ", esi!=old_esi?"\033[7m":"",esi, esi,"\033[0m");
    printf("\033[%d;%dH│\n", 2, cols); /* Moves the cursor to the end of the terminal line and puts the closing character */
    printf("│ %sEDX : 0x%08x %010u%s %*s%sEBX : 0x%08x %010u%s %*s%sEDI : 0x%08x %010u%s\n", edx!=old_edx?"\033[7m":"",edx, edx, "\033[0m",spaces, " ", ebx!=old_ebx?"\033[7m":"",ebx, ebx,"\033[0m" ,spaces, " ", edi!=old_edi?"\033[7m":"",edi, edi, "\033[0m");
    printf("\033[%d;%dH│\n", 3, cols); /* Moves the cursor to the end of the terminal line and puts the closing character */
    printf("│ %sESP : 0x%08x %010u%s %*sEIP : 0x%08x %010u %*s%sEBP : 0x%08x %010u%s\n", esp!=old_esp?"\033[7m":"",esp, esp, "\033[0m",spaces, " ", eip, eip,spaces, " ", ebp!=old_ebp?"\033[7m":"",ebp, ebp, "\033[0m");
    printf("\033[%d;%dH│\n", 4, cols); /* Moves the cursor to the end of the terminal line and puts the closing character */
    printf("│ DS  : 0x%08x %010u %*sES  : 0x%08x %010u %*sEFLAGS : 0x%08x\n", ds, ds, spaces, " ", es, es,spaces, " ", eflags);
    printf("\033[%d;%dH│\n", 5, cols); /* Moves the cursor to the end of the terminal line and puts the closing character */
    printf("│ SS  : 0x%08x %010u %*sFS  : 0x%08x %010u %*s%s[ %2s %2s %2s %2s %2s %2s %2s]%s\n", ss, ss, spaces, " ", fs, fs,spaces, " ", eflags!=old_eflags?"\033[7m":"" ,c?"CF":"",p?"PF":"",z?"ZF":"",s?"SF":"",o?"OF":"",a?"AF":"",i?"IF":"", "\033[0m");
    printf("\033[%d;%dH│\n", 6, cols); /* Moves the cursor to the end of the terminal line and puts the closing character */
    printf("│ CS  : 0x%08x %010u %*sGS  : 0x%08x %010u\n", cs, cs, spaces, " ", gs, gs);
    printf("\033[%d;%dH│\n", 7, cols); /* Moves the cursor to the end of the terminal line and puts the closing character */
    printf("└%*s┘\n", cols-2, lines);

    /* Update old values */
    old_eax = eax;
    old_ebx = ebx;
    old_ecx = ecx;
    old_edx = edx;
    old_ebp = ebp;
    old_esp = esp;
    old_esi = esi;
    old_edi = edi;
    old_eflags = eflags;

    /* Code and Stack Box Top Line*/
    printf("┌%*s┐┌%*s┐\n", w_code-2, code, w_stack-2, stack);

    /* Code */
    for (int i=0; i<rows-H_REGS-5; i++){ /* 5 lines left, 2 for the code box and 3 for stdin */
        if (i < count){
            /* Clear line and Print code addr in blue */
            printf("\033[K│ <%s%s%s> : ", "\033[34m", lineas[i*2], "\033[0m"); 
            if(eip_ind >= 0 && eip_ind == i){
                /* Highlight current instruction (EIP) */
                printf("%s%s%s", "\033[7m",lineas[i*2+1],"\033[0m");
            }else{
                /* Print normal instruction */
                printf("%s", lineas[i*2+1]);
            }
            /* Close box line */
            printf("\033[%d;%dH│\n", H_REGS+i+2, w_code); /* +2 because 1 is to skip the box line and other 1 is in case i=0 (so it would write in the box again)*/
        }
    }

    /* Stack */
    /* First position to draw from the stack (Starting i doublewords from ESP) */
    int ii = scr_s;
    /* Lines drawn counter */
    int k=0;
    /* Rows available to draw */
    int lim = rows - H_REGS - 2 - 3; /* 2 is the width of top and bottom box lines and 3 is number of lines reserved to stdin */ 
    /* Substraction with unsigned ints so we must cast to int */
    /* Limit of rows available to draw so it doesnt overflow STACK_BOTTOM*/
    int j = ((int)(STACK_BOTTOM - esp))/4;
    do{
        if (j > 0 && esp + 4*ii < STACK_BOTTOM){
            printf("\033[%d;%dH│ %s0x%08x%s : 0x%08x \033[%d;%dH│\n", H_REGS+k+2, w_code+1, "\033[32m", esp+4*ii, "\033[0m", *((uint32_t *)(mem+(esp+4*ii))), H_REGS+k+2, cols);   
        }
        if (esp + 4*ii >= STACK_BOTTOM){
            break;
        }
        ii++;
        k++;
    }while(k < lim && ii < j);
    while (k < lim){
        printf("\033[%d;%dH\033[K│%*s│\n", H_REGS+k+2, w_code+1, w_stack-2, "");
        k++;
    }

    /* Code and Stack Box Bottom Line*/
    printf("└%*s┘└%*s┘\n", w_code-2, code, w_stack-2, stack);
}



__inline void move(int r){
    printf("\033[%d;1H", r);
    fflush(stdout);
}

__inline void movev(int r){
    printf("\033[%d;1H", r);
    fflush(stdout);
}

__inline void cleann(){
    printf("\033[K");
    fflush(stdout);
}

__inline void clean(){
    printf("\033[K\n");
    fflush(stdout);
}

void cleanv(int i, int f){
    for(int j=i; j<=f; j++){
        printf("\033[%d;1H\033[K", j);
    }
    fflush(stdout);
}

void print(char * txt){
    move(rows-1);
    printf("%s", txt);
    

}

void get_str(char * str, char * ret, size_t size, int c){
    /* Move pointer to first of the stdin 3 lines */
    cleanv(rows-2, rows);
    //move(rows-2);
    
    /* Print getter string on the line before the last (last row-1, character 1) */
    printf("\033[%d;%dH %s%s\n ", rows-2, 1, c?"Wrong format, ":"",str);

    disable_raw_mode();
    fgets(ret, size, stdin);
    enable_raw_mode();
}

void exit_interface(){
    free(lines);
    free(code);
    free(stack);
}


