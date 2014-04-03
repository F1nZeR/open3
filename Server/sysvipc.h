#include <unistd.h>

void initSysV(char *ftokPath);
void disposeSysV();
void send_message_sysv(long targetPid, char message[200], int target);
struct mymsg recieve_message_sysv();
void reguser_sysv(pid_t pid);
void unreguser_sysv(pid_t pid);