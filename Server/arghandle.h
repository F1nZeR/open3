struct arg_data
{
	int logfile_fd;
	char *executeFile;
	int multiplex;
	char *ftokPath;
	int ipcType;
};

struct arg_data handleArgs(int argc, char **argv);