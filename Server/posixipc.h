#include <unistd.h>

void init_posix(char *ftokPath);
void dispose_posix();
void send_message_posix(long targetPid, char message[200], int target);
struct mymsg recieve_message_posix();
void reguser_posix(pid_t pid);
void unreguser_posix(pid_t pid);