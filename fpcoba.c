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
pthread_t cs[1000];
int safe = 1;
int kill_run = -1;
char argument[1000][1000];

void ins(int pos,char* str);
void* run(void* arg);
void readcron();
void convert_time(int arr[], char* str);
int check(int arr[]);
int exec_time(int arr[]);
void parse(char* cronf,char* command, char* format);

int main(int argc, char const *argv[]) {
    pid_t pid, sid;
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    umask(0);
    sid = setsid();
    if (sid < 0) {
        exit(EXIT_FAILURE);
    }
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    readcron();
    return 0;
}
void* run(void* arg)
{    
    int i;
    pthread_t id = pthread_self();
    for(i=0;i<1000;i++)
    {
        if(cs[i]==id) break;
    }
    char command[100000];
    char format[100000];
    parse(argument[i],command,format);
    int timer[6];
    convert_time(timer,format);
    int deter = check(timer);
    int tunggu;
    if(deter!=0) pthread_exit(NULL);
    while(!safe);
    if(exec_time(timer)){
        pid_t child;
        child = fork();
        if(child==0){
            execl("/bin/bash","/bin/bash","-c",command,NULL);
        }
        else waitpid(child, &tunggu, 0);
        // system(command);
    }
    pthread_exit(NULL);
}
int exec_time(int arr[])
{
    time_t rawtime;
    time(&rawtime);
    struct tm *ptm = localtime(&rawtime);
    if  ((ptm->tm_sec == 0) &&
         ((ptm->tm_min == arr[0]) || (arr[0] == BYPASS_RUN)) &&
         ((ptm->tm_hour == arr[1]) || (arr[1] == BYPASS_RUN)) &&
         ((ptm->tm_mday == arr[2]) || (arr[2] == BYPASS_RUN)) &&
         ((ptm->tm_mon == arr[3]-1) || (arr[3] == BYPASS_RUN)) &&
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
    int next_thread=0; 
    safe = 1;  
    while(1){
        stat(path,&sh);
        FILE *fd;
        char buffer[1000000];
        fd = fopen(path,"r");
        if(fd == NULL){
            perror("Error membuka file:");
            exit(EXIT_FAILURE);
        }
        while(fgets(buffer,1000000,fd)!=NULL){
            if(strcmp(buffer,"\n")==0) continue;
            ins(next_thread,buffer);
            pthread_create(&cs[next_thread],NULL,run,NULL);
            next_thread++;
            memset(buffer,0,sizeof(buffer));
        }
        for(int i=0;i<next_thread;i++)
        {
            pthread_join(cs[i],NULL);
        }
        next_thread=0;
        fclose(fd);
        now = sh.st_mtime;
        safe = 1;
        memset(&sh,0,sizeof(sh));
        time_t rawtime;
        time(&rawtime);
        struct tm *ptm = localtime(&rawtime);
        int max_wait = 60 - ptm->tm_sec;
        sleep(max_wait);
    }
}
void ins(int num,char* str)
{
    strcpy(argument[num],"\0");
    strcpy(argument[num],str);
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


