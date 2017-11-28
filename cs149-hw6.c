#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <string.h>

#define RUN_DURATION 30
#define BUFFER_SIZE 512
#define READ_END 0
#define WRITE_END 1



int main()
{
	int i = 0, message_number = 1;
	pid_t pid;
	int write_file, result, rbytes, maxfd;
	fd_set rfds;
	struct timeval tv, start, end, current;
	char readbuffer[BUFFER_SIZE];
	double time_spent = 0;
	double current_time;
	char message_output[BUFFER_SIZE];
	int pipes[10];
	
	srand(time(NULL));	
	
	for(i=0; i<5; i++) {
		pipe(pipes + 2*i);
	}
		
	char *pos;
		
	gettimeofday(&start, NULL);
	int process_ID[5];

	if((pid = fork()) == 0) //FIRST CHILD [0][1]
	{
		srand(time(NULL) + 0);
		process_ID[0] = pid;	
		close(pipes[READ_END]);
		while(time_spent < RUN_DURATION) {
		
			gettimeofday(&current, NULL);
			current_time = (double) (current.tv_usec - start.tv_usec) / 1000000 + 
						   (double) (current.tv_sec - start.tv_sec);

			sprintf(message_output, "%.3f: Child 1: Message #%d", current_time, message_number++);

			write(pipes[WRITE_END], message_output, sizeof(message_output));
			gettimeofday(&end, NULL);
			time_spent = (double)(end.tv_sec - start.tv_sec);
			sleep(rand() % 3);
		}
		exit(0);
	} 
	else 
	{
		process_ID[1] = pid;
		if((pid = fork())==0)
			//SECOND CHILD [2][3]
		{
			srand(time(NULL) + 1);
			close(pipes[READ_END + 2]);		
			// second child goes in this loop		
			while(time_spent < RUN_DURATION)
			{
				gettimeofday(&current, NULL);
				current_time = (double) (current.tv_usec - start.tv_usec) / 1000000 + 
						   	(double) (current.tv_sec - start.tv_sec);
				sprintf(message_output, "%.3f: Child 2: Message #%d", current_time, message_number++);
				write(pipes[WRITE_END + 2], message_output, sizeof(message_output));
				gettimeofday(&end, NULL);
				time_spent = (double)(end.tv_sec - start.tv_sec);
				sleep(rand() % 3);
			}
			exit(0);
		} 
		else  
		{
			if((pid = fork())==0)
			{			//THIRD CHILD [4][5]
				srand(time(NULL) + 2);
				process_ID[2] = pid;	
				close(pipes[READ_END + 4]);
				while(time_spent < RUN_DURATION)
				{
					gettimeofday(&current, NULL);
					current_time = (double) (current.tv_usec - start.tv_usec) / 1000000 + 
						   (double) (current.tv_sec - start.tv_sec);

					sprintf(message_output, "%.3f: Child 3: Message #%d", current_time, message_number++);

					write(pipes[WRITE_END + 4], message_output, sizeof(message_output));
					gettimeofday(&end, NULL);
					time_spent = (double)(end.tv_sec - start.tv_sec);
					sleep(rand() % 3);
				}
				exit(0);
			}
			else
			{
				
				if((pid = fork())==0)
					//FOURTH CHILD	[6][7]
				{
					srand(time(NULL) + 3);
					process_ID[3] = pid;
					close(pipes[READ_END + 6]);
					while(time_spent < RUN_DURATION)
					{
						gettimeofday(&current, NULL);
						current_time = (double) (current.tv_usec - start.tv_usec) / 1000000 + 
						   	(double) (current.tv_sec - start.tv_sec);
						sprintf(message_output, "%.3f: Child 4: Message #%d", current_time, message_number++);
						write(pipes[WRITE_END + 6], message_output, sizeof(message_output));
						gettimeofday(&end, NULL);
						time_spent = (double)(end.tv_sec - start.tv_sec);
						sleep(rand() % 3);
					}
					
					exit(0);
				} 
				else
				{
					if((pid = fork())==0)
						// FIFTH CHILD [8][9]
					{
						srand(time(NULL) + 4);
						process_ID[4] = pid;	
						char buf[BUFFER_SIZE];
						while(time_spent < RUN_DURATION)
						{
							printf("%s: ", "Enter 5th Child Message");
							fgets(buf, BUFFER_SIZE, stdin);
							gettimeofday(&current, NULL);
							current_time = (double) (current.tv_usec - start.tv_usec) / 1000000 + 
										   (double) (current.tv_sec - start.tv_sec);
							sprintf(message_output, "%.3f: Child 5: Message %s", current_time, buf);
							if ((pos=strchr(message_output, '\n')) != NULL) {
								*pos = '\0';
							}
							write(pipes[WRITE_END + 8], message_output, strlen(message_output));
							gettimeofday(&end, NULL);
							time_spent = (double)(end.tv_sec - start.tv_sec);
						}
						exit(0);
					} 
					else //PARENT 
					{	

						write_file = open("output.txt", O_CREAT | O_RDWR | O_TRUNC, 0777);
						
						do {
							maxfd = 0;
							FD_ZERO(&rfds);
							for(i=0; i<5; i++) {
								FD_SET(pipes[i*2], &rfds);
								maxfd = (maxfd > pipes[i*2])? maxfd : pipes[i*2];
							}
							tv.tv_sec = 2;
							tv.tv_usec = 0;
							result = select(maxfd+1, &rfds, NULL, NULL, &tv);
							if(result == -1) {
								perror("select");
							}
							else if(result) {
								for(i=0; i<5; i++) {									
									if(FD_ISSET(pipes[i*2], &rfds)) {
										gettimeofday(&current, NULL);
										current_time = (double) (current.tv_usec - start.tv_usec) / 1000000 + 
													   (double) (current.tv_sec - start.tv_sec);
										memset(readbuffer,0,strlen(readbuffer));
										rbytes = read(pipes[i*2], readbuffer, BUFFER_SIZE);
										if(rbytes > 0) {
											sprintf(message_output, "%s has arrived at %.3f\n", readbuffer, current_time);
											write(write_file, message_output, strlen(message_output));
										}
										break;
									}
								}
							}
								
							gettimeofday(&end, NULL);
							time_spent = (double)(end.tv_sec-start.tv_sec);
						} while(time_spent < RUN_DURATION);
						close(write_file);
					}					
				}				
			}		
		}	
	}
	
	return 0;
}
