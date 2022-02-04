#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#define BUFF_SIZE 25
#define R 0
#define W 1

int main(int argc, char** argv){
	
	if(argc < 2){
		printf("input file name required\n");
		return 0;
	}

	pid_t pid = fork();

	char r_msg[BUFF_SIZE];
	char w_msg[BUFF_SIZE];

	int fd[2];

	if(pid == 0){
		printf("--child--\n");
	}
	else if(pid > 0){
		printf("--parent--\n");
	}
	else{
		printf("fork() failed\n")
		return 1;
	}

	return 0;
}
