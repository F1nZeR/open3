#include <sys/types.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/mman.h>
#include <mqueue.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixipc.h"
#include <string.h>
#include <fcntl.h>
#include "workfunctions.h"

#define SEM_NAME "/sem"
#define SHM_NAME "/shm"
#define MQ_NAME "/mq"
#define MSGS_IN_MQ 10

sem_t *sem;
short sarray[2];
int rc, shmid;
mqd_t mq;
struct register_info *shm_address;

void init_posix(char *ftokPath)
{
	if ((sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
	{
		perror("error on sem_open()");
		exit(1);
	}
	sem_post(sem);
	// todo: only one instance may be runned
	
	// shared memory
	size_t structSize = sizeof(struct register_info) * 10;
	shmid = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0666);
	if (shmid == -1)
	{
		perror("error on shm_open()");
		exit(1);
	}
	if (ftruncate(shmid, structSize) == -1)
	{
		perror("error on ftruncate()");
		exit(1);
	}

	shm_address = mmap(0, structSize, PROT_WRITE | PROT_READ, MAP_SHARED, shmid, 0);
	if (shm_address == (struct register_info *) -1)
	{
		perror("error on mmap()");
		exit(1);
	}

	// msg queue
	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = MSGS_IN_MQ;
	attr.mq_msgsize = sizeof(struct mymsg);
	attr.mq_curmsgs = 0;
	mq = mq_open(MQ_NAME, O_CREAT | O_RDWR | O_NONBLOCK, 0666, &attr);
	if (mq == -1)
	{
		perror("error on mqopen()");
		exit(1);
	}
}

void dispose_posix()
{
	if (sem_close(sem) == -1)
	{
		perror("error on sem_close()");
		exit(1);
	}

	if (sem_unlink(SEM_NAME) == -1)
	{
		perror("error on sem_unlink()");
		exit(1);
	}

	rc = mq_close(mq);
	if (rc == -1)
	{
		perror("error on mq_close()");
		exit(1);
	}

	rc = mq_unlink(MQ_NAME);
	if (rc == -1)
	{
		perror("error on mq_unlink()");
		exit(1);
	}

	close(shmid);
	munmap(shm_address, sizeof(struct register_info)*10);
	shm_unlink(SHM_NAME);
}

void getSemaphoreControl_posix()
{
	sem_wait(sem);
}

void freeSemaphore_posix()
{
	sem_post(sem);
}

void reguser_posix(pid_t pid)
{
	getSemaphoreControl_posix();
	int i;
	// не должно быть таких юзеров
	for (i = 0; i < 10; ++i)
	{
		if (shm_address[i].pid == pid)
		{
			freeSemaphore_posix();
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
	freeSemaphore_posix();
}

void unreguser_posix(pid_t pid)
{
	getSemaphoreControl_posix();
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
	freeSemaphore_posix();
}

void checkusers_posix()
{
	getSemaphoreControl_posix();
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
	freeSemaphore_posix();
}

void upduserstat_posix(pid_t pid, int target)
{
	getSemaphoreControl_posix();
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
	freeSemaphore_posix();
}

void send_message_posix(long targetPid, char message[200], int target)
{
	struct mymsg msg;
	strncpy(msg.msg, message, sizeof(msg.msg));
	msg.target = target;
	msg.type = targetPid;
	mq_send(mq, (char *) &msg, sizeof(struct mymsg), 1);
	upduserstat_posix(targetPid, target);
}

struct mymsg recieve_message_posix()
{
	struct mymsg msg;
	int cnt = 0;
	struct mymsg allmsgs[MSGS_IN_MQ];
	// все сообщения
	while (mq_receive(mq, (char *) &allmsgs[cnt], sizeof(struct mymsg), NULL) != -1)
	{
		cnt++;
	}

	int i, isFound = 0;
	for (i = 0; i < cnt; ++i)
	{
		if (allmsgs[i].type == 1 && isFound == 0)
		{
			isFound = 1;
			msg = allmsgs[i];
			continue;
		}

		mq_send(mq, (char *) &allmsgs[i], sizeof(struct mymsg), 1);
	}

	if (isFound == 0)
	{
		msg.type = -1;
	}
	else
	{
		upduserstat_posix(msg.type, msg.target);
	}
	return msg;
}