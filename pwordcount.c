#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>

#define BUFF_SIZE 25
#define R 0
#define W 1

#define FALSE 0
#define TRUE 1

#define CONTINUE 0x0
#define TERMINATE 0x1

int count_words(char *msg, int msg_len);
void itobuff(char *buff, int src);
void bufftoi(int *dest, char *buff);
void itoa(char *buff, unsigned int src);

void close_pipe_ends(int *fd_r, int *fd_w){
	close(fd_r[W]);
	close(fd_w[R]);
}

/*
 * code for the parent to execute
 *
 * *file -> file name string
 * *fd -> file descripters for pipe
 */
void exec_parent(char *file, int *fd_r, int *fd_w){
	int wc_total=0;
	unsigned int wc=0;
	close_pipe_ends(fd_r, fd_w);
	
	//initialize message buffer. +1 for null character
	char w_msg[BUFF_SIZE+1]={0};
	unsigned char r_msg[BUFF_SIZE+1]={0};
	
	//open target file
	FILE *src_fd;
	src_fd=fopen(file,"r");

	//read file
	size_t r_size;
	printf("p1: read file\n");
	while(r_size=fread(w_msg, 1, BUFF_SIZE, src_fd)){

		printf("r_size: %d\n", (int) r_size);
		printf("from file: %s\n", w_msg);

		//send message buffer to p2
		write(fd_w[W], w_msg,r_size+1);
		memset(w_msg, 0, BUFF_SIZE+1);
		printf("p1: sent data to p2\n");

		//read message buffer from p2
		read(fd_r[R], r_msg, BUFF_SIZE); 
		printf("p1: received  data from p2\n");

		//convert to integer
		bufftoi(&wc, r_msg);

		//increment total
		wc_total+=wc;

		//read new words
		printf("\n");
	}
	//print word count
	printf("p1: %d total words\n", wc_total);

	printf("p1: exit\n");

	//close pipes
	fclose(src_fd);
	close(fd_r[R]);
	close(fd_w[W]);
}

/*
 * code for the child process to execute
 *
 * *fd -> file descriptors for pipe
 */
void exec_child(int *fd_r, int *fd_w){
	int wc=0;

	close_pipe_ends(fd_r, fd_w);

	//initialize message buffer. +1 for null character
	unsigned char w_msg[BUFF_SIZE+1]={0};
	char r_msg[BUFF_SIZE+1]={0};

	int i=0;
	while(1){
		//read data from p1
		read(fd_r[R], r_msg, BUFF_SIZE);
		printf("p2: received data from p1\n");

		printf("p2 r_msg: %s\n", r_msg);
		//count words
		wc=count_words(r_msg, strlen(r_msg));
		memset(r_msg, 0, BUFF_SIZE);
		printf("p2: counted words\n");

		itobuff(w_msg, wc);

		//send word count back to p1
		write(fd_w[W], w_msg, sizeof(int)+1);
		printf("p2: sent result back to p1\n");
	}

	printf("p2: exit");
	
	//close pipes
	close(fd_r[R]);
	close(fd_w[W]);
}

void itobuff(char *buff, int src){
	int i;
	for(i=3; i>=0; i--){
		buff[3-i]=(char) (src>>(i*8))&0xFF;
	}
}

void bufftoi(int *dest, char *buff){
	int i;
	for(i=0; i<4; i++){
		*dest<<=8;
		*dest=buff[i]&0xFF;
	}
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

int main(int argc, char **argv){
	
	//check for input file name
	if(argc < 2){
		printf("input file name required\n");
		printf("usage: ./pwordcount <file_name>\n");
		return 0;
	}

	//init pipe
	int fd_p[2];
	int fd_c[2];

	if(pipe(fd_p) == -1){
		fprintf(stderr, "pipe 1 failed\n");
		return 1;
	}

	if(pipe(fd_c) == -1){
		fprintf(stderr, "pipe 2 failed\n");
		return 1;
	}

	//split between parent and child processes
	pid_t pid = fork();

	//code for parent to execute
	if(pid > 0){
		exec_parent(argv[1], fd_c, fd_p);
	}
	//code for child to execute
	else if(pid == 0){
		exec_child(fd_p, fd_c);
	}
	//code for error in fork()
	else{
		fprintf(stderr, "fork() failed\n");
		return 2;
	}

	return 0;
}
