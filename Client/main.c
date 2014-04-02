#include <stdio.h>
#include "arghandle.h"

int main(int argc, char **argv)
{
	struct arg_data res = handleArgs(argc, argv);
	puts(res.executeFile);
	return 0;
}