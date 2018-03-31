#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_SIZE 512
#define MAX_ARGS 32
char line[MAX_SIZE];
char *newargv[MAX_ARGS];

void show_help()
{
	printf("\n\tOption: t - interval, c - command, h -help\n\n");
}

void dump_command()
{
	int i = 0;

	printf("Run command: ");
	while (newargv[i] != NULL) {
		printf("%s ", newargv[i]);
		i++;
	}

	printf("\n");
}

int run_command(char *command)
{
	char *p = NULL;
	int i = 0;
	char *cc[] = {NULL};

	p = strtok(command, " ");
	while (p != NULL) {
		newargv[i] = p;
		i++;
		p = strtok(NULL, " ");
	}

	newargv[i] = NULL;

	dump_command();

	execve(newargv[0], newargv, cc);
	perror("execve");

	return 0;
	
}

int main(int argc, char *argv[])
{
	int opt, fd, interval;
	char *fname = NULL;
	FILE *file = NULL;
	char *command = NULL;
	pid_t pid;
	int signal = 3;

	while ((opt = getopt(argc, argv, "t:f:c:s:h")) != -1) {
		switch (opt) {
		case 't':
			interval = atoi(optarg);
			break;
		case 'f':
			fname = optarg;
			break;
		case 'c':
			command = optarg;
			break;
		case 's':
			signal = atoi(optarg);
			break;
		case 'h':
		default:
			show_help();
			break;
		}
	}

	//printf("command=%s\n", command);

	if ((pid = fork()) < 0) {
		perror("fork");
	} else if (pid == 0) {
		sleep(3);
		run_command(command);
	} else {
		printf("PID(%d) after %d seconds will recv %d signal\n", pid, interval, signal);
	}

	sleep(interval);
	kill(pid, signal);

	waitpid(pid, &opt, WNOHANG);

	return 0;
}
