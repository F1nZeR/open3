#include <unistd.h>
#include "helper.h"

void initSysV(char *ftokPath, pid_t targetPid);
void disposeSysV();
void send_message_sysv(char *message);
struct mymsg recieve_message_sysv();
void getRegisterInfo_sysv();