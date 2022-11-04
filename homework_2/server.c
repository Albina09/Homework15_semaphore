#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>


#define NAME_SEM_1 "/sem1"
#define NAME_SEM_2 "/sem2"
#define NAME_SHM_1 "/shm1"
#define NAME_SHM_2 "/shm2"
char ex[9];
struct users{
    char names[20];
};
struct message{
    char name_user_smh[21];
    char user[20];
    char msg[70];
};
struct message *msg_send;
struct users client[20];
int count_user = 0;

void nam(char nam[]);
void errorExit(char err[]);

void errorExit(char err[]){
    perror(err);
    exit(EXIT_FAILURE);
}

void nam(char buff[]){
    int l = 0;
    
    for(int i = -1; i < (count_user - 1); i++){

        if(strcmp(client[i+1].names, buff) == 0 && !l)
            l = 1;
    }
    
    if(l == 0){
        strcpy(client[count_user].names, buff);                              //копирует все имена в структуру с имами очередей
        count_user++;
        printf("------------\n");
        printf("Количество клиентов: %d\n", count_user);
        for(int i = 0; i <= (count_user - 1); i++)
            printf("%s\n", client[i].names);
        printf("------------\n");
    }
        
        
}

void *server(void *args){
  

    while(1){
        sem_t *fd_sem1 = sem_open(NAME_SEM_1, O_CREAT|O_RDWR, 0666,0);
        if(fd_sem1 == SEM_FAILED)
            errorExit("sem_open1");
        sem_wait(fd_sem1); 

        int fd_shm1 = shm_open(NAME_SHM_1 ,O_RDWR,0);                                                           
        if(fd_shm1 == -1){
            perror("shm_open1");
            exit(EXIT_FAILURE);
        }
        msg_send = mmap(NULL, sizeof(struct message), PROT_READ|PROT_WRITE, MAP_SHARED, fd_shm1,0);                                 
        if(msg_send == MAP_FAILED)
            errorExit("mmap1");
           
        nam(msg_send->user);
    
        struct message *msg_rece;
        sem_t  *fd_sem2 = sem_open(NAME_SEM_2, O_RDWR);
        if(fd_sem2 == SEM_FAILED)
            errorExit("sem_open2");
            
        int fd_shm2 = shm_open(NAME_SHM_2, O_CREAT|O_RDWR, 0666);                                                                                 
        if(fd_shm2 ==-1)
            errorExit("shm_open2");
           
        if(ftruncate(fd_shm2,sizeof(struct message)) == -1)                                                                           
            errorExit("ftruncate2");
            
        msg_rece = (struct message *)mmap(NULL, sizeof(struct message), PROT_READ|PROT_WRITE, MAP_SHARED, fd_shm2, 0);                              
        if(msg_rece == MAP_FAILED)
            errorExit("mmap1");
        strcpy(msg_rece->user, msg_send->user);
        strcpy(msg_rece->msg, msg_send->msg);
       
        for(int i = 0; i < count_user; i++)
            sem_post(fd_sem2);
        munmap(msg_rece,sizeof(struct message));
    }   
    
}

int main(void){
    
    pthread_t thread;

    printf("Для выхода из сервера напишите: exit\n");

    pthread_create(&thread,NULL, server, NULL);
  
     while(1){
        fgets(ex, 9,stdin);
        
        if(strcmp(ex,"exit\n") == 0){
            pthread_cancel(thread);
           
            
            shm_unlink(NAME_SHM_1);
            sem_unlink(NAME_SEM_1);
            shm_unlink(NAME_SHM_2);
            sem_unlink(NAME_SEM_2);

            exit(EXIT_SUCCESS); 
        }
    }
}
