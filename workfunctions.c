#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>

void child_out_handle(int signum);
int handle_io_select(int child_pid);
void start_timer();
void writelog(char *buffer, int target);

int pipe_rw[2], pipe_wr[2], pipe_err[2];
int isAnyIO = 0, secs_timer = 1, logfilefd, childpid;

int startProgram(char *path, int multiplex, int logfile_fd)
{
	logfilefd = logfile_fd;

	// wr: parent [w]rite, child [r]ead
	int status;
	pipe(pipe_wr);
	pipe(pipe_rw);
	pipe(pipe_err);

	pid_t pid = fork();
	if (pid == 0)
	{
		// child
		dup2(pipe_wr[0], STDIN_FILENO);
		dup2(pipe_rw[1], STDOUT_FILENO);
		dup2(pipe_err[1], STDERR_FILENO);

		//char *args[] = {path, NULL};
		//execv(args[0], args);
		system(path);
		perror("system is failed");
		exit(1);
	} 
	else 
	{
		// parent: handler
		childpid = pid;
		pid_t newPid = fork();
		if (newPid == 0)
		{
			// child: handle stdin
			char buffer[2048];
			while (1)
			{
				int read_cnt = read(STDIN_FILENO, buffer, sizeof(buffer));
				if (read_cnt > 0)
				{
					if (strcmp(buffer, "exit\n") == 0)
					{
						kill(pid, SIGTERM);
						return 0;
					}

					write(pipe_wr[1], buffer, read_cnt);
					writelog(buffer, STDIN_FILENO);
				}
			}
		}
		else
		{
			// parent: handle stdout
			signal(SIGTERM, SIG_DFL);
			signal(SIGCHLD, SIG_DFL);
			if (multiplex)
			{
				status = handle_io_select(pid);
			}
			else
			{
				signal(SIGIO, child_out_handle);
				signal(SIGABRT, child_out_handle);
				fcntl(pipe_rw[0], F_SETOWN, getpid());
				fcntl(pipe_rw[0], F_SETFL, FASYNC | O_NONBLOCK);
				start_timer();
				waitpid(pid, &status, 0);
			}
			printf("%d TERMINATED WITH EXIT CODE: %d\n", pid, status);
			kill(newPid, SIGTERM);
			close(logfilefd);
		}
		
	}
	return 0;
}

void sigalrm_handler()
{
	if (!isAnyIO)
	{
		writelog("NO I/O", -1);
	} 
	else 
	{
		isAnyIO = 0;
	}
}

void start_timer()
{
	struct itimerval it_val;
	if (signal(SIGALRM, sigalrm_handler) == SIG_ERR)
	{
		exit(1);
	}
	it_val.it_value.tv_sec = secs_timer;
	it_val.it_value.tv_usec = 0;
	it_val.it_interval = it_val.it_value;
	setitimer(ITIMER_REAL, &it_val, NULL);
}

void child_out_handle(int signum)
{
	//puts("SIGNAL");
	isAnyIO = 1;
	char buffer[2048];
	int read_cnt = read(pipe_rw[0], buffer, sizeof(buffer));
	if (read_cnt > 0)
	{
		writelog(buffer, STDOUT_FILENO);
		isAnyIO = 1;
	}
}

int handle_io_select(int child_pid)
{
	int ready_fd, status, res_pid, read_cnt;
	char buffer[2048];

	fd_set fdset_r;
	struct timeval timeout;
	timeout.tv_usec = 0;
	while (1)
	{
		FD_ZERO(&fdset_r);
		FD_SET(pipe_rw[0], &fdset_r);
		FD_SET(pipe_err[0], &fdset_r);
		timeout.tv_sec = secs_timer;
		ready_fd = select(sizeof(fdset_r), &fdset_r, NULL, NULL, &timeout);
		
		if (ready_fd < 0)
		{
			perror("select error");
		}
		else if (ready_fd == 0)
		{
			res_pid = waitpid(child_pid, &status, WNOHANG | WUNTRACED);
			if (res_pid == child_pid) return status;
			writelog("NO I/O", -1);
		} 
		else
		{
			//puts("SELECT");
			if (FD_ISSET(pipe_rw[0], &fdset_r))
			{
				read_cnt = read(pipe_rw[0], buffer, sizeof(buffer));
				if (read_cnt > 0)
				{
					writelog(buffer, STDOUT_FILENO);
				}
			} 
			else if (FD_ISSET(pipe_err[0], &fdset_r))
			{
				read_cnt = read(pipe_err[0], buffer, sizeof(buffer));
				if (read_cnt > 0) 
				{
					writelog(buffer, STDERR_FILENO);
				}
			}
		}
	}
}

void writelog(char *buffer, int target)
{
	char out_buffer[8192];
	time_t now = time(0);
	struct tm *t = localtime(&now);
	char time_str[128];
	sprintf(time_str, "%d.%d.%d %d:%d:%d", t->tm_mday, t->tm_mon+1,
		t->tm_year+1900, t->tm_hour, t->tm_min, t->tm_sec);

	if (target == -1)
	{
		// log
		sprintf(out_buffer, "%s, %s\n", time_str, buffer);
	} 
	else 
	{
		// out
		sprintf(out_buffer, "%d / >%d / %s", childpid,
			target, buffer);
		write(target, out_buffer, strlen(out_buffer));

		// log
		sprintf(out_buffer, "%s / >%d / %s", time_str,
			target, buffer);
	}
	write(logfilefd, out_buffer, strlen(out_buffer));
}