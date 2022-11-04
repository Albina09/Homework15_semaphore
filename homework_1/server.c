#include <stdio.h>
#include <stdlib.h>
#include <errno.h> 
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>


#define SHM_NAME1 "/shm"
#define SHM_NAME2 "/shm2"
#define SEM_NAME1 "/sem"
#define SEM_NAME2 "/sem2"
#define N 256

void errorExit(char err[]);

void errorExit(char err[]){
    perror(err);
    exit(EXIT_FAILURE);
}

int main(){

    sem_t *fd_sem1 = sem_open(SEM_NAME1, O_CREAT|O_RDWR, 0666, 0);
    if(fd_sem1 == SEM_FAILED)
        errorExit("sem_open1");
    sem_wait(fd_sem1); 

    int fd_shm1 = shm_open(SHM_NAME1, O_RDWR, 0);                                                           
    if(fd_shm1 == -1)
        errorExit("shm_open1");
        
    char *arr1 = (char*)mmap(NULL, N, PROT_READ, MAP_SHARED, fd_shm1, 0);                                 
    if(arr1 == MAP_FAILED)
        errorExit("mmap1");
    printf("%s\n", arr1);                                                                              
    
    
    sem_t  *fd_sem2 = sem_open(SEM_NAME2, O_RDWR);
    if(fd_sem2 == SEM_FAILED)
        errorExit("sem_open2");
        
    int fd_shm2 = shm_open(SHM_NAME2, O_CREAT|O_RDWR, 0666);                                                         
    if(fd_shm2 == -1)
        errorExit("shm_open2");
        
    if(ftruncate(fd_shm2, N) == -1)                                                                      
        errorExit("ftruncate2");

    char *arr2 = (char*)mmap(NULL, N, PROT_READ|PROT_WRITE, MAP_SHARED, fd_shm2, 0);                                
    if(arr2 == MAP_FAILED)
        errorExit("mmap2");

    strcpy(arr2, "Server: Hello");
    sem_post(fd_sem2);
     
    munmap(arr1, N);                                                                                  
    munmap(arr2, N);
    sem_close(fd_sem2);
    close(fd_shm2);
    sem_unlink(SEM_NAME1);                                                                                 
    shm_unlink(SHM_NAME1);                                                                                
    
    exit(EXIT_SUCCESS);                                                                           
}


