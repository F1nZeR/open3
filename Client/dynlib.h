#include <stdlib.h>
#include <stdio.h>

void load_libs(int ipcType);
void init_ipc_system(char *ftokPath, pid_t pid);
void dispose_ipc_system();
void send_message_ipc_system(char *message);
struct mymsg recieve_message_ipc_system();
void get_register_info_system();