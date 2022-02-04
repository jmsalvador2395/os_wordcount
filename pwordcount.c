#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#define BUFF_SIZE 25
#define R 0
#define W 1
#define FALSE 0
#define TRUE 1

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
	printf("parent R: %d, W: %d\n", fd[0], fd[1]);

	FILE *src_fd;
	src_fd=fopen(file,"r");

	size_t r_size;

	r_size=fread(w_msg, sizeof(char), BUFF_SIZE, src_fd);


	printf("w_msg: %s\n", w_msg);
	printf("parent before write\n");
	write(fd[W], w_msg, strlen(w_msg)+1);
	printf("parent finished write\n");

	sleep(5);

	printf("parent close\n");
	fclose(src_fd);
	close(fd[R]);
	close(fd[W]);
}


int count_words(char *msg, int msg_len){
	int count=0;

	//indicates a previous character being a space or carriage return
	char break_prev=TRUE;
	
	int i;
	for(i=0; i<msg_len; i++){
		if(msg[i] != '\n' && msg[i] != ' '){
			if(break_prev){
				count++;
				break_prev=FALSE;
			}
		}
		//goes here if a space or carriage return is encountered
		else{
			if(!break_prev)
				break_prev=TRUE;
		}
	}

	return count;
}



/*
 * code for the child process to execute
 *
 * *fd -> file descriptors for pipe
 */
void exec_child(int *fd){
	int wc=0;

	//initialize message buffer. +1 for null character
	char r_msg[BUFF_SIZE+1]={0};

	sleep(1);

	printf("--child--\n");
	printf("child R: %d, W: %d\n", fd[0], fd[1]);

	close(fd[W]);

	int i=0;

	read(fd[R], r_msg, BUFF_SIZE);

	printf("child message: %s\n", r_msg);

	wc+=count_words(r_msg, strlen(r_msg));

	printf("word count: %d\n", wc);

	close(fd[R]);
	printf("child close\n");
}

int main(int argc, char **argv){
	
	//check for input file name
	if(argc < 2){
		printf("input file name required\n");
		printf("usage: ./pwordcount <file_name>\n");
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
