#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "sysvipc.h"
#include <string.h>
#include <unistd.h>
#include "helper.h"

key_t key;
struct sembuf operations[2];
short sarray[1];
int rc, semid, shmid, msgid;
struct register_info *shm_address;
struct shmid_ds shmid_struct;
long pid;

void initSysV(char *ftokPath, pid_t targetPid)
{
	pid = targetPid;
	key = ftok(ftokPath, 1);
	if (key == -1)
	{
		perror("error on ftok()");
		exit(1);
	}

	// semaphor
	semid = semget(key, 1, 0666);
	if (semid == -1)
	{
		perror("error on semget()");
		exit(1);
	}

	// shared memory
	size_t structSize = sizeof(struct register_info) * 10;
	shmid = shmget(key, structSize, 0666);
	if (shmid == -1)
	{
		perror("error on shmget()");
		exit(1);
	}

	shm_address = shmat(shmid, NULL, 0);
	if (shm_address == NULL)
	{
		perror("error on shmat()");
		exit(1);
	}

	// msg queue
	msgid = msgget(key, 0666);
	if (msgid == -1)
	{
		perror("error on msgget");
		exit(1);
	}
}

void disposeSysV()
{
	rc = shmdt(shm_address);
	if (rc == -1)
	{
		perror("error on shmdt");
		exit(1);
	}
}

void getSemaphoreControl_sysv()
{
	operations[0].sem_num = 1;
	operations[0].sem_op = 0;
	operations[0].sem_flg = 0;
	operations[1].sem_num = 1;
	operations[1].sem_op = 1;
	operations[1].sem_flg = 0;
	semop(semid, operations, 2);
}

void freeSemaphore_sysv()
{
	operations[0].sem_num = 1;
	operations[0].sem_op = -1;
	operations[0].sem_flg = 0;
	semop(semid, operations, 1);
}

void getRegisterInfo_sysv()
{
	getSemaphoreControl_sysv();
	int i;
	for (i = 0; i < 10; ++i)
	{
		if (shm_address[i].pid > 0)
		{
			printf("PID: %d (%d/%d/%d)\n", shm_address[i].pid, shm_address[i].stdin_count,
				shm_address[i].stdout_count, shm_address[i].stderr_count);
		}
	}
	freeSemaphore_sysv();
}

void send_message_sysv(char *message)
{
	struct mymsg msg;
	msg.type = 1;
	msg.target = pid;
	strncpy(msg.msg, message, sizeof(msg.msg));
	if (msgsnd(msgid, &msg, sizeof(struct mymsg) - sizeof(long), 0) < 0)
	{
		perror("error on msgsnd");
		exit(1);
	}
}

struct mymsg recieve_message_sysv()
{
	struct mymsg msg;
	if (msgrcv(msgid, &msg, sizeof(struct mymsg) - sizeof(long), pid, 0) < 0)
	{
		perror("error on msgrcv");
		exit(1);
	}
	return msg;
}