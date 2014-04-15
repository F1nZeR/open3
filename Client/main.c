#include <stdio.h>
#include <signal.h>
#include "arghandle.h"
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include "helper.h"
#include "dynlib.h"

void startWork();

typedef struct MsgList
{
	struct MsgList *next;
	struct mymsg msg;
} MsgList;

pthread_mutex_t sndmsg_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t sndmsg_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t output_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t output_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t err_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t err_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t stdin_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t stdin_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t list_mutex	= PTHREAD_MUTEX_INITIALIZER;

struct arg_data argData;
MsgList msgList;

void init_list()
{
	msgList.next = NULL;
	struct mymsg msg;
	msg.target = -5;
	msgList.msg = msg;
}

int main(int argc, char **argv)
{
	argData = handleArgs(argc, argv);
	load_libs(argData.ipcType);
	init_list();
	startWork();
	return 0;
}

void add_msg(struct mymsg *msg)
{
	struct MsgList *curMsg = (struct MsgList *) malloc(sizeof(struct MsgList));
	curMsg->msg = *msg;
	curMsg->next = NULL;

	MsgList *trv = &msgList;
	while (trv->next != NULL)
	{
		trv = trv->next;
	}
	trv->next = curMsg;
}

struct mymsg get_msg(int target)
{
	MsgList *trv = &msgList, *trv_prev = &msgList;
	while ((trv->msg.target != target) && (trv->next != NULL))
	{
		trv_prev = trv;
		trv = trv->next;
	}
	trv_prev->next = trv->next;
	return trv->msg;
}

void *handle_stdin()
{
	while (1)
	{
		// читаем stdin
		char buffer[2048];
		memset(buffer, '\0', sizeof(buffer));
		int read_cnt = read(STDIN_FILENO, buffer, sizeof(buffer));
		if (read_cnt > 0)
		{
			if (strcmp(buffer, "status\n") == 0)
			{
				get_register_info_system();
				continue;
			}

			// шлём сообщеньице
			struct mymsg *msg = (struct mymsg*) malloc(sizeof(struct mymsg));
			msg->target = -1;
			strncpy(msg->msg, buffer, sizeof(msg->msg));

			pthread_mutex_lock(&list_mutex);
			add_msg(msg);
			pthread_mutex_unlock(&list_mutex);

			pthread_mutex_lock(&sndmsg_mutex);
			pthread_cond_signal(&sndmsg_cond);
			pthread_mutex_unlock(&sndmsg_mutex);

			if (strcmp(buffer, "exit\n") == 0)
			{
				send_message_ipc_system(buffer);
				dispose_ipc_system();
				exit(0);							
			}
		}
	}
}

void *handle_stdout()
{
	while (1)
	{
		pthread_mutex_lock(&output_mutex);
		pthread_cond_wait(&output_cond, &output_mutex);
		pthread_mutex_unlock(&output_mutex);

		pthread_mutex_lock(&list_mutex);
		struct mymsg msg = get_msg(1);
		pthread_mutex_unlock(&list_mutex);

		printf(">%d / %s", msg.target, msg.msg);	
	}
}

void *handle_stdin_out()
{
	while (1)
	{
		pthread_mutex_lock(&stdin_mutex);
		pthread_cond_wait(&stdin_cond, &stdin_mutex);
		pthread_mutex_unlock(&stdin_mutex);

		pthread_mutex_lock(&list_mutex);
		struct mymsg msg = get_msg(0);
		pthread_mutex_unlock(&list_mutex);

		printf(">%d / %s", msg.target, msg.msg);	
	}
}

void *handle_stderr()
{
	while (1)
	{
		pthread_mutex_lock(&err_mutex);
		pthread_cond_wait(&err_cond, &err_mutex);
		pthread_mutex_unlock(&err_mutex);

		pthread_mutex_lock(&list_mutex);
		struct mymsg msg = get_msg(2);
		pthread_mutex_unlock(&list_mutex);

		printf(">%d / %s", msg.target, msg.msg);	
	}
}

void *handle_rcv_msgs()
{
	while (1)
	{
		struct mymsg msg = recieve_message_ipc_system();
		if (msg.type != -1)
		{
			pthread_mutex_lock(&list_mutex);
			add_msg(&msg);
			pthread_mutex_unlock(&list_mutex);

			switch (msg.target)
			{
				case 0:
					pthread_mutex_lock(&stdin_mutex);
					pthread_cond_signal(&stdin_cond);
					pthread_mutex_unlock(&stdin_mutex);
					break;

				case 1:
					pthread_mutex_lock(&output_mutex);
					pthread_cond_signal(&output_cond);
					pthread_mutex_unlock(&output_mutex);
					break;

				case 2:
					pthread_mutex_lock(&err_mutex);
					pthread_cond_signal(&err_cond);
					pthread_mutex_unlock(&err_mutex);
					break;
			}
		}
		else if (argData.ipcType == 0)
		{
			usleep(200 * 1000); // 200 msecs
		}
	}
}

void *handle_send_msgs()
{
	while (1)
	{
		pthread_mutex_lock(&sndmsg_mutex);
		pthread_cond_wait(&sndmsg_cond, &sndmsg_mutex);
		pthread_mutex_unlock(&sndmsg_mutex);

		pthread_mutex_lock(&list_mutex);
		struct mymsg msg = get_msg(-1);
		pthread_mutex_unlock(&list_mutex);

		send_message_ipc_system(msg.msg);
	}
}


void startWork()
{
	init_ipc_system(argData.ftokPath, getpid());
	
	pthread_t thr_stdin, thr_stdinout, thr_stdout, thr_stderr, thr_rcv_msgs, thr_send_msgs;

	pthread_create(&thr_rcv_msgs, NULL, handle_rcv_msgs, NULL);
	pthread_create(&thr_stdin, NULL, handle_stdin, NULL);
	pthread_create(&thr_send_msgs, NULL, handle_send_msgs, NULL);
	pthread_create(&thr_stdinout, NULL, handle_stdin_out, NULL);
	pthread_create(&thr_stdout, NULL, handle_stdout, NULL);
	pthread_create(&thr_stderr, NULL, handle_stderr, NULL);

	pthread_join(thr_rcv_msgs, NULL);
	pthread_join(thr_stdin, NULL);
	pthread_join(thr_send_msgs, NULL);
	pthread_join(thr_stdinout, NULL);
	pthread_join(thr_stdout, NULL);
	pthread_join(thr_stderr, NULL);
}