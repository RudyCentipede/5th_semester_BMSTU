#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

int flag = 0;

void signal_handler(int signal) 
{
    flag = 1;
}

int main(void)
{
    pid_t child_pids[2];
    int pipefd[2];
    const char *messages[] = {"aaa", "bbbbbbbbbbb"};
    char buf[12];
    ssize_t num_read;
    
    
    if (signal(SIGINT, signal_handler) == SIG_ERR)
    {
    	perror("Can't signal.\n");
    	exit(1);
    }
    
    sleep(2);
    
    if (pipe(pipefd) == -1)
        err(EXIT_FAILURE, "pipe");
    for (int i = 0; i < 2; i++)
    {
        if ((child_pids[i] = fork()) == -1)
        {
            perror("Can not fork\n");
            return 1;
        }
        if (child_pids[i] == 0)
        {
            if (close(pipefd[0]) == -1)
                err(EXIT_FAILURE, "close");
            if(flag) 
            {
            if (write(pipefd[1], messages[i], strlen(messages[i])) == -1)
                err(EXIT_FAILURE, "write");
            }
            exit(0);
        }
    }
    //printf("Parent: PID = %d, group = %d, Child: PIDs = %d, %d\n", getpid(), getpgrp(), child_pids[0], child_pids[1]);
    for (int i = 0; i < 2; i++)
    {
        int status;
        wait(&status);
        if (WIFEXITED(status))
        {
            //printf("\tChild %d exited with code = %d\n", child_pids[i], WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            printf("\tChild %d with (signal %d) killed\n", child_pids[i], WTERMSIG(status));
        }
        else if (WIFSTOPPED(status))
        {
            printf("\tChild %d with (signal %d) stopped\n", child_pids[i], WSTOPSIG(status));
        }
        else
        {
            printf("\tUnexpected status for Child (0x%x)\n", status);
        }
    }
    if (close(pipefd[1]) == -1)
        err(EXIT_FAILURE, "close");
    for (int i = 0; i < 2; i++)
    {
        size_t s = read(pipefd[0], &buf, strlen(messages[i]));
        printf("%ld, %s\n", s, buf);
        buf[0] = '\0';
    }
    return 0;
}
