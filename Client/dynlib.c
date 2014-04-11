#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include "dynlib.h"
#include "helper.h"

typedef void (*init_ipc)(char *ftokPath, pid_t targetPid);
typedef void (*dispose_ipc)();
typedef void (*send_message_ipc)(char *message);
typedef struct mymsg (*recieve_message_ipc)();
typedef void (*getRegisterInfo_ipc)();

init_ipc init_func;
dispose_ipc dispose_func;
send_message_ipc send_message_func;
recieve_message_ipc recieve_message_func;
getRegisterInfo_ipc getRegisterInfo_ipc_func;

int ipc_type;
void *module;

void load_libs(int ipcType)
{
	ipc_type = ipcType;


	if (ipc_type == 0)
	{
		module = dlopen("/usr/local/bin/posixstuff.so.1", RTLD_LAZY);
		if (!module) 
		{
			fprintf(stderr, "Couldn't open lib: %s\n", dlerror());
			exit(1);
		}
		
		init_func = dlsym(module, "init_posix");
		dispose_func = dlsym(module, "dispose_posix");
		send_message_func = dlsym(module, "send_message_posix");
		recieve_message_func = dlsym(module, "recieve_message_posix");
		getRegisterInfo_ipc_func = dlsym(module, "getRegisterInfo_posix");
	}
	else
	{
		module = dlopen("/usr/local/bin/sysvstuff.so.1", RTLD_LAZY);
		if (!module) 
		{
			fprintf(stderr, "Couldn't open lib: %s\n", dlerror());
			exit(1);
		}

		init_func = dlsym(module, "initSysV");
		dispose_func = dlsym(module, "disposeSysV");
		send_message_func = dlsym(module, "send_message_sysv");
		recieve_message_func = dlsym(module, "recieve_message_sysv");
		getRegisterInfo_ipc_func = dlsym(module, "getRegisterInfo_sysv");
	}
}

void init_ipc_system(char *ftokPath, pid_t pid)
{
	init_func(ftokPath, pid);
}

void dispose_ipc_system()
{
	dispose_func();
}

void send_message_ipc_system(char *message)
{
	send_message_func(message);
}

struct mymsg recieve_message_ipc_system()
{
	return recieve_message_func();
}

void get_register_info_system()
{
	getRegisterInfo_ipc_func();
}