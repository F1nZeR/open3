#include <sys/types.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <mqueue.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixipc.h"
#include <string.h>
#include <fcntl.h>
#include "sysvipc.h"

#define SEM1_NAME "/sem1"
#define SEM2_NAME "/sem2"
#define SHM_NAME "/shm"
#define MQ_NAME "/mq"
#define MSGS_IN_MQ 10

sem_t *sem1, *sem2;
short sarray[2];
int rc, shmid;
mqd_t mq;
struct register_info *shm_address;
pid_t pid;

void init_posix(char *ftokPath, pid_t curPid)
{
	pid = curPid;
	if ((sem1 = sem_open(SEM1_NAME, 0)) == SEM_FAILED)
	{
		perror("error on semget()");
		exit(1);
	}

	if ((sem2 = sem_open(SEM2_NAME, 0)) == SEM_FAILED)
	{
		perror("error on sem_open()");
		exit(1);
	}
	sem_post(sem2);
	// todo: only one instance may be runned
	
	// shared memory
	size_t structSize = sizeof(struct register_info) * 10;
	shmid = shm_open(SHM_NAME, O_RDWR, 0666);
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
	mq = mq_open(MQ_NAME, O_RDWR | O_NONBLOCK);
	if (mq == -1)
	{
		perror("error on mqopen()");
		exit(1);
	}
}

void dispose_posix()
{
	close(shmid);
	munmap(shm_address, sizeof(struct register_info)*10);
}

void getSemaphoreControl_posix()
{
	sem_wait(sem2);
}

void freeSemaphore_posix()
{
	sem_post(sem2);
}

void getRegisterInfo_posix()
{
	getSemaphoreControl_posix();
	int i;
	for (i = 0; i < 10; ++i)
	{
		if (shm_address[i].pid > 0)
		{
			printf("PID: %d (%d/%d/%d)\n", shm_address[i].pid, shm_address[i].stdin_count,
				shm_address[i].stdout_count, shm_address[i].stderr_count);
		}
	}
	freeSemaphore_posix();
}

void send_message_posix(char *message)
{
	struct mymsg msg;
	strncpy(msg.msg, message, sizeof(msg.msg));
	msg.target = pid;
	msg.type = 1;
	mq_send(mq, (char *) &msg, sizeof(struct mymsg), 1);
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
		if (allmsgs[i].type == pid && isFound == 0)
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
	return msg;
}