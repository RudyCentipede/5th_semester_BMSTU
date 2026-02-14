#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <pthread.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define LOCKFILE "/var/run/daemon.pid"
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
sigset_t mask;
int already_running(void)
{
	int fd;
	char buf[16];
	struct flock fl;
	fd = open(LOCKFILE, O_CREAT | O_EXCL |  O_RDWR, LOCKMODE);
	if (fd == -1)
	{
		syslog(LOG_ERR, "open %s: %s", LOCKFILE, strerror(errno));
		exit(1);
	}
	fl.l_type = F_WRLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;
	if (fcntl(fd, F_SETLK, &fl) == -1)
	{
		if (errno == EACCES || errno == EAGAIN)
		{
			close(fd);
			return 1;
		}
		syslog(LOG_ERR, "fcntl %s: %s", LOCKFILE, strerror(errno));
		exit(1);
	}
	ftruncate(fd, 0);
	sprintf(buf, "%d", getpid());
	write(fd, buf, strlen(buf) + 1);
	return 0;
}
void daemonize(const char *cmd)
{
	int fd0, fd1, fd2;
	pid_t pid;
	struct rlimit rl;
	struct sigaction sa;
	umask(0);
	if (getrlimit(RLIMIT_NOFILE, &rl) == -1)
	{
		perror("getrlimit");
		printf("errno: %d\n", errno);
	}
	if ((pid = fork()) == -1)
	{
		perror("fork");
		printf("errno: %d\n", errno);
	}
	else if (pid > 0)
		exit(0);
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGHUP, &sa, NULL) == -1)
	{
		perror("sigaction");
		printf("errno: %d\n", errno);
		exit(1);
	}
	if (setsid() == -1)
	{
		perror("setsid");
		printf("errno: %d\n", errno);
		exit(1);
	}
	if (chdir("/") == -1)
	{
		perror("chdir");
		printf("errno: %d\n", errno);
		exit(1);
	}
	if (rl.rlim_max == RLIM_INFINITY)
		rl.rlim_max = 1024;
	for (size_t i = 0; i < rl.rlim_max; i++)
		close(i);
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);
	openlog(cmd, LOG_CONS, LOG_DAEMON);
	if (fd0 != 0)
	{
		syslog(LOG_ERR, "stdin");
		exit(1);
	}
	if (fd1 != 1)
	{
		syslog(LOG_ERR, "stdout");
		exit(1);
	}
	if (fd2 != 2)
	{
		syslog(LOG_ERR, "stderr");
		exit(1);
	}
}
void reread(void)
{
    int fd;
    char data[256];

    if ((fd = open("/etc/my_daemon.conf", O_RDONLY)) == -1)
    {
		syslog(LOG_ERR, "open");
		exit(1);
	}
	
	if (read(fd, NULL, 0) == -1)
	{
		syslog(LOG_ERR, "read");
		exit(1);
	}

	if (close(fd) == -1)
	{
		syslog(LOG_ERR, "close");
		exit(1);
	}
}
void *thr_fn(void *arg)
{
	syslog(LOG_INFO, "tid=%d", gettid());
	int err, signo;
	for (;;)
	{
		err = sigwait(&mask, &signo);
		if (err != 0)
		{
			syslog(LOG_ERR, "sigwait");
			exit(1);
		}
		switch(signo)
		{
			case SIGHUP:
			{
				syslog(LOG_INFO, "catch SIGHUP");
				reread();
				break;
			}
			case SIGTERM:
			{
				syslog(LOG_INFO, "catch SIGTERM");
				pthread_exit(0);
			}
			default:
				syslog(LOG_INFO, "catch signal %d\n", signo);
		}
		
	}
	return 0;
}
int main(int argc, char *argv[])
{
	int err;
	pthread_t tid;
	char *cmd = "daemon";
	struct sigaction sa;
	daemonize(cmd);
	if (already_running() != 0)
	{
		syslog(LOG_ERR, "already_running\n");
		exit(1);
	}
	sa.sa_handler = SIG_DFL;
	if (sigemptyset(&sa.sa_mask) == -1)
	{
		syslog(LOG_ERR, "sigemptyset");
		exit(1);
	}
	sa.sa_flags = 0;
	if (sigaction(SIGHUP, &sa, NULL) == -1)
	{
		syslog(LOG_ERR, "%s: sigaction SIGHUP", cmd);
		exit(1);
	}
	if (sigfillset(&mask) == -1)
	{
		syslog(LOG_ERR, "sigfillset");
		exit(1);
	}
	if ((err = pthread_sigmask(SIG_BLOCK, &mask, NULL)) == -1)
	{
		syslog(LOG_ERR, "SIG_BLOCK %d", err);
		exit(1);
	}
	if ((err = pthread_create(&tid, NULL, thr_fn, NULL)) == -1)
	{
		syslog(LOG_ERR, "pthread_create %d", err);
		exit(1);
	}
	long int ttime;
	while (1)
	{
		ttime = time(NULL);
		syslog(LOG_INFO, "time: %s\n", ctime(&ttime));
		sleep(10);
	}
}
