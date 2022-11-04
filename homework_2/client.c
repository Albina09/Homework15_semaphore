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

struct message{
    char name_user_smh[21];
    char user[20];
    char msg[70];
};
struct message *msg_send;
void errorExit(char err[]);

void errorExit(char err[]){
    perror(err);
    exit(EXIT_FAILURE);
}

void *send(void *args){
    char name[20];

    printf("Введите имя:");
    fgets(name, sizeof(name), stdin);
    printf("Для выхода из чата напишите: exit\n");

    for(int i = 0; i < 20; i++){

        if(name[i] == '\n')
            name[i] = '\0';
    }
    
    while(1){
        sem_t  *fd_sem1 = sem_open(NAME_SEM_1, O_RDWR);
        if(fd_sem1 == SEM_FAILED)
            errorExit("sem_open1");
            
        int fd_shm1 = shm_open(NAME_SHM_1, O_CREAT|O_RDWR, 0666);                                                                                 
        if(fd_shm1 == -1)
            errorExit("shm_open1");
            
        if(ftruncate(fd_shm1, sizeof(struct message)) == -1)                                                                            
            errorExit("ftruncate1");
            
        msg_send = mmap(NULL, sizeof(struct message), PROT_READ|PROT_WRITE, MAP_SHARED, fd_shm1, 0);                              
        if(msg_send == MAP_FAILED)
            errorExit("mmap1");
        strcpy(msg_send->user, name);
        fgets(msg_send->msg, sizeof(msg_send->msg), stdin);  
        

        if(strcmp(msg_send->msg,"exit\n") == 0){
            munmap(msg_send, sizeof(struct message));
            sem_post(fd_sem1);
            pthread_exit(0);
        }
        sem_post(fd_sem1);
        munmap(msg_send, sizeof(struct message));
        sem_close(fd_sem1);
        close(fd_shm1);

    }   

}
void *receive(void *args){
    struct message *msg_rece;

    while(1){
        int value;

        sem_t *fd_sem2 = sem_open(NAME_SEM_2, O_CREAT|O_RDWR, 0666, 0);
        if(fd_sem2 == SEM_FAILED)
            errorExit("sem_open1");
            
        sem_wait(fd_sem2);

        int fd_shm2 = shm_open(NAME_SHM_2, O_RDWR, 0);                                                           
        if(fd_shm2 == -1)
            errorExit("shm_open1");
        
        msg_rece = mmap(NULL, sizeof(struct message), PROT_READ, MAP_SHARED, fd_shm2, 0);                                 
        if(msg_rece == MAP_FAILED)
            errorExit("mmap1");
        
        printf("%s: %s\n",msg_send->user, msg_send->msg); 
        munmap(msg_rece, sizeof(struct message));
        sem_close(fd_sem2);
        close(fd_shm2);  
    }
}
int main(void){
    pthread_t thread[2]; 
    //name(attr);
    
    if(pthread_create(&thread[0], NULL, send, NULL) == -1)
        errorExit("pthread_create");

    if(pthread_create(&thread[1], NULL, receive, NULL) == -1)
        errorExit("pthread_create");
    
    pthread_join(thread[0], NULL);
    pthread_cancel(thread[1]);
    
    exit(EXIT_SUCCESS);
}