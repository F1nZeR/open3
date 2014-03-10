struct arg_data
{
	int logfile_fd;
	char *executeFile;
	int multiplex;
};

struct arg_data handleArgs(int argc, char **argv);