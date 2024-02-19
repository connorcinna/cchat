#include "common.h"

void handle_signal(int signal);

void usage(void);

void init(char* s_addr, char* s_port);

void init_ncurses(void);

void determine_win_size(void);

void size_warning(void);

void redraw_main(void);

void redraw_msg(void);

void redraw_clients(void);

char* read_name(char* name);

void write_name(char* name);

void handle_resp(void);
