#ifndef INTERFACE_H
#define INTERFACE_H

#define H_REGS 8
#define W_STACK 27
#define H_CMD 5


#define REGS_BUFF_S 500
#define ADDR_TXT_S 15

int init_interface();
void draw_screen(int scr_s, int scr_c, char ** lineas, int count, int eip_ind);
void get_str(char * str, char * ret, size_t size);
void print(char * txt);
void exit_interface();

#endif