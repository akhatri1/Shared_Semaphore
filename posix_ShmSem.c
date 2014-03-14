#include<stdio.h>
#include<unistd.h>
#include<signal.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<string.h>
#include<errno.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include <sys/shm.h>
#include<semaphore.h>
#include<sys/ipc.h>
#include<sys/mman.h>

#define MAXBUFF 4096
#define semPath1 "semfile1"
#define semPath2 "semfile2"
/*Message Structure*/
struct Message
{
	int msg_len;
	int msg_type;
	char msg_buff[MAXBUFF];
}msg;

void executeCommand(char*);

int main(int argc, char **argv)
{
	/*Initializing the Variables*/
	int n=0, len, readfd_client,writefd_client,readfd_server,writefd_server, flag = 0, fd;
	pid_t childPid;
	char buff[512],command[512],command1[512],*Com = NULL,*checkOpcode = NULL, *reply, *shm, *shm1;
	setbuf(stdout,NULL);
	fd = shm_open("/myregion", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	
	if (ftruncate(fd, 512) ==-1){ printf("Error changing the size\n");}
	shm = mmap(NULL, 512, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	
	if (shm == MAP_FAILED){printf("Error in mapping \n");}	

	/*Semaphore Initialization */
	sem_t *sem1,*sem2;
	sem1= sem_open("psem",O_CREAT, 0644,0);
	sem2= sem_open("psem1",O_CREAT, 0644,0);
						childPid = fork();
						if(childPid == 0)
						{
							/*Server Process*/
							
							while(1){
							
							//sleep(5);
							sem_wait(sem1);
							strcpy(msg.msg_buff,shm);
							
							
							executeCommand(msg.msg_buff);
							strcpy(shm, msg.msg_buff);
							sem_post(sem2);
							
							
							}
						
						
							
						}
						else
						{
							
								while(1)
								{
									
		 							printf("Enter the command:\n");
									/*Take the input from the console*/
		
									gets(command);
									strcpy(command1,command);
			
									checkOpcode = strtok(command1," ");
									if(checkOpcode == NULL)
									{
										printf("Please enter a proper command\n");
									}
									else
									{
										/*Validating Opcode*/
										if(strcmp(checkOpcode,"Read") == 0 || strcmp(checkOpcode,"Delete") == 0 || strcmp(checkOpcode,"Exit") == 0)
										{
											if(strcmp(checkOpcode,"Exit") == 0)	
											{	
											///*Garbage Collection*/
											sem_close(sem1);
											sem_close(sem2);
											sem_unlink("psem");
											sem_unlink("psem1");
											shm_unlink("/myregion");
											kill(childPid,SIGKILL);
											
											exit(0);
											}
											else
											{
												
												strcpy(shm,command);
												sem_post(sem1);
												
												/*Displaying the Output on the console*/
												//sleep(5);
												sem_wait(sem2);
												printf("Client :%s\n",shm);
											}
										}
										else
										{
											printf("Please specify the Opcode Correctly\n");
											printf("\tRead\n");
											printf("\tDelete\n");
											printf("\tExit\n");
										}
									}
								}
	
							}
}

/*Function for Reading the file, Deleting the file*/
void executeCommand(char *com)
{
	char response[512], *filename, *opcode, ch,com1[512];
	int i=0, Status;
	FILE *fp1 = NULL;
	strcpy(com1,com);
	opcode = strtok(com1," ");
	filename = strtok(NULL," ");
	
	if((fp1 = fopen(filename,"r")) == NULL)
	{
		strcpy(com,"File not found :: Unsuccessful Operation");
		return;
	}
	fclose(fp1);
	
	fp1 = NULL;
	
	/*Read operation*/
	if(strcmp(opcode,"Read") == 0)
	{
		
		fp1 = fopen(filename,"r");
		while(1)
    	        {
      			ch = fgetc(fp1);

      			if(ch==EOF)
         		break;
      			else
          		{ response[i] = ch;
				i++;
			}
    		}
		response[i] = '\0';
		strcpy(com,response);
		fclose(fp1);
	}

	/*Delete operation*/	
	if(strcmp(opcode,"Delete") == 0)
	{
			
		
		/*Removing the requested file*/
		Status = remove(filename);
		if(Status == 0)
		{
			strcpy(com,"File is removed");
		}
		/*What if the file is present but the directory doesn't have write permission*/
		else
		{
			strcpy(com,"Error File cannot be deleted: Seems like the parent directory doesn't have write permissions");
		}
		
	}
	return; 
	/*Finishing the process*/
	
}
