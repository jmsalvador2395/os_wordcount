#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <math.h>

#define BUFF_SIZE 25
#define R 0
#define W 1

#define FALSE 0
#define TRUE 1

#define CONTINUE 0x0
#define TERMINATE 0x1

int count_words(char *msg, int msg_len, unsigned char *flag);
void itobuff(char *buff, int src);
void bufftoi(int *dest, char *buff);
void itoa(char *buff, unsigned int src);
unsigned char is_space(char *c);


/*
 * code for the parent to execute
 *
 * *file -> file name string
 * *fd -> file descripters for pipe
 */
void exec_parent(char *file, int *fd_p, int *fd_c){
	int wc_total=0;
	unsigned int wc=0;

	close(fd_p[R]);
	close(fd_c[W]);
	
	//initialize message buffer. +1 for null character
	char w_msg[BUFF_SIZE+2]={0};
	unsigned char r_msg[BUFF_SIZE+2]={0};
	
	//open target file
	FILE *fp;
	fp=fopen(file,"r");

	//read file
	size_t r_size;
	char bytes[BUFF_SIZE+1];
	int i;
	while(1){

		//read data from target file
		fgets(w_msg, BUFF_SIZE, fp);
		printf("p: read data from file\n");

		//termination condition
		if(strlen(w_msg)==0){
			w_msg[BUFF_SIZE+1]=1;
			write(fd_p[W], w_msg, BUFF_SIZE+2);
			printf("p: sent termination signal\n");
			
			printf("p: exit\n");
			break;
		}

		//send message buffer to c
		write(fd_p[W], w_msg, strlen(w_msg)+1);
		printf("p: sent data to c\n");
		memset(w_msg, 0, BUFF_SIZE+2);

		//read message buffer from c
		printf("p: waiting for response\n");
		read(fd_c[R], r_msg, BUFF_SIZE); 
		printf("p: received  data from c\n");

		//convert to integer
		bufftoi(&wc, r_msg);
		printf("p: convert message to integer\n");

		//increment total
		wc_total+=wc;
		printf("p: running total: %d\n", wc_total);
	}


	//wait for child to exit
	int status;
	wait(&status);
	printf("p: exit\n");

	//print word count
	printf("p: %d total words\n", wc_total);

	//close pipes
	fclose(fp);
	close(fd_p[W]);
	close(fd_c[R]);
}

/*
 * code for the child process to execute
 *
 * *fd -> file descriptors for pipe
 */
void exec_child(int *fd_p, int *fd_c){
	int wc=0;

	close(fd_p[W]);
	close(fd_c[R]);

	//initialize message buffer. +1 for null character. +1 for flags
	unsigned char w_msg[BUFF_SIZE+2]={0};
	char r_msg[BUFF_SIZE+2]={0};

	//used to tell whether the last character in the previous
	//buffer was a character or space/newline (1 for normal characters)
	unsigned char flag=0x0;

	int i=0;
	while(1){
		//read data from p
		printf("c: waiting for data\n");
		read(fd_p[R], r_msg, BUFF_SIZE+2);
		printf("c: received data from p\n");

		//check for termination condition
		if(r_msg[BUFF_SIZE+1] == 1){
			printf("c: received termination signal\n");
			break;
		}
		//count words
		wc=count_words(r_msg, strlen(r_msg), &flag);
		printf("c: counted words\n");

		itobuff(w_msg, wc);

		//send word count back to p
		write(fd_c[W], w_msg, sizeof(int)+1);
		printf("c: sent result back to p\n");

	}

	printf("c: exit\n");
	
	//close pipes
	close(fd_p[R]);
	close(fd_c[W]);
}

/*
 * push integer bits into buffer.
 * i should have just passed my data back as an integer but here i am.
 */
void itobuff(char *buff, int src){
	int i;
	for(i=3; i>=0; i--){
		buff[3-i]=(char) (src>>(i*8))&0xFF;
	}
}

/*
 * convert buffer bits back into an integer. 
 * again, i really shouldn't have wasted time doing this
 */
void bufftoi(int *dest, char *buff){
	int i;
	for(i=0; i<4; i++){
		*dest<<=8;
		*dest=buff[i]&0xFF;
	}
}

/*
 * this is used to identify spaces or carriage returns
 */
unsigned char is_space(char *c){
	if(*c == '\n' || *c == ' ')
		return 1;
	return 0;
}

/*
 * counts the words in a given buffer
 * flag is used to indicate whether or not the previous buffer
 * ended with a space or not. 
 * 1 if it ended with a normal char, 2 if it ended in a newline or space
 */
int count_words(char *msg, int msg_len, unsigned char *flag){
	//why waste time if the length is 0;
	if(msg_len == 0)
		return 0;

	//initialize count
	int count=0;

	//flag check
	if(*flag && !is_space(&msg[0]))
		count-=1;

	//reset flag
	*flag=0;
	if(!is_space(&msg[msg_len-1]))
		*flag=1;

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

	//init pipes
	int fd_p[2];
	int fd_c[2];
	if(pipe(fd_p) == -1 || pipe(fd_c) == -1){
		fprintf(stderr, "pipe creation failed\n");
		return 1;
	}

	//split between parent and child processes
	pid_t pid = fork();

	//code for parent to execute
	if(pid > 0){
		exec_parent(argv[1], fd_p, fd_c);
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
