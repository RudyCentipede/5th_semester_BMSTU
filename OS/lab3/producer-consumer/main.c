#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define SB 0
#define SE 1
#define SF 2

#define P -1
#define V  1

struct sembuf start_produce[2] = { {SE, P, 0}, {SB, P, 0} };
struct sembuf stop_produce[2] =  { {SB, V, 0}, {SF, V, 0} };
struct sembuf start_consume[2] = { {SF, P, 0}, {SB, P, 0} };
struct sembuf stop_consume[2] =  { {SB, V, 0}, {SE, V, 0} };

int flag = 1;
void alarm_handler(int sig) 
{
    printf("Process %d catch signal: %d\n", getpid(), sig);
    flag = 0;
    exit(14);
}

void producer(const int semid, const int shmid)
{
    char *addr = shmat(shmid, NULL, 0);
    if (addr == (char *) -1)
    {
        perror("shmat.\n");
        exit(1);
    }

    char **prod_ptr = (char **) addr;
    char **cons_ptr = prod_ptr + 1;
    char *letter = (char*) (cons_ptr + 1);

    while(flag)
    {
        int sem_op_p = semop(semid, start_produce, 2);
        if (sem_op_p == -1)
        {
            perror("semop\n");
            exit(1);
        }

        **prod_ptr = *letter;
        printf("Producer %d - %c\n", getpid(), **prod_ptr);

        if (*letter == 'z')
        {
            *letter = 'a';
            (*prod_ptr) = (char*)((char**)addr + 2) + 1;
        }
        else
        {
            (*prod_ptr)++;
            (*letter)++;

        }

        //sleep(rand() % 2);

        int sem_op_v = semop(semid, stop_produce, 2);
        if (sem_op_v == -1)
        {
            perror("semop\n");
            exit(1);
        }
    }

    if (shmdt((void *) prod_ptr) == -1)
    {
        perror("shmdt\n");
        exit(1);
    }

    exit(0);
}

void consumer(const int semid, const int shmid)
{   
    char *addr = shmat(shmid, NULL, 0);
    if (addr == (char *) -1)
    {
        perror("shmat\n");
        exit(1);
    }

    char **prod_ptr = (char **) addr;
    char **cons_ptr = prod_ptr + 1;
    char *letter = (char*) (cons_ptr + 1);

    while(flag)
    {
        int sem_op_p = semop(semid, start_consume, 2);
        if (sem_op_p == -1)
        {
            perror("semop\n");
            exit(1);
        }
        
        printf("Consumer %d - %c\n", getpid(), **cons_ptr);

        if (**cons_ptr == 'z')
            (*cons_ptr) = (char*)((char**)addr + 2) + 1;
        else
            (*cons_ptr)++;
        
        //sleep(rand() % 3);

        int sem_op_v = semop(semid, stop_consume, 2);
        if (sem_op_v == -1)
        {
            perror("semop\n");
            exit(1);
        }
    }

    if (shmdt((void *) addr) == -1)
    {
        perror("shmdt\n");
        exit(1);
    }

    exit(0);
}

int main()
{   
    int shmid, semid;
    int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    char** prod_ptr;
    char** cons_ptr;
    char* letter;
    pid_t chpid[7];
    
    if (signal(SIGALRM, alarm_handler) == -1)
    {
        perror("signal\n");
        exit(1);
    }

    if ((semid = semget(100, 3, IPC_CREAT | perms)) == -1)
    {
        perror("semget\n");
        exit(1);
    }

    if (semctl(semid, SB, SETVAL, 1) == -1)
    {
        perror("semctl\n");
        exit(1);
    }
    
    if (semctl(semid, SE, SETVAL, 26) == -1)
    {
        perror("semctl\n");
        exit(1);
    }

    if (semctl(semid, SF, SETVAL, 0) == -1)
    {
        perror("semctl\n");
        exit(1);
    }

    if ((shmid = shmget(100, 29, IPC_CREAT | perms)) == -1)
    {
        perror("shmget\n");
        exit(1);
    }

    char *storage = shmat(shmid, NULL, 0);
    if (storage == (char *) -1)
    {
        perror("shmat\n");
        exit(1);
    }

    prod_ptr = (char **) storage;
    cons_ptr = prod_ptr + 1;
    letter = (char*) (cons_ptr + 1);

    *cons_ptr = letter + 1;
    *prod_ptr = *cons_ptr;
    *letter = 'a';

    for (int i = 0; i < 3; i++)
    {
        chpid[i] = fork();
        if (chpid[i] == -1)
        {
            perror("Can't fork producer\n");
            exit(1);
        }

        if (chpid[i] == 0)
        {
            alarm(1);
            producer(semid, shmid);
        }
    }

    for (int i = 3; i < 7; i++)
    {
        chpid[i] = fork();
        if (chpid[i] == -1)
        {
            perror("Can't fork consumer\n");
            exit(1);
        }

        if (chpid[i] == 0)
        {
            alarm(1);
            consumer(semid, shmid);
        }
    }

    for (int i = 0; i < 6; i++)
    {
        int status;
        int pid_child = wait(&status);
        int e = errno;
        printf("errno %d\n", e);
        if (WIFEXITED(status))
            printf("Child %d exited with status %d\n", pid_child, WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
            printf("Child %d was terminated by signal %d\n", pid_child, WTERMSIG(status));
        else if (WIFSTOPPED(status))
            printf("Child %d was stopped by signal %d\n", pid_child, WSTOPSIG(status));
        else
            printf("Unexpected child status\n");
    }

    if (shmdt((void *) prod_ptr) == -1)
    {
        perror("shmdt.\n");
        exit(1);
    }

    if (semctl(semid, 1, IPC_RMID, NULL) == -1)
    {
        perror("semctl.\n");
        exit(1);
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1) 
    {
        perror("shmctl.\n");
        exit(1);
    }
    return 0;
}
