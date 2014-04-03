#include <stdio.h>
#include "arghandle.h"
#include <unistd.h>
#include "sysvipc.h"
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
	initSysV(res->ftokPath, getpid());
	pid_t pid = fork();
	if (pid == 0)
	{
		while (1)
		{
			struct mymsg msg = recieve_message_sysv();
			printf(">%d / %s", msg.target, msg.msg);
			// получаем сообщения
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
			send_message_sysv(buffer);
		}
	}
}