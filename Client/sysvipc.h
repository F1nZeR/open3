#include <unistd.h>

struct mymsg
{
	long type;
	int target;
	char msg[200];
};

struct register_info
{
	int pid;
	int stdin_count;
	int stdout_count;
	int stderr_count;
};

void initSysV(char *ftokPath, pid_t targetPid);
void disposeSysV();
void send_message_sysv(char *message);
struct mymsg recieve_message_sysv();
void getRegisterInfo_sysv();