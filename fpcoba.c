#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

char* path = "/home/duhbuntu/sisop/FP/crontab.data";

typedef struct Node{
    char string[100000];
    int search;
    struct Node* next;
}node;

typedef struct List{
    node* head;
    node* tail;
}list;

void ins(list* num,char* str)
{
    node* now = (node*) malloc(sizeof(node));
    strcpy(now->string,str);
    now->search = 0;
    now->next = NULL;
    if(num->head==NULL){
        num->head=now;
        num->tail=now;
    }
    else{
        num->tail->next = now;
        num->tail = now;
    }
}

void* run(void* arg)
{
    char cron[100000];
    char* cronf = (char*) arg;
    strcpy(cron,cronf);
    int i;
    int pos[6];
    int now=0;
    int count = 0;
    for(i=0;cron[i]!='\0';i++)
    {
        if(cron[i]==' ' && count!=5){
            now = i;
            count++;
        }
        if(count==5) break;
    }
    char command[100000];
    char format[100000];
    strcpy(command,cron+now+1);
    strncpy(format,cron,now);
    printf("%s\n%s\n",command,format);
    system(command);
}

void* readcron()
{
    time_t now = 0;
    struct stat sh;
    list cmd;
    cmd.head = NULL;
    cmd.tail = NULL;   
    while(1){
        stat(path,&sh);
        if(now == 0 || now < sh.st_mtime){
            FILE *fd;
            char buffer[1000000];
            fd = fopen(path,"r");
            if(fd == NULL){
                perror("Error membuka file:");
                exit(EXIT_FAILURE);
            }
            int signal = 0;
            while(fgets(buffer,1000000,fd)!=NULL){
                signal = 0;
                node* tmp = cmd.head;
                while(tmp!=NULL){
                    if(strcmp(buffer,tmp->string)==0){
                        signal = 1;
                        tmp->search = 1;
                        break;
                    }
                    tmp=tmp->next;
                }
                if(signal==0){
                    ins(&cmd,buffer);
                    pthread_t ted;
                    pthread_create(&ted,NULL,run,(void*) buffer);
                    printf("%s",buffer);
                }
                memset(buffer,0,sizeof(buffer));
            }
            fclose(fd);
            // printf("bisa\n");
            now = sh.st_mtime;
        }
        memset(&sh,0,sizeof(sh));
    }
}


int main(int argc, char const *argv[]) {
    pthread_t t[5];
    pthread_create(&t[0],NULL,readcron,NULL);
    while(1);
    return 0;
}