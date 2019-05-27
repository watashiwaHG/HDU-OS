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
int flag1=0;
int flag2=0;
pthread_t sender1_pid;
pthread_t sender2_pid;
pthread_t reader_pid;

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

void *reader(void *arg){
	msgbuf msg;
	msg.mtype = 1;
	while(1){
		sem_wait(&full);
		sem_wait(&mutex);
		msg.mtype=1;
		msgrcv(msgid,&msg,sizeof(msgbuf),1,0);
		printf("receiving from msg:%s \tmsgid=%d\n",msg.mtext,msgid);
		if(strcmp(msg.mtext,"end1") == 0){
			msg.mtype = 2;
			strncpy(msg.mtext,"over1",BUF_SIZE);
			msgsnd(msgid,&msg,sizeof(msgbuf),0);
			printf("final to sender1:%s,\tmsgid=%d\n",msg.mtext,msgid);
			flag1=1;
		}
		if(strcmp(msg.mtext,"end2") == 0){
                        msg.mtype = 2;
                        strncpy(msg.mtext,"over2",BUF_SIZE);
                        msgsnd(msgid,&msg,sizeof(msgbuf),0);
                        printf("final to sender2:%s,\tmsgid=%d\n",msg.mtext,msgid);
                        flag2=1;
                }
		if(flag1==1&&flag2==1){
			sem_post(&empty);
			sem_post(&mutex);
			break;
		}	
		//printf("receiving:%s \n",msg.mtext);
		sem_post(&empty);
		sem_post(&mutex);
	}
//	while(flag1!=0||flag2!=0);
	if(msgctl(msgid,IPC_RMID,0) == -1){
                fprintf(stderr,"removing queue ERROR:%s\n",strerror(errno));
                exit(EXIT_FAILURE);               
        }

	_exit(EXIT_SUCCESS);
}

void *sender1(void *arg){
	char input1[123];
	msgbuf msg;
	msg.mtype=1;
	while(1){
		sem_wait(&empty);
		sem_wait(&mutex);
		printf("sender1-Input message:");
		scanf("%s",input1);
		if(strcmp(input1,"exit")==0){
			strncpy(msg.mtext,"end1",BUF_SIZE);
			msgsnd(msgid,&msg,sizeof(msgbuf),0);
			printf("sender1 send end1 to msg:%d\n",msgid);
			sem_post(&full);
			sem_post(&mutex);
			break;
		}
		strncpy(msg.mtext,input1,BUF_SIZE);
		msgsnd(msgid,&msg,sizeof(msgbuf),0);
		printf("sent to msg:%s,\tmsgid=%d\n",msg.mtext,msgid);
		sem_post(&full);
		sem_post(&mutex);
	}
	while(flag1==0);
	//memset(&msg,'\0',sizeof(msgbuf));	
	msgrcv(msgid,&msg,sizeof(msgbuf),2,0);
	printf("sender1 final from rev:%s\n********************\n",msg.mtext);
//	flag1=0;
	/*if(msgctl(msgid,IPC_RMID,0) == -1){
		fprintf(stderr,"removing queue ERROR:%s\n",strerror(errno));
		exit(EXIT_FAILURE);
			
	}
	*/
	//exit(EXIT_SUCCESS);
}
void *sender2(void *arg){
        char input2[123];
        msgbuf msg;
        msg.mtype=1;
        while(1){
                sem_wait(&empty);
                sem_wait(&mutex);
                printf("sender2-Input message:");
                scanf("%s",input2);
                if(strcmp(input2,"exit")==0){
                        strncpy(msg.mtext,"end2",BUF_SIZE);
                        msgsnd(msgid,&msg,sizeof(msgbuf),0);
        		printf("sender2 send end2 to msg:%d\n",msgid);
	                sem_post(&full);
                        sem_post(&mutex);
                        break;
                }
                strncpy(msg.mtext,input2,BUF_SIZE);
                msgsnd(msgid,&msg,sizeof(msgbuf),0);
                printf("sender2 sent:%s,\tmsgid=%d\n",msg.mtext,msgid);
                sem_post(&full);
                sem_post(&mutex);
        }
        while(flag2==0);
        //memset(&msg,'\0',sizeof(msgbuf));     
        msgrcv(msgid,&msg,sizeof(msgbuf),2,0);
        printf("sender2 final from rev:%s\n********************\n",msg.mtext);
//        flag2=0;
        /*if(msgctl(msgid,IPC_RMID,0) == -1){
                fprintf(stderr,"removing queue ERROR:%s\n",strerror(errno));
                exit(EXIT_FAILURE);
                        
        }
        */
        //exit(EXIT_SUCCESS);
}

int main(){
	init();
	pthread_create(&sender1_pid,NULL,sender1,NULL);	
	pthread_create(&sender2_pid,NULL,sender2,NULL);
	pthread_create(&reader_pid,NULL,reader,NULL);
	pthread_join(sender1_pid,NULL);
	pthread_join(sender2_pid,NULL);
	pthread_join(reader_pid,NULL);
	return 0;
}



