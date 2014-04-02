struct mymsg
{
	long type;
	int target;
	char msg[200];
};

void initSysV(char *ftokPath);
void disposeSysV();