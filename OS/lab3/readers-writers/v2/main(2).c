#include <sys/wait.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define WAITING_READERS 0
#define ACTIVE_READERS 1
#define WAITING_WRITERS 2
#define IS_WRITING 3

#define P -1
#define V 1

int flag = 1;
void alarm_handler(int sig) 
{
    printf("Process %d catch signal: %d\n", getpid(), sig);
    flag = 0;
}

struct sembuf start_read[5] = {
    {WAITING_READERS, V, 0},
    {IS_WRITING, 0, 0},
    {WAITING_WRITERS, 0, 0},
    {ACTIVE_READERS, V, 0},
    {WAITING_READERS, P, 0},
};
struct sembuf stop_read[1] = {
    {ACTIVE_READERS, P, 0},
};
struct sembuf start_write[5] = {
    {WAITING_WRITERS, V, 0},
    {ACTIVE_READERS, 0, 0},
    {IS_WRITING, 0, 0},
    {IS_WRITING, V, 0},
    {WAITING_WRITERS, P, 0},
};
struct sembuf stop_write[1] = {
    {IS_WRITING, P, 0},
};

void writer(int semid, int shmid, char *addr)
{
    char *letter = addr;

    while (flag)
    {
        if (semop(semid, start_write, 5) == -1)
        {
            printf("start_write, errno = %d\n", errno);
            exit(1);
        }
        if (*letter == 'z')
            *letter = 'a';
        else
            (*letter)++;
        printf("%d wrote: %c\n", getpid(), *letter);
        if (semop(semid, stop_write, 1) == -1) 
        {
            printf("stop_write, errno = %d\n", errno);
            exit(1);
        }
    }
    exit(0);
}

void reader(int semid, int shmid, char *addr)
{
    char *letter = addr;

    while (flag)
    {
        if (semop(semid, start_read, 5) == -1)
        {
            printf("start_read, errno = %d\n", errno);
            exit(1);
        }
        printf("%d read: %c\n", getpid(),*letter);
        if (semop(semid, stop_read, 1) == -1)
        {
            printf("stop_read, errno = %d\n", errno);
            exit(1);
        }
    }
    exit(0);
}

int main(void)
{
    int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    pid_t child_pids[7];
    char *letter;
    int shmid;
    int semid;

    if (signal(SIGALRM, alarm_handler) == -1)
    {
        perror("signal\n");
        exit(1);
    }

    semid = semget(100, 4, IPC_CREAT | perms);
    if (semid == -1)
    {
        printf("semget\n");
        exit(1);
    }

    if (semctl(semid, WAITING_READERS, SETVAL, 0) == -1)
    {
        printf("semctl\n");
        exit(1);
    }
    if (semctl(semid, ACTIVE_READERS, SETVAL, 0) == -1)
    {
        printf("semctl\n");
        exit(1);
    }
    if (semctl(semid, WAITING_WRITERS, SETVAL, 0) == -1)
    {
        printf("semctl\n");
        exit(1);
    }
    if (semctl(semid, IS_WRITING, SETVAL, 0) == -1)
    {
        printf("semctl\n");
        exit(1);
    }

    shmid = shmget(100, sizeof(char), IPC_CREAT | perms);
    if (shmid == -1)
    {
        printf("shmget\n");
        exit(1);
    }

    letter = (char *)shmat(shmid, NULL, 0);
    if (letter == (void *)-1)
    {
        printf("shmat\n");
        exit(1);
    }

    *letter = 'a';

    for (int i = 0; i < 4; i++)
    {
        if ((child_pids[i] = fork()) == -1)
        {
            printf("fork");
            exit(1);
        }
        if (child_pids[i] == 0)
        {
            char *addr = (char*)shmat(shmid, NULL, 0);
            alarm(1);
            writer(semid, shmid, addr);
        }
    }
    for (int i = 4; i < 7; i++)
    {
        if ((child_pids[i] = fork()) == -1)
        {
            printf("fork");
            exit(1);
        }
        if (child_pids[i] == 0)
        {
            char *addr = (char*)shmat(shmid, NULL, 0);
            alarm(1);
            reader(semid, shmid, addr);
        }
    }

    for (int i = 0; i < 7; i++)
    {
        int status;
        int child_pid = wait(&status);

        if (WIFEXITED(status))
            printf("Child with pid %d has finished with code: %d \n", child_pid, WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
            printf("Child with pid %d has finished by unhandlable signal, signum: %d \n", child_pid, WTERMSIG(status));
        else if (WIFSTOPPED(status))
            printf("Child with pid %d has finished by signal, signum: %d \n", child_pid, WSTOPSIG(status));
    }

    if (shmdt(letter) == -1)
    {
        printf("shmdt\n");
        exit(1);
    }
    if (semctl(semid, 1, IPC_RMID, NULL) == -1)
    {
        printf("semctl\n");
        exit(1);
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
    {
        printf("shmctl\n");
        exit(1);
    }

    return 0;
}