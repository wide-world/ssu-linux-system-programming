#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void signal_handler(int signo);
int timelimit;

int main(int argc, char *argv[])
{
signal(SIGALRM, signal_handler);

	// timelimit 입력되지 않은 경우
	if (argv[1] == NULL) {
		while (1) {
			if (fork() == 0)
				execl("/bin/date", "date", (char *)0);

			sleep(1);
		}
	}
	
	// timelimit 입력된 경우 
	if (atoi(argv[1]) != 0) {
		timelimit = atoi(argv[1]);
		alarm(timelimit);
		while (1) {
			if (fork() == 0)
				execl("/bin/date", "date", (char *)0);
			
			wait((int *)0);
			if (timelimit <= 5)
				printf("종료 %d초 전...\n", timelimit);

			timelimit--;
			sleep(1);
		}
	}
	exit(0);
}

void signal_handler(int signo) {
	raise(SIGTERM);
}
