#include <stdio.h>
#include "arghandle.h"
#include <unistd.h>
#include "sysvipc.h"
#include "posixipc.h"
#include <string.h>

void startWork(struct arg_data *res);

int main(int argc, char **argv)
{
	struct arg_data res = handleArgs(argc, argv);
	startWork(&res);
	return 0;
}

void startWork(struct arg_data *res)
{
	if (res->ipcType == 0)
	{
		init_posix(res->ftokPath, getpid());
	}
	else
	{
		initSysV(res->ftokPath, getpid());
	}
	pid_t pid = fork();
	if (pid == 0)
	{
		while (1)
		{
			if (res->ipcType == 0)
			{
				struct mymsg msg = recieve_message_posix();
				if (msg.type != -1)
				{
					printf(">%d / %s", msg.target, msg.msg);	
				}
				else
				{
					sleep(1);
				}
			}
			else
			{
				struct mymsg msg = recieve_message_sysv();
				printf(">%d / %s", msg.target, msg.msg);
			}
		}
	}

	// parent listen messages
	while (1)
	{
		// читаем stdin
		char buffer[2048];
		memset(buffer, '\0', sizeof(buffer));
		int read_cnt = read(STDIN_FILENO, buffer, sizeof(buffer));
		if (read_cnt > 0)
		{
			if (strcmp(buffer, "status\n") == 0)
			{
				if (res->ipcType == 0)
				{
					getRegisterInfo_posix();
				}
				else
				{
					getRegisterInfo_sysv();
				}
				continue;
			}

			if (res->ipcType == 0)
			{
				send_message_posix(buffer);
			}
			else
			{
				send_message_sysv(buffer);
			}
		}
	}
}