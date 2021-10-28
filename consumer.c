#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>

int main(void)
{
    const char *name = "memSeg";
    const int SIZE = 32;
    char *shm_base;

    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        printf("Shared memory failed");
        exit(1);
    }

    shm_base = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    //initiate semaphores
    sem_t* fillTable = sem_open("fillTable", O_CREAT, 0666, 0);
    sem_t* mutex = sem_open("mutex", O_CREAT, 0666, 0);
    sem_t* available = sem_open("available", O_CREAT, 0666, 2);

    for (int i = 1; i < SIZE; i++)
    {
        sem_wait(fillTable);

        int wait = rand() % 2 + 1;
        sleep(wait);

        sem_wait(mutex);
        --(*shm_base);
        sem_post(mutex);

        printf("Item consumed %d items remain.\n", *shm_base);

        sem_post(available);
    }

    sem_close(fillTable);
    sem_close(available);
    sem_close(mutex);

    sem_unlink(name);
    sem_unlink("fillTable");
    sem_unlink("available");
    sem_unlink("mutex");

    munmap(shm_base, SIZE);
    close(shm_fd);
    
    printf("Consumer cleaned up.\n");
}