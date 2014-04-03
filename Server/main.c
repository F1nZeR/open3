#include <stdio.h>
#include "arghandle.h"
#include "workfunctions.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char **argv)
{
	pid_t pid = fork();
	if (pid == 0)
	{
		umask(0);
		struct arg_data res = handleArgs(argc, argv);
		if (setsid() < 0) return 1;
		if (chdir("/") < 0) return 1;
		close(STDIN_FILENO);
        //close(STDOUT_FILENO);
        //close(STDERR_FILENO);

		startProgram(res.executeFile, res.multiplex, res.logfile_fd, res.ftokPath, res.ipcType);
	}
	return 0;
}