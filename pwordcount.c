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
	printf("p: fd_p=%d, fd_c=%d\n", (int) fd_p[W], (int)fd_c[R]);
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
	//for(i=0;i<3;i++){

		fgets(w_msg, BUFF_SIZE, fp);

		printf("p: read data from file\n");
		if(strlen(w_msg)==0){
			printf("p: exit\n");
			break;
		}

		//send message buffer to c
		write(fd_p[W], w_msg, strlen(w_msg)+1);
		printf("p: sent data to c\n");
		memset(w_msg, 0, BUFF_SIZE+1);

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

	printf("p: pause\n");
	//print word count
	printf("p: %d total words\n", wc_total);

	printf("p: exit\n");

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

	//initialize message buffer. +1 for null character, +1 for header at idx 0
	unsigned char w_msg[BUFF_SIZE+2]={0};
	char r_msg[BUFF_SIZE+2]={0};

	//used to tell whether the last character in the previous
	//buffer was a character or space/newline (1 for normal characters)
	unsigned char flag=0x0;


	int i=0;
	while(1){
		//read data from p
		printf("c: waiting for data\n");
		read(fd_p[R], r_msg, BUFF_SIZE);
		printf("c: received data from p\n");

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

unsigned char is_space(char *c){
	if(*c == '\n' || *c == ' ')
		return 1;
	return 0;
}

int count_words(char *msg, int msg_len, unsigned char *flag){
	if(msg_len == 0)
		return 0;

	int count=0;

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

	//init pipe
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
