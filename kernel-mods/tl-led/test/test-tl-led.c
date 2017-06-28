#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

void help()
{
	fprintf(stderr, "USAGE: ./test-tl-led LEDNR:STATE, with LEDNR={0|1|2} and STATE={0|1}\n");
}

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		fprintf(stderr, "Not enough input args\n");
		help();
		return -1;
	}

	char *arg1 = malloc(strlen(argv[1]));
	if (arg1)
	{
		if (strlen(argv[1]) != 3)
		{
			help();
			free(arg1);
			return -1;
		}
		else
		{
			if (argv[1][1] == ':')
			{
				char lednr = argv[1][0];
				char state = argv[1][2];

				if ('0' > lednr || lednr > '2')
				{
					help();
					free(arg1);
					return -1;
				}

				if ('0' > state || state > '1')
				{
					help();
					free(arg1);
					return -1;
				}

				char msg[3];
				msg[0] = lednr;
				msg[1] = state;
				msg[2] = 0;

				//Now open file
				printf("Now opening /dev/tl-led, sending STATE=%c to LED=%c ...\n", lednr, state);
				int fd, ret;
				fd = open("/dev/tl-led", O_RDWR);
				if (fd < 0)
				{
					perror("Failed to open device");
					free(arg1);
					return errno;
				}

				ret = write(fd, msg, strlen(msg));

				if (ret < 0)
				{
					perror("Error writing to device");
					free(arg1);
					close(fd);
					return errno;
				}

			}
			else
			{
				help();
				free(arg1);
				return -1;
			}
		}

	}
	return 0;
}
