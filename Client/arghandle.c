#include <stdlib.h>
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
		{"ftok", required_argument, 0, 'f'},
		{"ipc", required_argument, 0, 'i'}
	};
	int c, exitFlag = 0;
	struct arg_data argData;

	while (!exitFlag)
	{
		c = getopt_long(argc, argv, "f:i:", long_options, 0);
		switch (c)
		{
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
	
	return argData;
}
