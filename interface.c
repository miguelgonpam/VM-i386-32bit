#include <ncurses.h>

#define ALTO_REGISTROS 6
#define ANCHO_PILA 40

int main() {
    initscr();              // Inicia ncurses
    noecho();               // No mostrar entrada
    cbreak();               // Modo sin buffering
    keypad(stdscr, TRUE);   // Habilita teclas especiales

    int filas, columnas;
    getmaxyx(stdscr, filas, columnas);

    // Crear ventanas
    WINDOW *win_registros = newwin(ALTO_REGISTROS, columnas, 0, 0);
    WINDOW *win_pila = newwin(filas - ALTO_REGISTROS, ANCHO_PILA, ALTO_REGISTROS, columnas - ANCHO_PILA);
    WINDOW *win_codigo = newwin(filas - ALTO_REGISTROS, columnas - ANCHO_PILA, ALTO_REGISTROS, 0);

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
    int scroll = 0;

    // Dibujar estáticos
    mvwprintw(win_registros, 1, 1, "REGISTROS: A=5 B=3 C=2 D=8");
    box(win_registros, 0, 0);
    wrefresh(win_registros);

    mvwprintw(win_pila, 1, 1, "PILA:");
    for (int i = 0; i < 5; i++) {
        mvwprintw(win_pila, i + 2, 2, "0x%X", 0xFF - i);
    }
    box(win_pila, 0, 0);
    wrefresh(win_pila);

    // Scroll loop
    int ch;
    while ((ch = getch()) != 'q') {
        // --- Dibujar registros ---
        werase(win_registros);
        box(win_registros, 0, 0);
        mvwprintw(win_registros, 1, 1, "REGISTROS: A=5 B=3 C=2 D=8");
        wrefresh(win_registros);

        // --- Dibujar pila ---
        werase(win_pila);
        box(win_pila, 0, 0);
        mvwprintw(win_pila, 1, 1, "PILA:");
        for (int i = 0; i < 5; i++) {
            mvwprintw(win_pila, i + 2, 2, "0x%X", 0xFF - i);
        }
        wrefresh(win_pila);

        // --- Dibujar código con scroll ---
        werase(win_codigo);
        box(win_codigo, 0, 0);

        for (int i = 0; i < (filas - ALTO_REGISTROS - 2); i++) {
            if (scroll + i < num_lineas) {
                mvwprintw(win_codigo, i + 1, 1, "%s", lineas[scroll + i]);
            }
        }
        wrefresh(win_codigo);

        // Control de scroll
        if (ch == KEY_DOWN && scroll < num_lineas - 1){
            scroll++;
        }else if (ch == KEY_UP && scroll > 0){
            scroll--;
        }
    }
    // Limpieza
    delwin(win_registros);
    delwin(win_pila);
    delwin(win_codigo);
    endwin();
    return 0;
}