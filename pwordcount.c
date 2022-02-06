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
void bufftoi(int dest, char *buff);
void itoa(char *buff, int src);

void close_pipes(int *fd){
	close(fd[R]);
	close(fd[W]);
}
/*
 * code for the parent to execute
 *
 * *file -> file name string
 * *fd -> file descripters for pipe
 */
void exec_parent(char *file, int *fd){
	int wc_total=0;
	int wc=0;
	
	//initialize message buffer. +1 for null character
	char w_msg[BUFF_SIZE+1]={0};
	unsigned char r_msg[BUFF_SIZE+1]={0};
	
	//open target file
	FILE *src_fd;
	src_fd=fopen(file,"r");

	//read file
	size_t r_size;
	printf("p1: read file\n");
	r_size=fread(w_msg, 1, BUFF_SIZE, src_fd);

	//send message buffer to p2
	printf("p1: send data to p2\n");
	write(fd[W], w_msg,strlen(w_msg)+1);

	sleep(1);
	//read message buffer from p2
	printf("p1: read data from p2\n");
	read(fd[R], r_msg, BUFF_SIZE); 

	int i;
	/*
	printf("from p2: ");
	for(i=0; i<BUFF_SIZE; i++){
		printf("%02x ", r_msg[i]);
	}
	printf("\n");
	
	bufftoi(wc, r_msg);
	*/
	wc=atoi(r_msg);
	printf("wc: %d\n", wc);

	wc_total+=wc;

	//print word count
	printf("p1: %d total words\n", wc_total);

	printf("p1: exit\n");
	fclose(src_fd);
	close_pipes(fd);
}

/*
 * code for the child process to execute
 *
 * *fd -> file descriptors for pipe
 */
void exec_child(int *fd){
	int wc=0;

	//initialize message buffer. +1 for null character
	unsigned char w_msg[BUFF_SIZE+1]={0};
	char r_msg[BUFF_SIZE+1]={0};

	int i=0;

	//read data from p1
	printf("p2: read data from p1\n");
	read(fd[R], r_msg, BUFF_SIZE);

	//count words
	printf("p2: counting words\n");
	wc=count_words(r_msg, strlen(r_msg));
	
	/*
	itobuff(w_msg, wc);

	printf("to p1: ");
	for(i=0;i<4;i++){
		printf("%02x ", w_msg[i]);
	}
	printf("\n");
	*/
	printf("converting to str\n");

	itoa(w_msg, wc);

	printf("itoa: %s\n", w_msg);
	//send word count back to p1
	printf("p2: send result back to p1\n");
	write(fd[W], w_msg, 5);
	
	close_pipes(fd);
}

void itobuff(char *buff, int src){
	int i;
	for(i=3; i>=0; i--){
		buff[3-i]=(char) (src>>(i*8))&0xFF;
	}
}

void bufftoi(int dest, char *buff){
	int i;
	for(i=3; i>=0; i--){
		dest=buff[i]&0xFF;
		dest<<=8;
	}
}

void itoa(char *buff, unsigned int src){

	int temp=src;
	int power=1;

	while(power<temp)
		power*=10;
	power/=10;

	char digit;
	int i;
	while(temp != 0){
		digit= ((temp/power)&0xFF) + 0x30;
		printf("%d ", digit);
		//buff[i]=digit;

		i+=1;
		if(digit!=0)
			temp=temp-digit*power;
		if(power!=1)
			power/=10;
	}
	printf("\n");
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
	int fd[2];
	if(pipe(fd) == -1){
		fprintf(stderr, "pipe failed\n");
		return 1;
	}

	//split between parent and child processes
	pid_t pid = fork();

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
