# FP_SISOP19_C13
(http://cdn2.tstatic.net/tribunnews/foto/bank/images/ilustrasi-ramadhan_20180516_093405.jpg)
1. Mohammad Rizaldi Huzein Prastomo -   05111740000024
2. Muhammad Husni Ridhart Azzikry   -   05111740000122
#
## Soal
Buatlah program C yang menyerupai crontab menggunakan daemon dan thread. Ada sebuah file crontab.data untuk menyimpan config dari crontab. Setiap ada perubahan file tersebut maka secara otomatis program menjalankan config yang sesuai dengan perubahan tersebut tanpa perlu diberhentikan. Config hanya sebatas * dan 0-9 (tidak perlu /,- dan yang lainnya)
#
## Penyelesaian
Crontab memiliki format

`* * * * * [string command]`

Dengan masing-masing `*`, memiliki arti:

1. Menit eksekusi argumen, dinotasikan dengan 0 - 59
2. Jam eksekusi argumen, dinotasikan dengan 0 - 23
3. Tanggal eksekusi argumen, dinotasikan dengan 1 - 31
4. Bulan eksekusi argumen, dinotasikan dengan 1 - 12
5. Hari dalam satu minggu (Senin - Sabtu), dinotasikan dengan 0 - 6
6. String argumen yang akan dijalankan

Dari format crontab tersebut, maka kita hanya perlu membaca sebuah file `crontab.data` , dan membaca string setiap barisnya apakah ia memenuhi format crontab atau tidak.
Sehingga langkah awal yang perlu kita lakukan adalah membaca file `crontab.data` secara berkala untuk menentukan apakah argumen tersebut perlu dijalankan saat ini atau tidak.

Sehingga langkah pertama yang perlu kita lakukan adalah membaca file `crontab.data`

`void readcron();`

```c
void readcron()
{
    time_t now = 0;
    struct stat sh;
    int next_thread=0;   
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
        memset(&sh,0,sizeof(sh));
        time_t rawtime;
        time(&rawtime);
        struct tm *ptm = localtime(&rawtime);
        int max_wait = 60 - ptm->tm_sec;
        sleep(max_wait);
    }
}
```

`readcron()` bertugas membaca file `crontab.data` secara berkala setiap 60 detik untuk mencari tahu apakah ada argumen yang perlu dijalankan. Ia akan menyimpan setiap argumen yang ada pada `crontab.data` pada `char argumen[][]` untuk di gunakan pada thread nantinya dengan `void ins(next_thread,buffer)`. Sehingga argumen saat itu akan disimpan pada array yang nantinya akan di check pada thread lain. Lalu fungsi `readcron()` akan memanggil thread 
```c
pthread_create(&cs[next_thread],NULL,run,NULL);
next_thread++;
```
Dengan tujuan untuk menjalankan argumen tersebut apabila ia sesuai dengan waktu eksekusinya.

Setelah `crontab.data` dibaca, maka setiap thread akan menjalankan `run()`,
```c
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
```
dimana ia akan membagi argumen tersebut yang merupakan string `* * * * * [argumen]` menjadi dua bagian yakni `format[] = {"* * * * *"}` dan `command[] = {"[argumen]"}` dengan `parse(argumen[i],command,format)` dengan `argumen[i]` sebagai argumen yang dijalankan oleh thread ke-i. 

Kemudian string `format[]` tersebut akan di convert ke array integer `timer[]` yang menandakan waktu dalam bentuk integer, dengan argumen `*` di asumsikan sebagai angka `111 atau BYPASS_RUN` karena dengan `*` berarti argumen tersebut akan dijalankan setiap menit/jam/hari/bulan, tergantung posisi. Sehingga waktu akan dicek menggunakan `deter=check(timer)`, dimana `check(timer)` berisi
```c
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
```
menandakan apakah format yang diinputkan telah sesuai format waktu crontab, apabila tidak, maka thread akan di terminate. Apabila telah sesuai maka thread akan memeriksa apakah waktu sekarang adalah waktu eksekusi argumen tersebut dengan menggunakan `exec_time(timer)`,
```c
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
```
dimana fungsi tersebut akan memeriksa waktu sekarang dengan `localtime(&rawtime)` dan menyimpan hasilnya pada `struct tm *ptm`, lalu kita cek setiap waktu sekarang dengan waktu format yang ada pada config yang disimpan pada array `timer[]`. Apabila waktunya sama, atau formatnya `*` maka return **TRUE**, dan **FALSE** apabila tidak memenuhi format.

Setelah dicek waktunya, apabila merupakan waktunya, maka argumen akan dijalankan dengan 
```c
if(exec_time(timer)){
    pid_t child;
    child = fork();
    if(child==0){
        execl("/bin/bash","/bin/bash","-c",command,NULL);
    }
    else waitpid(child, &tunggu, 0);
    // system(command);
}
```
Dan apabila thread sudah dieksekusi maka exit thread, dan tunggu 60 detik untuk membaca `crontab.data` lagi, karena, sesuai referensi [crontab.guru](https://crontab.guru/), argumen akan dieksekusi setiap detik ke 00.
