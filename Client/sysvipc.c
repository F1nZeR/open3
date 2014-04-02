#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

key_t key;
struct sembuf operations[2];
short sarray[2];
int rc, semid, shmid, msgid;
void *shm_address;
struct shmid_ds shmid_struct;

void initSysV(char *ftokPath)
{
	key = ftok(ftokPath, 1);
	if (key == -1)
	{
		perror("error on ftok()");
		exit(1);
	}

	// semaphor
	// 1 - is running
	// 2 - is being used
	semid = semget(key, 2, 0666);
	if (semid == -1)
	{
		perror("error on semget()");
		exit(1);
	}
	sarray[0] = 1;
	sarray[1] = 0;
	// todo: only one instance may be runned
	rc = semctl(semid, 1, SETALL, sarray);
	if (rc == -1)
	{
		perror("semctl error");
		exit(1);
	}

	// shared memory
	shmid = shmget(key, 50, 0666);
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