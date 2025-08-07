#ifndef INTERFACE_H
#define INTERFACE_H

#define H_REGS 8
#define W_STACK 27
#define H_CMD 5


#define REGS_BUFF_S 500

void init_interface();
void draw_regs();
void draw_stack();
void draw_code(char ** lineas, int count, int eip_ind);
void cmd_get_str(char * str, size_t size, uint8_t c);
void draw_cmd();
void exit_interface();






#endif