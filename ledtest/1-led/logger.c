#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>

#include "strutils.h"
#include "logger.h"

bool silent = false;

int write_2_led(const char* value)
{
	int ret;
	fled = open(GLED01_DEV, O_RDWR);
    if (fled < 0) return -1;
	ret = write(fled, value, 1);
	close(fled);
	if (ret < 0) return errno; 
	return 0;
}

void remove_log_file()
{
	if (!unlink(LOG_PATH)) logger ("Success!");
	else logger (msg_err("Error", errno) );
}

void set_silent(bool s)
{
	silent = s;
}

void logger(const char* msg)
{
	char buffer[200];
	strcpy(buffer, "[");
	time_t current_time;
	char *c_time_string;
	current_time = time(NULL);
	c_time_string = ctime(&current_time);
	c_time_string[strlen(c_time_string)-1] = 0;
	strcat(buffer, c_time_string);
	strcat(buffer, "] ");
	strcat(buffer, msg);
	strcat(buffer, "\n");
	
	if (!silent)
	{
        printf("%s", buffer);	
	}
	else
	{
		fl = fopen(LOG_PATH, "a");
		if (fl != NULL)
		{
			fputs(buffer, fl);
			fclose(fl);
		}
	}
}

int read_from_file(const char* name, char* buffer)
{
	char io_state[1];
	buffer = malloc(2);
	fd = fopen(name, "r");
	if (fd == NULL) return -1;
	if (fgets (io_state, 1, fd) != NULL)
	{
		strcpy(buffer,io_state);
		buffer[1] = 0; //null terminated
		fclose(fd);
		return 0;
	}
	else
	{
		return -1;
	}
}
