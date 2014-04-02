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
		{"logfile", required_argument, 0, 'l'},
		{"execute",	required_argument, 0, 'e'},
		{"multiplex", required_argument, 0, 'm'}
	};
	int c, exitFlag = 0, is_there_execute = 0;
	struct arg_data argData;
	argData.multiplex = 1;
	int logfile_fd = 2;

	while (!exitFlag)
	{
		c = getopt_long(argc, argv, "l:e:m:", long_options, 0);
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
				break;

			case 'e':
				argData.executeFile = optarg;
				is_there_execute = 1;
				break;

			case 'm':
				argData.multiplex = strtold(optarg, 0);
				break;

			case '?':
				abort();

			default:
				exitFlag = 1;
				break;
		}
	}

	argData.logfile_fd = logfile_fd;
	if (!is_there_execute)
	{
		char *buffer = "There is nothing to execute\n";
		write(2, buffer, strlen(buffer));
		exit(1);
	}
	return argData;
}
