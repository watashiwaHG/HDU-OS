#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#define TRUE 1
#define BUF_SIZE 255

typedef struct msgbuf msgbuf;

struct msgbuf{
	long mtype;
	char mtext[BUF_SIZE+1];
};

sem_t full;
sem_t empty;
sem_t mutex;
int flag=0;
pthread_t write_pid;
pthread_t read_pid;

key_t key=1000;

int msgid;
struct msgbuf msg;
void init(){
	sem_init(&full,0,0);
	sem_init(&empty,0,1);
	sem_init(&mutex,0,1);

	if((msgid = msgget(key,IPC_CREAT|S_IRUSR|S_IWUSR)) == -1){
		fprintf(stderr,"queue error %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}	
}

void *receiver(void *arg){
	msgbuf msg;
	msg.mtype = 1;
	while(1){
		sem_wait(&full);
		sem_wait(&mutex);
		msgrcv(msgid,&msg,sizeof(msgbuf),1,0);
		if(strcmp(msg.mtext,"end") == 0){
			msg.mtype = 2;
			strncpy(msg.mtext,"over",BUF_SIZE);
			msgsnd(msgid,&msg,sizeof(msgbuf),0);
			printf("final to sender:%s\n",msg.mtext);
			sem_post(&empty);
			sem_post(&mutex);
			flag=1;
			break;
		}	
		printf("receiving:%s \n",msg.mtext);
		sem_post(&empty);
		sem_post(&mutex);
	}
	while(flag==1);
	exit(EXIT_SUCCESS);
}

void *sender(void *arg){
	char input[123];
	msgbuf msg;
	msg.mtype=1;
	while(1){
		sem_wait(&empty);
		sem_wait(&mutex);
		printf("Input message:");
		scanf("%s",input);
		if(strcmp(input,"exit")==0){
			strncpy(msg.mtext,"end",BUF_SIZE);
			msgsnd(msgid,&msg,sizeof(msgbuf),0);
			sem_post(&full);
			sem_post(&mutex);
			break;
		}
		strncpy(msg.mtext,input,BUF_SIZE);
		msgsnd(msgid,&msg,sizeof(msgbuf),0);
		printf("sent:%s\n",msg.mtext);
		sem_post(&full);
		sem_post(&mutex);
	}
	while(flag==0);
	//memset(&msg,'\0',sizeof(msgbuf));	
	msgrcv(msgid,&msg,sizeof(msgbuf),2,0);
	printf("final from rev:%s\n",msg.mtext);
	flag=0;
	if(msgctl(msgid,IPC_RMID,0) == -1){
		fprintf(stderr,"removing queue ERROR:%s\n",strerror(errno));
		exit(EXIT_FAILURE);
			
	}
	
	exit(EXIT_SUCCESS);
}

int main(){
	init();
	pthread_create(&write_pid,NULL,sender,NULL);	
	pthread_create(&read_pid,NULL,receiver,NULL);
	pthread_join(write_pid,NULL);
	pthread_join(read_pid,NULL);
	return 0;
}



