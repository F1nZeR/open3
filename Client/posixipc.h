#include <unistd.h>

void init_posix(char *ftokPath, pid_t curPid);
void dispose_posix();
void send_message_posix(char *message);;
struct mymsg recieve_message_posix();
void getRegisterInfo_posix();