#include <unistd.h>

struct mymsg
{
	long type;
	int target;
	char msg[200];
};

void initSysV(char *ftokPath);
void disposeSysV();
void send_message_sysv(long targetPid, char message[200], int target);
struct mymsg recieve_message_sysv();