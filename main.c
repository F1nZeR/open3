#include <stdio.h>
#include "arghandle.h"
#include "workfunctions.h"

int main(int argc, char **argv)
{
	struct arg_data res = handleArgs(argc, argv);
	startProgram(res.executeFile, res.multiplex, res.logfile_fd);
	return 0;
}