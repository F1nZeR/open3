#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "arghandle.h"

struct arg_data handleArgs(int argc, char **argv)
{
	struct option long_options[] =
	{
		{"logfile", required_argument, 0, 'l'},
		{"execute",	required_argument, 0, 'e'},
		{"multiplex", required_argument, 0, 'm'},
		{"ftok", required_argument, 0, 'f'},
		{"ipc", required_argument, 0, 'i'}
	};
	int c, exitFlag = 0, is_there_execute = 0;
	struct arg_data argData;
	argData.multiplex = 1;
	int logfile_fd = 2;
	argData.ipcType = 0;

	while (!exitFlag)
	{
		c = getopt_long(argc, argv, "l:e:m:f:i:", long_options, 0);
		switch (c)
		{
			case 'l':
				logfile_fd = open(optarg, O_WRONLY | O_CREAT | O_TRUNC, 770);
				if (logfile_fd < 0)
				{
					char *buffer = strerror(errno);
					write(2, buffer, strlen(buffer));
					logfile_fd = 2;	
				}
				argData.logfile_fd = logfile_fd;
				break;

			case 'e':
				argData.executeFile = optarg;
				is_there_execute = 1;
				break;

			case 'm':
				argData.multiplex = strtold(optarg, 0);
				break;

			case 'f':
				argData.ftokPath = optarg;
				break;

			case 'i':
				if (strcmp(optarg, "posix") == 0)
				{
					argData.ipcType = 0;
				}
				else if (strcmp(optarg, "sysv") == 0)
				{
					argData.ipcType = 1;
				}
				else
				{
					char *buffer = "You should set IPC type\n";
					write(2, buffer, strlen(buffer));
					exit(1);
				}
				break;

			case '?':
				abort();

			default:
				exitFlag = 1;
				break;
		}
	}

	if (!is_there_execute)
	{
		char *buffer = "There is nothing to execute\n";
		write(2, buffer, strlen(buffer));
		exit(1);
	}
	return argData;
}
