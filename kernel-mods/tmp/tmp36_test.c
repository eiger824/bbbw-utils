#include <stdio.h>
#include <fcntl.h>

int main(int argc, char* argv[])
{
	int fd = open("/dev/tmp36", O_RDWR);
	char buffer[100];	
	int ret;

	if ( fd >= 0)
	{
		printf("File opened, requesting read\n");
		if ( (ret = read (fd, buffer, 2)) < 0 )
		{
			perror("Error reading");
		}
		else
		{
			buffer[2] = 0;
			printf("Obtained sample: %s\n", buffer);
		}
		close(fd);
	}
	else
	{
		perror("Error opening file");
	}
	return 0;
}
