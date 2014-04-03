int startProgram(char *path, int multiplex, int logfile_fd, char *ftokPath, int ipcType);

struct mymsg
{
	long type;
	int target;
	char msg[200];
};

struct register_info
{
	int pid;
	int stdin_count;
	int stdout_count;
	int stderr_count;
};