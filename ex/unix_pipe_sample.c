/**
 * Example program demonstrating UNIX pipes.
 *
 * Figures 3.25 & 3.26
 *
 * @author Silberschatz, Galvin, and Gagne
 * Operating System Concepts  - Ninth Edition
 * Copyright John Wiley & Sons - 2013
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#define BUFFER_SIZE 25
#define READ_END	0
#define WRITE_END	1

int main(void)
{
	char write_msg[BUFFER_SIZE] = "Greetings";
	char read_msg[BUFFER_SIZE];
	pid_t pid;
	int fd[2];
	int fd2[2];

	/* create the pipe */
	if (pipe(fd) == -1 || pipe(fd2) == -1) {
		fprintf(stderr,"Pipe failed");
		return 1;
	}

	/* now fork a child process */
	pid = fork();

	if (pid < 0) {
		fprintf(stderr, "Fork failed");
		return 1;
	}

	if (pid > 0) {  /* parent process */
		/* close the unused end of the pipe */
		close(fd[READ_END]);

		/* write to the pipe */
		int i;
		for(i=0; i<10; i++){
			write(fd[WRITE_END], write_msg, strlen(write_msg)+1); 
			printf("p sends message\n\n");

			read(fd2[READ_END], read_msg, BUFFER_SIZE);
			printf("p gets message: %s\n\n", read_msg);
		}
		memset(write_msg, 0, BUFFER_SIZE);
		write(fd[WRITE_END], write_msg, strlen(write_msg)+1); 


		/* close the write end of the pipe */
		close(fd[WRITE_END]);
	}
	else { /* child process */
		/* close the unused end of the pipe */
		close(fd[WRITE_END]);
		int i;
		for(i=0; i<10; i++){
			/* read from the pipe */
			read(fd[READ_END], read_msg, BUFFER_SIZE);
			printf("child read %s\n\n",read_msg);

			strcpy(write_msg, "ack");
			write(fd2[WRITE_END], write_msg, strlen(write_msg)+1);
			printf("child send\n\n");
		}

		/* close the write end of the pipe */
		close(fd[READ_END]);
	}

	return 0;
}
