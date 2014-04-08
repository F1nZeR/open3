#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "sysvipc.h"
#include <signal.h>
#include <string.h>
#include "workfunctions.h"

key_t key;
struct sembuf operations[2];
short sarray[1];
int rc, semid, shmid, msgid;
struct shmid_ds shmid_struct;
struct register_info *shm_address;

void initSysV(char *ftokPath)
{
	key = ftok(ftokPath, 1);
	if (key == -1)
	{
		perror("error on ftok()");
		exit(1);
	}

	semid = semget(key, 1, 0666 | IPC_CREAT | IPC_EXCL);
	if (semid == -1)
	{
		perror("error on semget()");
		exit(1);
	}
	sarray[0] = 0; // is being used
	rc = semctl(semid, 1, SETALL, sarray);
	if (rc == -1)
	{
		perror("semctl error");
		exit(1);
	}

	// shared memory
	size_t structSize = sizeof(struct register_info) * 10;
	shmid = shmget(key, structSize, 0666 | IPC_CREAT);
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
	msgid = msgget(key, IPC_CREAT | 0666);
	if (msgid == -1)
	{
		perror("error on msgget");
		exit(1);
	}
}

void disposeSysV()
{
	rc = semctl(semid, 1, IPC_RMID);
	if (rc == -1)
	{
		perror("error on semctl()");
		exit(1);
	}

	rc = shmctl(shmid, IPC_RMID, &shmid_struct);
	if (rc == -1)
	{
		perror("error on shmctl");
		exit(1);
	}


	rc = shmdt(shm_address);
	if (rc == -1)
	{
		perror("error on shmdt");
		exit(1);
	}

	rc = msgctl(msgid, IPC_RMID, 0);
	if (rc == -1)
	{
		perror("error on msgctl()");
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

void kill_users_sysv()
{
	getSemaphoreControl_sysv();
	int i;
	for (i = 0; i < 10; ++i)
	{
		if (shm_address[i].pid != 0)
		{
			kill(shm_address[i].pid, SIGTERM);
		}
	}
	freeSemaphore_sysv();
}

void reguser_sysv(pid_t pid)
{
	getSemaphoreControl_sysv();
	int i;
	// не должно быть таких юзеров
	for (i = 0; i < 10; ++i)
	{
		if (shm_address[i].pid == pid)
		{
			freeSemaphore_sysv();
			return;
		}
	}

	// первый пустой слот занимаем
	for (i = 0; i < 10; ++i)
	{
		if (shm_address[i].pid == 0)
		{
			shm_address[i].pid = pid;
			break;
		}
	}
	freeSemaphore_sysv();
}

void checkusers_sysv()
{
	getSemaphoreControl_sysv();
	int i;
	for (i = 0; i < 10; ++i)
	{
		if (shm_address[i].pid > 0)
		{
			if (kill(shm_address[i].pid, 0) != 0)
			{
				shm_address[i].pid = 0;
				shm_address[i].stdin_count = 0;
				shm_address[i].stdout_count = 0;
				shm_address[i].stderr_count = 0;
			}
		}
	}
	freeSemaphore_sysv();
}

void unreguser_sysv(pid_t pid)
{
	getSemaphoreControl_sysv();
	int i;
	for (i = 0; i < 10; ++i)
	{
		if (shm_address[i].pid == pid)
		{
			shm_address[i].pid = 0;
			shm_address[i].stdin_count = 0;
			shm_address[i].stdout_count = 0;
			shm_address[i].stderr_count = 0;
			break;
		}
	}
	freeSemaphore_sysv();
}

void upduserstat_sysv(pid_t pid, int target)
{
	getSemaphoreControl_sysv();
	int i;
	for (i = 0; i < 10; ++i)
	{
		if (shm_address[i].pid == pid)
		{
			switch (target)
			{
				case 0:
					shm_address[i].stdin_count++;
					break;

				case 1:
					shm_address[i].stdout_count++;
					break;

				case 2:
					shm_address[i].stderr_count++;
					break;

				default:
					break;
			}
			break;
		}
	}
	freeSemaphore_sysv();
}

void send_message_sysv(long targetPid, char message[200], int target)
{
	struct mymsg msg;
	strncpy(msg.msg, message, sizeof(msg.msg));
	msg.target = target;
	msg.type = targetPid;
	msgsnd(msgid, &msg, sizeof(struct mymsg) - sizeof(long), 0);
	upduserstat_sysv(targetPid, target);
}

struct mymsg recieve_message_sysv()
{
	struct mymsg msg;
	if (msgrcv(msgid, &msg, sizeof(struct mymsg) - sizeof(long), 1, IPC_NOWAIT) == -1)
	{
		msg.type = -1;
	}
	else
	{
		upduserstat_sysv(msg.type, msg.target);
	}
	return msg;
}