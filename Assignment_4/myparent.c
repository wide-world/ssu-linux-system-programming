#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

void print_process_list(int signo);

int numofchild;
pid_t* process_list;
pid_t pid;

int main(int argc, char *argv[])
{
	signal(SIGALRM, print_process_list);
	numofchild = atoi(argv[1]);
	process_list = (pid_t *)malloc(sizeof(int) * numofchild);

	for (int i = 0; i < numofchild; i++) {
		pid = fork();
		if (pid < 0) {
			fprintf(stderr, "fork error\n");
			exit(1);
		}
		else if (pid == 0) {
			while (1) {
				sleep(5);
				printf("#order %d: %d\n", i + 1, getpid());
			}
		}			
		else
			process_list[i] = pid;
	}
	alarm(5);
	sleep(5);
	for (int i = numofchild - 1; i >= 0; i--) {
		kill(process_list[i], SIGKILL);
		sleep(1);
	}
	free(process_list);
	raise(SIGTERM);
}

void print_process_list(int signo) {
	system("date");
	printf("\n");
	alarm(5);
}
