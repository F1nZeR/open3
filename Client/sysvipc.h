#include <unistd.h>

struct mymsg
{
	long type;
	int target;
	char msg[200];
};

void initSysV(char *ftokPath, pid_t targetPid);
void disposeSysV();
void send_message_sysv(char *message);
struct mymsg recieve_message_sysv();