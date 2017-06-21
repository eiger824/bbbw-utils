#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>

int fd; // file descriptor

void sig_handler(int signo)
{
	if (signo == SIGINT)
	{
		printf("\nSIGINT received, closing file\n");
		if (fd >= 0)
			close(fd);
		exit(0);
	}
}

int main(int argc, char* argv[])
{
	fd = open("/dev/tmp36", O_RDWR);
	char buffer[100];	
	int ret;

	if (signal(SIGINT, sig_handler) == SIG_ERR)
	{
		fprintf(stderr, "Could not catch signal\n");
	}

	while (1)
	{
		if ( fd >= 0)
		{
			if ( (ret = read (fd, buffer, 2)) < 0 )
			{
				perror("Error reading");
			}
			else
			{
				buffer[2] = 0;
				printf("Obtained sample: %s\n", buffer);
			}
		}
		else
		{
			perror("Error opening file");
		}
		sleep(1);
	}
	return 0;
}
