#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>


int main(void)
{
    const char *const param[2] = { "/home/admin1/Downloads/C/var5/app.exe", "/home/admin1/Downloads/C/var6/app.exe" };
    
    pid_t childpid[2];
       
    for (size_t i = 0; i < 2; i++)
    {
        childpid[i] = fork();
	    if (childpid[i] == -1)
	    {
	        perror("Can't fork.\n");
	        exit(1);
	    }
	    else if (childpid[i] == 0)
	    {       
	        int rc = execlp(param[i], param[i], 0);
	       
	        if (rc == -1)
	        {
	           	perror("Can't exec.\n");
	           	exit(1);
	        }
	    }
	    else 
	    {
	         printf("%d\n", getpid());
        	    
        	int status;
	        pid_t child_pid = wait(&status);
	        
	        if (child_pid == -1)
	        {
	        	perror("Can't wait.\n");
	        	exit(1);
	        }
	        
	        //printf("Child #%ld has finished: PID = %d\n", i + 1, child_pid);
	    
	        if (WIFEXITED(status))
	          printf("\nChild #%ld exited with code %d\n", i + 1, WEXITSTATUS(status));
	        else if(WIFSIGNALED(status))
	          printf("\nChild #%ld terminated, recieved signal %d\n", i + 1, WTERMSIG(status));
	        else if (WIFSTOPPED(status))
	          printf("\nChild #%ld stopped, recieved signal %d\n", i + 1, WSTOPSIG(status));   
          }
    }
    
    return 0;
}
