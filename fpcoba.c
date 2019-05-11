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
#include <limits.h>
#define max_hour 23
#define max_minute 59
#define max_day 31
#define max_month 12
#define max_dayoweek 7
#define BYPASS_RUN 111

char* path = "/home/duhbuntu/sisop/FP/crontab.data";
pthread_t cs[100000];
int safe = 1;
int kill_run = -1;

typedef struct Node{
    char string[100000];
    int search;
    struct Node* next;
}node;

typedef struct List{
    node* head;
    node* tail;
}list;

void ins(list* num,char* str);
void* run(void* arg);
void readcron();
void convert_time(int arr[], char* str);
int check(int arr[]);
int exec_time(int arr[]);
void parse(char* cronf,char* command, char* format);

int main(int argc, char const *argv[]) {
    readcron();
    return 0;
}

void* run(void* arg)
{    
    char* cronf = (char*) arg;    
    char command[100000];
    char format[100000];
    parse(cronf,command,format);
    int time[6];
    convert_time(time,format);
    int deter = check(time);
    if(deter!=0) pthread_exit(NULL);
    // printf("%s\n%s\n",command,format);
    // for(int j=0;j<5;j++){
    //     printf("%d ",time[j]);
    // }
    // printf("\n");
    
    while(1){
        while(!safe);
        if(exec_time(time)){
            system(command);
        }
        sleep(60);
    }
}
int exec_time(int arr[]){
    time_t rawtime;
    time(&rawtime);
    struct tm *ptm = localtime(&rawtime);
    if  ((ptm->tm_sec == 0) &&
         ((ptm->tm_min == arr[0]) || (arr[0] == BYPASS_RUN)) &&
         ((ptm->tm_hour == arr[1]) || (arr[1] == BYPASS_RUN)) &&
         ((ptm->tm_mday == arr[2]) || (arr[2] == BYPASS_RUN)) &&
         ((ptm->tm_mon == arr[3]) || (arr[3] == BYPASS_RUN)) &&
         ((ptm->tm_wday == arr[4]) || (arr[4] == BYPASS_RUN))
        ) return 1;
    
    return 0;
}
void convert_time(int arr[], char* str)
{
    int k=0;
    int count = 0;
    int now =0;
    while(1){
        // printf("bisa\n");
        if(str[k]==' ' || str[k]=='\0'){

            char temp[10];
            strncpy(temp,str+k-count,count);
            
            if(strstr(temp,"*")!=NULL) arr[now] = BYPASS_RUN;
            else arr[now] = atoi(temp);

            count=0;
            now++;

            memset(temp,0,sizeof(temp));
            if(str[k]=='\0') return;
        }
        else count++;
        k++;
    }

}
void readcron()
{
    time_t now = 0;
    struct stat sh;
    list cmd;
    cmd.head = NULL;
    cmd.tail = NULL;
    int next_thread=0; 
    safe = 1;  
    while(1){
        stat(path,&sh);
        if(now == 0 || now < sh.st_mtime){
            safe = 0;
            FILE *fd;
            char buffer[1000000];
            fd = fopen(path,"r");
            if(fd == NULL){
                perror("Error membuka file:");
                exit(EXIT_FAILURE);
            }
            int signal = 0;
            while(fgets(buffer,1000000,fd)!=NULL){
                if(strcmp(buffer,"\n")==0) continue;
                signal = 0;
                node* tmp = cmd.head;
                while(tmp!=NULL && signal!=1){
                    if(strcmp(buffer,tmp->string)==0){
                        signal = 1;
                        tmp->search = 1;
                        break;
                    }
                    tmp=tmp->next;
                }
                if(signal==0){
                    ins(&cmd,buffer);
                    char temp[10000];
                    strcpy(temp,buffer);
                    pthread_create(&cs[next_thread],NULL,run,(void*) temp);
                    next_thread++;
                }
                memset(buffer,0,sizeof(buffer));
            }
            fclose(fd);
            now = sh.st_mtime;
            safe = 1;
        }
        memset(&sh,0,sizeof(sh));
    }
}
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
int check(int arr[])
{
    int base[] = {max_minute,max_hour,max_day,max_month,max_dayoweek};
    if(arr[2]<1 || arr[3]<1) return -1;
    for(int i=0;i<5;i++)
    {
        if((arr[i]>base[i] || arr[i]<0)&& arr[i]!= BYPASS_RUN) return -1;
    }
    return 0;
}
void parse(char* cronf,char* command, char* format)
{
    char cron[100000];
    strcpy(cron,cronf);
    int count = 0;
    int now = 0;
    for(int i=0;cron[i]!='\0';i++)
    {
        if(cron[i]==' ' && count!=5){
            now = i;
            count++;
        }
        if(count==5) break;
    }

    strcpy(command,cron+now+1);
    strncpy(format,cron,now);
}


