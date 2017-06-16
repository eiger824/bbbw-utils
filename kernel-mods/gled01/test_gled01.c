#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

int fd; //File descriptor

void sig_handler(int signo)
{
	if (signo == SIGINT)
	{
		printf("\nreceived SIGINT, closing device\n");
		if (fd >= 0)
		{
			close(fd);
			exit(0);
		}
	}
}

int main(int argc, char* argv[])
{
	int ret;
	char c;
	bool state = true;
	
	if ( signal(SIGINT, sig_handler) == SIG_ERR)
	{
		perror("Warning, could not catch signal");
	}

	fd = open("/dev/gled01", O_RDWR);
	if (fd < 0)
	{
		perror("Failed to open device");
		return errno;
	}
	
	while (1)
	{
		printf("Press enter to toggle led");
		getchar();
		if (state) ret = write(fd, "0", 1);
		else ret = write(fd, "1", 1);;
		state = !state;
		if (ret < 0)
		{
			perror("Error writing to device");
			close(fd);
			return errno;
		}
	}
	return 0;
}
