#ifndef INTERFACE_H
#define INTERFACE_H

#define H_REGS 8
#define W_STACK 27
#define H_CMD 5

#define KEY_UP 1001
#define KEY_DOWN 1002
#define KEY_LEFT 1003
#define KEY_RIGHT 1004

#define REGS_BUFF_S 500
#define ADDR_TXT_S 15

int init_interface();
void init_raw_mode();
void enable_raw_mode();
void disable_raw_mode();
void draw_screen(int scr_s, int scr_c, char ** lineas, int count, int eip_ind);
void get_str(char * str, char * ret, size_t size, int c);
void print(char * txt);
int getch();
void exit_interface();
void move(int r);
void movev(int r);
void cleanv(int i, int f);
void clean();
void cleann();

#endif