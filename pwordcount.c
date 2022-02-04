#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#define BUFF_SIZE 25
#define R 0
#define W 1

int main(int argc, char **argv){
	
	if(argc < 2){
		printf("input file name required\n");
		return 0;
	}

	pid_t pid = fork();

	char r_msg[BUFF_SIZE];
	char w_msg[BUFF_SIZE]="hello\n";

	int fd[2];

	if(pipe(fd) == -1){
		fprintf(stderr, "pipe failed\n");
		return 1;
	}

	if(pid > 0){
		printf("--parent--\n");
		close(fd[R]);

		FILE *src_fd;

		src_fd=fopen(argv[1],"r");

		size_t r_size;

		r_size=fread(src_fd, w_msg, BUFF_SIZE);

		printf("fd: %s\n", w_msg);

		fclose(src_fd);

		write(fd[W], w_msg, strlen(w_msg)+1);

		close(fd[W]);
	}
	else if(pid == 0){
		printf("--child--\n");
		close(fd[W]);

		read(fd[R], r_msg, BUFF_SIZE);

		printf("child msg: %s\n", r_msg);

		close(fd[R]);
	}
	else{
		fprintf(stderr, "fork() failed\n");
		return 2;
	}

	return 0;
}
