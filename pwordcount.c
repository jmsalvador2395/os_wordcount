#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#define BUFF_SIZE 25
#define R 0
#define W 1

/*
 * code for the parent to execute
 *
 * *file -> file name string
 * *fd -> file descripters for pipe
 */
void exec_parent(char *file, int *fd){

	//initialize message buffer. +1 for null character
	char w_msg[BUFF_SIZE+1]={0};

	printf("--parent--\n");
	close(fd[R]);

	FILE *src_fd;

	src_fd=fopen(file,"r");

	size_t r_size;

	r_size=fread(w_msg, sizeof(char), BUFF_SIZE, src_fd);

	printf("r_size: %ld\nfd: %s\n", r_size, w_msg);

	fclose(src_fd);

	write(fd[W], w_msg, strlen(w_msg)+1);

	close(fd[W]);
}

/*
 * code for the child process to execute
 *
 * *fd -> file descriptors for pipe
 */
void exec_child(int *fd){

	//initialize message buffer. +1 for null character
	char r_msg[BUFF_SIZE+1]={0};

	printf("--child--\n");
	close(fd[W]);

	read(fd[R], r_msg, BUFF_SIZE);

	printf("child msg: %s\n", r_msg);

	close(fd[R]);

}


int main(int argc, char **argv){
	
	//check for input file name
	if(argc < 2){
		printf("input file name required\n");
		printf("usage: ./pwordcount <file_name>\n")
		return 0;
	}

	//split between parent and child processes
	pid_t pid = fork();
	
	//init pipe
	int fd[2];
	if(pipe(fd) == -1){
		fprintf(stderr, "pipe failed\n");
		return 1;
	}

	//code for parent to execute
	if(pid > 0){
		exec_parent(argv[1], fd);
	}
	//code for child to execute
	else if(pid == 0){
		exec_child(fd);
	}
	//code for error in fork()
	else{
		fprintf(stderr, "fork() failed\n");
		return 2;
	}

	return 0;
}
