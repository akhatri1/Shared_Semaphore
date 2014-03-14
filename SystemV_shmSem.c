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
#include<sys/sem.h>
#include<sys/ipc.h>

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

union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
};
void executeCommand(char*);

int main(int argc, char **argv)
{
	/*Initializing the Variables*/
	int n=0, len, readfd_client,writefd_client,readfd_server,writefd_server, flag = 0;
	pid_t childPid;
	char buff[512],command[512],command1[512],*Com = NULL,*checkOpcode = NULL, *reply, *shm, *shm1;
	setbuf(stdout,NULL);
	union semun arg1, arg2;
	struct sembuf postop, waitop;
	int id = shmget(IPC_PRIVATE,512,IPC_CREAT | 0666);
	//int id1 = shmget(IPC_PRIVATE,512,IPC_CREAT | 0666);
	
	int semid1 = semget(IPC_PRIVATE,1, 0666 | IPC_CREAT | IPC_EXCL);	
	int semid3;
	//struct sem s1;
	arg1.val = 0;
	if (semctl(semid1, 0, SETVAL, arg1) == -1) {
	perror("semctl"), exit(1);
    	}



	//semctl(semid1,0, SETVAL, arg1);
	n = semctl(semid1,0,GETVAL,arg1);
	//printf("Value of semid1: %d\n",n);
	semid3 = semget(IPC_PRIVATE,1, 0666 | IPC_CREAT | IPC_EXCL);
	arg1.val = 0;
	semctl(semid3,0, SETVAL, arg1);
	n = semctl(semid3,0,GETVAL,arg1);
	//printf("Value of semid3: %d\n",n);
	//printf("%d %d",semid1, semid3);
	postop.sem_num = 0;
	postop.sem_op = 1;
	postop.sem_flg = SEM_UNDO;
	waitop.sem_num = 0;
	waitop.sem_op = -1;
	waitop.sem_flg = SEM_UNDO;
						
	childPid = fork();
	if(childPid == 0)
     	{
		/*Server Process*/

		shm = shmat(id, NULL, 0);
							
		while(1){
							
		semop(semid3,&waitop,1);
		strcpy(msg.msg_buff,shm);
							
							
		executeCommand(msg.msg_buff);
		strcpy(shm, msg.msg_buff);
		semop(semid1,&postop,1);
							
							
		}
						
						
							
	}
	else
	{
		shm = shmat(id, NULL, 0);
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
				semctl(semid1,0,IPC_RMID);
				semctl(semid3,0,IPC_RMID);
				shmdt(shm);
				shmctl(id,IPC_RMID,NULL);
											
				kill(childPid,SIGKILL);
											
				exit(0);
			}
			else
			{
												
				strcpy(shm,command);
				semop(semid3,&postop,1);
												
				/*Displaying the Output on the console*/
												
				semop(semid1,&waitop,1);
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
